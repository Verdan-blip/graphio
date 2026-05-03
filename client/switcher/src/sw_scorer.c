#include <stdlib.h>
#include <math.h>

#include "include/sw_scorer.h"

static inline double to_seconds(long microseconds) {
    return (double) microseconds / 1000.0;
}

struct sw_scorer* sw_scorer_create(struct sw_scorer_params params) {
    struct sw_scorer *scorer = malloc(sizeof(struct sw_scorer));

    scorer->params = params;

    return scorer;
}

void sw_scorer_destroy(struct sw_scorer *scorer) {
    free(scorer);
}

double sw_scorer_apply_decay(
    struct sw_scorer *scorer, 
    double prev_score,
    long now,
    long last_update_time
) {
    long delta = now - last_update_time;

    if (delta <= 0) return prev_score;

    double delta_sec = to_seconds(delta);

    return prev_score * exp(-scorer->params.decay_rate * delta_sec);
}

double sw_scorer_reward(
    struct sw_scorer *scorer, 
    double prev_score,
    enum sw_scorer_event_type event_type,
    long now,
    long last_update_time
) {
    double decayed_score = sw_scorer_apply_decay(scorer, prev_score, now, last_update_time);
    double reward_weight = sw_scorer_get_event_weight(event_type);

    double new_score = (1.0 - scorer->params.alpha) * decayed_score + scorer->params.alpha * reward_weight;

    return new_score;
}

double sw_scorer_get_event_weight(enum sw_scorer_event_type event_type) {
    switch (event_type) {
        case SW_SCORER_EVENT_TYPE_FOCUS:
            return 10.0;
        case SW_SCORER_EVENT_TYPE_OPEN:
            return 5.0;
        case SW_SCORER_EVENT_TYPE_CLOSE:
            return 0.0;
        default:
            return 0.0;
    }
}