import 'package:flutter/material.dart';
import '../services/udp_service.dart';

/// Test Page for debugging robot communication and motors
class TestPage extends StatefulWidget {
  final UdpService udpService;
  final String targetIp;
  
  const TestPage({
    super.key, 
    required this.udpService,
    required this.targetIp,
  });

  @override
  State<TestPage> createState() => _TestPageState();
}

class _TestPageState extends State<TestPage> {
  final List<String> _logs = [];
  
  @override
  void initState() {
    super.initState();
    _addLog("Test Mode Started");
    _addLog("Target: ${widget.targetIp}");
    
    // Listen for Arduino responses
    widget.udpService.onMessage = (msg, senderIp) {
      if (senderIp == widget.targetIp) {
        // Filter out heartbeat noise so we can see real command ACKs
        if (!msg.contains("CartFollower")) {
          _addLog("📥 $msg");
        }
      }
    };
  }
  
  @override
  void dispose() {
    // Restore original handler (or set to null)
    widget.udpService.onMessage = null;
    super.dispose();
  }
  
  void _addLog(String msg) {
    setState(() {
      _logs.insert(0, "[${DateTime.now().toString().substring(11, 19)}] $msg");
      if (_logs.length > 50) _logs.removeLast();
    });
  }
  
  void _sendTest(String cmd) {
    bool success = widget.udpService.sendCommand(cmd, widget.targetIp);
    _addLog("${success ? '✓' : '✗'} $cmd");
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFF0F172A),
      appBar: AppBar(
        title: const Text("🔧 TEST MODE", style: TextStyle(letterSpacing: 2)),
        backgroundColor: Colors.orange.withOpacity(0.2),
        leading: IconButton(
          icon: const Icon(Icons.arrow_back),
          onPressed: () => Navigator.pop(context),
        ),
      ),
      body: SafeArea(
        child: Column(
          children: [
          // Motor Test Grid
          Padding(
            padding: const EdgeInsets.all(16),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                const Text("MOTOR TESTS", style: TextStyle(color: Colors.orange, fontWeight: FontWeight.bold)),
                const SizedBox(height: 12),
                Row(
                  mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                  children: [
                    _TestButton(
                      icon: Icons.arrow_upward,
                      label: "FWD",
                      color: Colors.green,
                      onTap: () => _sendTest("TEST:FWD"),
                    ),
                  ],
                ),
                const SizedBox(height: 8),
                Row(
                  mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                  children: [
                    _TestButton(
                      icon: Icons.rotate_left,
                      label: "LEFT",
                      color: Colors.blue,
                      onTap: () => _sendTest("TEST:LEFT"),
                    ),
                    _TestButton(
                      icon: Icons.stop,
                      label: "STOP",
                      color: Colors.red,
                      onTap: () => _sendTest("CMD:STOP"),
                    ),
                    _TestButton(
                      icon: Icons.rotate_right,
                      label: "RIGHT",
                      color: Colors.blue,
                      onTap: () => _sendTest("TEST:RIGHT"),
                    ),
                  ],
                ),
                const SizedBox(height: 8),
                Row(
                  mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                  children: [
                    _TestButton(
                      icon: Icons.arrow_downward,
                      label: "BWD",
                      color: Colors.amber,
                      onTap: () => _sendTest("TEST:BWD"),
                    ),
                  ],
                ),
              ],
            ),
          ),
          
          const Divider(color: Colors.white24),
          
          // Nav Command Tests
          Padding(
            padding: const EdgeInsets.all(16),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                const Text("NAV COMMANDS", style: TextStyle(color: Colors.purple, fontWeight: FontWeight.bold)),
                const SizedBox(height: 12),
                Row(
                  mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                  children: [
                    _TestButton(
                      icon: Icons.turn_left,
                      label: "GO_LEFT",
                      color: Colors.purple,
                      onTap: () => _sendTest("NAV:GO_LEFT"),
                    ),
                    _TestButton(
                      icon: Icons.arrow_upward,
                      label: "STRAIGHT",
                      color: Colors.purple,
                      onTap: () => _sendTest("NAV:GO_STRAIGHT"),
                    ),
                    _TestButton(
                      icon: Icons.turn_right,
                      label: "GO_RIGHT",
                      color: Colors.purple,
                      onTap: () => _sendTest("NAV:GO_RIGHT"),
                    ),
                  ],
                ),
              ],
            ),
          ),
          
          const Divider(color: Colors.white24),
          
          // Ping & Calibrate & Explore
          Padding(
            padding: const EdgeInsets.symmetric(horizontal: 16),
            child: Column(
              children: [
                Row(
                  children: [
                    Expanded(
                      child: ElevatedButton.icon(
                        icon: const Icon(Icons.rocket_launch),
                        label: const Text("EXPLORE"),
                        style: ElevatedButton.styleFrom(
                          backgroundColor: Colors.green,
                          padding: const EdgeInsets.symmetric(vertical: 12),
                        ),
                        onPressed: () => _sendTest("CMD:EXPLORE"),
                      ),
                    ),
                  ],
                ),
                const SizedBox(height: 8),
                Row(
                  children: [
                    Expanded(
                      child: ElevatedButton.icon(
                        icon: const Icon(Icons.network_ping),
                        label: const Text("PING"),
                        style: ElevatedButton.styleFrom(backgroundColor: Colors.teal),
                        onPressed: () => _sendTest("CMD:PING"),
                      ),
                    ),
                    const SizedBox(width: 12),
                    Expanded(
                      child: ElevatedButton.icon(
                        icon: const Icon(Icons.settings),
                        label: const Text("CALIBRATE"),
                        style: ElevatedButton.styleFrom(backgroundColor: Colors.deepOrange),
                        onPressed: () => _sendTest("CMD:CALIBRATE"),
                      ),
                    ),
                  ],
                ),
              ],
            ),
          ),
          
          const SizedBox(height: 16),
          
          // Log Console
          Expanded(
            child: Container(
              margin: EdgeInsets.fromLTRB(16, 16, 16, 16 + MediaQuery.of(context).viewPadding.bottom),
              padding: const EdgeInsets.all(12),
              decoration: BoxDecoration(
                color: Colors.black54,
                borderRadius: BorderRadius.circular(12),
                border: Border.all(color: Colors.white12),
              ),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Row(
                    children: [
                      const Icon(Icons.terminal, color: Colors.green, size: 18),
                      const SizedBox(width: 8),
                      const Text("CONSOLE", style: TextStyle(color: Colors.green, fontWeight: FontWeight.bold)),
                      const Spacer(),
                      IconButton(
                        icon: const Icon(Icons.delete_outline, color: Colors.white38, size: 20),
                        onPressed: () => setState(() => _logs.clear()),
                      ),
                    ],
                  ),
                  const Divider(color: Colors.white12),
                  Expanded(
                    child: ListView.builder(
                      itemCount: _logs.length,
                      itemBuilder: (ctx, i) => Text(
                        _logs[i],
                        style: const TextStyle(
                          color: Colors.white70, 
                          fontFamily: 'monospace',
                          fontSize: 12
                        ),
                      ),
                    ),
                  ),
                ],
              ),
            ),
          ),
        ],
        ),
      ),
    );
  }
}

class _TestButton extends StatelessWidget {
  final IconData icon;
  final String label;
  final Color color;
  final VoidCallback onTap;
  
  const _TestButton({
    required this.icon,
    required this.label,
    required this.color,
    required this.onTap,
  });

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: onTap,
      child: Container(
        width: 80,
        height: 80,
        decoration: BoxDecoration(
          color: color.withOpacity(0.2),
          borderRadius: BorderRadius.circular(16),
          border: Border.all(color: color.withOpacity(0.5)),
        ),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Icon(icon, color: color, size: 28),
            const SizedBox(height: 4),
            Text(label, style: TextStyle(color: color, fontSize: 10, fontWeight: FontWeight.bold)),
          ],
        ),
      ),
    );
  }
}
