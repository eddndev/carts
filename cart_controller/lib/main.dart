import 'dart:io';
import 'package:flutter/material.dart';

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
  // Configuration
  final int robotPort = 4210;

  RawDatagramSocket? _socket;
  String _status = "Disconnected";
  final TextEditingController _ipController = TextEditingController(text: "");
  bool _isConnected = false;
  
  // Scanning
  List<String> _foundDevices = [];
  bool _isScanning = false;

  // DFS Exploration State
  int _nodeCount = 0;
  List<String> _directionStack = []; // Stack for backtracking
  String _lastDecision = "";
  bool _isExploring = false;

  late AnimationController _pulseController;
  late Animation<double> _pulseAnimation;

  @override
  void initState() {
    super.initState();
    _pulseController = AnimationController(
        vsync: this, duration: const Duration(seconds: 2))..repeat(reverse: true);
    _pulseAnimation = Tween<double>(begin: 1.0, end: 1.2).animate(
        CurvedAnimation(parent: _pulseController, curve: Curves.easeInOut));
        
    // Auto-init UDP
    _connect(); 
  }

  @override
  void dispose() {
    _socket?.close();
    _pulseController.dispose();
    _ipController.dispose();
    super.dispose();
  }

  Future<void> _connect() async {
    try {
      // Bind to any port to allow receiving responses
      _socket = await RawDatagramSocket.bind(InternetAddress.anyIPv4, 0);
      _socket!.broadcastEnabled = true; // IMPORTANT for discovery
      
      _socket!.listen((RawSocketEvent e) {
        if (e == RawSocketEvent.read) {
          Datagram? d = _socket!.receive();
          if (d != null) {
            String msg = String.fromCharCodes(d.data);
            String senderIp = d.address.address;
            
            // ACK for discovery
            if (msg.startsWith("ACK:")) {
                if (!_foundDevices.contains(senderIp)) {
                    setState(() {
                        _foundDevices.add(senderIp);
                    });
                }
            }
            
            // Telemetry JSON from robot (Hybrid Arch)
            if (msg.startsWith("{")) {
                _handleTelemetry(msg, senderIp);
            }
          }
        }
      });

      setState(() {
        _isConnected = true;
        _status = "Ready. Scan or Enter IP.";
      });
    } catch (e) {
      setState(() {
        _status = "Socket Error: $e";
        _isConnected = false;
      });
    }
  }
  
  // --- DFS EXPLORATION LOGIC --- 
  void _handleTelemetry(String json, String senderIp) {
    // Simple JSON parsing (format: {"s":state, "v":sensors, "d":dist})
    // State 4 = NAV_WAITING_HOST (from Navigator.h enum)
    try {
      // Extract state value
      RegExp stateRegex = RegExp(r'"s":(\d+)');
      var match = stateRegex.firstMatch(json);
      if (match != null) {
        int robotState = int.parse(match.group(1)!);
        
        // NAV_WAITING_HOST = 4 (from enum)
        if (robotState == 4 && _isExploring) {
          _nodeCount++;
          String decision = _dfsDecide();
          _sendNavCommand(decision, senderIp);
          
          setState(() {
            _lastDecision = decision;
            _status = "Node $_nodeCount: $decision";
          });
        }
      }
    } catch (e) {
      print("Telemetry parse error: $e");
    }
  }
  
  String _dfsDecide() {
    // Right-Hand Rule for maze exploration
    // Priority: RIGHT -> STRAIGHT -> LEFT -> BACK
    // For simplicity, we cycle through decisions
    // A real implementation would track visited edges
    
    List<String> options = ["GO_RIGHT", "GO_STRAIGHT", "GO_LEFT"];
    
    // Simple heuristic: alternate to explore
    String choice = options[_nodeCount % options.length];
    
    // Track for backtracking
    _directionStack.add(choice);
    
    return choice;
  }
  
  void _sendNavCommand(String decision, String targetIp) {
    if (_socket == null) return;
    
    String cmd = "NAV:$decision";
    try {
      _socket!.send(cmd.codeUnits, InternetAddress(targetIp), robotPort);
      print("Sent: $cmd to $targetIp");
    } catch (e) {
      print("Nav send error: $e");
    }
  }

  void _startScan() async {
    if (_socket == null) return;
    
    setState(() {
        _foundDevices.clear();
        _isScanning = true;
        _status = "Scanning all networks...";
    });
    
    // 1. Global Broadcast (255.255.255.255)
    try {
        _socket!.send("CMD:PING".codeUnits, InternetAddress("255.255.255.255"), robotPort);
    } catch (e) {
        print("Global broadcast error: $e");
    }

    // 2. Targeted Broadcasts (Subnet specific)
    try {
        // Get all active network interfaces
        List<NetworkInterface> interfaces = await NetworkInterface.list(
            includeLoopback: false, 
            type: InternetAddressType.IPv4
        );

        for (var interface in interfaces) {
            for (var addr in interface.addresses) {
                // Heuristic: Assume /24 subnet (Standard for Hotspots/Home WiFi)
                // Transform x.x.x.y -> x.x.x.255
                List<String> parts = addr.address.split('.');
                if (parts.length == 4) {
                    parts[3] = '255';
                    String subnetBroadcast = parts.join('.');
                    
                    print("Broadcasting to subnet: $subnetBroadcast");
                    try {
                        _socket!.send("CMD:PING".codeUnits, InternetAddress(subnetBroadcast), robotPort);
                    } catch (e) {
                        print("Subnet send error: $e");
                    }
                }
            }
        }
    } catch (e) {
        print("Interface scan error: $e");
    }

    // Stop scanning after 3 seconds
    Future.delayed(const Duration(seconds: 3), () {
        if (mounted) {
            setState(() {
                _isScanning = false;
                if (_foundDevices.isEmpty) _status = "No robots found. Check WiFi.";
                else _status = "Found ${_foundDevices.length} robots.";
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
              borderRadius: BorderRadius.vertical(top: Radius.circular(20))),
          builder: (ctx) {
              return StatefulBuilder(
                  builder: (context, setSheetState) {
                      return Padding(
                          padding: const EdgeInsets.all(24),
                          child: Column(
                              mainAxisSize: MainAxisSize.min,
                              crossAxisAlignment: CrossAxisAlignment.start,
                              children: [
                                  Row(
                                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                                      children: [
                                          const Text("Available Robots", style: TextStyle(color: Colors.white, fontSize: 20, fontWeight: FontWeight.bold)),
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
                                          child: Text("No devices found via UDP Broadcast.\nEnsure all devices are on the same WiFi.", style: TextStyle(color: Colors.white54)),
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
                                  )).toList()
                              ],
                          ),
                      );
                  }
              );
          }
      );
  }

  void _sendCommand(String cmd) {
    if (_socket == null) return;
    
    // If IP is empty, prompt scan
    if (_ipController.text.isEmpty) {
        _startScan();
        return;
    }
    
    String targetIp = _ipController.text;
    try {
      _socket!.send(
        cmd.codeUnits, 
        InternetAddress(targetIp), 
        robotPort
      );
      setState(() {
        _status = "Sent: $cmd";
      });
    } catch (e) {
      setState(() {
        _status = "Send Error: $e";
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text("ANTIGRAVITY CONTROL", style: TextStyle(letterSpacing: 2, fontWeight: FontWeight.bold)),
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
              // Connection Status Card
              Container(
                padding: const EdgeInsets.all(16),
                decoration: BoxDecoration(
                  color: Colors.white.withOpacity(0.05),
                  borderRadius: BorderRadius.circular(16),
                  border: Border.all(color: Colors.white.withOpacity(0.1)),
                ),
                child: Column(
                  children: [
                    Row(
                      children: [
                        const Icon(Icons.wifi_tethering, color: Colors.cyanAccent),
                        Expanded(
                          child: Padding(
                            padding: const EdgeInsets.symmetric(horizontal: 12),
                            child: TextField(
                              controller: _ipController,
                              style: const TextStyle(color: Colors.white, fontFamily: 'monospace'),
                              decoration: const InputDecoration(
                                labelText: "Target IP",
                                hintText: "Select or Enter IP",
                                border: InputBorder.none,
                                isDense: true,
                              ),
                            ),
                          ),
                        ),
                        
                        // SCAN BUTTON
                        IconButton(
                            icon: const Icon(Icons.radar, color: Colors.cyanAccent),
                            onPressed: _startScan,
                            tooltip: "Scan Devices",
                        ),
                      ],
                    ),
                    const Divider(color: Colors.white24),
                    Text(_status, style: const TextStyle(color: Colors.white54, fontSize: 12)),
                  ],
                ),
              ),
              
              const Spacer(),
              
              // MAIN CONTROLS
              Center(
                child: GestureDetector(
                  onTap: () {
                    setState(() {
                      _isExploring = true;
                      _nodeCount = 0;
                      _directionStack.clear();
                    });
                    _sendCommand("CMD:EXPLORE");
                  },
                  child: ScaleTransition(
                    scale: _isConnected ? _pulseAnimation : const AlwaysStoppedAnimation(1.0),
                    child: Container(
                      width: 280,
                      height: 280,
                      decoration: BoxDecoration(
                        shape: BoxShape.circle,
                        gradient: LinearGradient(
                          begin: Alignment.topLeft,
                          end: Alignment.bottomRight,
                          colors: [
                            _isExploring 
                              ? Colors.greenAccent.withOpacity(0.3)
                              : Colors.cyanAccent.withOpacity(0.2),
                            Colors.blueAccent.withOpacity(0.1),
                          ],
                        ),
                        border: Border.all(
                          color: _isExploring 
                            ? Colors.greenAccent.withOpacity(0.7) 
                            : Colors.cyanAccent.withOpacity(0.5), 
                          width: 2
                        ),
                        boxShadow: [
                            BoxShadow(
                                color: _isExploring 
                                  ? Colors.greenAccent.withOpacity(0.3)
                                  : Colors.cyanAccent.withOpacity(0.2),
                                blurRadius: 30,
                                spreadRadius: 5
                            )
                        ],
                      ),
                      child: Column(
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: [
                          Icon(
                            _isExploring ? Icons.explore : Icons.rocket_launch, 
                            size: 64, 
                            color: Colors.white
                          ),
                          const SizedBox(height: 16),
                          Text(
                            _isExploring ? "EXPLORING..." : "EXPLORE", 
                            style: const TextStyle(
                              color: Colors.white, 
                              fontSize: 28, 
                              fontWeight: FontWeight.bold,
                              letterSpacing: 2
                            )
                          ),
                          if (_isExploring) ...[
                            const SizedBox(height: 8),
                            Text(
                              "Nodes: $_nodeCount",
                              style: TextStyle(color: Colors.white70, fontSize: 14)
                            ),
                          ]
                        ],
                      ),
                    ),
                  ),
                ),
              ),
              
              const Spacer(),
              
              // STOP & RESET ROW
              Row(
                children: [
                   Expanded(
                    child: _ControlBtn(
                      label: "STOP",
                      icon: Icons.stop_circle_outlined,
                      color: Colors.redAccent,
                      onTap: () {
                        setState(() => _isExploring = false);
                        _sendCommand("CMD:STOP");
                      },
                    ),
                  ),
                  const SizedBox(width: 16),
                  Expanded(
                    child: _ControlBtn(
                      label: "RESET",
                      icon: Icons.refresh,
                      color: Colors.orangeAccent,
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

class _ControlBtn extends StatelessWidget {
  final String label;
  final IconData icon;
  final Color color;
  final VoidCallback onTap;

  const _ControlBtn({required this.label, required this.icon, required this.color, required this.onTap});

  @override
  Widget build(BuildContext context) {
    return ElevatedButton(
      onPressed: onTap,
      style: ElevatedButton.styleFrom(
        backgroundColor: color.withOpacity(0.2),
        foregroundColor: color,
        padding: const EdgeInsets.symmetric(vertical: 24),
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(16)),
        side: BorderSide(color: color.withOpacity(0.5)),
      ),
      child: Column(
        children: [
          Icon(icon, size: 32),
          const SizedBox(height: 8),
          Text(label, style: const TextStyle(fontWeight: FontWeight.bold)),
        ],
      ),
    );
  }
}
