#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#include "forward.h"

struct Instruction {
    char *name;
    uint8_t (*exec)(void);
    uint8_t (*mode)(void);
    uint8_t cycles;
};

struct Cpu {
    Bus *bus;
    uint8_t (*read)(uint16_t);
    void (*write)(uint16_t, uint8_t);
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t sp;
    uint16_t pc;
    uint8_t status;
    uint8_t opcode;
    uint8_t cycles;
};

enum FLAGS6502 {
    C = (1 << 0), // Carry Bit
    Z = (1 << 1), // Zero
    I = (1 << 2), // Disable Interrupts
    D = (1 << 3), // Decimal Mode (unused in this implementation)
    B = (1 << 4), // Break
    U = (1 << 5), // Unused
    V = (1 << 6), // Overflow
    N = (1 << 7), // Negative
};

typedef struct disasm {
    char *inst;
    struct disasm *prev;
    struct disasm *next;
} disasm;

Cpu *cpu_new(Bus *bus);
void cpu_free(Cpu *cpu);

uint8_t get_cpu_flag(enum FLAGS6502 flag);

void cpu_clock(void);
void cpu_reset(void);
void cpu_irq(void);
void cpu_nmi(void);

uint8_t cpu_fetch(void);

void disasm_addr(Bus *bus, uint16_t addr);
disasm *disassemble(Bus *bus, uint16_t nStart, uint16_t nStop);

#ifdef IMPLEMENT_CPU

uint8_t cpu_read(uint16_t addr);
void cpu_write(uint16_t addr, uint8_t data);

// Const

constexpr uint16_t BASE_STACK = 0x0100;

// Util

Cpu *cpu;
uint16_t result;
uint8_t fetched;
uint16_t addr;
uint16_t branch_addr;

void set_carry(void);
void set_zero(void);
void set_interrupt(void);
void set_decimal(void);
void set_break(void);
void set_negative(void);
void set_overflow(void);
void set_unused(void);
void clear_carry(void);
void clear_zero(void);
void clear_interrupt(void);
void clear_decimal(void);
void clear_break(void);
void clear_negative(void);
void clear_overflow(void);
void clear_unused(void);
uint8_t get_carry(void);
uint8_t get_zero(void);
uint8_t get_interrupt(void);
uint8_t get_decimal(void);
uint8_t get_break(void);
uint8_t get_negative(void);
uint8_t get_overflow(void);
uint8_t get_unused(void);
uint16_t get_carry_word(void);
uint16_t get_zero_word(void);
uint16_t get_interrupt_word(void);
uint16_t get_decimal_word(void);
uint16_t get_break_word(void);
uint16_t get_negative_word(void);
uint16_t get_overflow_word(void);
uint16_t get_unused_word(void);
void set_carry_value(bool value);
void set_zero_value(bool value);
void set_interrupt_value(bool value);
void set_decimal_value(bool value);
void set_break_value(bool value);
void set_negative_value(bool value);
void set_overflow_value(bool value);
void set_unused_value(bool value);

bool is_carry_set(void);
bool is_zero_set(void);
bool is_interrupt_set(void);
bool is_decimal_set(void);
bool is_break_set(void);
bool is_negative_set(void);
bool is_overflow_set(void);
bool is_unused_set(void);

void branch(void);

void update_zero_flag(uint16_t value);
void update_negative_flag(uint16_t value);
void update_carry_flag(uint16_t value);

void push_word(uint16_t value);
void push_byte(uint8_t value);
uint16_t pop_word(void);
uint8_t pop_byte(void);

#define set_acc(n) cpu->a = (uint8_t)((n) & 0x00FF)
void set_value(uint16_t value);

// Address Modes

uint8_t IMP(void);
uint8_t IMM(void);
uint8_t ZP0(void);
uint8_t ZPX(void);
uint8_t ZPY(void);
uint8_t REL(void);
uint8_t ABS(void);
uint8_t ABX(void);
uint8_t ABY(void);
uint8_t IND(void);
uint8_t IZX(void);
uint8_t IZY(void);

// Opcodes

uint8_t ADC(void);
uint8_t AND(void);
uint8_t ASL(void);
uint8_t BCC(void);
uint8_t BCS(void);
uint8_t BEQ(void);
uint8_t BIT(void);
uint8_t BMI(void);
uint8_t BNE(void);
uint8_t BPL(void);
uint8_t BRK(void);
uint8_t BVC(void);
uint8_t BVS(void);
uint8_t CLC(void);
uint8_t CLD(void);
uint8_t CLI(void);
uint8_t CLV(void);
uint8_t CMP(void);
uint8_t CPX(void);
uint8_t CPY(void);
uint8_t DEC(void);
uint8_t DEX(void);
uint8_t DEY(void);
uint8_t EOR(void);
uint8_t INC(void);
uint8_t INX(void);
uint8_t INY(void);
uint8_t JMP(void);
uint8_t JSR(void);
uint8_t LDA(void);
uint8_t LDX(void);
uint8_t LDY(void);
uint8_t LSR(void);
uint8_t NOP(void);
uint8_t ORA(void);
uint8_t PHA(void);
uint8_t PHP(void);
uint8_t PLA(void);
uint8_t PLP(void);
uint8_t ROL(void);
uint8_t ROR(void);
uint8_t RTI(void);
uint8_t RTS(void);
uint8_t SBC(void);
uint8_t SEC(void);
uint8_t SED(void);
uint8_t SEI(void);
uint8_t STA(void);
uint8_t STX(void);
uint8_t STY(void);
uint8_t TAX(void);
uint8_t TAY(void);
uint8_t TSX(void);
uint8_t TXA(void);
uint8_t TXS(void);
uint8_t TYA(void);

uint8_t ZZZ(void);

Instruction lut[256] = {
    {"BRK", &BRK, &IMM, 7}, // 0 (0x0)
    {"ORA", &ORA, &IZX, 6}, // 1 (0x1)
    {"???", &ZZZ, &IMP, 2}, // 2 (0x2)
    {"???", &ZZZ, &IMP, 8}, // 3 (0x3)
    {"???", &NOP, &IMP, 3}, // 4 (0x4)
    {"ORA", &ORA, &ZP0, 3}, // 5 (0x5)
    {"ASL", &ASL, &ZP0, 5}, // 6 (0x6)
    {"???", &ZZZ, &IMP, 5}, // 7 (0x7)
    {"PHP", &PHP, &IMP, 3}, // 8 (0x8)
    {"ORA", &ORA, &IMM, 2}, // 9 (0x9)
    {"ASL", &ASL, &IMP, 2}, // 10 (0xA)
    {"???", &ZZZ, &IMP, 2}, // 11 (0xB)
    {"???", &NOP, &IMP, 4}, // 12 (0xC)
    {"ORA", &ORA, &ABS, 4}, // 13 (0xD)
    {"ASL", &ASL, &ABS, 6}, // 14 (0xE)
    {"???", &ZZZ, &IMP, 6}, // 15 (0xF)
    {"BPL", &BPL, &REL, 2}, // 16 (0x10)
    {"ORA", &ORA, &IZY, 5}, // 17 (0x11)
    {"???", &ZZZ, &IMP, 2}, // 18 (0x12)
    {"???", &ZZZ, &IMP, 8}, // 19 (0x13)
    {"???", &NOP, &IMP, 4}, // 20 (0x14)
    {"ORA", &ORA, &ZPX, 4}, // 21 (0x15)
    {"ASL", &ASL, &ZPX, 6}, // 22 (0x16)
    {"???", &ZZZ, &IMP, 6}, // 23 (0x17)
    {"CLC", &CLC, &IMP, 2}, // 24 (0x18)
    {"ORA", &ORA, &ABY, 4}, // 25 (0x19)
    {"???", &NOP, &IMP, 2}, // 26 (0x1A)
    {"???", &ZZZ, &IMP, 7}, // 27 (0x1B)
    {"???", &NOP, &IMP, 4}, // 28 (0x1C)
    {"ORA", &ORA, &ABX, 4}, // 29 (0x1D)
    {"ASL", &ASL, &ABX, 7}, // 30 (0x1E)
    {"???", &ZZZ, &IMP, 7}, // 31 (0x1F)
    {"JSR", &JSR, &ABS, 6}, // 32 (0x20)
    {"AND", &AND, &IZX, 6}, // 33 (0x21)
    {"???", &ZZZ, &IMP, 2}, // 34 (0x22)
    {"???", &ZZZ, &IMP, 8}, // 35 (0x23)
    {"BIT", &BIT, &ZP0, 3}, // 36 (0x24)
    {"AND", &AND, &ZP0, 3}, // 37 (0x25)
    {"ROL", &ROL, &ZP0, 5}, // 38 (0x26)
    {"???", &ZZZ, &IMP, 5}, // 39 (0x27)
    {"PLP", &PLP, &IMP, 4}, // 40 (0x28)
    {"AND", &AND, &IMM, 2}, // 41 (0x29)
    {"ROL", &ROL, &IMP, 2}, // 42 (0x2A)
    {"???", &ZZZ, &IMP, 2}, // 43 (0x2B)
    {"BIT", &BIT, &ABS, 4}, // 44 (0x2C)
    {"AND", &AND, &ABS, 4}, // 45 (0x2D)
    {"ROL", &ROL, &ABS, 6}, // 46 (0x2E)
    {"???", &ZZZ, &IMP, 6}, // 47 (0x2F)
    {"BMI", &BMI, &REL, 2}, // 48 (0x30)
    {"AND", &AND, &IZY, 5}, // 49 (0x31)
    {"???", &ZZZ, &IMP, 2}, // 50 (0x32)
    {"???", &ZZZ, &IMP, 8}, // 51 (0x33)
    {"???", &NOP, &IMP, 4}, // 52 (0x34)
    {"AND", &AND, &ZPX, 4}, // 53 (0x35)
    {"ROL", &ROL, &ZPX, 6}, // 54 (0x36)
    {"???", &ZZZ, &IMP, 6}, // 55 (0x37)
    {"SEC", &SEC, &IMP, 2}, // 56 (0x38)
    {"AND", &AND, &ABY, 4}, // 57 (0x39)
    {"???", &NOP, &IMP, 2}, // 58 (0x3A)
    {"???", &ZZZ, &IMP, 7}, // 59 (0x3B)
    {"???", &NOP, &IMP, 4}, // 60 (0x3C)
    {"AND", &AND, &ABX, 4}, // 61 (0x3D)
    {"ROL", &ROL, &ABX, 7}, // 62 (0x3E)
    {"???", &ZZZ, &IMP, 7}, // 63 (0x3F)
    {"RTI", &RTI, &IMP, 6}, // 64 (0x40)
    {"EOR", &EOR, &IZX, 6}, // 65 (0x41)
    {"???", &ZZZ, &IMP, 2}, // 66 (0x42)
    {"???", &ZZZ, &IMP, 8}, // 67 (0x43)
    {"???", &NOP, &IMP, 3}, // 68 (0x44)
    {"EOR", &EOR, &ZP0, 3}, // 69 (0x45)
    {"LSR", &LSR, &ZP0, 5}, // 70 (0x46)
    {"???", &ZZZ, &IMP, 5}, // 71 (0x47)
    {"PHA", &PHA, &IMP, 3}, // 72 (0x48)
    {"EOR", &EOR, &IMM, 2}, // 73 (0x49)
    {"LSR", &LSR, &IMP, 2}, // 74 (0x4A)
    {"???", &ZZZ, &IMP, 2}, // 75 (0x4B)
    {"JMP", &JMP, &ABS, 3}, // 76 (0x4C)
    {"EOR", &EOR, &ABS, 4}, // 77 (0x4D)
    {"LSR", &LSR, &ABS, 6}, // 78 (0x4E)
    {"???", &ZZZ, &IMP, 6}, // 79 (0x4F)
    {"BVC", &BVC, &REL, 2}, // 80 (0x50)
    {"EOR", &EOR, &IZY, 5}, // 81 (0x51)
    {"???", &ZZZ, &IMP, 2}, // 82 (0x52)
    {"???", &ZZZ, &IMP, 8}, // 83 (0x53)
    {"???", &NOP, &IMP, 4}, // 84 (0x54)
    {"EOR", &EOR, &ZPX, 4}, // 85 (0x55)
    {"LSR", &LSR, &ZPX, 6}, // 86 (0x56)
    {"???", &ZZZ, &IMP, 6}, // 87 (0x57)
    {"CLI", &CLI, &IMP, 2}, // 88 (0x58)
    {"EOR", &EOR, &ABY, 4}, // 89 (0x59)
    {"???", &NOP, &IMP, 2}, // 90 (0x5A)
    {"???", &ZZZ, &IMP, 7}, // 91 (0x5B)
    {"???", &NOP, &IMP, 4}, // 92 (0x5C)
    {"EOR", &EOR, &ABX, 4}, // 93 (0x5D)
    {"LSR", &LSR, &ABX, 7}, // 94 (0x5E)
    {"???", &ZZZ, &IMP, 7}, // 95 (0x5F)
    {"RTS", &RTS, &IMP, 6}, // 96 (0x60)
    {"ADC", &ADC, &IZX, 6}, // 97 (0x61)
    {"???", &ZZZ, &IMP, 2}, // 98 (0x62)
    {"???", &ZZZ, &IMP, 8}, // 99 (0x63)
    {"???", &NOP, &IMP, 3}, // 100 (0x64)
    {"ADC", &ADC, &ZP0, 3}, // 101 (0x65)
    {"ROR", &ROR, &ZP0, 5}, // 102 (0x66)
    {"???", &ZZZ, &IMP, 5}, // 103 (0x67)
    {"PLA", &PLA, &IMP, 4}, // 104 (0x68)
    {"ADC", &ADC, &IMM, 2}, // 105 (0x69)
    {"ROR", &ROR, &IMP, 2}, // 106 (0x6A)
    {"???", &ZZZ, &IMP, 2}, // 107 (0x6B)
    {"JMP", &JMP, &IND, 5}, // 108 (0x6C)
    {"ADC", &ADC, &ABS, 4}, // 109 (0x6D)
    {"ROR", &ROR, &ABS, 6}, // 110 (0x6E)
    {"???", &ZZZ, &IMP, 6}, // 111 (0x6F)
    {"BVS", &BVS, &REL, 2}, // 112 (0x70)
    {"ADC", &ADC, &IZY, 5}, // 113 (0x71)
    {"???", &ZZZ, &IMP, 2}, // 114 (0x72)
    {"???", &ZZZ, &IMP, 8}, // 115 (0x73)
    {"???", &NOP, &IMP, 4}, // 116 (0x74)
    {"ADC", &ADC, &ZPX, 4}, // 117 (0x75)
    {"ROR", &ROR, &ZPX, 6}, // 118 (0x76)
    {"???", &ZZZ, &IMP, 6}, // 119 (0x77)
    {"SEI", &SEI, &IMP, 2}, // 120 (0x78)
    {"ADC", &ADC, &ABY, 4}, // 121 (0x79)
    {"???", &NOP, &IMP, 2}, // 122 (0x7A)
    {"???", &ZZZ, &IMP, 7}, // 123 (0x7B)
    {"???", &NOP, &IMP, 4}, // 124 (0x7C)
    {"ADC", &ADC, &ABX, 4}, // 125 (0x7D)
    {"ROR", &ROR, &ABX, 7}, // 126 (0x7E)
    {"???", &ZZZ, &IMP, 7}, // 127 (0x7F)
    {"???", &NOP, &IMP, 2}, // 128 (0x80)
    {"STA", &STA, &IZX, 6}, // 129 (0x81)
    {"???", &NOP, &IMP, 2}, // 130 (0x82)
    {"???", &ZZZ, &IMP, 6}, // 131 (0x83)
    {"STY", &STY, &ZP0, 3}, // 132 (0x84)
    {"STA", &STA, &ZP0, 3}, // 133 (0x85)
    {"STX", &STX, &ZP0, 3}, // 134 (0x86)
    {"???", &ZZZ, &IMP, 3}, // 135 (0x87)
    {"DEY", &DEY, &IMP, 2}, // 136 (0x88)
    {"???", &NOP, &IMP, 2}, // 137 (0x89)
    {"TXA", &TXA, &IMP, 2}, // 138 (0x8A)
    {"???", &ZZZ, &IMP, 2}, // 139 (0x8B)
    {"STY", &STY, &ABS, 4}, // 140 (0x8C)
    {"STA", &STA, &ABS, 4}, // 141 (0x8D)
    {"STX", &STX, &ABS, 4}, // 142 (0x8E)
    {"???", &ZZZ, &IMP, 4}, // 143 (0x8F)
    {"BCC", &BCC, &REL, 2}, // 144 (0x90)
    {"STA", &STA, &IZY, 6}, // 145 (0x91)
    {"???", &ZZZ, &IMP, 2}, // 146 (0x92)
    {"???", &ZZZ, &IMP, 6}, // 147 (0x93)
    {"STY", &STY, &ZPX, 4}, // 148 (0x94)
    {"STA", &STA, &ZPX, 4}, // 149 (0x95)
    {"STX", &STX, &ZPY, 4}, // 150 (0x96)
    {"???", &ZZZ, &IMP, 4}, // 151 (0x97)
    {"TYA", &TYA, &IMP, 2}, // 152 (0x98)
    {"STA", &STA, &ABY, 5}, // 153 (0x99)
    {"TXS", &TXS, &IMP, 2}, // 154 (0x9A)
    {"???", &ZZZ, &IMP, 5}, // 155 (0x9B)
    {"???", &NOP, &IMP, 5}, // 156 (0x9C)
    {"STA", &STA, &ABX, 5}, // 157 (0x9D)
    {"???", &ZZZ, &IMP, 5}, // 158 (0x9E)
    {"???", &ZZZ, &IMP, 5}, // 159 (0x9F)
    {"LDY", &LDY, &IMM, 2}, // 160 (0xA0)
    {"LDA", &LDA, &IZX, 6}, // 161 (0xA1)
    {"LDX", &LDX, &IMM, 2}, // 162 (0xA2)
    {"???", &ZZZ, &IMP, 6}, // 163 (0xA3)
    {"LDY", &LDY, &ZP0, 3}, // 164 (0xA4)
    {"LDA", &LDA, &ZP0, 3}, // 165 (0xA5)
    {"LDX", &LDX, &ZP0, 3}, // 166 (0xA6)
    {"???", &ZZZ, &IMP, 3}, // 167 (0xA7)
    {"TAY", &TAY, &IMP, 2}, // 168 (0xA8)
    {"LDA", &LDA, &IMM, 2}, // 169 (0xA9)
    {"TAX", &TAX, &IMP, 2}, // 170 (0xAA)
    {"???", &ZZZ, &IMP, 2}, // 171 (0xAB)
    {"LDY", &LDY, &ABS, 4}, // 172 (0xAC)
    {"LDA", &LDA, &ABS, 4}, // 173 (0xAD)
    {"LDX", &LDX, &ABS, 4}, // 174 (0xAE)
    {"???", &ZZZ, &IMP, 4}, // 175 (0xAF)
    {"BCS", &BCS, &REL, 2}, // 176 (0xB0)
    {"LDA", &LDA, &IZY, 5}, // 177 (0xB1)
    {"???", &ZZZ, &IMP, 2}, // 178 (0xB2)
    {"???", &ZZZ, &IMP, 5}, // 179 (0xB3)
    {"LDY", &LDY, &ZPX, 4}, // 180 (0xB4)
    {"LDA", &LDA, &ZPX, 4}, // 181 (0xB5)
    {"LDX", &LDX, &ZPY, 4}, // 182 (0xB6)
    {"???", &ZZZ, &IMP, 4}, // 183 (0xB7)
    {"CLV", &CLV, &IMP, 2}, // 184 (0xB8)
    {"LDA", &LDA, &ABY, 4}, // 185 (0xB9)
    {"TSX", &TSX, &IMP, 2}, // 186 (0xBA)
    {"???", &ZZZ, &IMP, 4}, // 187 (0xBB)
    {"LDY", &LDY, &ABX, 4}, // 188 (0xBC)
    {"LDA", &LDA, &ABX, 4}, // 189 (0xBD)
    {"LDX", &LDX, &ABY, 4}, // 190 (0xBE)
    {"???", &ZZZ, &IMP, 4}, // 191 (0xBF)
    {"CPY", &CPY, &IMM, 2}, // 192 (0xC0)
    {"CMP", &CMP, &IZX, 6}, // 193 (0xC1)
    {"???", &NOP, &IMP, 2}, // 194 (0xC2)
    {"???", &ZZZ, &IMP, 8}, // 195 (0xC3)
    {"CPY", &CPY, &ZP0, 3}, // 196 (0xC4)
    {"CMP", &CMP, &ZP0, 3}, // 197 (0xC5)
    {"DEC", &DEC, &ZP0, 5}, // 198 (0xC6)
    {"???", &ZZZ, &IMP, 5}, // 199 (0xC7)
    {"INY", &INY, &IMP, 2}, // 200 (0xC8)
    {"CMP", &CMP, &IMM, 2}, // 201 (0xC9)
    {"DEX", &DEX, &IMP, 2}, // 202 (0xCA)
    {"???", &ZZZ, &IMP, 2}, // 203 (0xCB)
    {"CPY", &CPY, &ABS, 4}, // 204 (0xCC)
    {"CMP", &CMP, &ABS, 4}, // 205 (0xCD)
    {"DEC", &DEC, &ABS, 6}, // 206 (0xCE)
    {"???", &ZZZ, &IMP, 6}, // 207 (0xCF)
    {"BNE", &BNE, &REL, 2}, // 208 (0xD0)
    {"CMP", &CMP, &IZY, 5}, // 209 (0xD1)
    {"???", &ZZZ, &IMP, 2}, // 210 (0xD2)
    {"???", &ZZZ, &IMP, 8}, // 211 (0xD3)
    {"???", &NOP, &IMP, 4}, // 212 (0xD4)
    {"CMP", &CMP, &ZPX, 4}, // 213 (0xD5)
    {"DEC", &DEC, &ZPX, 6}, // 214 (0xD6)
    {"???", &ZZZ, &IMP, 6}, // 215 (0xD7)
    {"CLD", &CLD, &IMP, 2}, // 216 (0xD8)
    {"CMP", &CMP, &ABY, 4}, // 217 (0xD9)
    {"NOP", &NOP, &IMP, 2}, // 218 (0xDA)
    {"???", &ZZZ, &IMP, 7}, // 219 (0xDB)
    {"???", &NOP, &IMP, 4}, // 220 (0xDC)
    {"CMP", &CMP, &ABX, 4}, // 221 (0xDD)
    {"DEC", &DEC, &ABX, 7}, // 222 (0xDE)
    {"???", &ZZZ, &IMP, 7}, // 223 (0xDF)
    {"CPX", &CPX, &IMM, 2}, // 224 (0xE0)
    {"SBC", &SBC, &IZX, 6}, // 225 (0xE1)
    {"???", &NOP, &IMP, 2}, // 226 (0xE2)
    {"???", &ZZZ, &IMP, 8}, // 227 (0xE3)
    {"CPX", &CPX, &ZP0, 3}, // 228 (0xE4)
    {"SBC", &SBC, &ZP0, 3}, // 229 (0xE5)
    {"INC", &INC, &ZP0, 5}, // 230 (0xE6)
    {"???", &ZZZ, &IMP, 5}, // 231 (0xE7)
    {"INX", &INX, &IMP, 2}, // 232 (0xE8)
    {"SBC", &SBC, &IMM, 2}, // 233 (0xE9)
    {"NOP", &NOP, &IMP, 2}, // 234 (0xEA)
    {"???", &SBC, &IMP, 2}, // 235 (0xEB)
    {"CPX", &CPX, &ABS, 4}, // 236 (0xEC)
    {"SBC", &SBC, &ABS, 4}, // 237 (0xED)
    {"INC", &INC, &ABS, 6}, // 238 (0xEE)
    {"???", &ZZZ, &IMP, 6}, // 239 (0xEF)
    {"BEQ", &BEQ, &REL, 2}, // 240 (0xF0)
    {"SBC", &SBC, &IZY, 5}, // 241 (0xF1)
    {"???", &ZZZ, &IMP, 2}, // 242 (0xF2)
    {"???", &ZZZ, &IMP, 8}, // 243 (0xF3)
    {"???", &NOP, &IMP, 4}, // 244 (0xF4)
    {"SBC", &SBC, &ZPX, 4}, // 245 (0xF5)
    {"INC", &INC, &ZPX, 6}, // 246 (0xF6)
    {"???", &ZZZ, &IMP, 6}, // 247 (0xF7)
    {"SED", &SED, &IMP, 2}, // 248 (0xF8)
    {"SBC", &SBC, &ABY, 4}, // 249 (0xF9)
    {"NOP", &NOP, &IMP, 2}, // 250 (0xFA)
    {"???", &ZZZ, &IMP, 7}, // 251 (0xFB)
    {"???", &NOP, &IMP, 4}, // 252 (0xFC)
    {"SBC", &SBC, &ABX, 4}, // 253 (0xFD)
    {"INC", &INC, &ABX, 7}, // 254 (0xFE)
    {"???", &ZZZ, &IMP, 7}, // 255 (0xFF)
};

#endif

#endif // CPU_H
