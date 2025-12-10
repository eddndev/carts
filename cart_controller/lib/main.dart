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
            
            if (msg.startsWith("ACK:")) {
                // Found a device!
                if (!_foundDevices.contains(senderIp)) {
                    setState(() {
                        _foundDevices.add(senderIp);
                    });
                }
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
                  onTap: () => _sendCommand("CMD:EXPLORE"),
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
                            Colors.cyanAccent.withOpacity(0.2),
                            Colors.blueAccent.withOpacity(0.1),
                          ],
                        ),
                        border: Border.all(color: Colors.cyanAccent.withOpacity(0.5), width: 2),
                        boxShadow: [
                            BoxShadow(
                                color: Colors.cyanAccent.withOpacity(0.2),
                                blurRadius: 30,
                                spreadRadius: 5
                            )
                        ],
                      ),
                      child: Column(
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: const [
                          Icon(Icons.rocket_launch, size: 64, color: Colors.white),
                          SizedBox(height: 16),
                          Text("EXPLORE", 
                            style: TextStyle(
                              color: Colors.white, 
                              fontSize: 32, 
                              fontWeight: FontWeight.bold,
                              letterSpacing: 2
                            )
                          ),
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
                      onTap: () => _sendCommand("CMD:STOP"),
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
