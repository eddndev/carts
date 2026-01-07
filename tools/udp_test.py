import socket
import time

PORT = 4210
# Listen on all interfaces
IP = "0.0.0.0" 

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
try:
    sock.bind((IP, PORT))
    print(f"Listening for UDP packets on port {PORT}...")
    print("Press Ctrl+C to stop.")
    
    while True:
        data, addr = sock.recvfrom(1024)
        print(f"[{time.strftime('%H:%M:%S')}] Received from {addr}: {data.decode()}")

except OSError as e:
    print(f"Error binding to port: {e}")
    print("Is another instance running? Or Firewall blocking?")
except KeyboardInterrupt:
    print("\nStopped.")
except Exception as e:
    print(f"Error: {e}")
