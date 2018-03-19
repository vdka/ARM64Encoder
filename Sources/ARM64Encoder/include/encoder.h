
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#if __has_attribute(swift_wrapper)
#define SWIFT_ENUM __attribute__((swift_wrapper(enum)))
#else
#define SWIFT_ENUM
#endif

typedef uint32_t u32;
#define EXPORT static

typedef u32 ARM64Reg SWIFT_ENUM;
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

typedef u32 ARM64FloatType SWIFT_ENUM;
ARM64FloatType const ARM64FloatTypeSingle = 0b00;
ARM64FloatType const ARM64FloatTypeDouble = 0b01;
ARM64FloatType const ARM64FloatTypeHalf   = 0b11;

typedef u32 ARM64Cond SWIFT_ENUM;
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
EXPORT u32 encode##NAME(u32 imm21, ARM64Reg Rd) { \
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
EXPORT u32 encode ## NAME ## i(bool use64Bits, u32 shift, u32 imm12, ARM64Reg Rn, ARM64Reg Rd) { \
    assert(canPack(shift, 2)); \
    assert(canPack(imm12, 12)); \
    u32 sf = use64Bits ? 1 : 0; \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (sf << 31) | (op << 30) | (S << 29) | (0b10001 << 24) | (shift << 22) | (imm12 << 10) | (Rn << 5) | Rd; \
}

encode(ADD,  0, 0);
encode(ADDS, 0, 1);
encode(SUB,  1, 0);
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
EXPORT u32 encode ## NAME ## i(bool use64Bits, u32 imm, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) { \
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















// MARK: Branches

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──────────────┬─────────────────────────────────────────────────────────────────────────────┤
 │ 0│ 0  0  1  0  1│                                 imm26                                       │
 └──┴──────────────┴─────────────────────────────────────────────────────────────────────────────┘
 */
EXPORT u32 encodeB(u32 imm26) {
    assert(canPack(imm26, 26));
    return (0b00101 << 24) | (imm26 << 0);
}

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├────────────────────┬──┬────────────────────────────────────────────────────────┬──┬───────────┤
 │ 0  1  0  1  0  1  0│ 0│                          imm19                         │ 0│   cond    │
 └────────────────────┴──┴────────────────────────────────────────────────────────┴──┴───────────┘
 */
EXPORT u32 encodeBcond(u32 imm19, ARM64Cond cond) {
    assert(canPack(imm19, 19));
    return (0b0101010 << 25) | (imm19 << 5) | cond;
}

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬──────────────┬────────┬──────────────┬────────┬────────┬──────────────┬──────────────┤
 │sf│ 1│ 1│ 0  1  0  1  1│ 0  0  1│      Rm      │ option │  imm3  │      Rn      │ 1  1  1  1  1│
 └──┴──┴──┴──────────────┴────────┴──────────────┴────────┴────────┴──────────────┴──────────────┘
 */
EXPORT u32 encodeCMP(bool use64Bits, ARM64Reg Rm, ARM64Reg Rn) {
    u32 sf     = use64Bits ? 1 : 0;
    u32 option = 0b000;
    u32 imm3   = 0b000;

    Rn = encodeARM64Reg(Rn, ARM64RegSP);
    Rm = encodeARM64Reg(Rm, ARM64RegZR);

    return (sf << 31) | (1 << 30) | (1 << 29) | (0b01011 << 24) | (0b001 << 21) | (Rm << 16) | (option << 13) | (imm3 << 10) | (Rn << 5) | 0b11111;
}

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬──────────────┬─────┬───────────────────────────────────┬──────────────┬──────────────┤
 │sf│ 1│ 1│ 1  0  0  0  1│shift│               imm12               │      Rn      │ 1  1  1  1  1│
 └──┴──┴──┴──────────────┴─────┴───────────────────────────────────┴──────────────┴──────────────┘
 */
EXPORT u32 encodeCMPi(bool use64Bits, u32 shift, u32 imm12, ARM64Reg Rn) {
    assert(canPack(shift, 2));
    assert(canPack(imm12, 12));

    u32 sf = use64Bits ? 1 : 0;
    Rn = encodeARM64Reg(Rn, ARM64RegSP);

    return (sf << 31) | (1 << 30) | (1 << 29) | (0b10001 << 24) | (shift << 22) | (imm12 << 10) | (Rn << 5) | 0b11111;
}


// MARK: Load and Stores

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────┬────────┬──┬─────┬─────┬──┬──────────────┬────────┬──┬─────┬──────────────┬──────────────┤
 │ size│ 1  1  1│ 0│ 0  0│ 0  1│ 1│      Rm      │ option │ S│ 1  0│      Rn      │      Rt      │
 └─────┴────────┴──┴─────┴─────┴──┴──────────────┴────────┴──┴─────┴──────────────┴──────────────┘
                           opc
 */
EXPORT u32 encodeLD(u32 size, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rt) {
    assert(canPack(size, 2));

    Rm = encodeARM64Reg(Rm, ARM64RegSP);
    Rn = encodeARM64Reg(Rn, ARM64RegZR); // FIXME: What is permitted here?
    Rt = encodeARM64Reg(Rt, ARM64RegSP);

    u32 option = 0b000;
    u32 S      = 0b0;

    return (size << 30) | (0b111000011 << 21) | (Rm << 16) | (option << 13) | (S << 12) | (0b10 << 10) | (Rn << 5) | Rt;
}

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────┬────────┬──┬─────┬─────┬───────────────────────────────────┬──────────────┬──────────────┤
 │ size│ 1  1  1│ 0│ 0  1│ 0  1│               imm12               │      Rn      │      Rd      │
 └─────┴────────┴──┴─────┴─────┴───────────────────────────────────┴──────────────┴──────────────┘
                           opc
 */
EXPORT u32 encodeLDi(u32 size, u32 imm12, ARM64Reg Rn, ARM64Reg Rd) {
    assert(canPack(size, 2));
    assert(canPack(imm12, 12));

    Rn = encodeARM64Reg(Rn, ARM64RegSP);
    Rd = encodeARM64Reg(Rd, ARM64RegSP);

    return (size << 30) | (0b11100101 << 22) | (imm12 << 10) | (Rn << 5) | Rd;
}

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────┬────────┬──┬─────┬─────┬─────────────────────────────┬─────┬──────────────┬──────────────┤
 │ size│ 1  1  1│ 0│ 0  0│ 0  1│              imm9           │ 0  0│      Rn      │      Rd      │
 └─────┴────────┴──┴─────┴─────┴─────────────────────────────┴─────┴──────────────┴──────────────┘
                           opc
 */
EXPORT u32 encodeLDU(u32 size, u32 imm9, ARM64Reg Rn, ARM64Reg Rd) {
    assert(canPack(size, 2));
    assert(canPack(imm9, 9));

    Rn = encodeARM64Reg(Rn, ARM64RegSP);
    Rd = encodeARM64Reg(Rd, ARM64RegSP);

    return (size << 30) | (0b11100001 << 22) | (imm9 << 12) | (Rn << 5) | Rd;
}

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────┬────────┬──┬─────┬─────┬──┬──────────────┬────────┬──┬─────┬──────────────┬──────────────┤
 │ size│ 1  1  1│ 0│ 0  0│ 0  0│ 1│      Rm      │ option │ S│ 1  0│      Rn      │      Rt      │
 └─────┴────────┴──┴─────┴─────┴──┴──────────────┴────────┴──┴─────┴──────────────┴──────────────┘
                           opc
 */
EXPORT u32 encodeSTR(u32 size, u32 imm12, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rt) {
    assert(canPack(size, 2));

    u32 option = 0b000;
    u32 S      = 0b0;

    Rm = encodeARM64Reg(Rm, ARM64RegSP);
    Rn = encodeARM64Reg(Rn, ARM64RegSP);
    Rt = encodeARM64Reg(Rt, ARM64RegSP);

    return (size << 30) | (0b111000011 << 21) | (Rm << 16) | (option << 13) | (S << 12) | (0b10 << 10) | (Rn << 5) | Rt;
}

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├─────┬────────┬──┬─────┬─────┬───────────────────────────────────┬──────────────┬──────────────┤
 │ size│ 1  1  1│ 0│ 0  0│ 0  0│               imm12               │      Rn      │      Rt      │
 └─────┴────────┴──┴─────┴─────┴───────────────────────────────────┴──────────────┴──────────────┘
                           opc
 */
EXPORT u32 encodeSTRi(u32 size, u32 imm12, ARM64Reg Rn, ARM64Reg Rt) {
    assert(canPack(size, 2));
    assert(canPack(imm12, 12));

    u32 opc = 0b00;

    Rn = encodeARM64Reg(Rn, ARM64RegSP);
    Rt = encodeARM64Reg(Rt, ARM64RegSP);

    return (size << 30) | (0b111 << 27) | (opc << 22) | (imm12 << 10) | (Rn << 5) | Rt;
}


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
EXPORT u32 encode##NAME(bool use64Bits, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) { \
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
 │sf│ 1│ 0│ 1  1  0  1  0  1  1  0│      Rm      │      opcode     │      Rn      │      Rd      │
 └──┴──┴──┴───────────────────────┴──────────────┴─────────────────┴──────────────┴──────────────┘
         S
 */
#define encode(NAME, opcode2, opcode) \
EXPORT u32 encode##NAME(bool use64Bits, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) { \
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
EXPORT u32 encode##NAME(bool use64Bits, u32 shift, ARM64Reg Rm, u32 imm6, ARM64Reg Rn, ARM64Reg Rd) { \
    assert(canPack(imm6, 5)); /* NOTE: 1xxxxx is UNALLOCATED and there for imm6 is really an imm5 */ \
    u32 sf = use64Bits ? 1 : 0; \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rm = encodeARM64Reg(Rm, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (sf << 31) | (opc << 29) | (0b11010 << 24) | (shift << 22) | (N << 21) | (Rm << 16) | (imm6 << 10) | (Rn << 5) | Rd; \
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
EXPORT u32 encode##NAME(bool use64Bits, u32 shift, ARM64Reg Rm, u32 imm6, ARM64Reg Rn, ARM64Reg Rd) { \
    assert(canPack(imm6, 5)); /* NOTE: 1xxxxx is UNALLOCATED and there for imm6 is really an imm5 */ \
    u32 sf = use64Bits ? 1 : 0; \
    Rn = encodeARM64Reg(Rn, ARM64RegSP); \
    Rm = encodeARM64Reg(Rm, ARM64RegSP); \
    Rd = encodeARM64Reg(Rd, ARM64RegSP); \
    return (sf << 31) | (op << 30) | (S << 29) | (0b11010 << 24) | (shift << 22) | (Rm << 16) | (imm6 << 10) | (Rn << 5) | Rd; \
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
EXPORT u32 encode## NAME ## X(bool use64Bits, ARM64Reg Rm, u32 option, u32 imm3, ARM64Reg Rn, ARM64Reg Rd) { \
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
EXPORT u32 encode##NAME(bool use64Bits, ARM64Reg Rm, u32 option, u32 imm3, ARM64Reg Rn, ARM64Reg Rd) { \
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
EXPORT u32 encode##NAME(bool use64Bits, ARM64Reg Rm, u32 cond, ARM64Reg Rn, u32 nzcv) { \
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
EXPORT u32 encode##NAME ## i(bool use64Bits, u32 imm5, u32 cond, ARM64Reg Rn, u32 nzcv) { \
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
EXPORT u32 encode##NAME(bool use64Bits, u32 cond, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) { \
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
EXPORT u32 encod ##NAME(bool use64Bits, ARM64Reg Rm, ARM64Reg Ra, ARM64Reg Rn, ARM64Reg Rd) { \
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


/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬─────┬──────────────┬─────┬──┬──────────────┬─────────────────┬──────────────┬──────────────┤
 │sf│ 0  1│ 0  1  0  1  0│ 0  0│ 0│      Rm      │ 0  0  0  0  0  0│ 1  1  1  1  1│      Rd      │
 └──┴─────┴──────────────┴─────┴──┴──────────────┴─────────────────┴──────────────┴──────────────┘
      opc                        N                      imm6              Rn
 */
EXPORT u32 encodeMOV(bool use64Bits, ARM64Reg Rm, ARM64Reg Rd) {
    u32 sf   = use64Bits ? 1 : 0;
    u32 opc  = 0b01;
    u32 N    = 0;
    u32 imm6 = 0b000000;
    u32 Rn   = 0b11111;

    Rm = encodeARM64Reg(Rm, ARM64RegSP);
    Rd = encodeARM64Reg(Rd, ARM64RegSP);

    return (sf << 31) | (opc << 29) | (0b01010 << 24) | (N << 21) | (Rm << 16) | (imm6 << 10) | (Rn << 5) | Rd;
}

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬─────┬─────────────────┬─────┬───────────────────────────────────────────────┬──────────────┤
 │sf│ 1  0│ 1  0  0  1  0  1│  hw │                    imm16                      │      Rd      │
 └──┴─────┴─────────────────┴─────┴───────────────────────────────────────────────┴──────────────┘
      opc
 */
EXPORT u32 encodeMOVi(bool use64Bits, u32 imm16, ARM64Reg Rd) {
    assert(canPack(imm16, 16));

    u32 sf  = use64Bits ? 1 : 0;
    u32 opc = 0b10;
    u32 hw  = 0b00;

    Rd = encodeARM64Reg(Rd, ARM64RegSP);

    return (sf << 31) | (opc << 29) | (0b100101 << 23) | (hw << 21) | (imm16 << 5) | Rd;
}













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
EXPORT u32 encode##NAME(ARM64FloatType type, ARM64Reg Rn, ARM64Reg Rd) { \
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
EXPORT u32 encode##NAME(ARM64FloatType type, ARM64Reg Rm, ARM64Reg Rn) { \
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
EXPORT u32 encode##NAME(ARM64FloatType type, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) { \
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
EXPORT u32 encode##NAME(ARM64FloatType type, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) { \
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
EXPORT u32 encode##NAME(ARM64FloatType type, ARM64Reg Rm, ARM64Reg Ra, ARM64Reg Rn, ARM64Reg Rd) { \
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
