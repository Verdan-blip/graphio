import time
import logging

logger = logging.getLogger("GraphModel")

class GraphNode:
    def __init__(self, app_id, win_id, last_update_time, score=0.0):
        self.app_id = app_id
        self.win_id = win_id
        self.score = score
        self.last_timestamp = last_update_time

    def __repr__(self):
        return f"Node({self.app_id}, score={self.score:.2f})"

class GraphModel:
    def __init__(self):
        self.nodes = []
        self.on_topology_change = None

    def update_node(self, new_node):
        if not new_node or new_node.win_id is None:
            return

        self.nodes = [n for n in self.nodes if n and n.win_id != new_node.win_id]
        
        self.nodes.append(new_node)
        
        self.nodes.sort(
            key=lambda x: (x.score if x and x.score is not None else 0.0), 
            reverse=True
        )
        
        self._notify()

    def remove_node(self, win_id):
        initial_count = len(self.nodes)
        self.nodes = [n for n in self.nodes if n.win_id != win_id]
        
        if len(self.nodes) != initial_count:
            self._notify()

    def _notify(self):
        if self.on_topology_change:
            self.on_topology_change(self.nodes)

    def find_node(self, win_id):
        for n in self.nodes:
            if n.win_id == win_id:
                return n
        return None

    def rebalance(self):
        self.nodes.sort(key=lambda x: x.score, reverse=True)
        self._notify()
