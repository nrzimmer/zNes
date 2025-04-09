#include <stdint.h>

void set_io(uint8_t (*funcRead)(uint16_t), void (*funcWrite)(uint16_t, uint8_t));
void reset6502(void);
void exec6502(uint32_t tickcount);
void step6502(void);
