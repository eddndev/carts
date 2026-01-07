import 'package:flutter/material.dart';

/// A reusable D-Pad style controller for robot movement
class ControlPad extends StatelessWidget {
  final Function(String command) onCommand;
  final bool enabled;

  const ControlPad({
    super.key,
    required this.onCommand,
    this.enabled = true,
  });

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        // UP
        _PadButton(
          icon: Icons.arrow_upward_rounded,
          color: Colors.cyanAccent,
          onTap: () => onCommand("TEST:FWD"),
          enabled: enabled,
        ),
        const SizedBox(height: 12),
        Row(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            // LEFT
            _PadButton(
              icon: Icons.arrow_back_rounded,
              color: Colors.cyanAccent,
              onTap: () => onCommand("TEST:LEFT"),
              enabled: enabled,
            ),
            const SizedBox(width: 12),
            // STOP (Center)
            _PadButton(
              icon: Icons.stop_rounded,
              color: Colors.redAccent,
              onTap: () => onCommand("CMD:STOP"),
              size: 70,
              enabled: enabled,
            ),
            const SizedBox(width: 12),
            // RIGHT
            _PadButton(
              icon: Icons.arrow_forward_rounded,
              color: Colors.cyanAccent,
              onTap: () => onCommand("TEST:RIGHT"),
              enabled: enabled,
            ),
          ],
        ),
        const SizedBox(height: 12),
        // DOWN
        _PadButton(
          icon: Icons.arrow_downward_rounded,
          color: Colors.cyanAccent,
          onTap: () => onCommand("TEST:BWD"),
          enabled: enabled,
        ),
      ],
    );
  }
}

class _PadButton extends StatelessWidget {
  final IconData icon;
  final Color color;
  final VoidCallback onTap;
  final double size;
  final bool enabled;

  const _PadButton({
    required this.icon,
    required this.color,
    required this.onTap,
    this.size = 60,
    this.enabled = true,
  });

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: enabled ? onTap : null,
      onTapDown: enabled ? (_) => onTap() : null, // Support press-and-hold logic if needed later
      child: Container(
        width: size,
        height: size,
        decoration: BoxDecoration(
          color: enabled ? color.withOpacity(0.15) : Colors.white12,
          borderRadius: BorderRadius.circular(16),
          border: Border.all(
            color: enabled ? color.withOpacity(0.5) : Colors.transparent,
            width: 2,
          ),
          boxShadow: enabled
              ? [
                  BoxShadow(
                    color: color.withOpacity(0.2),
                    blurRadius: 10,
                    spreadRadius: 2,
                  )
                ]
              : [],
        ),
        child: Icon(
          icon,
          color: enabled ? color : Colors.white24,
          size: size * 0.5,
        ),
      ),
    );
  }
}
