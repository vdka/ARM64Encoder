
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

typedef uint32_t u32;
#define EXPORT static

typedef enum ARM64Reg {
    ARM64REG_R0  =  0,
    ARM64REG_R1  =  1,
    ARM64REG_R2  =  2,
    ARM64REG_R3  =  3,
    ARM64REG_R4  =  4,
    ARM64REG_R5  =  5,
    ARM64REG_R6  =  6,
    ARM64REG_R7  =  7,
    ARM64REG_R8  =  8,
    ARM64REG_R9  =  9,
    ARM64REG_R10 = 10,
    ARM64REG_R11 = 11,
    ARM64REG_R12 = 12,
    ARM64REG_R13 = 13,
    ARM64REG_R14 = 14,
    ARM64REG_R15 = 15,
    ARM64REG_R16 = 16,
    ARM64REG_R17 = 17,
    ARM64REG_R18 = 18,
    ARM64REG_R19 = 19,
    ARM64REG_R20 = 20,
    ARM64REG_R21 = 21,
    ARM64REG_R22 = 22,
    ARM64REG_R23 = 23,
    ARM64REG_R24 = 24,
    ARM64REG_R25 = 25,
    ARM64REG_R26 = 26,
    ARM64REG_R27 = 27,
    ARM64REG_R28 = 28,
    ARM64REG_R29 = 29,
    ARM64REG_R30 = 30,
    ARM64REG_SP  = 31,

    ARM64REG_ZR  = 32,
    ARM64REG_LR  = 33,
    ARM64REG_FR  = 34,
} ARM64Reg;

/*
    when '000' result = (PSTATE.Z == '1');                          // EQ or NE
    when '001' result = (PSTATE.C == '1');                          // CS or CC
    when '010' result = (PSTATE.N == '1');                          // MI or PL
    when '011' result = (PSTATE.V == '1');                          // VS or VC
    when '100' result = (PSTATE.C == '1' && PSTATE.Z == '0');       // HI or LS
    when '101' result = (PSTATE.N == PSTATE.V);                     // GE or LT
    when '110' result = (PSTATE.N == PSTATE.V && PSTATE.Z == '0');  // GT or LE
    when '111' result = TRUE;                                       // AL

    // Condition flag values in the set '111x' indicate always true
    // Otherwise, invert condition if necessary.
    if cond<0> == '1' && cond != '1111' then
        result = !result;
*/
typedef enum ARM64COND {
    ARM64COND_EQ = 0b0000,
    ARM64COND_NE = 0b0001,
    ARM64COND_MI = 0b0100,
    ARM64COND_PL = 0b0101,
    ARM64COND_VS = 0b0110,
    ARM64COND_VC = 0b0111,
    ARM64COND_HI = 0b1000,
    ARM64COND_LS = 0b1001,
    ARM64COND_GE = 0b1010,
    ARM64COND_LT = 0b1011,
    ARM64COND_GT = 0b1100,
    ARM64COND_LE = 0b1101,
    ARM64COND_AL = 0b1110,
} ARM64COND;

bool canPack(u32 val, u32 bits) {
    u32 highestBit = 32 - __builtin_clz(val);
    return highestBit <= bits;
}

EXPORT u32 encodeARM64Reg(ARM64Reg reg, ARM64Reg permittedStackAlias) {
    switch (reg) {
    case ARM64REG_FR:
        return 29;

    case ARM64REG_LR:
        return 30;

    case ARM64REG_SP:
    case ARM64REG_ZR:
        assert(permittedStackAlias == reg);
        return 31;

    default:
        return reg;
    }
}


// MARK: Data Processing

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬──────────────┬────────┬──────────────┬────────┬────────┬──────────────┬──────────────┤
 │sf│op│ 0│ 1  0  0  0  1│ 0  0  1│      Rm      │ option │  imm3  │      Rn      │      Rd      │
 └──┴──┴──┴──────────────┴────────┴──────────────┴────────┴────────┴──────────────┴──────────────┘
 */
EXPORT u32 encodeADD(bool use64Bits, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) {
    u32 sf     = use64Bits ? 1 : 0;
    u32 op     = 0;
    u32 option = 0;
    u32 imm3   = 0;


    Rm = encodeARM64Reg(Rm, ARM64REG_SP);
    Rn = encodeARM64Reg(Rn, ARM64REG_SP);
    Rd = encodeARM64Reg(Rd, ARM64REG_SP);

    return (sf << 31) | (op << 30) | (0b10001 << 24) | (0b001 << 21) | (Rm << 16) | (option << 13) | (imm3 << 10) | (Rn << 5) | Rd;
}

EXPORT u32 encodeSUB(bool use64Bits, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) {
    u32 sf     = use64Bits ? 1 : 0;
    u32 op     = 1;
    u32 option = 0;
    u32 imm3   = 0;


    Rm = encodeARM64Reg(Rm, ARM64REG_SP);
    Rn = encodeARM64Reg(Rn, ARM64REG_SP);
    Rd = encodeARM64Reg(Rd, ARM64REG_SP);

    return (sf << 31) | (op << 30) | (0b10001 << 24) | (0b001 << 21) | (Rm << 16) | (option << 13) | (imm3 << 10) | (Rn << 5) | Rd;
}

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬──────────────┬─────┬───────────────────────────────────┬──────────────┬──────────────┤
 │sf│op│ S│ 1  0  0  0  1│shift│               imm12               │      Rn      │      Rd      │
 └──┴──┴──┴──────────────┴─────┴───────────────────────────────────┴──────────────┴──────────────┘
*/
EXPORT u32 encodeADDi(bool use64Bits, bool signExtend, u32 shift, u32 imm12, ARM64Reg Rn, ARM64Reg Rd) {
    assert(canPack(shift, 2));
    assert(canPack(imm12, 12));

    u32 sf = use64Bits ? 1 : 0;
    u32 op = 0;
    u32 S  = signExtend ? 1 : 0;

    Rn = encodeARM64Reg(Rn, ARM64REG_SP);
    Rd = encodeARM64Reg(Rd, ARM64REG_SP);

    return (sf << 31) | (op << 30) | (S << 29) | (0b10001 << 24) | (shift << 22) | (imm12 << 10) | (Rn << 5) | Rd;
}

EXPORT u32 encodeSUBi(bool use64Bits, bool signExtend, u32 shift, u32 imm12, ARM64Reg Rn, ARM64Reg Rd) {
    assert(canPack(shift, 2));
    assert(canPack(imm12, 12));

    u32 sf = use64Bits ? 1 : 0;
    u32 op = 1;
    u32 S  = signExtend ? 1 : 0;

    Rn = encodeARM64Reg(Rn, ARM64REG_SP);
    Rd = encodeARM64Reg(Rd, ARM64REG_SP);

    return (sf << 31) | (op << 30) | (S << 29) | (0b10001 << 24) | (shift << 22) | (imm12 << 10) | (Rn << 5) | Rd;
}

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬─────┬──────────────┬────────────────────────────────────────────────────────┬──────────────┤
 │op│immlo│ 1  0  0  0  0│                         immhi                          │      Rd      │
 └──┴─────┴──────────────┴────────────────────────────────────────────────────────┴──────────────┘
 */
EXPORT u32 encodeADR(bool page, u32 imm21, ARM64Reg Rd) {
    assert(canPack(imm21, 21));

    u32 imm   = imm << (page ? 12 : 0);
    u32 immlo = imm & 0b11;
    u32 immhi = imm >> 2;
    u32 op    = page ? 1 : 0;

    Rd  = encodeARM64Reg(Rd, ARM64REG_SP);

    return (op << 31) | (immlo << 29) | (0b10000 << 24) | (immhi << 5) | Rd;
}


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
EXPORT u32 encodeBcond(u32 imm19, ARM64COND cond) {
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

    Rn = encodeARM64Reg(Rn, ARM64REG_SP);
    Rm = encodeARM64Reg(Rm, ARM64REG_ZR);

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
    Rn = encodeARM64Reg(Rn, ARM64REG_SP);

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

    Rm = encodeARM64Reg(Rm, ARM64REG_SP);
    Rn = encodeARM64Reg(Rn, ARM64REG_ZR); // FIXME: What is permitted here?
    Rt = encodeARM64Reg(Rt, ARM64REG_SP);

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

    Rn = encodeARM64Reg(Rn, ARM64REG_SP);
    Rd = encodeARM64Reg(Rd, ARM64REG_SP);

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

    Rn = encodeARM64Reg(Rn, ARM64REG_SP);
    Rd = encodeARM64Reg(Rd, ARM64REG_SP);

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

    Rm = encodeARM64Reg(Rm, ARM64REG_SP);
    Rn = encodeARM64Reg(Rn, ARM64REG_SP);
    Rt = encodeARM64Reg(Rt, ARM64REG_SP);

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

    Rn = encodeARM64Reg(Rn, ARM64REG_SP);
    Rt = encodeARM64Reg(Rt, ARM64REG_SP);

    return (size << 30) | (0b111 << 27) | (opc << 22) | (imm12 << 10) | (Rn << 5) | Rt;
}



/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬─────┬──────────────┬─────┬──┬──────────────┬─────────────────┬──────────────┬──────────────┤
 │sf│ 0  1│ 0  1  0  1  0│ 0  0│ 0│      Rm      │ 0  0  0  0  0  0│ 1  1  1  1  1│      Rd      │
 └──┴─────┴──────────────┴─────┴──┴──────────────┴─────────────────┴──────────────┴──────────────┘
      opc                        N                      imm6              Rn
 */
EXPORT u32 encodeMOV(bool use32Bits, ARM64Reg Rm, ARM64Reg Rd) {
    u32 sf   = use32Bits ? 1 : 0;
    u32 opc  = 0b01;
    u32 N    = 0;
    u32 imm6 = 0b000000;
    u32 Rn   = 0b11111;

    Rm = encodeARM64Reg(Rm, ARM64REG_SP);
    Rd = encodeARM64Reg(Rd, ARM64REG_SP);

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

    Rd = encodeARM64Reg(Rd, ARM64REG_SP);

    return (sf << 31) | (opc << 29) | (0b100101 << 23) | (hw << 21) | (imm16 << 5) | Rd;
}


// MARK: More Data Processing

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬─────┬──────────────┬────────┬──────────────┬──┬──────────────┬──────────────┬──────────────┤
 │sf│ 0  0│ 1  1  0  1  1│ 0  0  0│      Rm      │o0│      Ra      │      Rn      │      Rd      │
 └──┴─────┴──────────────┴────────┴──────────────┴──┴──────────────┴──────────────┴──────────────┘
 */
EXPORT u32 encodeMADD(bool use64Bits, ARM64Reg Rm, ARM64Reg Ra, ARM64Reg Rn, ARM64Reg Rd) {
    u32 o0 = 0;

    u32 sf = use64Bits ? 1 : 0;

    Rm = encodeARM64Reg(Rm, ARM64REG_SP);
    Ra = encodeARM64Reg(Ra, ARM64REG_SP);
    Rn = encodeARM64Reg(Rn, ARM64REG_SP);
    Rd = encodeARM64Reg(Rd, ARM64REG_SP);

    return (sf << 31) | (0b11011 << 24) | (Rm << 16) | (o0 << 15) | (Ra << 10) | (Rn << 5) | Rd;
}

EXPORT u32 encodeMSUB(bool use64Bits, ARM64Reg Rm, ARM64Reg Ra, ARM64Reg Rn, ARM64Reg Rd) {
    u32 o0 = 1;

    u32 sf = use64Bits ? 1 : 0;

    Rm = encodeARM64Reg(Rm, ARM64REG_SP);
    Ra = encodeARM64Reg(Ra, ARM64REG_SP);
    Rn = encodeARM64Reg(Rn, ARM64REG_SP);
    Rd = encodeARM64Reg(Rd, ARM64REG_SP);

    return (sf << 31) | (0b11011 << 24) | (Rm << 16) | (o0 << 15) | (Ra << 10) | (Rn << 5) | Rd;
}

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬───────────────────────┬──────────────┬──────────────┬──┬──────────────┬──────────────┤
 │sf│ 0│ 0│ 1  1  0  1  0  1  1  0│      Rm      │ 0  0  0  0  1│o1│      Rn      │      Rd      │
 └──┴──┴──┴───────────────────────┴──────────────┴──────────────┴──┴──────────────┴──────────────┘
 */
EXPORT u32 encodeUDIV(bool use64Bits, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) {
    u32 sf = use64Bits ? 1 : 0;
    u32 o1 = 0;

    Rm = encodeARM64Reg(Rm, ARM64REG_SP);
    Rn = encodeARM64Reg(Rn, ARM64REG_SP);
    Rd = encodeARM64Reg(Rd, ARM64REG_SP);

    return (sf << 31) | (0b11010110 << 21) | (Rm << 16) | (0b00001 << 11) | (o1 << 10) | (Rn << 5) | Rd;
}

EXPORT u32 encodeSDIV(bool use64Bits, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) {
    u32 sf = use64Bits ? 1 : 0;
    u32 o1 = 1;

    Rm = encodeARM64Reg(Rm, ARM64REG_SP);
    Rn = encodeARM64Reg(Rn, ARM64REG_SP);
    Rd = encodeARM64Reg(Rd, ARM64REG_SP);

    return (sf << 31) | (0b11010110 << 21) | (Rm << 16) | (0b00001 << 11) | (o1 << 10) | (Rn << 5) | Rd;
}

/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬──┬──┬───────────────────────┬──────────────┬───────────┬─────┬──────────────┬──────────────┤
 │sf│ 0│ 0│ 1  1  0  1  0  1  1  0│      Rm      │ 0  0  1  0│ op2 │      Rn      │      Rd      │
 └──┴──┴──┴───────────────────────┴──────────────┴───────────┴─────┴──────────────┴──────────────┘
 */
EXPORT u32 encodeLSL(bool use64Bits, ARM64Reg Rm, ARM64Reg Rn, ARM64Reg Rd) {
    u32 op2 = 0b00;

    u32 sf = use64Bits ? 1 : 0;

    Rm = encodeARM64Reg(Rm, ARM64REG_SP);
    Rn = encodeARM64Reg(Rn, ARM64REG_SP);
    Rd = encodeARM64Reg(Rd, ARM64REG_SP);

    return (sf << 31) | (0b11010110 << 21) | (Rm << 16) | (0b00010 << 11) | (op2 << 10) | (Rn << 5) | Rd;
}


/*
 ┌───────────────────────────────────────────────────────────────────────────────────────────────┐
 │31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0│
 ├──┬─────┬─────────────────┬──┬─────────────────┬─────────────────┬──────────────┬──────────────┤
 │sf│ opc │ 1  0  0  1  1  0│ N│       immr      │       imms      │      Rn      │      Rd      │
 └──┴─────┴─────────────────┴──┴─────────────────┴─────────────────┴──────────────┴──────────────┘
*/
EXPORT u32 encodeLSLi(bool use64Bits, u32 imm, ARM64Reg Rn, ARM64Reg Rd) {
    return 0xFFFFFFFF;
}
