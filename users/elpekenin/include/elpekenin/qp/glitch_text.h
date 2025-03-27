typedef enum {
    FILLING,
    COPYING,
    DONE,
} anim_phase_t;

typedef struct {
    size_t       used;
    bool         running;
    uint64_t     mask;
    anim_phase_t state;
    char         curr[65]; // u64 mask + '\0'
    char         dest[65]; // u64 mask + '\0'
    void (*callback)(void *);
} glitch_text_state_t;

// TODO: better API, a-la scrolling_text
uint32_t glitch_text_callback(uint32_t trigger_time, void *cb_arg);
