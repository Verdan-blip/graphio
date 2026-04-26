#ifndef SW_SCORER
#define SW_SCORER

#include <stdbool.h>

enum sw_scorer_event_type {
    SW_SCORER_EVENT_TYPE_FOCUS,
    SW_SCORER_EVENT_TYPE_OPEN,
    SW_SCORER_EVENT_TYPE_CLOSE
};

struct sw_scorer_params {
    double alpha;
    double decay_rate;
};

struct sw_scorer {
    struct sw_scorer_params params;
};

struct sw_scorer* sw_scorer_create(struct sw_scorer_params params);
void sw_scorer_destroy(struct sw_scorer *scorer);

double sw_scorer_reward(
    struct sw_scorer *scorer, 
    double prev_score,
    enum sw_scorer_event_type event_type,
    long now,
    long last_update_time
);

double sw_scorer_apply_decay(
    struct sw_scorer *scorer, 
    double prev_score,
    long now,
    long last_update_time
);

double sw_scorer_get_event_weight(enum sw_scorer_event_type event_type);

#endif