#include <stdio.h>
#include <stdlib.h>

#define IMPLEMENT_CPU

#include "bus.h"
#include "cpu.h"
#include "forward.h"
#include <string.h>

Cpu *cpu_new(Bus *bus) {
    cpu = calloc(1, sizeof(Cpu));
    cpu->bus = bus;
    cpu->read = &cpu_read;
    cpu->write = &cpu_write;
    return cpu;
}

void cpu_free(Cpu *cpu_) {
    free(cpu_);
    cpu = nullptr;
}

inline uint8_t get_cpu_flag(const enum FLAGS6502 flag) { return cpu->status & flag; }

inline uint8_t cpu_read(const uint16_t addr) { return cpu->bus->read(addr); }

inline void cpu_write(const uint16_t addr, const uint8_t data) { cpu->bus->write(addr, data); }

uint8_t IMP(void) {
    fetched = cpu->a;
    return 0;
}

uint8_t IMM(void) {
    addr = cpu->pc++;
    return 0;
}

uint8_t ZP0(void) {
    addr = cpu_read(cpu->pc);
    cpu->pc++;
    addr &= 0x00FF;
    return 0;
}

uint8_t ZPX(void) {
    addr = (uint16_t)cpu_read(cpu->pc) + cpu->x;
    cpu->pc++;
    addr &= 0x00FF;
    return 0;
}

uint8_t ZPY(void) {
    addr = (uint16_t)cpu_read(cpu->pc) + cpu->y;
    cpu->pc++;
    addr &= 0x00FF;
    return 0;
}

// Address Mode: Relative
uint8_t REL(void) {
    branch_addr = cpu_read(cpu->pc);
    cpu->pc++;
    if (branch_addr & 0x80)
        branch_addr |= 0xFF00;
    return 0;
}

uint8_t ABS(void) {
    const uint16_t lo = cpu_read(cpu->pc);
    cpu->pc++;
    const uint16_t hi = cpu_read(cpu->pc);
    cpu->pc++;
    addr = hi << 8 | lo;
    return 0;
}

uint8_t ABX(void) {
    const uint16_t lo = cpu_read(cpu->pc);
    cpu->pc++;
    const uint16_t hi = cpu_read(cpu->pc);
    cpu->pc++;

    addr = hi << 8 | lo;
    addr += cpu->x;

    if ((addr & 0xFF00) != hi << 8)
        return 1;
    return 0;
}

uint8_t ABY(void) {
    const uint16_t lo = cpu_read(cpu->pc);
    cpu->pc++;
    const uint16_t hi = cpu_read(cpu->pc);
    cpu->pc++;

    addr = hi << 8 | lo;
    addr += cpu->y;

    if ((addr & 0xFF00) != hi << 8)
        return 1;
    return 0;
}

uint8_t IND(void) {
    const uint16_t lo = cpu_read(cpu->pc);
    cpu->pc++;
    const uint16_t hi = cpu_read(cpu->pc);
    cpu->pc++;

    const uint16_t ptr = hi << 8 | lo;
    if (lo == 0x00FF) {
        addr = cpu_read(ptr & 0xFF00) << 8 | cpu_read(ptr);
    } else {
        addr = cpu_read(ptr + 1) << 8 | cpu_read(ptr);
    }

    return 0;
}

uint8_t IZX(void) {
    const uint16_t ptr = cpu_read(cpu->pc);
    cpu->pc++;
    const uint16_t lo = cpu_read((ptr + (uint16_t)cpu->x) & 0x00FF);
    const uint16_t hi = cpu_read((ptr + (uint16_t)cpu->x + 1) & 0x00FF);
    addr = hi << 8 | lo;

    return 0;
}

uint8_t IZY(void) {
    const uint16_t ptr = cpu_read(cpu->pc);
    cpu->pc++;

    const uint16_t lo = cpu_read(ptr & 0x00FF);
    const uint16_t hi = cpu_read((ptr + 1) & 0x00FF);

    addr = hi << 8 | lo;
    addr += cpu->y;

    if ((addr & 0xFF00) != hi << 8)
        return 1;
    return 0;
}

uint8_t ADC(void) {
    cpu_fetch();
    result = fetched + (uint16_t)cpu->a + get_carry_word();
    update_carry_flag(result);
    update_zero_flag(result);
    const uint16_t part1 = ~((uint16_t)cpu->a ^ (uint16_t)fetched);
    const uint16_t part2 = (uint16_t)cpu->a ^ result;
    set_overflow_value(part1 & part2 & 0x0080);
    update_negative_flag(result);
    set_acc(result);
    return 1;
}

uint8_t AND(void) {
    cpu_fetch();
    cpu->a = cpu->a & fetched;
    update_zero_flag(cpu->a);
    update_negative_flag(cpu->a);
    return 1;
}

uint8_t ASL(void) {
    cpu_fetch();
    result = (uint16_t)fetched << 1;
    update_carry_flag(result);
    update_zero_flag(result);
    update_negative_flag(result);
    set_value(result);
    return 0;
}

uint8_t BCC(void) {
    if (!is_carry_set())
        branch();
    return 0;
}

uint8_t BCS(void) {
    if (is_carry_set())
        branch();
    return 0;
}

uint8_t BEQ(void) {
    if (is_zero_set())
        branch();
    return 0;
}

uint8_t BIT(void) {
    cpu_fetch();
    result = fetched & (uint16_t)cpu->a;
    update_zero_flag(result);
    set_negative_value(fetched & (1 << 7));
    set_overflow_value(fetched & (1 << 6));
    return 0;
}

uint8_t BMI(void) {
    if (is_negative_set())
        branch();
    return 0;
}

uint8_t BNE(void) {
    if (!is_zero_set())
        branch();
    return 0;
}

uint8_t BPL(void) {
    if (!is_negative_set())
        branch();
    return 0;
}

uint8_t BRK(void) {
    cpu->pc++;
    set_interrupt();
    push_word(cpu->pc);
    push_byte(cpu->status | B);
    cpu->pc = (uint16_t)cpu_read(0xFFFE) | ((uint16_t)cpu_read(0xFFFF) << 8);
    return 0;
}

uint8_t BVC(void) {
    if (!is_overflow_set())
        branch();
    return 0;
}

uint8_t BVS(void) {
    if (is_overflow_set())
        branch();
    return 0;
}

uint8_t CLC(void) {
    clear_carry();
    return 0;
}

uint8_t CLD(void) {
    clear_decimal();
    return 0;
}

uint8_t CLI(void) {
    clear_interrupt();
    return 0;
}

uint8_t CLV(void) {
    clear_overflow();
    return 0;
}

uint8_t CMP(void) {
    cpu_fetch();
    result = (uint16_t)cpu->a - (uint16_t)fetched;
    set_carry_value(cpu->a >= fetched);
    update_zero_flag(result);
    update_negative_flag(result);
    return 1;
}

uint8_t CPX(void) {
    cpu_fetch();
    result = (uint16_t)cpu->x - (uint16_t)fetched;
    set_carry_value(cpu->x >= fetched);
    update_zero_flag(result);
    update_negative_flag(result);
    return 0;
}

uint8_t CPY(void) {
    cpu_fetch();
    result = (uint16_t)cpu->y - (uint16_t)fetched;
    set_carry_value(cpu->y >= fetched);
    update_zero_flag(result);
    update_negative_flag(result);
    return 0;
}

uint8_t DEC(void) {
    cpu_fetch();
    result = fetched - 1;
    update_zero_flag(result);
    update_negative_flag(result);
    cpu_write(addr, result & 0x00FF);
    return 0;
}

uint8_t DEX(void) {
    cpu->x--;
    update_zero_flag(cpu->x);
    update_negative_flag(cpu->x);
    return 0;
}

uint8_t DEY(void) {
    cpu->y--;
    update_zero_flag(cpu->y);
    update_negative_flag(cpu->y);
    return 0;
}

uint8_t EOR(void) {
    cpu_fetch();
    cpu->a = cpu->a ^ fetched;
    update_zero_flag(cpu->a);
    update_negative_flag(cpu->a);
    return 1;
}

uint8_t INC(void) {
    cpu_fetch();
    result = fetched + 1;
    cpu_write(addr, result & 0x00FF);
    update_zero_flag(result);
    update_negative_flag(result);
    return 0;
}

uint8_t INX(void) {
    cpu->x++;
    update_zero_flag(cpu->x);
    update_negative_flag(cpu->x);
    return 0;
}

uint8_t INY(void) {
    cpu->y++;
    update_zero_flag(cpu->y);
    update_negative_flag(cpu->y);
    return 0;
}

uint8_t JMP(void) {
    cpu->pc = addr;
    return 0;
}

uint8_t JSR(void) {
    cpu->pc--;
    push_word(cpu->pc);
    cpu->pc = addr;
    return 0;
}

uint8_t LDA(void) {
    cpu_fetch();
    cpu->a = fetched;
    update_zero_flag(cpu->a);
    update_negative_flag(cpu->a);
    return 1;
}

uint8_t LDX(void) {
    cpu_fetch();
    cpu->x = fetched;
    update_zero_flag(cpu->x);
    update_negative_flag(cpu->x);
    return 1;
}

uint8_t LDY(void) {
    cpu_fetch();
    cpu->y = fetched;
    update_zero_flag(cpu->y);
    update_negative_flag(cpu->y);
    return 1;
}

uint8_t LSR(void) {
    cpu_fetch();
    set_carry_value(fetched & 0x01);
    result = fetched >> 1;
    update_zero_flag(result);
    update_negative_flag(result);
    set_value(result);
    return 0;
}

uint8_t NOP(void) {
    switch (cpu->opcode) {
        case 0x1C:
        case 0x3C:
        case 0x5C:
        case 0x7C:
        case 0xDC:
        case 0xFC:
            return 1;
        default:
            return 0;
    }
}

uint8_t ORA(void) {
    cpu_fetch();
    cpu->a = cpu->a | fetched;
    update_zero_flag(cpu->a);
    update_negative_flag(cpu->a);
    return 1;
}

uint8_t PHA(void) {
    push_byte(cpu->a);
    return 0;
}

uint8_t PHP(void) {
    push_byte(cpu->status | B | U);
    clear_break();
    clear_unused();
    return 0;
}

uint8_t PLA(void) {
    cpu->a = pop_byte();
    update_zero_flag(cpu->a);
    update_negative_flag(cpu->a);
    return 0;
}

uint8_t PLP(void) {
    cpu->status = pop_byte();
    set_unused();
    return 0;
}

uint8_t ROL(void) {
    cpu_fetch();
    result = get_carry_word() | ((uint16_t)fetched << 1);
    update_carry_flag(result);
    update_zero_flag(result);
    update_negative_flag(result);
    set_value(result);
    return 0;
}

uint8_t ROR(void) {
    cpu_fetch();
    result = (get_carry_word() << 7) | (fetched >> 1);
    set_carry_value(fetched & 0x01);
    update_zero_flag(result);
    update_negative_flag(result);
    set_value(result);
    return 0;
}

uint8_t RTI(void) {
    cpu->status = pop_byte();
    cpu->status &= ~B;
    cpu->status &= ~U;
    cpu->pc = pop_word();
    return 0;
}

uint8_t RTS(void) {
    cpu->pc = pop_word();
    cpu->pc++;
    return 0;
}

uint8_t SBC(void) {
    cpu_fetch();
    const uint16_t value = ((uint16_t)fetched) ^ 0x00FF;
    result = (uint16_t)cpu->a + value + get_carry_word();
    update_carry_flag(result);
    update_zero_flag(result);
    const uint16_t part1 = (result ^ (uint16_t)cpu->a);
    const uint16_t part2 = (result ^ value);
    set_overflow_value(part1 & part2 & 0x0080);
    update_negative_flag(result);
    set_acc(result);
    return 1;
}

uint8_t SEC(void) {
    set_carry();
    return 0;
}

uint8_t SED(void) {
    set_decimal();
    return 0;
}

uint8_t SEI(void) {
    set_interrupt();
    return 0;
}

uint8_t STA(void) {
    cpu_write(addr, cpu->a);
    return 0;
}

uint8_t STX(void) {
    cpu_write(addr, cpu->x);
    return 0;
}

uint8_t STY(void) {
    cpu_write(addr, cpu->y);
    return 0;
}

uint8_t TAX(void) {
    cpu->x = cpu->a;
    update_zero_flag(cpu->x);
    update_negative_flag(cpu->x);
    return 0;
}

uint8_t TAY(void) {
    cpu->y = cpu->a;
    update_zero_flag(cpu->y);
    update_negative_flag(cpu->y);
    return 0;
}

uint8_t TSX(void) {
    cpu->x = cpu->sp;
    update_zero_flag(cpu->x);
    update_negative_flag(cpu->x);
    return 0;
}

uint8_t TXA(void) {
    cpu->a = cpu->x;
    update_zero_flag(cpu->a);
    update_negative_flag(cpu->a);
    return 0;
}

uint8_t TXS(void) {
    cpu->sp = cpu->x;
    return 0;
}

uint8_t TYA(void) {
    cpu->a = cpu->y;
    update_zero_flag(cpu->a);
    update_negative_flag(cpu->a);
    return 0;
}

uint8_t ZZZ(void) {
    fprintf(stderr, "Unknown opcode: %02X (%d) at [%04X]\n", cpu->opcode, cpu->opcode, cpu->pc - 1);
    // exit(1);
    return 0;
}

void cpu_clock(void) {
    if (cpu->cycles == 0) {
        cpu->opcode = cpu_read(cpu->pc);
        set_unused();
        cpu->pc++;

        cpu->cycles = lut[cpu->opcode].cycles;
        const uint8_t add_cycle1 = lut[cpu->opcode].mode();
        const uint8_t add_cycle2 = lut[cpu->opcode].exec();
        cpu->cycles += add_cycle1 & add_cycle2;
        set_unused();
    }

    cpu->cycles--;
}

void cpu_reset(void) {
    cpu->sp = 0xFD;
    cpu->a = 0x00;
    cpu->x = 0x00;
    cpu->y = 0x00;
    cpu->status = 0x00 | U;
    addr = 0xFFFC;
    const uint16_t lo = cpu_read(addr);
    const uint16_t hi = cpu_read(addr + 1);
    cpu->pc = hi << 8 | lo;
    // NESTEST
    // cpu->pc = 0xC000;
    cpu->opcode = 0x00;
    cpu->cycles = 0;
    addr = 0x00;
    fetched = 0x00;
    cpu->cycles = 8;
}

void cpu_irq(void) {
    if (get_interrupt() == 0) {
        push_word(cpu->pc);

        clear_break();
        set_unused();
        set_interrupt();
        push_byte(cpu->status);

        addr = 0xFFFE;
        const uint16_t lo = cpu_read(addr);
        const uint16_t hi = cpu_read(addr + 1);
        cpu->pc = (hi << 8) | lo;

        cpu->cycles = 7;
    }
}

void cpu_nmi(void) {
    push_word(cpu->pc);

    clear_break();
    set_unused();
    set_interrupt();
    push_byte(cpu->status);

    addr = 0xFFFA;
    const uint16_t lo = cpu_read(addr);
    const uint16_t hi = cpu_read(addr + 1);
    cpu->pc = hi << 8 | lo;

    cpu->cycles = 8;
}

uint8_t cpu_fetch(void) {
    if (!(lut[cpu->opcode].mode == &IMP)) {
        fetched = cpu_read(addr);
    }
    return fetched;
}

inline void set_carry(void) { cpu->status |= C; }

inline void set_zero(void) { cpu->status |= Z; }

inline void set_interrupt(void) { cpu->status |= I; }

inline void set_decimal(void) { cpu->status |= D; }

inline void set_break(void) { cpu->status |= B; }

inline void set_negative(void) { cpu->status |= N; }

inline void set_overflow(void) { cpu->status |= V; }

inline void set_unused(void) { cpu->status |= U; }

inline void clear_carry(void) { cpu->status &= ~C; }

inline void clear_zero(void) { cpu->status &= ~Z; }

inline void clear_interrupt(void) { cpu->status &= ~I; }

inline void clear_decimal(void) { cpu->status &= ~D; }

inline void clear_break(void) { cpu->status &= ~B; }

inline void clear_negative(void) { cpu->status &= ~N; }

inline void clear_overflow(void) { cpu->status &= ~V; }

inline void clear_unused(void) { cpu->status &= ~U; }

inline uint8_t get_carry(void) { return cpu->status & C; }

inline uint8_t get_zero(void) { return cpu->status & Z; }

inline uint8_t get_interrupt(void) { return cpu->status & I; }

inline uint8_t get_decimal(void) { return cpu->status & D; }

inline uint8_t get_break(void) { return cpu->status & B; }

inline uint8_t get_negative(void) { return cpu->status & N; }

inline uint8_t get_overflow(void) { return cpu->status & V; }

inline uint8_t get_unused(void) { return cpu->status & U; }

inline uint16_t get_carry_word(void) { return (uint16_t)get_carry(); }

inline uint16_t get_zero_word(void) { return (uint16_t)get_zero(); }

inline uint16_t get_interrupt_word(void) { return (uint16_t)get_interrupt(); }

inline uint16_t get_decimal_word(void) { return (uint16_t)get_decimal(); }

inline uint16_t get_break_word(void) { return (uint16_t)get_break(); }

inline uint16_t get_negative_word(void) { return (uint16_t)get_negative(); }

inline uint16_t get_overflow_word(void) { return (uint16_t)get_overflow(); }

inline uint16_t get_unused_word(void) { return (uint16_t)get_unused(); }

inline void set_carry_value(const bool value) {
    if (value)
        set_carry();
    else
        clear_carry();
}

inline void set_zero_value(const bool value) {
    if (value)
        set_zero();
    else
        clear_zero();
}

inline void set_interrupt_value(const bool value) {
    if (value)
        set_interrupt();
    else
        clear_interrupt();
}

inline void set_decimal_value(const bool value) {
    if (value)
        set_decimal();
    else
        clear_decimal();
}

inline void set_break_value(const bool value) {
    if (value)
        set_break();
    else
        clear_break();
}

inline void set_negative_value(const bool value) {
    if (value)
        set_negative();
    else
        clear_negative();
}

inline void set_overflow_value(const bool value) {
    if (value)
        set_overflow();
    else
        clear_overflow();
}

inline void set_unused_value(const bool value) {
    if (value)
        set_unused();
    else
        clear_unused();
}

inline void push_word(const uint16_t value) {
    push_byte((value >> 8) & 0xFF);
    push_byte(value & 0xFF);
}

inline void push_byte(const uint8_t value) {
    cpu_write(BASE_STACK + (uint16_t)cpu->sp, value);
    cpu->sp--;
}

inline uint16_t pop_word(void) {
    const uint16_t lo = pop_byte();
    const uint16_t hi = pop_byte();
    return (hi << 8) | lo;
}

inline uint8_t pop_byte(void) {
    cpu->sp++;
    return cpu_read(BASE_STACK + (uint16_t)cpu->sp);
}

inline void update_zero_flag(const uint16_t value) {
    if ((value & 0x00FF) == 0)
        set_zero();
    else
        clear_zero();
}

inline void update_negative_flag(const uint16_t value) {
    if (value & 0x0080)
        set_negative();
    else
        clear_negative();
}

inline void update_carry_flag(const uint16_t value) {
    if (value & 0xFF00)
        set_carry();
    else
        clear_carry();
}

inline void set_value(const uint16_t value) {
    if (lut[cpu->opcode].mode == &IMP) {
        set_acc(value);
    } else {
        cpu_write(addr, (uint8_t)(value & 0x00FF));
    }
}

inline bool is_carry_set(void) { return (get_carry() == C); }

inline bool is_zero_set(void) { return (get_zero() == Z); }

inline bool is_interrupt_set(void) { return (get_interrupt() == I); }

inline bool is_decimal_set(void) { return (get_decimal() == D); }

inline bool is_break_set(void) { return (get_break() == B); }

inline bool is_negative_set(void) { return (get_negative() == N); }

inline bool is_overflow_set(void) { return (get_overflow() == V); }

inline bool is_unused_set(void) { return (get_unused() == U); }

inline void branch(void) {
    cpu->cycles++;
    addr = cpu->pc + branch_addr;

    if ((addr & 0xFF00) != (cpu->pc & 0xFF00))
        cpu->cycles++;

    cpu->pc = addr;
}

// Disasm for UI

void disasm_addr(Bus *bus, uint16_t addr) {
    uint8_t value = 0x0;
    uint8_t lo = 0x0;
    uint8_t hi = 0x0;
    // Prefix line with instruction address
    char sInst[1024] = "";
    char buff[1024];
    sprintf(buff, "$%04X: ", addr);
    strcat(sInst, buff);

    // Read instruction, and get its readable name
    uint8_t opcode = bus->read(addr);
    addr++;
    strcat(sInst, lut[opcode].name);
    strcat(sInst, " ");

    // Get oprands from desired locations, and form the
    // instruction based upon its addressing mode. These
    // routines mimmick the actual fetch routine of the
    // 6502 in order to get accurate data as part of the
    // instruction
    if (lut[opcode].mode == &IMP) {
        strcat(sInst, " {IMP}");
    } else if (lut[opcode].mode == &IMM) {
        value = bus->read(addr);
        addr++;
        sprintf(buff, "$%02X {IMM}", value);
        strcat(sInst, buff);
    } else if (lut[opcode].mode == &ZP0) {
        lo = bus->read(addr);
        addr++;
        hi = 0x00;
        sprintf(buff, "$%02X {ZP0}", lo);
        strcat(sInst, buff);
    } else if (lut[opcode].mode == &ZPX) {
        lo = bus->read(addr);
        addr++;
        hi = 0x00;
        sprintf(buff, "$%02X, X {ZPX}", lo);
        strcat(sInst, buff);
    } else if (lut[opcode].mode == &ZPY) {
        lo = bus->read(addr);
        addr++;
        hi = 0x00;
        sprintf(buff, "$%02X, Y {ZPY}", lo);
        strcat(sInst, buff);
    } else if (lut[opcode].mode == &IZX) {
        lo = bus->read(addr);
        addr++;
        hi = 0x00;
        sprintf(buff, "($%02X, X) {IZX}", lo);
        strcat(sInst, buff);
    } else if (lut[opcode].mode == &IZY) {
        lo = bus->read(addr);
        addr++;
        hi = 0x00;
        sprintf(buff, "($%02X), Y {IZY}", lo);
        strcat(sInst, buff);
    } else if (lut[opcode].mode == &ABS) {
        lo = bus->read(addr);
        addr++;
        hi = bus->read(addr);
        addr++;
        sprintf(buff, "$%04X {ABS}", (uint16_t)(hi << 8) | lo);
        strcat(sInst, buff);
    } else if (lut[opcode].mode == &ABX) {
        lo = bus->read(addr);
        addr++;
        hi = bus->read(addr);
        addr++;
        sprintf(buff, "$%04X, X {ABX}", (uint16_t)(hi << 8) | lo);
        strcat(sInst, buff);
    } else if (lut[opcode].mode == &ABY) {
        lo = bus->read(addr);
        addr++;
        hi = bus->read(addr);
        addr++;
        sprintf(buff, "$%04X, Y {ABY}", (uint16_t)(hi << 8) | lo);
        strcat(sInst, buff);
    } else if (lut[opcode].mode == &IND) {
        lo = bus->read(addr);
        addr++;
        hi = bus->read(addr);
        addr++;
        sprintf(buff, "($%04X) {IND}", (uint16_t)(hi << 8) | lo);
        strcat(sInst, buff);
    } else if (lut[opcode].mode == &REL) {
        value = bus->read(addr);
        addr++;
        sprintf(buff, "$%02X [$%04X] {REL}", value, addr + (int8_t)value);
        strcat(sInst, buff);
    }
    printf("%s\n", sInst);
}

disasm *disassemble(Bus *bus, uint16_t nStart, uint16_t nStop) {
    uint32_t addr = nStart;
    uint8_t value = 0x00, lo = 0x00, hi = 0x00;
    disasm *mapLines = calloc(0xFFFF, sizeof(disasm));
    uint16_t line_addr = 0;

    uint16_t lastline = 0;
    while (addr <= (uint32_t)nStop) {
        line_addr = addr;

        char sInst[1024] = "";
        char buff[1024];
        sprintf(buff, "$%04X: ", addr);
        strcat(sInst, buff);

        uint8_t opcode = bus->read(addr);
        addr++;
        strcat(sInst, lut[opcode].name);
        strcat(sInst, " ");

        if (lut[opcode].mode == &IMP) {
            strcat(sInst, " {IMP}");
        } else if (lut[opcode].mode == &IMM) {
            value = bus->read(addr);
            addr++;
            sprintf(buff, "$%02X {IMM}", value);
            strcat(sInst, buff);
        } else if (lut[opcode].mode == &ZP0) {
            lo = bus->read(addr);
            addr++;
            hi = 0x00;
            sprintf(buff, "$%02X {ZP0}", lo);
            strcat(sInst, buff);
        } else if (lut[opcode].mode == &ZPX) {
            lo = bus->read(addr);
            addr++;
            hi = 0x00;
            sprintf(buff, "$%02X, X {ZPX}", lo);
            strcat(sInst, buff);
        } else if (lut[opcode].mode == &ZPY) {
            lo = bus->read(addr);
            addr++;
            hi = 0x00;
            sprintf(buff, "$%02X, Y {ZPY}", lo);
            strcat(sInst, buff);
        } else if (lut[opcode].mode == &IZX) {
            lo = bus->read(addr);
            addr++;
            hi = 0x00;
            sprintf(buff, "($%02X, X) {IZX}", lo);
            strcat(sInst, buff);
        } else if (lut[opcode].mode == &IZY) {
            lo = bus->read(addr);
            addr++;
            hi = 0x00;
            sprintf(buff, "($%02X), Y {IZY}", lo);
            strcat(sInst, buff);
        } else if (lut[opcode].mode == &ABS) {
            lo = bus->read(addr);
            addr++;
            hi = bus->read(addr);
            addr++;
            sprintf(buff, "$%04X {ABS}", (uint16_t)(hi << 8) | lo);
            strcat(sInst, buff);
        } else if (lut[opcode].mode == &ABX) {
            lo = bus->read(addr);
            addr++;
            hi = bus->read(addr);
            addr++;
            sprintf(buff, "$%04X, X {ABX}", (uint16_t)(hi << 8) | lo);
            strcat(sInst, buff);
        } else if (lut[opcode].mode == &ABY) {
            lo = bus->read(addr);
            addr++;
            hi = bus->read(addr);
            addr++;
            sprintf(buff, "$%04X, Y {ABY}", (uint16_t)(hi << 8) | lo);
            strcat(sInst, buff);
        } else if (lut[opcode].mode == &IND) {
            lo = bus->read(addr);
            addr++;
            hi = bus->read(addr);
            addr++;
            sprintf(buff, "($%04X) {IND}", (uint16_t)(hi << 8) | lo);
            strcat(sInst, buff);
        } else if (lut[opcode].mode == &REL) {
            value = bus->read(addr);
            addr++;
            sprintf(buff, "$%02X [$%04X] {REL}", value, addr + (int8_t)value);
            strcat(sInst, buff);
        }

        mapLines[line_addr].inst = (char *)malloc(strlen(sInst) + 1);
        strcpy(mapLines[line_addr].inst, sInst);
        if (lastline > 0) {
            mapLines[lastline].next = &mapLines[line_addr];
            mapLines[line_addr].prev = &mapLines[lastline];
        }
        lastline = line_addr;
    }

    return mapLines;
}
