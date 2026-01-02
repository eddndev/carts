import 'dart:io';

/// Service for UDP communication with robot fleet
class UdpService {
  RawDatagramSocket? _socket;
  final int robotPort;
  Function(String message, String senderIp)? onMessage;
  
  bool get isConnected => _socket != null;
  
  UdpService({
    this.robotPort = 4210,
    this.onMessage,
  });
  
  /// Initialize UDP socket
  Future<bool> connect() async {
    try {
      _socket = await RawDatagramSocket.bind(InternetAddress.anyIPv4, 0);
      _socket!.broadcastEnabled = true;
      
      _socket!.listen((RawSocketEvent e) {
        if (e == RawSocketEvent.read) {
          Datagram? d = _socket!.receive();
          if (d != null) {
            String msg = String.fromCharCodes(d.data);
            String senderIp = d.address.address;
            onMessage?.call(msg, senderIp);
          }
        }
      });
      
      return true;
    } catch (e) {
      print("UDP Service Error: $e");
      return false;
    }
  }
  
  /// Close socket
  void disconnect() {
    _socket?.close();
    _socket = null;
  }
  
  /// Send command to specific IP
  bool sendCommand(String command, String targetIp) {
    if (_socket == null) return false;
    
    try {
      _socket!.send(command.codeUnits, InternetAddress(targetIp), robotPort);
      return true;
    } catch (e) {
      print("Send error: $e");
      return false;
    }
  }
  
  /// Broadcast command to all devices
  void broadcast(String command) {
    if (_socket == null) return;
    
    try {
      _socket!.send(command.codeUnits, InternetAddress("255.255.255.255"), robotPort);
    } catch (e) {
      print("Broadcast error: $e");
    }
  }
  
  /// Scan for devices on local subnet
  Future<void> scanSubnets() async {
    if (_socket == null) return;
    
    // Global broadcast
    broadcast("CMD:PING");
    
    // Subnet-specific broadcasts
    try {
      List<NetworkInterface> interfaces = await NetworkInterface.list(
        includeLoopback: false, 
        type: InternetAddressType.IPv4
      );

      for (var interface in interfaces) {
        for (var addr in interface.addresses) {
          List<String> parts = addr.address.split('.');
          if (parts.length == 4) {
            parts[3] = '255';
            String subnetBroadcast = parts.join('.');
            _socket!.send("CMD:PING".codeUnits, InternetAddress(subnetBroadcast), robotPort);
          }
        }
      }
    } catch (e) {
      print("Subnet scan error: $e");
    }
  }
}
