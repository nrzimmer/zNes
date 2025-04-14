#ifndef APU_H
#define APU_H

#include <stdint.h>

#include "forward.h"

typedef struct Sequencer {
    uint32_t sequence;
    uint32_t new_sequence;
    uint16_t timer;
    uint16_t reload;
    uint8_t output;
} Seq;

typedef struct LengthCounter {
    uint8_t counter;
} Length;

typedef struct Envelope {
    bool start;
    bool disable;
    uint16_t divider_count;
    uint16_t volume;
    uint16_t output;
    uint16_t decay_count;
} Env;

typedef struct Oscpulse {
    double frequency;
    double dutycycle;
    double amplitude; // = 1;
    double harmonics; // = 20;
} Osc;

typedef struct Sweeper {
    bool enabled;
    bool down;
    bool reload;
    uint8_t shift;
    uint8_t timer;
    uint8_t period;
    uint16_t change;
    bool mute;
} Sweep;

typedef struct APU {
    uint32_t frame_clock_counter;
    uint32_t clock_counter;
    bool bUseRawMode;
    double dGlobalTime;

    uint8_t (*read)(uint16_t);
    void (*write)(uint16_t, uint8_t);

    // Square Wave Pulse Channel 1
    bool pulse1_enable;
    bool pulse1_halt;
    double pulse1_sample;
    double pulse1_output;
    Seq pulse1_seq;
    Osc pulse1_osc;
    Env pulse1_env;
    Length pulse1_lc;
    Sweep pulse1_sweep;

    // Square Wave Pulse Channel 2
    bool pulse2_enable;
    bool pulse2_halt;
    double pulse2_sample;
    double pulse2_output;
    Seq pulse2_seq;
    Osc pulse2_osc;
    Env pulse2_env;
    Length pulse2_lc;
    Sweep pulse2_sweep;

    // Noise Channel
    bool noise_enable;
    bool noise_halt;
    Env noise_env;
    Length noise_lc;
    Seq noise_seq;
    double noise_sample;
    double noise_output;

    uint16_t pulse1_visual;
    uint16_t pulse2_visual;
    uint16_t noise_visual;
    uint16_t triangle_visual;
} APU;

void apu_cpu_write(uint16_t addr, uint8_t data);
uint8_t apu_cpu_read(uint16_t addr);
void apu_clock();
void apu_reset();

double get_sample();
APU *apu_new();

#ifdef APU_IMPLEMENTATION
uint8_t seq_clock(Seq *seq, bool bEnable, void (*func)(uint32_t *s));
uint8_t len_clock(Length *lc, bool bEnable, bool bHalt);
void env_clock(Env *ev, bool bLoop);
double osc_clock(Osc *op, double t);
void sweep_track(Sweep *sw, const uint16_t *target);
bool sweep_clock(Sweep *sw, uint16_t *target, bool channel);
double fast_sin(double t);
#endif

#endif // APU_H
