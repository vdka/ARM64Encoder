
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#if __has_attribute(swift_wrapper)
#define BRIDGE_ENUM_TO_SWIFT __attribute__((swift_wrapper(enum)))
#else
#define BRIDGE_ENUM_TO_SWIFT
#endif

#if __has_attribute(swift_name)
# define BRIDGE_TO_SWIFT_WITH_NAME(_NAME) __attribute__((swift_name(#_NAME)))
#else
# define BRIDGE_TO_SWIFT_WITH_NAME(_NAME)
#endif


typedef uint32_t u32;
#define EXPORT static

typedef u32 ARM64Reg BRIDGE_ENUM_TO_SWIFT;
ARM64Reg const ARM64RegR0  =  0;
ARM64Reg const ARM64RegR1  =  1;
ARM64Reg const ARM64RegR2  =  2;
ARM64Reg const ARM64RegR3  =  3;
ARM64Reg const ARM64RegR4  =  4;
ARM64Reg const ARM64RegR5  =  5;
ARM64Reg const ARM64RegR6  =  6;
ARM64Reg const ARM64RegR7  =  7;
ARM64Reg const ARM64RegR8  =  8;
ARM64Reg const ARM64RegR9  =  9;
ARM64Reg const ARM64RegR10 = 10;
ARM64Reg const ARM64RegR11 = 11;
ARM64Reg const ARM64RegR12 = 12;
ARM64Reg const ARM64RegR13 = 13;
ARM64Reg const ARM64RegR14 = 14;
ARM64Reg const ARM64RegR15 = 15;
ARM64Reg const ARM64RegR16 = 16;
ARM64Reg const ARM64RegR17 = 17;
ARM64Reg const ARM64RegR18 = 18;
ARM64Reg const ARM64RegR19 = 19;
ARM64Reg const ARM64RegR20 = 20;
ARM64Reg const ARM64RegR21 = 21;
ARM64Reg const ARM64RegR22 = 22;
ARM64Reg const ARM64RegR23 = 23;
ARM64Reg const ARM64RegR24 = 24;
ARM64Reg const ARM64RegR25 = 25;
ARM64Reg const ARM64RegR26 = 26;
ARM64Reg const ARM64RegR27 = 27;
ARM64Reg const ARM64RegR28 = 28;
ARM64Reg const ARM64RegR29 = 29;
ARM64Reg const ARM64RegR30 = 30;
ARM64Reg const ARM64RegSP  = 31;
ARM64Reg const ARM64RegZR  = 32;
ARM64Reg const ARM64RegLR  = 33;
ARM64Reg const ARM64RegFR  = 34;

typedef u32 ARM64FloatType BRIDGE_ENUM_TO_SWIFT;
ARM64FloatType const ARM64FloatTypeSingle = 0b00;
ARM64FloatType const ARM64FloatTypeDouble = 0b01;
ARM64FloatType const ARM64FloatTypeHalf   = 0b11;

typedef u32 ARM64Cond BRIDGE_ENUM_TO_SWIFT;
ARM64Cond const ARM64CondEQ = 0b0000;
ARM64Cond const ARM64CondNE = 0b0001;
ARM64Cond const ARM64CondMI = 0b0100;
ARM64Cond const ARM64CondPL = 0b0101;
ARM64Cond const ARM64CondVS = 0b0110;
ARM64Cond const ARM64CondVC = 0b0111;
ARM64Cond const ARM64CondHI = 0b1000;
ARM64Cond const ARM64CondLS = 0b1001;
ARM64Cond const ARM64CondGE = 0b1010;
ARM64Cond const ARM64CondLT = 0b1011;
ARM64Cond const ARM64CondGT = 0b1100;
ARM64Cond const ARM64CondLE = 0b1101;
ARM64Cond const ARM64CondAL = 0b1110;

bool canPack(u32 val, u32 bits) {
    u32 highestBit = 32 - __builtin_clz(val);
    return highestBit <= bits;
}

EXPORT u32 encodeARM64Reg(ARM64Reg reg, ARM64Reg permittedStackAlias) {
    switch (reg) {
    case ARM64RegFR:
        return 29;

    case ARM64RegLR:
        return 30;

    case ARM64RegSP:
    case ARM64RegZR:
        assert(permittedStackAlias == reg);
        return 31;

    default:
        return reg;
    }
}

// MARK: Data Processing -- Immediate
// MARK: PC-rel. addressing
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬─────┬──────────────┬────────────────────────────────────────────────────────┬──────────────┤
 │op│immlo│ 1  0  0  0  0│                         immhi                          │      Rd      │
 └──┴─────┴──────────────┴────────────────────────────────────────────────────────┴──────────────┘
 */
#define encode(NAME, op) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(imm21:_:)) \
EXPORT u32 encode ## NAME(u32 imm21, ARM64Reg Rd) { \
    assert(canPack(imm21, 21)); \
    u32 imm   = imm << (op ? 12 : 0); \
    u32 immlo = imm & 0b11; \
    u32 immhi = imm >> 2; \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (op << 31) | (immlo << 29) | (0b10000 << 24) | (immhi << 5) | Rd; \
}

encode(ADR,  0);
encode(ADRP, 1);
#undef encode


// MARK: Add/subtract (immediate)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬──────────────┬─────┬───────────────────────────────────┬──────────────┬──────────────┤
 │sf│op│ S│ 1  0  0  0  1│shift│               imm12               │      Rn      │      Rd      │
 └──┴──┴──┴──────────────┴─────┴───────────────────────────────────┴──────────────┴──────────────┘
 */
#define encode(NAME, op, S) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME ## i(use64Bits:shift:imm12:_:_:)) \
EXPORT u32 encode ## NAME ## i(bool use64Bits, u32 shift, u32 imm12, ARM64Reg Rn, ARM64Reg Rd) { \
    assert(canPack(shift, 2)); \
    assert(canPack(imm12, 12)); \
    u32 sf = use64Bits ? 1 : 0; \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (sf << 31) | (op << 30) | (S << 29) | (0b10001 << 24) | (shift << 22) | (imm12 << 10) | (Rn << 5) | Rd; \
}

encode(ADD, 0, 0);
encode(ADDS, 0, 1);
encode(SUB, 1, 0);
encode(SUBS, 1, 1);
#undef encode

// MARK: Logical (immediate)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬─────┬─────────────────┬──┬─────────────────┬─────────────────┬──────────────┬──────────────┤
 │sf│ opc │ 1  0  0  1  0  0│ N│       immr      │       imms      │      Rn      │      Rd      │
 └──┴─────┴─────────────────┴──┴─────────────────┴─────────────────┴──────────────┴──────────────┘
 */
#define encode(NAME, opc) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME ## i(use64Bits:imm:_:_:)) \
EXPORT u32 encode ## NAME ## i(bool use64Bits, u32 imm, ARM64Reg Rn, ARM64Reg Rd) { \
    assert(canPack(imm, use64Bits ? 13 : 12)); \
    u32 sf = use64Bits ? 1 : 0; \
    u32 N = imm & (1 << 12); \
    u32 immr = imm & 0b111111; \
    u32 imms = (imm >> 6) & 0b111111; \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (sf << 31) | (opc << 29) | (0b100100 << 23) | (N << 22) | (immr << 16) | (imms << 10) | (Rn << 5) | Rd; \
}

encode(AND,  0b00);
encode(ORR,  0b01);
encode(EOR,  0b10);
encode(ANDS, 0b11);
#undef encode

// MARK: Move wide (immediate)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬─────┬─────────────────┬─────┬───────────────────────────────────────────────┬──────────────┤
 │sf│ opc │ 1  0  0  1  0  1│  hw │                    imm16                      │      Rd      │
 └──┴─────┴─────────────────┴─────┴───────────────────────────────────────────────┴──────────────┘
 */
 #define encode(NAME, opc) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME ## i(use64Bits:hw:imm16:_:)) \
EXPORT u32 encode ## NAME ## i(bool use64Bits, u32 hw, u32 imm16, ARM64Reg Rd) { \
    assert(canPack(hw, 2)); \
    assert(canPack(imm16, 16)); \
    u32 sf  = use64Bits ? 1 : 0; \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (sf << 31) | (opc << 29) | (0b100101 << 23) | (hw << 21) | (imm16 << 5) | Rd; \
}

encode(MOVN, 0b00);
encode(MOVZ, 0b10);
encode(MOVK, 0b11);
#undef encode

// MARK: Bitfield
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬─────┬─────────────────┬──┬─────────────────┬─────────────────┬──────────────┬──────────────┤
 │sf│ opc │ 1  0  0  1  1  0│ N│       immr      │       imms      │      Rn      │      Rd      │
 └──┴─────┴─────────────────┴──┴─────────────────┴─────────────────┴──────────────┴──────────────┘
 */
#define encode(NAME, opc) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME ## i(use64Bits:imm:_:_:)) \
EXPORT u32 encode ## NAME ## i(bool use64Bits, u32 imm, ARM64Reg Rn, ARM64Reg Rd) { \
    assert(canPack(imm, use64Bits ? 13 : 12)); \
    u32 sf = use64Bits ? 1 : 0; \
    u32 N = sf; \
    u32 immr = imm & 0b111111; \
    u32 imms = (imm >> 6) & 0b111111; \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (sf << 31) | (opc << 29) | (0b100110 << 23) | (N << 22) | (immr << 16) | (imms << 10) | (Rn << 5) | Rd; \
}

encode(SBFM, 0b00);
encode(BFM,  0b01);
encode(UBFM, 0b10);
#undef encode

// MARK: Extract
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬─────┬─────────────────┬──┬──┬──────────────┬─────────────────┬──────────────┬──────────────┤
 │sf│ op21│ 1  0  0  1  1  1│ N│o0│      Rm      │       imms      │      Rn      │      Rd      │
 └──┴─────┴─────────────────┴──┴──┴──────────────┴─────────────────┴──────────────┴──────────────┘
 */
#define encode(NAME, op21, o0) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(use64Bits:imm:_:_:_:)) \
EXPORT u32 encode ## NAME(bool use64Bits, u32 imm, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) { \
    assert(canPack(imm, use64Bits ? 6 : 5)); \
    u32 sf = use64Bits ? 1 : 0; \
    u32 N = sf; \
    Rm = encodeARM64Reg(Rm, ARM64RegSP); \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (sf << 31) | (op21 << 29) | (0b100111 << 23) | (N << 22) | (o0 << 21) | (Rm << 16) | (imm << 10) | (Rn << 5) | Rd; \
}

encode(EXTR, 0b00, 0);
#undef encode


// MARK: Branches, Exception Generating and System instructions

// MARK: Conditional branch (immediate)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├────────────────────┬──┬────────────────────────────────────────────────────────┬──┬───────────┤
 │ 0  1  0  1  0  1  0│ 0│                          imm19                         │ 0│    cond   │
 └────────────────────┴──┴────────────────────────────────────────────────────────┴──┴───────────┘
                       o1                                                          o0
 */
#define encode(NAME, cond) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(imm19:)) \
EXPORT u32 encode ## NAME(u32 imm19) { \
    assert(canPack(imm19, 19)); \
    return (0b0101010 << 25) | ((imm19 * 4) << 5) | cond; \
}

encode(BEQ, 0b0000);
encode(BNE, 0b0001);
encode(BMI, 0b0100);
encode(BPL, 0b0101);
encode(BVS, 0b0110);
encode(BVC, 0b0111);
encode(BHI, 0b1000);
encode(BLS, 0b1001);
encode(BGE, 0b1010);
encode(BLT, 0b1011);
encode(BGT, 0b1100);
encode(BLE, 0b1101);
encode(BAL, 0b1110);
#undef encode

// MARK: Exception generation
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├───────────────────────┬────────┬───────────────────────────────────────────────┬────────┬─────┤
 │ 1  1  0  1  0  1  0  0│   opc  │                       imm16                   │   op2  │  LL │
 └───────────────────────┴────────┴───────────────────────────────────────────────┴────────┴─────┘
 */
#define encode(NAME, opc, op2, LL) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(imm16:)) \
EXPORT u32 encode ## NAME(u32 imm16) { \
    assert(canPack(imm16, 16)); \
    return (0b11010100 << 24) | (opc << 21) | (imm16 << 5) | (op2 << 2) | LL; \
}

encode(SVC,   0b000, 0b000, 0b01);
encode(HVC,   0b000, 0b000, 0b10);
encode(SMC,   0b000, 0b000, 0b11);
encode(BRK,   0b001, 0b000, 0b00);
encode(HLT,   0b010, 0b000, 0b00);
encode(DCPS1, 0b101, 0b000, 0b01);
encode(DCPS2, 0b101, 0b000, 0b10);
encode(DCPS3, 0b101, 0b000, 0b11);
#undef encode


// MARK: System
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────────────────────────────┬──┬─────┬────────┬───────────┬───────────┬────────┬──────────────┤
 │ 1  1  0  1  0  1  0  1  0  0│ L│ op0 │   op1  │    CRn    │    CRm    │   op2  │      Rt      │
 └─────────────────────────────┴──┴─────┴────────┴───────────┴───────────┴────────┴──────────────┘
 */
// NOTE: The following allow *all* parameters to be provided by the caller
#define encode(NAME, op2) \
EXPORT u32 encode ## NAME() { \
    return (0b1101010100 << 22) | (0b011 << 16) | (0b0010 << 12) | (op2 << 5) | 0b11111; \
}

encode(NOP,   0b000);
encode(YIELD, 0b001);
encode(WFE,   0b010);
encode(WFI,   0b011);
encode(SEV,   0b100);
encode(SEVL,  0b101);
#undef encode

BRIDGE_TO_SWIFT_WITH_NAME(encodeCLREX(imm4:))
EXPORT u32 encodeCLREX(u32 imm4) {
    assert(canPack(imm4, 4));
    return (0b1101010100 << 22) | (0b011 << 16) | (0b0011 << 12) | (imm4 << 8) | (0b010 << 5) | 0b11111;
}

#define encode(NAME, opc) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(imm4:)) \
EXPORT u32 encode ## NAME(u32 imm4) { \
    assert(canPack(imm4, 4)); \
    return (0b1101010100 << 22) | (0b011 << 16) | (0b0011 << 12) | (imm4 << 8) | (1 << 7) | (opc << 5) | 0b11111; \
}

encode(DSB, 0b00);
encode(DMB, 0b01);
encode(ISB, 0b10);
#undef encode

// TODO: System register wrappers
#define encode(NAME, L) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(op1:op2:Cn:Cm:_:)) \
EXPORT u32 encode ## NAME(u32 op1, u32 op2, u32 Cn, u32 Cm, ARM64Reg Rt) { \
    assert(canPack(op1, 3)); \
    assert(canPack(op2, 3)); \
    return (0b1101010100 << 22) | (L << 21) | (0b01 << 19) | (op1 << 16) | (Cn << 12) | (Cm << 8) | (op2 << 5) | Rt; \
}

encode(SYS,  0);
encode(SYSL, 1);
#undef encode

/* NOTE: Missing
- HINT - hints 8 to 15, and 24 to 127
- HINT - hints 6 and 7
- HINT - hints 18 to 23
- MSRi
- MSR
- MRS
*/


// MARK: Unconditional branch (register)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├────────────────────┬───────────┬──────────────┬─────────────────┬──────────────┬──────────────┤
 │ 1  1  0  1  0  1  1│    opc    │ 1  1  1  1  1│ 0  0  0  0  0  0│      Rn      │ 0  0  0  0  0│
 └────────────────────┴───────────┴──────────────┴─────────────────┴──────────────┴──────────────┘
                                         op2             op3                             op4
 */
#define encode(NAME, opc) \
EXPORT u32 encode ## NAME(ARM64Reg Rn) { \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    return (0b1101011 << 25) | (opc << 21) | (0b11111 << 16) | (Rn << 5); \
}

encode(BR,   0b0000);
encode(BLR,  0b0001);
encode(RET,  0b0010);
encode(ERET, 0b0100);
encode(DPRS, 0b0101);
#undef encode


// MARK: Unconditional branch (immediate)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──────────────┬─────────────────────────────────────────────────────────────────────────────┤
 │op│ 0  0  1  0  1│                                 imm26                                       │
 └──┴──────────────┴─────────────────────────────────────────────────────────────────────────────┘
 */
#define encode(NAME, op) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(imm26:)) \
EXPORT u32 encode ## NAME(u32 imm26) { \
    assert(canPack(imm26, 26)); \
    return (op << 31) | (0b00101 << 24) | imm26; \
}

encode(B,  0);
encode(BL, 1);
#undef encode


// MARK: Compare and branch (immediate)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬─────────────────┬──┬────────────────────────────────────────────────────────┬──────────────┤
 │sf│ 0  1  1  0  1  0│op│                          imm19                         │      Rt      │
 └──┴─────────────────┴──┴────────────────────────────────────────────────────────┴──────────────┘
 */
#define encode(NAME, op) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(use64Bits:imm19:_:)) \
EXPORT u32 encode ## NAME(bool use64Bits, u32 imm19, ARM64Reg Rt) { \
    assert(canPack(imm19, 19)); \
    u32 sf = use64Bits ? 1 : 0; \
    return (sf << 31) | (0b011010 << 25) | (op << 24) | (imm19 << 5) | Rt; \
}

encode(CBZ,  0);
encode(CBNZ, 1);
#undef encode


// MARK: Test and branch (immediate)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬─────────────────┬──┬──────────────┬─────────────────────────────────────────┬──────────────┤
 │sf│ 0  1  1  0  1  0│op│      b40     │                  imm14                  │      Rt      │
 └──┴─────────────────┴──┴──────────────┴─────────────────────────────────────────┴──────────────┘
 */
#define encode(NAME, op) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(use64Bits:b40:imm14:_:)) \
EXPORT u32 encode ## NAME(bool use64Bits, u32 b40, u32 imm14, ARM64Reg Rt) { \
    assert(canPack(b40, 5)); \
    assert(canPack(imm14, 14)); \
    u32 sf = use64Bits ? 1 : 0; \
    return (sf << 31) | (0b011010 << 25) | (op << 24) | (b40 << 19) | (imm14 << 5) | Rt; \
}

encode(TBZ,  0);
encode(TBNZ, 1);
#undef encode


// MARK: Load and Stores

// MARK: Load/store exclusive
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────┬─────────────────┬──┬──┬──┬──────────────┬──┬──────────────┬──────────────┬──────────────┤
 │ size│ 0  0  1  0  0  0│o2│ L│o1│      Rs      │o0│      Rt2     │      Rn      │      Rt      │
 └─────┴─────────────────┴──┴──┴──┴──────────────┴──┴──────────────┴──────────────┴──────────────┘
 */
#define encode(NAME, size, o2, L, o1, o0) \
EXPORT u32 encode ## NAME(ARM64Reg Rs, ARM64Reg Rt2, ARM64Reg Rn, ARM64Reg Rt) { \
    Rs  = encodeARM64Reg(Rs,  ARM64RegSP); \
    Rt2 = encodeARM64Reg(Rt2, ARM64RegSP); \
    Rn  = encodeARM64Reg(Rn,  ARM64RegZR); \
    Rt  = encodeARM64Reg(Rt,  ARM64RegSP); \
    return (size << 30) | (0b001000 << 24) | (o2 << 23) | (L << 22) | (o1 << 21) | (Rs << 16) | (o0 << 15) | (Rt2 << 10) | (Rn << 5) | Rt; \
}

encode(STXRB,   0b00, 0, 0, 0, 0);
encode(STLXRB,  0b00, 0, 0, 0, 1);
encode(LDXRB,   0b00, 0, 1, 0, 0);
encode(LDAXRB,  0b00, 0, 1, 0, 1);
encode(STLRB,   0b00, 1, 0, 0, 1);
encode(LDARB,   0b00, 1, 1, 0, 1);
encode(STXRH,   0b01, 0, 0, 0, 0);
encode(STLXRH,  0b01, 0, 0, 0, 1);
encode(LDXRH,   0b01, 0, 1, 0, 0);
encode(LDAXRH,  0b01, 0, 1, 0, 1);
encode(STLRH,   0b01, 1, 0, 0, 1);
encode(LDARH,   0b01, 1, 1, 0, 1);
encode(STXR32,  0b10, 0, 0, 0, 0);
encode(STLXR32, 0b10, 0, 0, 0, 1);
encode(STXP32,  0b10, 0, 0, 1, 0);
encode(STLXP32, 0b10, 0, 0, 1, 1);
encode(LDXR32,  0b10, 0, 1, 0, 0);
encode(LDAXR32, 0b10, 0, 1, 0, 1);
encode(LDXP32,  0b10, 0, 1, 1, 0);
encode(LDAXP32, 0b10, 0, 1, 1, 1);
encode(STLR32,  0b10, 1, 0, 0, 1);
encode(LDAR32,  0b10, 1, 1, 0, 1);
encode(STXR64,  0b11, 0, 0, 0, 0);
encode(STLXR64, 0b11, 0, 0, 0, 1);
encode(STXP64,  0b11, 0, 0, 1, 0);
encode(STLXP64, 0b11, 0, 0, 1, 1);
encode(LDXR64,  0b11, 0, 1, 0, 0);
encode(LDAXR64, 0b11, 0, 1, 0, 1);
encode(LDXP64,  0b11, 0, 1, 1, 0);
encode(LDAXP64, 0b11, 0, 1, 1, 1);
encode(STLR64,  0b11, 1, 0, 0, 1);
encode(LDAR64,  0b11, 1, 1, 0, 1);
#undef encode


// MARK: Load register (literal)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────┬────────┬──┬─────┬────────────────────────────────────────────────────────┬──────────────┤
 │ opc │ 0  1  1│ V│ 0  0│                           imm19                        │      Rt      │
 └─────┴────────┴──┴─────┴────────────────────────────────────────────────────────┴──────────────┘
 */
#define encode(NAME, opc, V) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME ## pcrel(imm19:_:)) \
EXPORT u32 encode ## NAME ## pcrel(u32 imm19, ARM64Reg Rt) { \
    assert(canPack(imm19 / 4, 19)); \
    Rt = encodeARM64Reg(Rt, ARM64RegSP); \
    return (opc << 30) | (0b011 << 27) | (V << 26) | ((imm19 * 4) << 5) | Rt; \
}

encode(LDR32,   0b00, 0);
encode(LDR32F,  0b00, 1);
encode(LDR64,   0b01, 0);
encode(LDR64F,  0b01, 1);
encode(LDRSW,   0b10, 0);
encode(LDR128F, 0b10, 1);
encode(PRFM,    0b11, 0);
#undef encode


// MARK: Load/store no-allocate pair (offset)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────┬────────┬──┬────────┬──┬────────────────────┬──────────────┬──────────────┬──────────────┤
 │ opc │ 1  0  1│ V│ 0  0  0│ L│        imm7        │      Rt2     │      Rn      │      Rt      │
 └─────┴────────┴──┴────────┴──┴────────────────────┴──────────────┴──────────────┴──────────────┘
 */
#define encode(NAME, opc, V, L) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME ## pairNoAllocate(imm7:_:_:_:)) \
EXPORT u32 encode ## NAME ## pairNoAllocate(u32 imm7, ARM64Reg Rt2, ARM64Reg Rn, ARM64Reg Rt) { \
    assert(canPack(imm7, 7)); \
    Rt2 = encodeARM64Reg(Rt2, ARM64RegSP); \
    Rn  = encodeARM64Reg(Rn,  ARM64RegSP); \
    Rt  = encodeARM64Reg(Rt,  ARM64RegSP); \
    return (opc << 30) | (0b101 << 27) | (V << 26) | (L << 22) | (imm7 << 15) | (Rt2 << 10) | (Rn << 5) | Rt; \
}

encode(STNP32,   0b00, 0, 0);
encode(LDNP32,   0b00, 0, 1);
encode(STNP32F,  0b00, 1, 0);
encode(LDNP32F,  0b00, 1, 1);
encode(STNP64F,  0b01, 1, 0);
encode(LDNP64F,  0b01, 1, 1);
encode(STNP64,   0b10, 0, 0);
encode(LDNP64,   0b10, 0, 1);
encode(STNP128F, 0b10, 1, 0);
encode(LDNP128F, 0b10, 1, 1);
#undef encode


// MARK: Load/store register pair (post-indexed)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────┬────────┬──┬────────┬──┬────────────────────┬──────────────┬──────────────┬──────────────┤
 │ opc │ 1  0  1│ V│ 0  0  1│ L│        imm7        │      Rt2     │      Rn      │      Rt      │
 └─────┴────────┴──┴────────┴──┴────────────────────┴──────────────┴──────────────┴──────────────┘
 */
// TODO


// MARK: Load/store register pair (offset)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────┬────────┬──┬────────┬──┬────────────────────┬──────────────┬──────────────┬──────────────┤
 │ opc │ 1  0  1│ V│ 0  1  0│ L│        imm7        │      Rt2     │      Rn      │      Rt      │
 └─────┴────────┴──┴────────┴──┴────────────────────┴──────────────┴──────────────┴──────────────┘
 */
#define encode(NAME, opc, V, L) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME ## pairOffset(imm7:_:_:_:)) \
EXPORT u32 encode ## NAME ## pairOffset(u32 imm7, ARM64Reg Rt2, ARM64Reg Rn, ARM64Reg Rt) { \
    assert(canPack(imm7, 7)); \
    Rt2 = encodeARM64Reg(Rt2, ARM64RegSP); \
    Rn  = encodeARM64Reg(Rn,  ARM64RegSP); \
    Rt  = encodeARM64Reg(Rt,  ARM64RegSP); \
    return (opc << 30) | (0b101 << 27) | (V << 26) | (0b010 << 23) | (L << 22) | (imm7 << 15) | (Rt2 << 10) | (Rn << 5) | Rt; \
}

encode(STP32, 0b00, 0, 0);
encode(LDP32, 0b00, 0, 1);
encode(STP32F, 0b00, 1, 0);
encode(LDP32F, 0b00, 1, 1);
encode(LDPSW, 0b01, 0, 1);
encode(STP64F, 0b01, 1, 0);
encode(LDP64F, 0b01, 1, 1);
encode(STP64, 0b10, 0, 0);
encode(LDP64, 0b10, 0, 1);
encode(STP128F, 0b10, 1, 0);
encode(LDP128F, 0b10, 1, 1);
#undef encode


// MARK: Load/store register pair (pre-indexed)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────┬────────┬──┬────────┬──┬────────────────────┬──────────────┬──────────────┬──────────────┤
 │ opc │ 1  0  1│ V│ 0  1  1│ L│        imm7        │      Rt2     │      Rn      │      Rt      │
 └─────┴────────┴──┴────────┴──┴────────────────────┴──────────────┴──────────────┴──────────────┘
 */


// MARK: Load/store register (unscaled immediate)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────┬────────┬──┬─────┬─────┬──┬──────────────────────────┬─────┬──────────────┬──────────────┤
 │ size│ 1  1  1│ V│ 0  0│ opc │ 0│           imm9           │ 0  0│      Rn      │      Rt      │
 └─────┴────────┴──┴─────┴─────┴──┴──────────────────────────┴─────┴──────────────┴──────────────┘
 */
#define encode(NAME, size, V, opc) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME ## unscaledImmediate(imm9:_:_:)) \
EXPORT u32 encode ## NAME ## unscaledImmediate(u32 imm9, ARM64Reg Rn, ARM64Reg Rt) { \
    assert(canPack(imm9, 9)); \
    Rn = encodeARM64Reg(Rn, ARM64RegZR); \
    Rt = encodeARM64Reg(Rt, ARM64RegSP); \
    return (size << 30) | (0b111 << 27) | (V << 26) | (opc << 22) | (imm9 << 12) | (Rn << 5) | Rt; \
}

encode(STUR8F,   0b00, 1, 0b00); // (SIMD&FP) — 8-bit
encode(LDUR8F,   0b00, 1, 0b01); // (SIMD&FP) — 8-bit
encode(STUR128F, 0b00, 1, 0b10); // (SIMD&FP) — 128-bit
encode(LDUR128F, 0b00, 1, 0b11); // (SIMD&FP) — 128-bit
encode(STUR16F,  0b01, 1, 0b00); // (SIMD&FP) — 16-bit
encode(LDUR16F,  0b01, 1, 0b01); // (SIMD&FP) — 16-bit
encode(STUR32F,  0b10, 1, 0b00); // (SIMD&FP) — 32-bit
encode(LDUR32F,  0b10, 1, 0b01); // (SIMD&FP) — 32-bit
encode(STUR64F,  0b11, 1, 0b00); // (SIMD&FP) — 64-bit
encode(LDUR64F,  0b11, 1, 0b01); // (SIMD&FP) — 64-bit
encode(PRFM,     0b11, 0, 0b10); // (unscaled offset)
encode(STURB,    0b00, 0, 00);
encode(LDURB,    0b00, 0, 01);
encode(STURH,    0b01, 0, 00);
encode(LDURH,    0b01, 0, 01);
encode(LDURSB,   0b00, 0, 11);
encode(LDURSH,   0b01, 0, 11);
encode(LDURSW,   0b10, 0, 10);
encode(STUR32,   0b10, 0, 00);
encode(LDUR32,   0b10, 0, 01);
encode(STUR64,   0b10, 0, 00);
encode(LDUR64,   0b10, 0, 01);
#undef encode


// MARK: Load/store register (immediate post-indexed)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────┬────────┬──┬─────┬─────┬──┬──────────────────────────┬─────┬──────────────┬──────────────┤
 │ size│ 1  1  1│ V│ 0  0│ opc │ 0│           imm9           │ 0  1│      Rn      │      Rt      │
 └─────┴────────┴──┴─────┴─────┴──┴──────────────────────────┴─────┴──────────────┴──────────────┘
 */


// MARK: Load/store register (unprivileged)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────┬────────┬──┬─────┬─────┬──┬──────────────────────────┬─────┬──────────────┬──────────────┤
 │ size│ 1  1  1│ V│ 0  0│ opc │ 0│           imm9           │ 1  0│      Rn      │      Rt      │
 └─────┴────────┴──┴─────┴─────┴──┴──────────────────────────┴─────┴──────────────┴──────────────┘
 */


// MARK: Load/store register (immediate pre-indexed)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────┬────────┬──┬─────┬─────┬──┬──────────────────────────┬─────┬──────────────┬──────────────┤
 │ size│ 1  1  1│ V│ 0  0│ opc │ 0│           imm9           │ 1  1│      Rn      │      Rt      │
 └─────┴────────┴──┴─────┴─────┴──┴──────────────────────────┴─────┴──────────────┴──────────────┘
 */


// MARK: Atomic memory operations
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────┬────────┬──┬─────┬──┬──┬──┬──────────────┬──┬────────┬─────┬──────────────┬──────────────┤
 │ size│ 1  1  1│ V│ 0  0│ A│ R│ 1│      Rs      │o3│   opc  │ 0  0│      Rn      │      Rt      │
 └─────┴────────┴──┴─────┴──┴──┴──┴──────────────┴──┴────────┴─────┴──────────────┴──────────────┘
 */
// NOTE: ARMv8.1 extension


// MARK: Load/store register (register offset)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────┬────────┬──┬─────┬─────┬──┬──────────────┬────────┬──┬─────┬──────────────┬──────────────┤
 │ size│ 1  1  1│ V│ 0  0│ opc │ 1│      Rm      │ option │ S│ 1  0│      Rn      │      Rt      │
 └─────┴────────┴──┴─────┴─────┴──┴──────────────┴────────┴──┴─────┴──────────────┴──────────────┘
 */
#define encode(NAME, size, V, opc) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME ## registerOffset(option:S:_:_:_:)) \
EXPORT u32 encode ## NAME ## registerOffset(u32 option, u32 S, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rt) { \
    assert(canPack(option, 3)); \
    assert(canPack(S, 1)); \
    Rm = encodeARM64Reg(Rm, ARM64RegZR); \
    Rn = encodeARM64Reg(Rn, ARM64RegZR); \
    Rt = encodeARM64Reg(Rt, ARM64RegSP); \
    return (size << 30) | (0b111 << 27) | (V << 26) | (opc << 22) | (1 << 21) | (Rm << 16) | (option << 13) | (S << 12) | (0b10 << 10) | (Rn << 5) | Rt; \
}

encode(STRB,    0b00, 0, 0b00);  // (register) — shifted register
encode(LDRB,    0b00, 0, 0b01);  // (register) — shifted register
encode(LDRSB64, 0b00, 0, 0b10);  // (register) — 64-bit with shifted register offset
encode(LDRSB32, 0b00, 0, 0b11);  // (register) — 32-bit with shifted register offset
encode(STR8F,   0b00, 1, 0b00);  // (register, SIMD&FP)
encode(LDR8F,   0b00, 1, 0b01);  // (register, SIMD&FP)
encode(STR128F, 0b00, 1, 0b10);  // (register, SIMD&FP)
encode(LDR128F, 0b00, 1, 0b11);  // (register, SIMD&FP)
encode(STRH,    0b01, 0, 0b00);  // (register)
encode(LDRH,    0b01, 0, 0b01);  // (register)
encode(LDRSH64, 0b01, 0, 0b10);  // (register) — 64-bit
encode(LDRSH32, 0b01, 0, 0b11);  // (register) — 32-bit
encode(STR16F,  0b01, 1, 0b00);  // (register, SIMD&FP)
encode(LDR16F,  0b01, 1, 0b01);  // (register, SIMD&FP)
encode(STR32,   0b10, 0, 0b00);  // (register) — 32-bit
encode(LDR32,   0b10, 0, 0b01);  // (register) — 32-bit
encode(LDRSW,   0b10, 0, 0b10);  // (register)
encode(STR32F,  0b10, 1, 0b00);  // (register, SIMD&FP)
encode(LDR32F,  0b10, 1, 0b01);  // (register, SIMD&FP)
encode(STR64,   0b11, 0, 0b00);  // (register) — 64-bit
encode(LDR64,   0b11, 0, 0b01);  // (register) — 64-bit
encode(PRFM,    0b11, 0, 0b10);  // (register)
encode(STR64F,  0b11, 1, 0b00);  // (register, SIMD&FP)
encode(LDR64F,  0b11, 1, 0b01);  // (register, SIMD&FP)
#undef encode


// MARK: Load/store register (pac)
// NOTE: ARMv8.3 extension


// MARK: Load/store register (unsigned immediate)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────┬────────┬──┬─────┬─────┬───────────────────────────────────┬──────────────┬──────────────┤
 │ size│ 1  1  1│ V│ 0  1│ opc │               imm12               │      Rn      │      Rt      │
 └─────┴────────┴──┴─────┴─────┴───────────────────────────────────┴──────────────┴──────────────┘
 */
#define encode(NAME, size, V, opc) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(imm12:_:_:)) \
EXPORT u32 encode ## NAME(u32 imm12, ARM64Reg Rn, ARM64Reg Rt) { \
    assert(canPack(imm12, 12)); \
    Rn = encodeARM64Reg(Rn, ARM64RegZR); \
    Rt = encodeARM64Reg(Rt, ARM64RegSP); \
    return (size << 30) | (0b111 << 27) | (V << 26) | (0b01 << 24) | (opc << 22) | (imm12 << 10) | (Rn << 5) | Rt; \
}

encode(STRB,    0b00, 0, 0b00);
encode(LDRB,    0b00, 0, 0b01);
encode(LDRSB64, 0b00, 0, 0b10);
encode(LDRSB32, 0b00, 0, 0b11);
encode(STR8F,   0b00, 1, 0b00);
encode(LDR8F,   0b00, 1, 0b01);
encode(STR128F, 0b00, 1, 0b10);
encode(LDR128F, 0b00, 1, 0b11);
encode(STRH,    0b01, 0, 0b00);
encode(LDRH,    0b01, 0, 0b01);
encode(LDRSH64, 0b01, 0, 0b10);
encode(LDRSH32, 0b01, 0, 0b11);
encode(STR16F,  0b01, 1, 0b00);
encode(LDR16F,  0b01, 1, 0b01);
encode(STR32,   0b10, 0, 0b00);
encode(LDR32,   0b10, 0, 0b01);
encode(LDRSW,   0b10, 0, 0b10);
encode(STR32F,  0b10, 1, 0b00);
encode(LDR32F,  0b10, 1, 0b01);
encode(STR64,   0b11, 0, 0b00);
encode(LDR64,   0b11, 0, 0b01);
encode(PRFM,    0b11, 0, 0b10);
encode(STR64F,  0b11, 1, 0b00);
encode(LDR64F,  0b11, 1, 0b01);
#undef encode

// MARK: Data Processing -- Register

// MARK: Data-processing (2 source)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬───────────────────────┬──────────────┬─────────────────┬──────────────┬──────────────┤
 │sf│ 0│ 0│ 1  1  0  1  0  1  1  0│      Rm      │      opcode     │      Rn      │      Rd      │
 └──┴──┴──┴───────────────────────┴──────────────┴─────────────────┴──────────────┴──────────────┘
         S
 */
#define encode(NAME, opcode) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(use64Bits:_:_:_:)) \
EXPORT u32 encode ## NAME(bool use64Bits, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) { \
    u32 sf = use64Bits ? 1 : 0; \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rm = encodeARM64Reg(Rm, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (sf << 31) | (0b11010110 << 21) | (Rm << 16) | (opcode << 10) | (Rn << 5) | Rd; \
}

encode(UDIV, 0b00010);
encode(SDIV, 0b00011);
encode(LSLV, 0b01000);
encode(LSRV, 0b01001);
encode(ASRV, 0b01010);
encode(RORV, 0b01011);
#undef encode


// MARK: Data-processing (1 source)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬───────────────────────┬──────────────┬─────────────────┬──────────────┬──────────────┤
 │sf│ 1│ 0│ 1  1  0  1  0  1  1  0│    opcode2   │      opcode     │      Rn      │      Rd      │
 └──┴──┴──┴───────────────────────┴──────────────┴─────────────────┴──────────────┴──────────────┘
         S
 */
#define encode(NAME, opcode2, opcode) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(use64Bits:_:_:)) \
EXPORT u32 encode ## NAME(bool use64Bits, ARM64Reg Rn, ARM64Reg Rd) { \
    u32 sf = use64Bits ? 1 : 0; \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (sf << 31) | (1 << 30) | (0b11010110 << 21) | (opcode2 << 16) | (opcode << 10) | (Rn << 5) | Rd; \
}

encode(RBIT,  0b00000, 0b00000);
encode(REV16, 0b00000, 0b00001);
encode(REV,   0b00000, 0b00010);
encode(CLZ,   0b00000, 0b00100);
encode(CLS,   0b00000, 0b00101);
#undef encode


// MARK: Logical (shifted register)

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬─────┬──────────────┬─────┬──┬──────────────┬─────────────────┬──────────────┬──────────────┤
 │sf│ opc │ 1  1  0  1  0│shift│ N│      Rm      │       imm6      │      Rn      │      Rd      │
 └──┴─────┴──────────────┴─────┴──┴──────────────┴─────────────────┴──────────────┴──────────────┘
 */
#define encode(NAME, opc, N) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(use64Bits:shift:imm5:_:_:_:)) \
EXPORT u32 encode ## NAME(bool use64Bits, u32 shift, u32 imm5, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) { \
    assert(canPack(imm5, 5)); /* NOTE: 1xxxxx is UNALLOCATED and therefore the imm6 in the above diagram is really an imm5 */ \
    u32 sf = use64Bits ? 1 : 0; \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rm = encodeARM64Reg(Rm, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (sf << 31) | (opc << 29) | (0b11010 << 24) | (shift << 22) | (N << 21) | (Rm << 16) | (imm5 << 10) | (Rn << 5) | Rd; \
}

encode(AND,  0b00, 0);
encode(BIC,  0b00, 1);
encode(ORR,  0b01, 0);
encode(ORN,  0b01, 1);
encode(EOR,  0b10, 0);
encode(EON,  0b10, 1);
encode(ANDS, 0b11, 0);
encode(BICS, 0b11, 1);
#undef encode


// MARK: Add/subtract (shifted register)

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬──────────────┬─────┬──┬──────────────┬─────────────────┬──────────────┬──────────────┤
 │sf│ o│ S│ 0  1  0  1  1│shift│ 0│      Rm      │       imm6      │      Rn      │      Rd      │
 └──┴──┴──┴──────────────┴─────┴──┴──────────────┴─────────────────┴──────────────┴──────────────┘
 */
#define encode(NAME, op, S) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(use64Bits:shift:imm5:_:_:_:)) \
EXPORT u32 encode ## NAME(bool use64Bits, u32 shift, u32 imm5, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) { \
    assert(canPack(imm5, 5)); /* NOTE: 1xxxxx is UNALLOCATED and therefore the imm6 in the above diagram is really an imm5 */ \
    u32 sf = use64Bits ? 1 : 0; \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rm = encodeARM64Reg(Rm, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (sf << 31) | (op << 30) | (S << 29) | (0b11010 << 24) | (shift << 22) | (Rm << 16) | (imm5 << 10) | (Rn << 5) | Rd; \
}

encode(ADD,  0, 0);
encode(ADDS, 0, 1);
encode(SUB,  1, 0);
encode(SUBS, 1, 1);
#undef encode

// MARK: Add/subtract (extended register)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬──────────────┬─────┬──┬──────────────┬────────┬────────┬──────────────┬──────────────┤
 │sf│op│ S│ 0  1  0  1  1│ opt │ 1│      Rm      │ option │  imm3  │      Rn      │      Rd      │
 └──┴──┴──┴──────────────┴─────┴──┴──────────────┴────────┴────────┴──────────────┴──────────────┘
 */
#define encode(NAME, op, S) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME ## x(use64Bits:option:imm3:_:_:_:)) \
EXPORT u32 encode## NAME ## x(bool use64Bits, u32 option, u32 imm3, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) { \
    assert(canPack(imm3, 3)); \
    assert(canPack(option, 3)); \
    u32 sf = use64Bits ? 1 : 0; \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rm = encodeARM64Reg(Rm, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (sf << 31) | (op << 30) | (S << 29) | (0b11010 << 24) | (Rm << 16) | (option << 13) | (imm3 << 10) | (Rn << 5) | Rd; \
}

encode(ADD,  0, 0);
encode(ADDS, 0, 1);
encode(SUB,  1, 0);
encode(SUBS, 1, 1);
#undef encode

// MARK: Add/subtract (with carry)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬───────────────────────┬──────────────┬─────────────────┬──────────────┬──────────────┤
 │sf│op│ S│ 1  1  0  1  0  0  0  0│      Rm      │ 0  0  0  0  0  0│      Rn      │      Rd      │
 └──┴──┴──┴───────────────────────┴──────────────┴─────────────────┴──────────────┴──────────────┘
                                                       opcode2
 */
#define encode(NAME, op, S) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(use64Bits:option:imm3:_:_:_:)) \
EXPORT u32 encode ## NAME(bool use64Bits, u32 option, u32 imm3, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) { \
    assert(canPack(imm3, 3)); \
    assert(canPack(option, 3)); \
    u32 sf = use64Bits ? 1 : 0; \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rm = encodeARM64Reg(Rm, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (sf << 31) | (op << 30) | (S << 29) | (0b11010000 << 21) | (Rm << 16) | (imm3 << 10) | (Rn << 5) | Rd; \
}

encode(ADC,  0, 0);
encode(ADCS, 0, 1);
encode(SBC,  1, 0);
encode(SBCS, 1, 1);
#undef encode

// MARK: Conditional compare (register)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬───────────────────────┬──────────────┬───────────┬──┬──┬──────────────┬──┬───────────┤
 │sf│op│ 1│ 1  1  0  1  0  0  1  0│      Rm      │    cond   │ 0│ 0│      Rn      │ 0│    nzcv   │
 └──┴──┴──┴───────────────────────┴──────────────┴───────────┴──┴──┴──────────────┴──┴───────────┘
         S                                                       o2                o3
 */
#define encode(NAME, op, S) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(use64Bits:cond:nzcv:_:_:)) \
EXPORT u32 encode ## NAME(bool use64Bits, u32 cond, u32 nzcv, ARM64Reg Rm, ARM64Reg Rn) { \
    assert(canPack(cond, 3)); \
    assert(canPack(nzcv, 4)); \
    u32 sf = use64Bits ? 1 : 0; \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rm = encodeARM64Reg(Rm, ARM64RegSP); \
    return (sf << 31) | (op << 30) | (1 << 29) | (0b11010010 << 21) | (Rm << 16) | (cond << 12) | (Rn << 5) | nzcv; \
}

encode(CCMN, 0, 1);
encode(CCMP, 1, 1);
#undef encode

// MARK: Conditional compare (immediate)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬───────────────────────┬──────────────┬───────────┬──┬──┬──────────────┬──┬───────────┤
 │sf│op│ 1│ 1  1  0  1  0  0  1  0│     imm5     │    cond   │ 1│ 0│      Rn      │ 0│    nzcv   │
 └──┴──┴──┴───────────────────────┴──────────────┴───────────┴──┴──┴──────────────┴──┴───────────┘
         S                                                       o2                o3
 */
#define encode(NAME, op, S) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME ## i(use64Bits:cond:nzcv:imm5:_:)) \
EXPORT u32 encode ## NAME ## i(bool use64Bits, u32 cond, u32 nzcv, u32 imm5, ARM64Reg Rn) { \
    assert(canPack(imm5, 5)); \
    assert(canPack(cond, 3)); \
    assert(canPack(nzcv, 4)); \
    u32 sf = use64Bits ? 1 : 0; \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    return (sf << 31) | (op << 30) | (S << 29) | (0b11010010 << 21) | (imm5 << 16) | (cond << 12) | (1 << 11) | (Rn << 5) | nzcv; \
}

encode(CCMN, 0, 1);
encode(CCMP, 1, 1);
#undef encode

// MARK: Conditional select
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬───────────────────────┬──────────────┬───────────┬─────┬──────────────┬──────────────┤
 │sf│op│ S│ 1  1  0  1  0  1  0  0│      Rm      │    cond   │ op2 │      Rn      │      Rd      │
 └──┴──┴──┴───────────────────────┴──────────────┴───────────┴─────┴──────────────┴──────────────┘
 */
#define encode(NAME, op, S, op2) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(use64Bits:cond:_:_:_:)) \
EXPORT u32 encode ## NAME(bool use64Bits, u32 cond, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) { \
    assert(canPack(cond, 3)); \
    u32 sf = use64Bits ? 1 : 0; \
    Rm = encodeARM64Reg(Rm, ARM64RegSP); \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (sf << 31) | (op << 30) | (S << 29) | (0b11010010 << 21) | (Rm << 16) | (cond << 12) | (op2 << 10) | (Rn << 5) | Rd; \
}

encode(CSEL,  0, 0, 0b00);
encode(CSINC, 0, 0, 0b01);
encode(CSINV, 1, 0, 0b00);
encode(CSNEG, 1, 0, 0b01);
#undef encode

// MARK: Data-processing (3 source)
/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬─────┬──────────────┬────────┬──────────────┬──┬──────────────┬──────────────┬──────────────┤
 │sf│ op54│ 1  1  0  1  1│  op31  │      Rm      │o0│      Ra      │      Rn      │      Rd      │
 └──┴─────┴──────────────┴────────┴──────────────┴──┴──────────────┴──────────────┴──────────────┘
 */
#define encode(NAME, op54, op31, o0) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(use64Bits:_:_:_:_:)) \
EXPORT u32 encod ## NAME(bool use64Bits, ARM64Reg Rm, ARM64Reg Ra, ARM64Reg Rn, ARM64Reg Rd) { \
    u32 sf = use64Bits ? 1 : 0; \
    Rm = encodeARM64Reg(Rm, ARM64RegSP); \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (sf << 31) | (op54 << 29) | (0b11011 << 24) | (op31 << 21) | (Rm << 16) | (o0 << 15) | (Ra << 10) | (Rn << 5) | Rd; \
}

encode(MADD,   0b00, 0b000, 0);
encode(MSUB,   0b00, 0b000, 1);
encode(SMADDL, 0b00, 0b001, 0);
encode(SMSUBL, 0b00, 0b001, 1);
encode(SMULH,  0b00, 0b010, 0);
encode(UMADDL, 0b00, 0b101, 0);
encode(UMSUBL, 0b00, 0b101, 1);
encode(UMULH,  0b00, 0b110, 0);
#undef encode


// MARK: Data Processing -- Scalar Floating-Point and Advanced SIMD
// MARK: Conversion between floating-point and integer

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬──────────────┬─────┬──┬─────┬────────┬─────────────────┬──────────────┬──────────────┤
 │sf│ 0│ 0│ 1  1  1  1  0│ type│ 1│rmode│ opcode │ 0  0  0  0  0  0│      Rn      │      Rd      │
 └──┴──┴──┴──────────────┴─────┴──┴─────┴────────┴─────────────────┴──────────────┴──────────────┘
         S
 */
#define encode(NAME, rmode, opcode) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(type:_:_:)) \
EXPORT u32 encode ## NAME(ARM64FloatType type, ARM64Reg Rn, ARM64Reg Rd) { \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (0b11110 << 24) | (type << 22) | (1 << 21) | (rmode << 19) | (opcode << 16) | (Rn << 5) | Rd; \
}

encode(FCVTNS,     0b00, 0b000);
encode(FCVTNU,     0b00, 0b001);
encode(SCVTF,      0b00, 0b010);
encode(UCVTF,      0b00, 0b011);
encode(FCVTAS,     0b00, 0b100);
encode(FCVTAU,     0b00, 0b101);
encode(FMOVToGP,   0b00, 0b110);
encode(FMOVFromGP, 0b00, 0b111);
encode(FCVTPS,     0b01, 0b000);
encode(FCVTPU,     0b01, 0b001);
encode(FCVTMS,     0b10, 0b000);
encode(FCVTMU,     0b10, 0b001);
encode(FCVTZS,     0b11, 0b000);
encode(FCVTZU,     0b11, 0b001);
#undef encode


// MARK: Floating-point compare

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬──────────────┬─────┬──┬──────────────┬─────┬───────────┬──────────────┬──────────────┤
 │ 0│ 0│ 0│ 1  1  1  1  0│ type│ 1│      Rm      │  op │ 1  0  0  0│      Rn      │    opcode2   │
 └──┴──┴──┴──────────────┴─────┴──┴──────────────┴─────┴───────────┴──────────────┴──────────────┘
   M     S
 */
#define encode(NAME, op, opcode2) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(type:_:_:)) \
EXPORT u32 encode ## NAME(ARM64FloatType type, ARM64Reg Rm, ARM64Reg Rn) { \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rm = encodeARM64Reg(Rm, ARM64RegSP); \
    return (0b11110 << 24) | (type << 22) | (1 << 21) | (Rm << 16) | (op << 14) | (0b1000 << 10) | (Rn << 5) | opcode2; \
}

encode(FCMP,   0b00, 0b00000);
encode(FCMPZ,  0b00, 0b01000);
encode(FCMPE,  0b00, 0b10000);
encode(FCMPZE, 0b00, 0b11000);
#undef encode


// MARK: Floating-point immediate

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬──────────────┬─────┬──┬───────────────────────┬────────┬──────────────┬──────────────┤
 │ 0│ 0│ 0│ 1  1  1  1  0│ type│ 1│          imm8         │ 1  0  0│ 0  0  0  0  0│      Rd      │
 └──┴──┴──┴──────────────┴─────┴──┴───────────────────────┴────────┴──────────────┴──────────────┘
   M     S
 */
BRIDGE_TO_SWIFT_WITH_NAME(encodeFMOVi(type:imm8:_:))
EXPORT u32 encodeFMOVi(ARM64FloatType type, u32 imm8, ARM64Reg Rd) {
    assert(canPack(imm8, 8));
    Rd = encodeARM64Reg(Rd, ARM64RegSP);
    return (0b11110 << 24) | (type << 22) | (1 << 21) | (imm8 << 13) | (0b100 << 10) | Rd;
}


// MARK: Floating-point data-processing (1 source)

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬──────────────┬─────┬──┬─────────────────┬──────────────┬──────────────┬──────────────┤
 │ 0│ 0│ 0│ 1  1  1  1  0│ type│ 1│      opcode     │ 1  0  0  0  0│      Rn      │      Rd      │
 └──┴──┴──┴──────────────┴─────┴──┴─────────────────┴──────────────┴──────────────┴──────────────┘
   M     S
 */
#define encode(NAME, opcode) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(type:_:_:)) \
EXPORT u32 encode ## NAME(ARM64FloatType type, ARM64Reg Rn, ARM64Reg Rd) { \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (0b11110 << 24) | (type << 22) | (1 << 21) | (opcode << 15) | (0b10000 << 10) | (Rn << 5) | Rd; \
}

encode(FMOV,   0b000000);
encode(FABS,   0b000001);
encode(FNEG,   0b000010);
encode(FSQRT,  0b000011);
encode(FCVT2S, 0b000100);
encode(FCVT2D, 0b000101);
encode(FCVT2H, 0b000111);
encode(FRINTN, 0b001000);
encode(FRINTP, 0b001001);
encode(FRINTM, 0b001010);
encode(FRINTZ, 0b001011);
encode(FRINTA, 0b001100);
encode(FRINTX, 0b001110);
encode(FRINTI, 0b001111);
#undef encode


// MARK: Floating-point data-processing (2 source)

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬──────────────┬─────┬──┬──────────────┬───────────┬─────┬──────────────┬──────────────┤
 │ 0│ 0│ 0│ 1  1  1  1  0│ type│ 1│      Rm      │   opcode  │ 1  0│      Rn      │      Rd      │
 └──┴──┴──┴──────────────┴─────┴──┴──────────────┴───────────┴─────┴──────────────┴──────────────┘
   M     S
 */
#define encode(NAME, opcode) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(type:_:_:_:)) \
EXPORT u32 encode ## NAME(ARM64FloatType type, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) { \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rm = encodeARM64Reg(Rm, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (0b11110 << 24) | (type << 22) | (1 << 21) | (Rm << 16) | (opcode << 12) | (0b10 << 10) | (Rn << 5) | Rd; \
}

encode(FMUL,   0b0001);
encode(FADD,   0b0010);
encode(FSUB,   0b0011);
encode(FMAX,   0b0100);
encode(FMIN,   0b0101);
encode(FMAXNM, 0b0110);
encode(FMINNM, 0b0111);
encode(FNMUL,  0b1000);
#undef encode


// MARK: Floating-point conditional select

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬──────────────┬─────┬──┬──────────────┬───────────┬─────┬──────────────┬──────────────┤
 │ 0│ 0│ 0│ 1  1  1  1  0│ type│ 1│      Rm      │   cond    │ 1  1│      Rn      │      Rd      │
 └──┴──┴──┴──────────────┴─────┴──┴──────────────┴───────────┴─────┴──────────────┴──────────────┘
   M     S
 */
BRIDGE_TO_SWIFT_WITH_NAME(encodeFCSEL(type:cond:_:_:_:))
EXPORT u32 encodeFCSEL(ARM64FloatType type, u32 cond, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) {
    assert(canPack(cond, 4));
    Rn = encodeARM64Reg(Rn, ARM64RegSP);
    Rm = encodeARM64Reg(Rm, ARM64RegSP);
    Rd = encodeARM64Reg(Rd, ARM64RegSP);
    return (0b11110 << 24) | (type << 22) | (1 << 21) | (Rm << 16) | (cond << 12) | (0b11 << 10) | (Rn << 5) | Rd;
}


// MARK: Floating-point data-processing (3 source)

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬──────────────┬─────┬──┬──────────────┬──┬──────────────┬──────────────┬──────────────┤
 │ 0│ 0│ 0│ 1  1  1  1  0│ type│o1│      Rm      │o0│      Ra      │      Rn      │      Rd      │
 └──┴──┴──┴──────────────┴─────┴──┴──────────────┴──┴──────────────┴──────────────┴──────────────┘
   M     S
 */
#define encode(NAME, o0, o1) \
BRIDGE_TO_SWIFT_WITH_NAME(encode ## NAME(type:_:_:_:_:)) \
EXPORT u32 encode ## NAME(ARM64FloatType type, ARM64Reg Rm, ARM64Reg Ra, ARM64Reg Rn, ARM64Reg Rd) { \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Ra = encodeARM64Reg(Ra, ARM64RegSP); \
    Rm = encodeARM64Reg(Rm, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (0b11111 << 24) | (type << 22) | (o1 << 21) | (Rm << 16) | (o0 << 15) | (Ra << 10) | (Rn << 5) | Rd; \
}

encode(FMADD,  0, 0);
encode(FMSUB,  0, 1);
encode(FNMADD, 1, 0);
encode(FNMSUB, 1, 1);
#undef encode
