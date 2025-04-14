#include <stdlib.h>

#define APU_IMPLEMENTATION
#include "apu.h"

uint8_t length_table[32] = {10, 254, 20, 2,  40, 4,  80, 6,  160, 8,  60, 10, 14, 12, 26, 14,
                            12, 16,  24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30};

APU *apu;

APU *apu_new() {
    apu = (APU *)calloc(1, sizeof(APU));
    apu->read = &apu_cpu_read;
    apu->write = &apu_cpu_write;
    apu->noise_seq.sequence = 0xDBDB;
    apu->pulse1_osc.amplitude = 1;
    apu->pulse1_osc.harmonics = 8; // 20 but too slow
    apu->pulse2_osc.amplitude = 1;
    apu->pulse2_osc.harmonics = 8;
    return apu;
}

void apu_cpu_write(const uint16_t addr, const uint8_t data) {
    switch (addr) {
        case 0x4000:
            switch ((data & 0xC0) >> 6) {
                case 0x00:
                    apu->pulse1_seq.new_sequence = 0b01000000;
                    apu->pulse1_osc.dutycycle = 0.125;
                    break;
                case 0x01:
                    apu->pulse1_seq.new_sequence = 0b01100000;
                    apu->pulse1_osc.dutycycle = 0.250;
                    break;
                case 0x02:
                    apu->pulse1_seq.new_sequence = 0b01111000;
                    apu->pulse1_osc.dutycycle = 0.500;
                    break;
                case 0x03:
                    apu->pulse1_seq.new_sequence = 0b10011111;
                    apu->pulse1_osc.dutycycle = 0.750;
                    break;
                default:
                    break;
            }
            apu->pulse1_seq.sequence = apu->pulse1_seq.new_sequence;
            apu->pulse1_halt = (data & 0x20);
            apu->pulse1_env.volume = (data & 0x0F);
            apu->pulse1_env.disable = (data & 0x10);
            break;

        case 0x4001:
            apu->pulse1_sweep.enabled = data & 0x80;
            apu->pulse1_sweep.period = (data & 0x70) >> 4;
            apu->pulse1_sweep.down = data & 0x08;
            apu->pulse1_sweep.shift = data & 0x07;
            apu->pulse1_sweep.reload = true;
            break;

        case 0x4002:
            apu->pulse1_seq.reload = (apu->pulse1_seq.reload & 0xFF00) | data;
            break;

        case 0x4003:
            apu->pulse1_seq.reload = (uint16_t)((data & 0x07)) << 8 | (apu->pulse1_seq.reload & 0x00FF);
            apu->pulse1_seq.timer = apu->pulse1_seq.reload;
            apu->pulse1_seq.sequence = apu->pulse1_seq.new_sequence;
            apu->pulse1_lc.counter = length_table[(data & 0xF8) >> 3];
            apu->pulse1_env.start = true;
            break;

        case 0x4004:
            switch ((data & 0xC0) >> 6) {
                case 0x00:
                    apu->pulse2_seq.new_sequence = 0b01000000;
                    apu->pulse2_osc.dutycycle = 0.125;
                    break;
                case 0x01:
                    apu->pulse2_seq.new_sequence = 0b01100000;
                    apu->pulse2_osc.dutycycle = 0.250;
                    break;
                case 0x02:
                    apu->pulse2_seq.new_sequence = 0b01111000;
                    apu->pulse2_osc.dutycycle = 0.500;
                    break;
                case 0x03:
                    apu->pulse2_seq.new_sequence = 0b10011111;
                    apu->pulse2_osc.dutycycle = 0.750;
                    break;
                default:
                    break;
            }
            apu->pulse2_seq.sequence = apu->pulse2_seq.new_sequence;
            apu->pulse2_halt = (data & 0x20);
            apu->pulse2_env.volume = (data & 0x0F);
            apu->pulse2_env.disable = (data & 0x10);
            break;

        case 0x4005:
            apu->pulse2_sweep.enabled = data & 0x80;
            apu->pulse2_sweep.period = (data & 0x70) >> 4;
            apu->pulse2_sweep.down = data & 0x08;
            apu->pulse2_sweep.shift = data & 0x07;
            apu->pulse2_sweep.reload = true;
            break;

        case 0x4006:
            apu->pulse2_seq.reload = (apu->pulse2_seq.reload & 0xFF00) | data;
            break;

        case 0x4007:
            apu->pulse2_seq.reload = (uint16_t)((data & 0x07)) << 8 | (apu->pulse2_seq.reload & 0x00FF);
            apu->pulse2_seq.timer = apu->pulse2_seq.reload;
            apu->pulse2_seq.sequence = apu->pulse2_seq.new_sequence;
            apu->pulse2_lc.counter = length_table[(data & 0xF8) >> 3];
            apu->pulse2_env.start = true;

            break;

        case 0x4008:
            break;

        case 0x400C:
            apu->noise_env.volume = (data & 0x0F);
            apu->noise_env.disable = (data & 0x10);
            apu->noise_halt = (data & 0x20);
            break;

        case 0x400E:
            switch (data & 0x0F) {
                case 0x00:
                    apu->noise_seq.reload = 0;
                    break;
                case 0x01:
                    apu->noise_seq.reload = 4;
                    break;
                case 0x02:
                    apu->noise_seq.reload = 8;
                    break;
                case 0x03:
                    apu->noise_seq.reload = 16;
                    break;
                case 0x04:
                    apu->noise_seq.reload = 32;
                    break;
                case 0x05:
                    apu->noise_seq.reload = 64;
                    break;
                case 0x06:
                    apu->noise_seq.reload = 96;
                    break;
                case 0x07:
                    apu->noise_seq.reload = 128;
                    break;
                case 0x08:
                    apu->noise_seq.reload = 160;
                    break;
                case 0x09:
                    apu->noise_seq.reload = 202;
                    break;
                case 0x0A:
                    apu->noise_seq.reload = 254;
                    break;
                case 0x0B:
                    apu->noise_seq.reload = 380;
                    break;
                case 0x0C:
                    apu->noise_seq.reload = 508;
                    break;
                case 0x0D:
                    apu->noise_seq.reload = 1016;
                    break;
                case 0x0E:
                    apu->noise_seq.reload = 2034;
                    break;
                case 0x0F:
                    apu->noise_seq.reload = 4068;
                    break;
                default:
                    break;
            }
            break;

        case 0x4015: // APU STATUS
            apu->pulse1_enable = data & 0x01;
            apu->pulse2_enable = data & 0x02;
            apu->noise_enable = data & 0x04;
            break;

        case 0x400F:
            apu->pulse1_env.start = true;
            apu->pulse2_env.start = true;
            apu->noise_env.start = true;
            apu->noise_lc.counter = length_table[(data & 0xF8) >> 3];
            break;
        default:
            break;
    }
}

uint8_t apu_cpu_read(const uint16_t addr) {
    uint8_t data = 0x00;

    if (addr == 0x4015) {
        data |= (apu->pulse1_lc.counter > 0) ? 0x01 : 0x00;
        data |= (apu->pulse2_lc.counter > 0) ? 0x02 : 0x00;
        data |= (apu->noise_lc.counter > 0) ? 0x04 : 0x00;
    }
    return data;
}

void right_shift(uint32_t *s) { *s = ((*s & 0x0001) << 7) | ((*s & 0x00FE) >> 1); }

void noise_func(uint32_t *s) { *s = (((*s & 0x0001) ^ ((*s & 0x0002) >> 1)) << 14) | ((*s & 0x7FFF) >> 1); }

void apu_clock() {
    apu->dGlobalTime += (0.3333333333 / 1789773);

    if (apu->clock_counter % 6 == 0) {
        bool bHalfFrameClock = false;
        bool bQuarterFrameClock = false;
        apu->frame_clock_counter++;

        if (apu->frame_clock_counter == 3729) {
            bQuarterFrameClock = true;
        }

        if (apu->frame_clock_counter == 7457) {
            bQuarterFrameClock = true;
            bHalfFrameClock = true;
        }

        if (apu->frame_clock_counter == 11186) {
            bQuarterFrameClock = true;
        }

        if (apu->frame_clock_counter == 14916) {
            bQuarterFrameClock = true;
            bHalfFrameClock = true;
            apu->frame_clock_counter = 0;
        }

        if (bQuarterFrameClock) {
            env_clock(&apu->pulse1_env, apu->pulse1_halt);
            env_clock(&apu->pulse2_env, apu->pulse2_halt);
            env_clock(&apu->noise_env, apu->noise_halt);
        }

        if (bHalfFrameClock) {
            len_clock(&apu->pulse1_lc, apu->pulse1_enable, apu->pulse1_halt);
            len_clock(&apu->pulse2_lc, apu->pulse2_enable, apu->pulse2_halt);
            len_clock(&apu->noise_lc, apu->noise_enable, apu->noise_halt);
            sweep_clock(&apu->pulse1_sweep, &apu->pulse1_seq.reload, 0);
            sweep_clock(&apu->pulse2_sweep, &apu->pulse2_seq.reload, 1);
        }

        if (apu->bUseRawMode) {
            seq_clock(&apu->pulse1_seq, apu->pulse1_enable, &right_shift);
            apu->pulse1_sample = (double)apu->pulse1_seq.output;
        } else {
            apu->pulse1_osc.frequency = 1789773.0 / (16.0 * (double)(apu->pulse1_seq.reload + 1));
            apu->pulse1_osc.amplitude = (double)(apu->pulse1_env.output - 1) / 16.0;
            apu->pulse1_sample = osc_clock(&apu->pulse1_osc, apu->dGlobalTime);

            if (apu->pulse1_lc.counter > 0 && apu->pulse1_seq.timer >= 8 && !apu->pulse1_sweep.mute && apu->pulse1_env.output > 2)
                apu->pulse1_output += (apu->pulse1_sample - apu->pulse1_output) * 0.5;
            else
                apu->pulse1_output = 0;
        }

        if (apu->bUseRawMode) {
            seq_clock(&apu->pulse2_seq, apu->pulse2_enable, &right_shift);
            apu->pulse2_sample = (double)apu->pulse2_seq.output;
        } else {
            apu->pulse2_osc.frequency = 1789773.0 / (16.0 * (double)(apu->pulse2_seq.reload + 1));
            apu->pulse2_osc.amplitude = (double)(apu->pulse2_env.output - 1) / 16.0;
            apu->pulse2_sample = osc_clock(&apu->pulse2_osc, apu->dGlobalTime);

            if (apu->pulse2_lc.counter > 0 && apu->pulse2_seq.timer >= 8 && !apu->pulse2_sweep.mute && apu->pulse2_env.output > 2)
                apu->pulse2_output += (apu->pulse2_sample - apu->pulse2_output) * 0.5;
            else
                apu->pulse2_output = 0;
        }

        seq_clock(&apu->noise_seq, apu->noise_enable, &noise_func);

        if (apu->noise_lc.counter > 0 && apu->noise_seq.timer >= 8) {
            apu->noise_output = (double)apu->noise_seq.output * ((double)(apu->noise_env.output - 1) / 16.0);
        }

        if (!apu->pulse1_enable)
            apu->pulse1_output = 0;
        if (!apu->pulse2_enable)
            apu->pulse2_output = 0;
        if (!apu->noise_enable)
            apu->noise_output = 0;
    }

    sweep_track(&apu->pulse1_sweep, &apu->pulse1_seq.reload);
    sweep_track(&apu->pulse2_sweep, &apu->pulse2_seq.reload);

    apu->pulse1_visual = (apu->pulse1_enable && apu->pulse1_env.output > 1 && !apu->pulse1_sweep.mute) ? apu->pulse1_seq.reload : 2047;
    apu->pulse2_visual = (apu->pulse2_enable && apu->pulse2_env.output > 1 && !apu->pulse2_sweep.mute) ? apu->pulse2_seq.reload : 2047;
    apu->noise_visual = (apu->noise_enable && apu->noise_env.output > 1) ? apu->noise_seq.reload : 2047;

    apu->clock_counter++;
}

double get_sample() {
    if (apu->bUseRawMode) {
        return 32000.0f * ((apu->pulse1_sample - 0.5) * 0.5 + (apu->pulse2_sample - 0.5) * 0.5);
    } else {
        return 32000.0f * (((1.0 * apu->pulse1_output) - 0.8) * 0.1 + ((1.0 * apu->pulse2_output) - 0.8) * 0.1 +
                           ((2.0 * (apu->noise_output - 0.5))) * 0.1);
    }
}

uint8_t seq_clock(Seq *seq, const bool bEnable, void (*func)(uint32_t *s)) {
    if (bEnable) {
        seq->timer--;
        if (seq->timer == 0xFFFF) {
            seq->timer = seq->reload;
            func(&seq->sequence);
            seq->output = seq->sequence & 0x00000001;
        }
    }
    return seq->output;
}

uint8_t len_clock(Length *lc, bool bEnable, bool bHalt) {
    if (!bEnable)
        lc->counter = 0;
    else if (lc->counter > 0 && !bHalt)
        lc->counter--;
    return lc->counter;
}

void env_clock(Env *ev, bool bLoop) {
    if (!ev->start) {
        if (ev->divider_count == 0) {
            ev->divider_count = ev->volume;

            if (ev->decay_count == 0) {
                if (bLoop) {
                    ev->decay_count = 15;
                }

            } else
                ev->decay_count--;
        } else
            ev->divider_count--;
    } else {
        ev->start = false;
        ev->decay_count = 15;
        ev->divider_count = ev->volume;
    }

    if (ev->disable) {
        ev->output = ev->volume;
    } else {
        ev->output = ev->decay_count;
    }
}

double fast_sin(double t) {
    double j = t * 0.15915;
    j = j - (int)j;
    return 20.785 * j * (j - 0.5) * (j - 1.0);
}

double osc_clock(Osc *op, double t) {
    static const double pi = 3.14159265358979323846;
    static const double two_pi = 2.0 * pi;
    static const double two_over_pi = 2.0 / pi;

    const double frequency_two_pi = op->frequency * two_pi;
    const double p = op->dutycycle * two_pi;
    const int harmonics = (int)op->harmonics; // Convert to integer for loop

    double a = 0.0;
    double b = 0.0;

    for (int n = 1; n < harmonics; n++) {
        const double n_float = (double)n;
        const double c = n_float * frequency_two_pi * t;
        const double inv_n = 1.0 / n_float;

        const double sin_c = fast_sin(c);
        a += -sin_c * inv_n;
        b += -fast_sin(c - p * n_float) * inv_n;
    }
    return two_over_pi * op->amplitude * (a - b);
}

void sweep_track(Sweep *sw, const uint16_t *target) {
    if (sw->enabled) {
        sw->change = *target >> sw->shift;
        sw->mute = (*target < 8) || (*target > 0x7FF);
    }
}

bool sweep_clock(Sweep *sw, uint16_t *target, const bool channel) {
    bool changed = false;
    if (sw->timer == 0 && sw->enabled && sw->shift > 0 && !sw->mute) {
        if (*target >= 8 && sw->change < 0x07FF) {
            if (sw->down) {
                *target -= sw->change - channel;
            } else {
                *target += sw->change;
            }
            changed = true;
        }
    }

    if (sw->enabled) {
        if (sw->timer == 0 || sw->reload) {
            sw->timer = sw->period;
            sw->reload = false;
        } else
            sw->timer--;

        sw->mute = (*target < 8) || (*target > 0x7FF);
    }

    return changed;
}
