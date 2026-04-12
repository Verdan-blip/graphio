import socket
import os
import sys
import struct
import time
import signal

from generated.contract.protobuf import window_events_pb2

SOCKET_PATH = "/tmp/window_events.sock"

class EventAnalyticNode:
    def __init__(self, socket_path):
        self.socket_path = socket_path
        self.client_sock = None
        self.running = True

    def handle_exit(self, signum, frame):
        print("\n[*] Завершение работы...")
        self.running = False
        if self.client_sock:
            self.client_sock.close()
        sys.exit(0)

    def process_scoring(self, event):
        try:
            try:
                t = window_events_pb2.WindowEventType.Name(event.type)
            except:
                t = f"TYPE_{event.type}"

            win_id = getattr(event, 'win_id', 0)
            app_id = getattr(event, 'app_id', "unknown")

            print(f"[{t}] ID: 0x{win_id:x} | App: {app_id}")

        except Exception as e:
            print(f"[!] Ошибка: {e}")

    def run(self):
        print(f"[*] Аналитика запущена. Подключение к: {self.socket_path}")

        while self.running:
            try:
                self.client_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
                
                try:
                    self.client_sock.connect(self.socket_path)
                except (FileNotFoundError, ConnectionRefusedError):
                    time.sleep(1) # Ждем, если композитор еще не запустился
                    continue

                print("[*] Успешное подключение к композитору!")
                
                while self.running:
                    header = self.client_sock.recv(4)
                    if not header:
                        print("[!] Соединение разорвано композитором")
                        break
                    
                    msg_len = struct.unpack("!I", header)[0]
                    
                    # 2. Читаем тело сообщения целиком
                    data = b""
                    while len(data) < msg_len:
                        chunk = self.client_sock.recv(msg_len - len(data))
                        if not chunk:
                            break
                        data += chunk
                    
                    if len(data) < msg_len:
                        break

                    # 3. Десериализация
                    event = window_events_pb2.WindowEvent()
                    event.ParseFromString(data)
                    self.process_scoring(event)

            except Exception as e:
                print(f"[!] Ошибка: {e}")
                time.sleep(1)
            finally:
                if self.client_sock:
                    self.client_sock.close()

if __name__ == "__main__":
    node = EventAnalyticNode(SOCKET_PATH)
    signal.signal(signal.SIGINT, node.handle_exit)
    node.run()