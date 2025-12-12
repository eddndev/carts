import 'package:flutter/material.dart';

// Services
import 'services/udp_service.dart';
import 'services/dfs_explorer.dart';

// Widgets
import 'widgets/control_button.dart';
import 'widgets/explore_button.dart';
import 'widgets/connection_card.dart';

// Pages
import 'pages/test_page.dart';

void main() {
  runApp(const RobotControllerApp());
}

class RobotControllerApp extends StatelessWidget {
  const RobotControllerApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Robot Controller',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        useMaterial3: true,
        colorScheme: ColorScheme.fromSeed(
          seedColor: Colors.cyanAccent,
          brightness: Brightness.dark,
          surface: const Color(0xFF101418), 
          primary: const Color(0xFF00E5FF),
        ),
        scaffoldBackgroundColor: const Color(0xFF0F172A),
      ),
      home: const ControllerPage(),
    );
  }
}

class ControllerPage extends StatefulWidget {
  const ControllerPage({super.key});

  @override
  State<ControllerPage> createState() => _ControllerPageState();
}

class _ControllerPageState extends State<ControllerPage> with SingleTickerProviderStateMixin {
  // Services
  late UdpService _udpService;
  final DfsExplorer _dfsExplorer = DfsExplorer();
  
  // UI State
  final TextEditingController _ipController = TextEditingController();
  String _status = "Disconnected";
  List<String> _foundDevices = [];
  bool _isScanning = false;

  late AnimationController _pulseController;
  late Animation<double> _pulseAnimation;

  @override
  void initState() {
    super.initState();
    
    _pulseController = AnimationController(
      vsync: this, 
      duration: const Duration(seconds: 2)
    )..repeat(reverse: true);
    
    _pulseAnimation = Tween<double>(begin: 1.0, end: 1.2).animate(
      CurvedAnimation(parent: _pulseController, curve: Curves.easeInOut)
    );
    
    // Initialize UDP Service with message handler
    _udpService = UdpService(
      onMessage: _handleMessage,
    );
    
    _connect();
  }

  @override
  void dispose() {
    _udpService.disconnect();
    _pulseController.dispose();
    _ipController.dispose();
    super.dispose();
  }

  Future<void> _connect() async {
    bool success = await _udpService.connect();
    setState(() {
      _status = success ? "Ready. Scan or Enter IP." : "Connection failed";
    });
  }
  
  void _handleMessage(String msg, String senderIp) {
    // ACK for discovery
    if (msg.startsWith("ACK:")) {
      if (!_foundDevices.contains(senderIp)) {
        setState(() => _foundDevices.add(senderIp));
      }
    }
    
    // Telemetry JSON from robot (Hybrid Arch)
    if (msg.startsWith("{")) {
      _handleTelemetry(msg, senderIp);
    }
  }
  
  void _handleTelemetry(String json, String senderIp) {
    try {
      RegExp stateRegex = RegExp(r'"s":(\d+)');
      var match = stateRegex.firstMatch(json);
      if (match != null) {
        int robotState = int.parse(match.group(1)!);
        
        // NAV_WAITING_HOST = 4
        if (robotState == 4 && _dfsExplorer.isExploring) {
          String decision = _dfsExplorer.decide();
          _udpService.sendCommand("NAV:$decision", senderIp);
          
          setState(() {
            _status = "Node ${_dfsExplorer.nodeCount}: $decision";
          });
        }
      }
    } catch (e) {
      print("Telemetry parse error: $e");
    }
  }

  void _startScan() async {
    setState(() {
      _foundDevices.clear();
      _isScanning = true;
      _status = "Scanning...";
    });
    
    await _udpService.scanSubnets();
    
    Future.delayed(const Duration(seconds: 3), () {
      if (mounted) {
        setState(() {
          _isScanning = false;
          _status = _foundDevices.isEmpty 
            ? "No robots found." 
            : "Found ${_foundDevices.length} robots.";
        });
        _showDeviceList();
      }
    });
  }
  
  void _showDeviceList() {
    showModalBottomSheet(
      context: context,
      backgroundColor: const Color(0xFF1E293B),
      shape: const RoundedRectangleBorder(
        borderRadius: BorderRadius.vertical(top: Radius.circular(20))
      ),
      builder: (ctx) => Padding(
        padding: const EdgeInsets.all(24),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                const Text("Available Robots", 
                  style: TextStyle(color: Colors.white, fontSize: 20, fontWeight: FontWeight.bold)
                ),
                if (_isScanning) 
                  const SizedBox(width: 20, height: 20, child: CircularProgressIndicator(strokeWidth: 2))
                else 
                  IconButton(
                    icon: const Icon(Icons.refresh, color: Colors.cyanAccent),
                    onPressed: () {
                      Navigator.pop(ctx);
                      _startScan();
                    }
                  )
              ],
            ),
            const SizedBox(height: 16),
            if (_foundDevices.isEmpty) 
              const Padding(
                padding: EdgeInsets.symmetric(vertical: 20),
                child: Text("No devices found.", style: TextStyle(color: Colors.white54)),
              ),
            ..._foundDevices.map((ip) => ListTile(
              leading: const Icon(Icons.smart_toy, color: Colors.cyanAccent),
              title: Text("Robot ($ip)", style: const TextStyle(color: Colors.white)),
              subtitle: const Text("Tap to Connect", style: TextStyle(color: Colors.white38)),
              trailing: const Icon(Icons.arrow_forward_ios, color: Colors.white24, size: 16),
              onTap: () {
                setState(() {
                  _ipController.text = ip;
                  _status = "Connected to $ip";
                });
                Navigator.pop(ctx);
              },
            )),
          ],
        ),
      ),
    );
  }

  void _sendCommand(String cmd) {
    if (_ipController.text.isEmpty) {
      _startScan();
      return;
    }
    
    bool success = _udpService.sendCommand(cmd, _ipController.text);
    setState(() {
      _status = success ? "Sent: $cmd" : "Send failed";
    });
  }
  
  void _startExploration() {
    _dfsExplorer.start();
    _sendCommand("CMD:EXPLORE");
    setState(() {});
  }
  
  void _stopExploration() {
    _dfsExplorer.stop();
    _sendCommand("CMD:STOP");
    setState(() {});
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text("ANTIGRAVITY CONTROL", 
          style: TextStyle(letterSpacing: 2, fontWeight: FontWeight.bold)
        ),
        centerTitle: true,
        backgroundColor: Colors.transparent,
        elevation: 0,
      ),
      body: SafeArea(
        child: Padding(
          padding: const EdgeInsets.all(24.0),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              // Connection Card
              ConnectionCard(
                ipController: _ipController,
                status: _status,
                onScan: _startScan,
              ),
              
              const Spacer(),
              
              // Explore Button
              Center(
                child: ExploreButton(
                  isExploring: _dfsExplorer.isExploring,
                  nodeCount: _dfsExplorer.nodeCount,
                  pulseAnimation: _pulseAnimation,
                  onTap: _startExploration,
                ),
              ),
              
              const Spacer(),
              
              // Control Buttons Row
              Row(
                children: [
                  Expanded(
                    child: ControlButton(
                      label: "STOP",
                      icon: Icons.stop_circle_outlined,
                      color: Colors.redAccent,
                      onTap: _stopExploration,
                    ),
                  ),
                  const SizedBox(width: 12),
                  Expanded(
                    child: ControlButton(
                      label: "TEST",
                      icon: Icons.build_outlined,
                      color: Colors.orange,
                      onTap: () {
                        if (_ipController.text.isEmpty) {
                          _startScan();
                          return;
                        }
                        Navigator.push(
                          context,
                          MaterialPageRoute(
                            builder: (_) => TestPage(
                              udpService: _udpService,
                              targetIp: _ipController.text,
                            ),
                          ),
                        );
                      },
                    ),
                  ),
                  const SizedBox(width: 12),
                  Expanded(
                    child: ControlButton(
                      label: "RESET",
                      icon: Icons.refresh,
                      color: Colors.amber,
                      onTap: () => _sendCommand("CMD:RESET"),
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 40),
            ],
          ),
        ),
      ),
    );
  }
}
