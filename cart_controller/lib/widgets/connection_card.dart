import 'package:flutter/material.dart';

/// Connection status card with IP input and scan button
class ConnectionCard extends StatelessWidget {
  final TextEditingController ipController;
  final String status;
  final VoidCallback onScan;

  const ConnectionCard({
    super.key,
    required this.ipController,
    required this.status,
    required this.onScan,
  });

  @override
  Widget build(BuildContext context) {
    return Container(
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
                    controller: ipController,
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
              IconButton(
                icon: const Icon(Icons.radar, color: Colors.cyanAccent),
                onPressed: onScan,
                tooltip: "Scan Devices",
              ),
            ],
          ),
          const Divider(color: Colors.white24),
          Text(status, style: const TextStyle(color: Colors.white54, fontSize: 12)),
        ],
      ),
    );
  }
}
