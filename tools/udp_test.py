import socket
import time

IP = "192.168.4.1"
PORT = 4210
MESSAGE = "CMD:PING"

print(f"Sending '{MESSAGE}' to {IP}:{PORT}...")

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(2)

try:
    sock.sendto(MESSAGE.encode(), (IP, PORT))
    print("message sent.")
    
    # Listen for reply
    print("Waiting for reply...")
    data, addr = sock.recvfrom(1024)
    print(f"Received reply from {addr}: {data.decode()}")

except socket.timeout:
    print("No reply received (Timeout).")
except Exception as e:
    print(f"Error: {e}")
