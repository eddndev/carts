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
  final String robotIp = "192.168.4.1"; // Default Robot IP in AP mode
  // If Client Mode (Robot connects to router), user can edit this
  final int robotPort = 4210;

  RawDatagramSocket? _socket;
  String _status = "Disconnected";
  final TextEditingController _ipController = TextEditingController(text: "192.168.4.1");
  bool _isConnected = false;
  
  // Animation for "Active" state
  late AnimationController _pulseController;
  late Animation<double> _pulseAnimation;

  @override
  void initState() {
    super.initState();
    _pulseController = AnimationController(
      vsync: this,
      duration: const Duration(seconds: 2),
    )..repeat(reverse: true);
    
    _pulseAnimation = Tween<double>(begin: 1.0, end: 1.2).animate(
      CurvedAnimation(parent: _pulseController, curve: Curves.easeInOut)
    );
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
      _socket = await RawDatagramSocket.bind(InternetAddress.anyIPv4, 0);
      setState(() {
        _isConnected = true;
        _status = "Ready to Command";
      });
      // Try to send a ping or just assume generic UDP "connectionless" state
      _showSnackbar("UDP Socket Initialized");
    } catch (e) {
      setState(() {
        _status = "Error: $e";
        _isConnected = false;
      });
    }
  }

  void _sendCommand(String cmd) {
    if (_socket == null) {
      _connect().then((_) => _sendInternally(cmd));
      return;
    }
    _sendInternally(cmd);
  }

  void _sendInternally(String cmd) {
    if (_socket == null) return;
    
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
      // Haptic feedback could go here
    } catch (e) {
      setState(() {
        _status = "Send Error: $e";
      });
    }
  }
  
  void _showSnackbar(String msg) {
    ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text(msg)));
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
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: [
                        const Icon(Icons.wifi_tethering, color: Colors.cyanAccent),
                        Expanded(
                          child: Padding(
                            padding: const EdgeInsets.symmetric(horizontal: 12),
                            child: TextField(
                              controller: _ipController,
                              style: const TextStyle(color: Colors.white, fontFamily: 'monospace'),
                              decoration: const InputDecoration(
                                labelText: "Robot IP Address",
                                border: InputBorder.none,
                                isDense: true,
                              ),
                            ),
                          ),
                        ),
                        _isConnected 
                          ? const Icon(Icons.check_circle, color: Colors.greenAccent)
                          : IconButton(
                              icon: const Icon(Icons.refresh), 
                              onPressed: _connect,
                              tooltip: "Initialize UDP",
                            )
                      ],
                    ),
                    const Divider(color: Colors.white24),
                    Text(_status, style: const TextStyle(color: Colors.white54, fontSize: 12)),
                  ],
                ),
              ),
              
              const Spacer(),
              
              // MAIN CONTROLS
              // EXPLORE BUTTON (Big & Pulsing)
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
                      onTap: () => _sendCommand("CMD:RESET"), // Assuming firmware supports this or ignores it
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
