import datetime
import math
from model import AppScore

class ScoringEngine:
    def __init__(self, alpha=0.1, decay_const=0.001):
        self.alpha = alpha
        self.decay_const = decay_const
        self.weights = {
            "WINDOW_EVENT_TYPE_FOCUS_IN": 5.0,
            "WINDOW_EVENT_TYPE_KEY_PRESS": 1.0,
            "WINDOW_EVENT_TYPE_CREATE": 2.0,
            "WINDOW_EVENT_TYPE_DESTROY": 1.0
        }

    def get_initial_score(self, app_id):
        app = AppScore.get_or_none(app_id=app_id)
        return float(app.score) if app and app.score is not None else 0.0

    def get_decayed_score(self, node, current_timestamp):
        delta_t = max(0, current_timestamp - node.last_timestamp)
        decay_factor = math.exp(-self.decay_const * delta_t)
        return node.score * decay_factor

    def calculate_new_score(self, current_score, event_type):
        activity_value = self.weights.get(event_type, 0.1)
        return (current_score * (1 - self.alpha)) + (activity_value * self.alpha)

    def apply_global_decay(self, current_nodes, active_win_id):
        decay_factor = 1 - (self.alpha * 0.5) 
        
        for node in current_nodes:
            if node.win_id != active_win_id:
                node.score *= decay_factor

    def sync_to_db(self, app_id, score):
        try:
            with db.atomic():
                AppScore.insert(
                    app_id=app_id, 
                    score=score, 
                    last_update=datetime.datetime.now()
                ).on_conflict(
                    conflict_target=[AppScore.app_id],
                    preserve=[AppScore.score, AppScore.last_update]
                ).execute()
        except Exception as e:
            print(f"[ERROR] DB Sync failed for {app_id}: {e}")