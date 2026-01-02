import 'package:flutter/material.dart';

/// Main exploration button with DFS state visualization
class ExploreButton extends StatelessWidget {
  final bool isExploring;
  final int nodeCount;
  final Animation<double> pulseAnimation;
  final VoidCallback onTap;

  const ExploreButton({
    super.key,
    required this.isExploring,
    required this.nodeCount,
    required this.pulseAnimation,
    required this.onTap,
  });

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: onTap,
      child: ScaleTransition(
        scale: pulseAnimation,
        child: Container(
          width: 280,
          height: 280,
          decoration: BoxDecoration(
            shape: BoxShape.circle,
            gradient: LinearGradient(
              begin: Alignment.topLeft,
              end: Alignment.bottomRight,
              colors: [
                isExploring 
                  ? Colors.greenAccent.withOpacity(0.3)
                  : Colors.cyanAccent.withOpacity(0.2),
                Colors.blueAccent.withOpacity(0.1),
              ],
            ),
            border: Border.all(
              color: isExploring 
                ? Colors.greenAccent.withOpacity(0.7) 
                : Colors.cyanAccent.withOpacity(0.5), 
              width: 2
            ),
            boxShadow: [
              BoxShadow(
                color: isExploring 
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
                isExploring ? Icons.explore : Icons.rocket_launch, 
                size: 64, 
                color: Colors.white
              ),
              const SizedBox(height: 16),
              Text(
                isExploring ? "EXPLORING..." : "EXPLORE", 
                style: const TextStyle(
                  color: Colors.white, 
                  fontSize: 28, 
                  fontWeight: FontWeight.bold,
                  letterSpacing: 2
                )
              ),
              if (isExploring) ...[
                const SizedBox(height: 8),
                Text(
                  "Nodes: $nodeCount",
                  style: const TextStyle(color: Colors.white70, fontSize: 14)
                ),
              ]
            ],
          ),
        ),
      ),
    );
  }
}
