import 'package:flutter/material.dart';

/// Cyberpunk-styled control button for robot commands
class ControlButton extends StatelessWidget {
  final String label;
  final IconData icon;
  final Color color;
  final VoidCallback onTap;

  const ControlButton({
    super.key,
    required this.label, 
    required this.icon, 
    required this.color, 
    required this.onTap,
  });

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
