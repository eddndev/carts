import 'package:flutter/material.dart';

class SensorBar extends StatelessWidget {
  final List<int> sensorValues; // Expecting 6 values (0-1000)

  const SensorBar({super.key, required this.sensorValues});

  @override
  Widget build(BuildContext context) {
    // Ensure we always have 6 slots displayed even if data is empty/partial
    final displayValues = List<int>.filled(6, 0);
    for (int i = 0; i < sensorValues.length && i < 6; i++) {
        displayValues[i] = sensorValues[i];
    }

    return Container(
      padding: const EdgeInsets.all(12),
      decoration: BoxDecoration(
        color: Colors.black45,
        borderRadius: BorderRadius.circular(12),
        border: Border.all(color: Colors.white10),
      ),
      child: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
           const Text("SENSOR ARRAY", style: TextStyle(color: Colors.white24, fontSize: 10, letterSpacing: 2)),
           const SizedBox(height: 8),
           Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: displayValues.map((val) => _SensorLed(value: val)).toList(),
          ),
        ],
      ),
    );
  }
}

class _SensorLed extends StatelessWidget {
  final int value;

  const _SensorLed({required this.value});

  @override
  Widget build(BuildContext context) {
    // Threshold > 600 means Line (Black) usually, or Surface? 
    // QTR Raw: High = Black (Low Reflectance), Low = White (High Reflectance).
    // Let's assume > 500 is "Active/Black".
    final bool isActive = value > 500;
    
    // Opacity based on value for analog feel?
    // value 0-1000. 
    double opacity = (value / 1000.0).clamp(0.0, 1.0);

    return Container(
      width: 20,
      height: 30, // Tall barcode style
      margin: const EdgeInsets.symmetric(horizontal: 4),
      decoration: BoxDecoration(
        color: isActive ? Colors.cyanAccent.withOpacity(opacity) : Colors.white10,
        borderRadius: BorderRadius.circular(4),
        border: Border.all(
          color: isActive ? Colors.cyanAccent : Colors.white24,
          width: 1
        ),
        boxShadow: isActive ? [
            BoxShadow(color: Colors.cyanAccent.withOpacity(0.5), blurRadius: 8)
        ] : []
      ),
    );
  }
}
