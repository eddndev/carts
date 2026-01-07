import 'package:flutter/material.dart';
import 'dart:async';
import 'dart:convert'; // For JSON

// Services
import 'services/udp_service.dart';

// Widgets
import 'widgets/control_pad.dart';
import 'widgets/sensor_bar.dart';

void main() {
  runApp(const RobotControllerApp());
}

class RobotControllerApp extends StatelessWidget {
  const RobotControllerApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Antigravity Fleet',
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
      home: const FleetDashboard(),
    );
  }
}

class FleetDashboard extends StatefulWidget {
  const FleetDashboard({super.key});

  @override
  State<FleetDashboard> createState() => _FleetDashboardState();
}

class _FleetDashboardState extends State<FleetDashboard> with SingleTickerProviderStateMixin {
  // Services
  late UdpService _udpService;
  
  // State
  List<String> _foundDevices = [];
  String _selectedIp = "ALL"; // Default to Broadcast
  bool _isScanning = false;
  String _lastLog = "Waiting for data...";
  List<int> _sensorData = [0,0,0,0,0,0];
  
  // Heartbeat
  Timer? _heartbeatTimer;

  @override
  void initState() {
    super.initState();
    
    // Initialize UDP
    _udpService = UdpService(
      onMessage: _handleMessage,
    );
    _connect();
    
    // Heartbeat every 2s
    _heartbeatTimer = Timer.periodic(const Duration(seconds: 2), (timer) {
      if (!_isScanning) {
        // Broadcast PING to keep finding devices and keep them alive
        _udpService.broadcast("CMD:PING");
      }
    });
  }

  @override
  void dispose() {
    _heartbeatTimer?.cancel();
    _udpService.disconnect();
    super.dispose();
  }

  Future<void> _connect() async {
    await _udpService.connect();
    _startScan();
  }
  
  void _handleMessage(String msg, String senderIp) {
    // 1. Discovery (Heartbeat or Manual Ping)
    if (msg.contains("PONG") || msg.contains("ACK")) {
      if (!_foundDevices.contains(senderIp)) {
        setState(() {
          _foundDevices.add(senderIp);
        });
      }
    }
    
    // 2. Telemetry / Auto Logic
    if (msg.startsWith("{")) {
       try {
        // Parse JSON
        Map<String, dynamic> data = jsonDecode(msg);
        
        // Auto Logic: State 4 = WAITING_HOST
        if (data.containsKey('s') && data['s'] == 4) {
           _udpService.sendCommand("NAV:GO_STRAIGHT", senderIp);
           setState(() => _lastLog = "Auto: STRAIGHT -> $senderIp");
        }
        
        // Sensor Data
        if (data.containsKey('v')) {
           List<dynamic> rawList = data['v'];
           setState(() {
              _sensorData = rawList.map((e) => (e as num).toInt()).toList();
           });
        }
        
       } catch (e) {
         // Ignore parse errors
       }
    }
    
    // 3. Mini-Console Logging
    if (!msg.contains("CartFollower") && !msg.startsWith("{")) {
      setState(() => _lastLog = "[$senderIp] $msg");
    }
  }

  void _startScan() async {
    setState(() => _isScanning = true);
    await _udpService.scanSubnets();
    Future.delayed(const Duration(seconds: 2), () {
      if (mounted) setState(() => _isScanning = false);
    });
  }

  void _sendCommand(String cmd) {
    if (_selectedIp == "ALL") {
      _udpService.broadcast(cmd);
      setState(() => _lastLog = "Broadcast: $cmd");
    } else {
      _udpService.sendCommand(cmd, _selectedIp);
      setState(() => _lastLog = "Sent: $cmd -> $_selectedIp");
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text("CARTS CONTROL CENTER", style: TextStyle(letterSpacing: 2, fontWeight: FontWeight.bold)),
        centerTitle: true,
        backgroundColor: Colors.transparent,
        elevation: 0,
        actions: [
          IconButton(
            icon: Icon(_isScanning ? Icons.wifi_find : Icons.refresh),
            onPressed: _startScan,
            tooltip: "Rescan Network",
          )
        ],
      ),
      body: SafeArea(
        child: Column(
          children: [
            // 1. FLEET SELECTOR (Carousel)
            Container(
              height: 100,
              margin: const EdgeInsets.symmetric(vertical: 16),
              child: ListView.builder(
                  scrollDirection: Axis.horizontal,
                  padding: const EdgeInsets.symmetric(horizontal: 16),
                  itemCount: _foundDevices.length + 1,
                  itemBuilder: (context, index) {
                    if (index == 0) {
                      final isSelected = _selectedIp == "ALL";
                      return _DeviceCard(
                        label: "ALL",
                        icon: Icons.groups, 
                        color: Colors.purpleAccent,
                        isSelected: isSelected,
                        onTap: () => setState(() => _selectedIp = "ALL"),
                      );
                    } else {
                      final ip = _foundDevices[index - 1]; 
                      final isSelected = ip == _selectedIp;
                      return _DeviceCard(
                        label: ip.split('.').last,
                        icon: Icons.smart_toy,
                        color: Colors.cyanAccent,
                        isSelected: isSelected,
                        onTap: () => setState(() => _selectedIp = ip),
                      );
                    }
                  },
              ),
            ),
            
            // 1.5 SENSOR BAR (Visualizer) - Only if specific robot selected
            if (_selectedIp != "ALL") ...[
              Padding(
                padding: const EdgeInsets.symmetric(horizontal: 24.0, vertical: 8.0),
                child: SensorBar(sensorValues: _sensorData),
              ),
            ],
            
            // 2. MINI CONSOLE
            Container(
              margin: const EdgeInsets.symmetric(horizontal: 24),
              padding: const EdgeInsets.symmetric(vertical: 8, horizontal: 16),
              decoration: BoxDecoration(
                color: Colors.black45,
                borderRadius: BorderRadius.circular(8),
                border: Border.all(color: Colors.white10)
              ),
              child: Row(
                children: [
                  const Icon(Icons.terminal, size: 16, color: Colors.greenAccent),
                  const SizedBox(width: 12),
                  Expanded(
                    child: Text(_lastLog, 
                      style: const TextStyle(fontFamily: 'monospace', color: Colors.greenAccent, fontSize: 12),
                      overflow: TextOverflow.ellipsis,
                    ),
                  ),
                ],
              ),
            ),

            const Spacer(),

            // 3. CONTROL PAD
            ControlPad(
              onCommand: _sendCommand,
              enabled: true, 
            ),
            
            const Spacer(),
            
            // 4. ACTION BAR (Ping, Calib, Auto)
            Padding(
              padding: const EdgeInsets.fromLTRB(24, 0, 24, 24),
              child: Row(
                mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                children: [
                  _ActionButton(
                    icon: Icons.network_ping, 
                    label: "PING", 
                    color: Colors.teal,
                    onTap: () => _sendCommand("CMD:PING"),
                  ),
                  _ActionButton(
                    icon: Icons.settings_backup_restore, 
                    label: "CALIB", 
                    color: Colors.orange,
                    onTap: () => _sendCommand("CMD:CALIBRATE"),
                  ),
                  _ActionButton(
                    icon: Icons.auto_mode, 
                    label: "AUTO", 
                    color: Colors.purpleAccent,
                    onTap: () => _sendCommand("CMD:AUTO"), 
                  ),
                ],
              ),
            )
          ],
        ),
      ),
    );
  }
}

class _DeviceCard extends StatelessWidget {
  final String label;
  final IconData icon;
  final Color color;
  final bool isSelected;
  final VoidCallback onTap;

  const _DeviceCard({
    required this.label, 
    required this.icon, 
    required this.color, 
    required this.isSelected, 
    required this.onTap
  });

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: onTap,
      child: AnimatedContainer(
        duration: const Duration(milliseconds: 200),
        margin: const EdgeInsets.only(right: 12),
        width: 80,
        decoration: BoxDecoration(
          color: isSelected ? color.withOpacity(0.2) : Colors.white10,
          borderRadius: BorderRadius.circular(16),
          border: Border.all(
            color: isSelected ? color : Colors.transparent,
            width: 2
          )
        ),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Icon(icon, 
              color: isSelected ? color : Colors.white24, 
              size: 32
            ),
            const SizedBox(height: 8),
            Text(label, 
              style: TextStyle(
                color: isSelected ? Colors.white : Colors.white54,
                fontWeight: FontWeight.bold
              )
            ),
          ],
        ),
      ),
    );
  }
}

class _ActionButton extends StatelessWidget {
  final IconData icon;
  final String label;
  final Color color;
  final VoidCallback onTap;

  const _ActionButton({required this.icon, required this.label, required this.color, required this.onTap});

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        IconButton.filledTonal(
          onPressed: onTap,
          icon: Icon(icon),
          style: IconButton.styleFrom(
            backgroundColor: color.withOpacity(0.2),
            foregroundColor: color,
            fixedSize: const Size(56, 56)
          ),
        ),
        const SizedBox(height: 4),
        Text(label, style: TextStyle(color: color, fontSize: 10, fontWeight: FontWeight.bold))
      ],
    );
  }
}
