/// DFS Exploration Service for Robot Navigation
class DfsExplorer {
  int nodeCount = 0;
  final List<String> directionStack = [];
  bool isExploring = false;
  
  /// Start exploration session
  void start() {
    isExploring = true;
    nodeCount = 0;
    directionStack.clear();
  }
  
  /// Stop exploration session
  void stop() {
    isExploring = false;
  }
  
  /// Make a decision at a node using Right-Hand Rule
  /// Priority: RIGHT -> STRAIGHT -> LEFT -> BACK
  String decide() {
    if (!isExploring) return "WAIT";
    
    nodeCount++;
    
    // Right-Hand Rule exploration
    List<String> options = ["GO_RIGHT", "GO_STRAIGHT", "GO_LEFT"];
    String choice = options[nodeCount % options.length];
    
    // Track for potential backtracking
    directionStack.add(choice);
    
    return choice;
  }
  
  /// Get the opposite direction for backtracking
  String getBacktrackDirection(String original) {
    switch (original) {
      case "GO_LEFT": return "GO_RIGHT";
      case "GO_RIGHT": return "GO_LEFT";
      case "GO_STRAIGHT": return "GO_STRAIGHT";
      default: return "GO_STRAIGHT";
    }
  }
  
  /// Pop last decision and return backtrack direction
  String? backtrack() {
    if (directionStack.isEmpty) return null;
    String lastMove = directionStack.removeLast();
    return getBacktrackDirection(lastMove);
  }
}
