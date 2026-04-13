import socket
import struct
import time
import signal
import sys
import logging

from generated.contract.protobuf import window_events_pb2

from model import init_db
from scoring import ScoringEngine
from graph_model import GraphModel, GraphNode

SOCKET_PATH = "/tmp/window_events.sock"

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(message)s',
    datefmt='%H:%M:%S',
    handlers=[
        logging.StreamHandler(sys.stdout)
    ]
)
logger = logging.getLogger("Analytics")

class EventAnalyticNode:
    def __init__(self, socket_path):
        self.socket_path = socket_path
        self.client_sock = None
        self.running = True
        
        self.engine = ScoringEngine(alpha=0.1)
        self.graph = GraphModel()
        
        self.graph.on_topology_change = self.handle_topology_update
        
        init_db()
        self._load_initial_state()

    def _load_initial_state(self):
        logger.info("Loading historical scores from DB...")

        pass

    def decode_and_process(self, data):
        try:
            event = window_events_pb2.WindowEvent()
            event.ParseFromString(data)
            
            t = self._get_event_name(event.type)
            app_id = getattr(event, 'app_id', "unknown")
            win_id = getattr(event, 'win_id', 0)
            t_now = event.timestamp

            for node in self.graph.nodes:
                node.score = self.engine.get_decayed_score(node, t_now)
                node.last_timestamp = t_now

            if t == "WINDOW_EVENT_TYPE_CREATE":
                score = self.engine.get_initial_score(app_id)
                node = GraphNode(app_id, win_id, t_now, score)
                self.graph.update_node(node)
                logger.info(f"Created: {app_id} (0x{win_id:x}) | Initial Score: {score:.2f}")
                return

            if t == "WINDOW_EVENT_TYPE_DESTROY":
                target_node = self.graph.find_node(win_id)
                if target_node:
                    self.engine.sync_to_db(target_node.app_id, target_node.score)
                self.graph.remove_node(win_id)
                logger.info(f"Destroyed: 0x{win_id:x}, stats saved to DB")
                return

            target_node = self.graph.find_node(win_id)
            if target_node:
                target_node.score = self.engine.calculate_new_score(target_node.score, t)
                self.graph.rebalance()
            else:
                score = self.engine.get_initial_score(app_id)
                new_node = GraphNode(app_id, win_id, t_now, score)
                self.graph.update_node(new_node)

        except Exception as e:
            logger.error(f"Processing error: {e}")

    def handle_topology_update(self, nodes):
        logger.info(f"Updated ranking. Active windows: {len(nodes)}")
        for i, node in enumerate(nodes):
            prefix = f"TOP {i+1}" if i < 4 else "SLOT"
            logger.info(f"  {prefix}: {node.app_id} (ID: 0x{node.win_id:x}) | Score: {node.score:.2f}")

    def _get_event_name(self, event_type):
        try:
            return window_events_pb2.WindowEventType.Name(event_type)
        except (ValueError, KeyError):
            return f"UNKNOWN_{event_type}"

    def run(self):
        logger.info(f"Starting Node. Target socket: {self.socket_path}")

        while self.running:
            try:
                self.client_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
                self.client_sock.connect(self.socket_path)
                logger.info("Connected to composer")
                
                while self.running:
                    header = self.client_sock.recv(4)
                    if not header:
                        logger.warning("Connection closed by composer")
                        break
                    
                    msg_len = struct.unpack("!I", header)[0]
                    data = self._read_full(msg_len)
                    if data:
                        self.decode_and_process(data)

            except (ConnectionRefusedError, FileNotFoundError):
                # Не спамим ошибкой, просто ждем
                time.sleep(1)
            except Exception as e:
                logger.error(f"Runtime socket error: {e}")
                time.sleep(1)
            finally:
                if self.client_sock:
                    self.client_sock.close()

    def _read_full(self, n):
        data = b""
        while len(data) < n:
            chunk = self.client_sock.recv(n - len(data))
            if not chunk: return None
            data += chunk
        return data

    def stop(self, signum, frame):
        logger.info("Received stop signal")
        self.running = False
        sys.exit(0)

if __name__ == "__main__":
    node = EventAnalyticNode(SOCKET_PATH)
    signal.signal(signal.SIGINT, node.stop)
    node.run()