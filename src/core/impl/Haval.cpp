#include "Haval.h"
#include "../HashAlgorithmFactory.h"

namespace core {
namespace impl {

// HAVAL boolean functions (from haval1.cpp)
#define F1(x6, x5, x4, x3, x2, x1, x0) \
    (((x1) & ((x0) ^ (x4))) ^ ((x2) & (x5)) ^ ((x3) & (x6)) ^ (x0))

#define F2(x6, x5, x4, x3, x2, x1, x0) \
    (((x2) & (((x1) & ~(x3)) ^ ((x4) & (x5)) ^ (x6) ^ (x0))) ^ (((x4) & ((x1) ^ (x5))) ^ ((x3) & (x5)) ^ (x0)))

#define F3(x6, x5, x4, x3, x2, x1, x0) \
    (((x3) & (((x1) & (x2)) ^ (x6) ^ (x0))) ^ ((x1) & (x4)) ^ ((x2) & (x5)) ^ (x0))

#define F4(x6, x5, x4, x3, x2, x1, x0) \
    (((x4) & (((x5) & ~(x2)) ^ ((x3) & ~(x6)) ^ (x1) ^ (x6) ^ (x0))) ^ ((x3) & (((x1) & (x2)) ^ (x5) ^ (x6))) ^ ((x2) & (x6)) ^ (x0))

#define F5(x6, x5, x4, x3, x2, x1, x0) \
    (((x0) & (((x1) & (x2) & (x3)) ^ ~(x5))) ^ ((x1) & (x4)) ^ ((x2) & (x5)) ^ ((x3) & (x6)))

// Phi permutations for 3-pass
#define Fphi1_3(x6,x5,x4,x3,x2,x1,x0) F1(x1,x0,x3,x5,x6,x2,x4)
#define Fphi2_3(x6,x5,x4,x3,x2,x1,x0) F2(x4,x2,x1,x0,x5,x3,x6)
#define Fphi3_3(x6,x5,x4,x3,x2,x1,x0) F3(x6,x1,x2,x3,x4,x5,x0)

// Phi permutations for 4-pass
#define Fphi1_4(x6,x5,x4,x3,x2,x1,x0) F1(x2,x6,x1,x4,x5,x3,x0)
#define Fphi2_4(x6,x5,x4,x3,x2,x1,x0) F2(x3,x5,x2,x0,x1,x6,x4)
#define Fphi3_4(x6,x5,x4,x3,x2,x1,x0) F3(x1,x4,x3,x6,x0,x2,x5)
#define Fphi4_4(x6,x5,x4,x3,x2,x1,x0) F4(x6,x4,x0,x5,x2,x1,x3)

// Phi permutations for 5-pass
#define Fphi1_5(x6,x5,x4,x3,x2,x1,x0) F1(x3,x4,x1,x0,x5,x2,x6)
#define Fphi2_5(x6,x5,x4,x3,x2,x1,x0) F2(x6,x2,x1,x0,x3,x4,x5)
#define Fphi3_5(x6,x5,x4,x3,x2,x1,x0) F3(x2,x6,x0,x4,x3,x1,x5)
#define Fphi4_5(x6,x5,x4,x3,x2,x1,x0) F4(x1,x5,x3,x2,x0,x4,x6)
#define Fphi5_5(x6,x5,x4,x3,x2,x1,x0) F5(x2,x5,x0,x6,x4,x3,x1)

// Round function macros
#define FF1(x7,x6,x5,x4,x3,x2,x1,x0,w,Fphi) \
    (x7) = CryptoPP::rotrFixed(Fphi(x6,x5,x4,x3,x2,x1,x0), 7) + CryptoPP::rotrFixed((x7), 11) + (w)

#define FF2(x7,x6,x5,x4,x3,x2,x1,x0,w,c,Fphi) \
    (x7) = CryptoPP::rotrFixed(Fphi(x6,x5,x4,x3,x2,x1,x0), 7) + CryptoPP::rotrFixed((x7), 11) + (w) + (c)

// Constants for pass 2
static const CryptoPP::word32 C2[32] = {
    0x452821E6, 0x38D01377, 0xBE5466CF, 0x34E90C6C, 0xC0AC29B7, 0xC97C50DD, 0x3F84D5B5, 0xB5470917,
    0x9216D5D9, 0x8979FB1B, 0xD1310BA6, 0x98DFB5AC, 0x2FFD72DB, 0xD01ADFB7, 0xB8E1AFED, 0x6A267E96,
    0xBA7C9045, 0xF12C7F99, 0x24A19947, 0xB3916CF7, 0x0801F2E2, 0x858EFC16, 0x636920D8, 0x71574E69,
    0xA458FEA3, 0xF4933D7E, 0x0D95748F, 0x728EB658, 0x718BCD58, 0x82154AEE, 0x7B54A41D, 0xC25A59B5
};

// Constants for pass 3
static const CryptoPP::word32 C3[32] = {
    0x9C30D539, 0x2AF26013, 0xC5D1B023, 0x286085F0, 0xCA417918, 0xB8DB38EF, 0x8E79DCB0, 0x603A180E,
    0x6C9E0E8B, 0xB01E8A3E, 0xD71577C1, 0xBD314B27, 0x78AF2FDA, 0x55605C60, 0xE65525F3, 0xAA55AB94,
    0x57489862, 0x63E81440, 0x55CA396A, 0x2AAB10B6, 0xB4CC5C34, 0x1141E8CE, 0xA15486AF, 0x7C72E993,
    0xB3EE1411, 0x636FBC2A, 0x2BA9C55D, 0x741831F6, 0xCE5C3E16, 0x9B87931E, 0xAFD6BA33, 0x6C24CF5C
};

// Constants for pass 4
static const CryptoPP::word32 C4[32] = {
    0x7A325381, 0x28958677, 0x3B8F4898, 0x6B4BB9AF, 0xC4BFE81B, 0x66282193, 0x61D809CC, 0xFB21A991,
    0x487CAC60, 0x5DEC8032, 0xEF845D5D, 0xE98575B1, 0xDC262302, 0xEB651B88, 0x23893E81, 0xD396ACC5,
    0x0F6D6FF3, 0x83F44239, 0x2E0B4482, 0xA4842004, 0x69C8F04A, 0x9E1F9B5E, 0x21C66842, 0xF6E96C9A,
    0x670C9C61, 0xABD388F0, 0x6A51A0D2, 0xD8542F68, 0x960FA728, 0xAB5133A3, 0x6EEF0B6C, 0x137A3BE4
};

// Constants for pass 5
static const CryptoPP::word32 C5[32] = {
    0xBA3BF050, 0x7EFB2A98, 0xA1F1651D, 0x39AF0176, 0x66CA593E, 0x82430E88, 0x8CEE8619, 0x456F9FB4,
    0x7D84A5C3, 0x3B8B5EBE, 0xE06F75D8, 0x85C12073, 0x401A449F, 0x56C16AA6, 0x4ED3AA62, 0x363F7706,
    0x1BFEDF72, 0x429B023D, 0x37D0D724, 0xD00A1248, 0xDB0FEAD3, 0x49F1C09B, 0x075372C9, 0x80991B7B,
    0x25D479D8, 0xF6E8DEF7, 0xE3FE501A, 0xB6794C3B, 0x976CE0BD, 0x04C006BA, 0xC1A94FB6, 0x409F60C4
};

// Word ordering for pass 2
static const unsigned char wp2[32] = {
    5,14,26,18,11,28,7,16,0,23,20,22,1,10,4,8,30,3,21,9,17,24,29,6,19,12,15,13,2,25,31,27
};

// Word ordering for pass 3
static const unsigned char wp3[32] = {
    19,9,4,20,28,17,8,22,29,14,25,12,24,30,16,26,31,15,7,3,1,0,18,27,13,6,21,10,23,11,5,2
};

// Word ordering for pass 4
static const unsigned char wp4[32] = {
    24,4,0,14,2,7,28,23,26,6,30,20,18,25,19,3,22,11,31,21,8,27,12,9,1,29,5,15,17,10,16,13
};

// Word ordering for pass 5
static const unsigned char wp5[32] = {
    27,3,21,26,17,11,20,29,19,0,12,7,13,8,31,10,5,9,14,30,18,6,28,24,2,23,16,22,4,1,25,15
};

template <unsigned int DIGEST_BITS, unsigned int PASSES>
void Haval<DIGEST_BITS, PASSES>::Transform(const CryptoPP::byte *block) {
    CryptoPP::word32 W[32];
    for (int i = 0; i < 32; ++i) {
        W[i] = CryptoPP::GetWord<CryptoPP::word32>(false, CryptoPP::LITTLE_ENDIAN_ORDER, block + i * 4);
    }

    CryptoPP::word32 t0 = m_state[0], t1 = m_state[1], t2 = m_state[2], t3 = m_state[3];
    CryptoPP::word32 t4 = m_state[4], t5 = m_state[5], t6 = m_state[6], t7 = m_state[7];

    // Pass 1 - no constants
    if (PASSES == 3) {
        #define PASS1_ROUND(i) FF1(t7,t6,t5,t4,t3,t2,t1,t0,W[i],Fphi1_3); \
            { CryptoPP::word32 tmp=t7; t7=t6; t6=t5; t5=t4; t4=t3; t3=t2; t2=t1; t1=t0; t0=tmp; }
        PASS1_ROUND(0);PASS1_ROUND(1);PASS1_ROUND(2);PASS1_ROUND(3);PASS1_ROUND(4);PASS1_ROUND(5);PASS1_ROUND(6);PASS1_ROUND(7);
        PASS1_ROUND(8);PASS1_ROUND(9);PASS1_ROUND(10);PASS1_ROUND(11);PASS1_ROUND(12);PASS1_ROUND(13);PASS1_ROUND(14);PASS1_ROUND(15);
        PASS1_ROUND(16);PASS1_ROUND(17);PASS1_ROUND(18);PASS1_ROUND(19);PASS1_ROUND(20);PASS1_ROUND(21);PASS1_ROUND(22);PASS1_ROUND(23);
        PASS1_ROUND(24);PASS1_ROUND(25);PASS1_ROUND(26);PASS1_ROUND(27);PASS1_ROUND(28);PASS1_ROUND(29);PASS1_ROUND(30);PASS1_ROUND(31);
        #undef PASS1_ROUND

        #define PASS2_ROUND(i) FF2(t7,t6,t5,t4,t3,t2,t1,t0,W[wp2[i]],C2[i],Fphi2_3); \
            { CryptoPP::word32 tmp=t7; t7=t6; t6=t5; t5=t4; t4=t3; t3=t2; t2=t1; t1=t0; t0=tmp; }
        PASS2_ROUND(0);PASS2_ROUND(1);PASS2_ROUND(2);PASS2_ROUND(3);PASS2_ROUND(4);PASS2_ROUND(5);PASS2_ROUND(6);PASS2_ROUND(7);
        PASS2_ROUND(8);PASS2_ROUND(9);PASS2_ROUND(10);PASS2_ROUND(11);PASS2_ROUND(12);PASS2_ROUND(13);PASS2_ROUND(14);PASS2_ROUND(15);
        PASS2_ROUND(16);PASS2_ROUND(17);PASS2_ROUND(18);PASS2_ROUND(19);PASS2_ROUND(20);PASS2_ROUND(21);PASS2_ROUND(22);PASS2_ROUND(23);
        PASS2_ROUND(24);PASS2_ROUND(25);PASS2_ROUND(26);PASS2_ROUND(27);PASS2_ROUND(28);PASS2_ROUND(29);PASS2_ROUND(30);PASS2_ROUND(31);
        #undef PASS2_ROUND

        #define PASS3_ROUND(i) FF2(t7,t6,t5,t4,t3,t2,t1,t0,W[wp3[i]],C3[i],Fphi3_3); \
            { CryptoPP::word32 tmp=t7; t7=t6; t6=t5; t5=t4; t4=t3; t3=t2; t2=t1; t1=t0; t0=tmp; }
        PASS3_ROUND(0);PASS3_ROUND(1);PASS3_ROUND(2);PASS3_ROUND(3);PASS3_ROUND(4);PASS3_ROUND(5);PASS3_ROUND(6);PASS3_ROUND(7);
        PASS3_ROUND(8);PASS3_ROUND(9);PASS3_ROUND(10);PASS3_ROUND(11);PASS3_ROUND(12);PASS3_ROUND(13);PASS3_ROUND(14);PASS3_ROUND(15);
        PASS3_ROUND(16);PASS3_ROUND(17);PASS3_ROUND(18);PASS3_ROUND(19);PASS3_ROUND(20);PASS3_ROUND(21);PASS3_ROUND(22);PASS3_ROUND(23);
        PASS3_ROUND(24);PASS3_ROUND(25);PASS3_ROUND(26);PASS3_ROUND(27);PASS3_ROUND(28);PASS3_ROUND(29);PASS3_ROUND(30);PASS3_ROUND(31);
        #undef PASS3_ROUND
    }
    else if (PASSES == 4) {
        #define PASS1_ROUND(i) FF1(t7,t6,t5,t4,t3,t2,t1,t0,W[i],Fphi1_4); \
            { CryptoPP::word32 tmp=t7; t7=t6; t6=t5; t5=t4; t4=t3; t3=t2; t2=t1; t1=t0; t0=tmp; }
        PASS1_ROUND(0);PASS1_ROUND(1);PASS1_ROUND(2);PASS1_ROUND(3);PASS1_ROUND(4);PASS1_ROUND(5);PASS1_ROUND(6);PASS1_ROUND(7);
        PASS1_ROUND(8);PASS1_ROUND(9);PASS1_ROUND(10);PASS1_ROUND(11);PASS1_ROUND(12);PASS1_ROUND(13);PASS1_ROUND(14);PASS1_ROUND(15);
        PASS1_ROUND(16);PASS1_ROUND(17);PASS1_ROUND(18);PASS1_ROUND(19);PASS1_ROUND(20);PASS1_ROUND(21);PASS1_ROUND(22);PASS1_ROUND(23);
        PASS1_ROUND(24);PASS1_ROUND(25);PASS1_ROUND(26);PASS1_ROUND(27);PASS1_ROUND(28);PASS1_ROUND(29);PASS1_ROUND(30);PASS1_ROUND(31);
        #undef PASS1_ROUND

        #define PASS2_ROUND(i) FF2(t7,t6,t5,t4,t3,t2,t1,t0,W[wp2[i]],C2[i],Fphi2_4); \
            { CryptoPP::word32 tmp=t7; t7=t6; t6=t5; t5=t4; t4=t3; t3=t2; t2=t1; t1=t0; t0=tmp; }
        PASS2_ROUND(0);PASS2_ROUND(1);PASS2_ROUND(2);PASS2_ROUND(3);PASS2_ROUND(4);PASS2_ROUND(5);PASS2_ROUND(6);PASS2_ROUND(7);
        PASS2_ROUND(8);PASS2_ROUND(9);PASS2_ROUND(10);PASS2_ROUND(11);PASS2_ROUND(12);PASS2_ROUND(13);PASS2_ROUND(14);PASS2_ROUND(15);
        PASS2_ROUND(16);PASS2_ROUND(17);PASS2_ROUND(18);PASS2_ROUND(19);PASS2_ROUND(20);PASS2_ROUND(21);PASS2_ROUND(22);PASS2_ROUND(23);
        PASS2_ROUND(24);PASS2_ROUND(25);PASS2_ROUND(26);PASS2_ROUND(27);PASS2_ROUND(28);PASS2_ROUND(29);PASS2_ROUND(30);PASS2_ROUND(31);
        #undef PASS2_ROUND

        #define PASS3_ROUND(i) FF2(t7,t6,t5,t4,t3,t2,t1,t0,W[wp3[i]],C3[i],Fphi3_4); \
            { CryptoPP::word32 tmp=t7; t7=t6; t6=t5; t5=t4; t4=t3; t3=t2; t2=t1; t1=t0; t0=tmp; }
        PASS3_ROUND(0);PASS3_ROUND(1);PASS3_ROUND(2);PASS3_ROUND(3);PASS3_ROUND(4);PASS3_ROUND(5);PASS3_ROUND(6);PASS3_ROUND(7);
        PASS3_ROUND(8);PASS3_ROUND(9);PASS3_ROUND(10);PASS3_ROUND(11);PASS3_ROUND(12);PASS3_ROUND(13);PASS3_ROUND(14);PASS3_ROUND(15);
        PASS3_ROUND(16);PASS3_ROUND(17);PASS3_ROUND(18);PASS3_ROUND(19);PASS3_ROUND(20);PASS3_ROUND(21);PASS3_ROUND(22);PASS3_ROUND(23);
        PASS3_ROUND(24);PASS3_ROUND(25);PASS3_ROUND(26);PASS3_ROUND(27);PASS3_ROUND(28);PASS3_ROUND(29);PASS3_ROUND(30);PASS3_ROUND(31);
        #undef PASS3_ROUND

        #define PASS4_ROUND(i) FF2(t7,t6,t5,t4,t3,t2,t1,t0,W[wp4[i]],C4[i],Fphi4_4); \
            { CryptoPP::word32 tmp=t7; t7=t6; t6=t5; t5=t4; t4=t3; t3=t2; t2=t1; t1=t0; t0=tmp; }
        PASS4_ROUND(0);PASS4_ROUND(1);PASS4_ROUND(2);PASS4_ROUND(3);PASS4_ROUND(4);PASS4_ROUND(5);PASS4_ROUND(6);PASS4_ROUND(7);
        PASS4_ROUND(8);PASS4_ROUND(9);PASS4_ROUND(10);PASS4_ROUND(11);PASS4_ROUND(12);PASS4_ROUND(13);PASS4_ROUND(14);PASS4_ROUND(15);
        PASS4_ROUND(16);PASS4_ROUND(17);PASS4_ROUND(18);PASS4_ROUND(19);PASS4_ROUND(20);PASS4_ROUND(21);PASS4_ROUND(22);PASS4_ROUND(23);
        PASS4_ROUND(24);PASS4_ROUND(25);PASS4_ROUND(26);PASS4_ROUND(27);PASS4_ROUND(28);PASS4_ROUND(29);PASS4_ROUND(30);PASS4_ROUND(31);
        #undef PASS4_ROUND
    }
    else { // PASSES == 5
        #define PASS1_ROUND(i) FF1(t7,t6,t5,t4,t3,t2,t1,t0,W[i],Fphi1_5); \
            { CryptoPP::word32 tmp=t7; t7=t6; t6=t5; t5=t4; t4=t3; t3=t2; t2=t1; t1=t0; t0=tmp; }
        PASS1_ROUND(0);PASS1_ROUND(1);PASS1_ROUND(2);PASS1_ROUND(3);PASS1_ROUND(4);PASS1_ROUND(5);PASS1_ROUND(6);PASS1_ROUND(7);
        PASS1_ROUND(8);PASS1_ROUND(9);PASS1_ROUND(10);PASS1_ROUND(11);PASS1_ROUND(12);PASS1_ROUND(13);PASS1_ROUND(14);PASS1_ROUND(15);
        PASS1_ROUND(16);PASS1_ROUND(17);PASS1_ROUND(18);PASS1_ROUND(19);PASS1_ROUND(20);PASS1_ROUND(21);PASS1_ROUND(22);PASS1_ROUND(23);
        PASS1_ROUND(24);PASS1_ROUND(25);PASS1_ROUND(26);PASS1_ROUND(27);PASS1_ROUND(28);PASS1_ROUND(29);PASS1_ROUND(30);PASS1_ROUND(31);
        #undef PASS1_ROUND

        #define PASS2_ROUND(i) FF2(t7,t6,t5,t4,t3,t2,t1,t0,W[wp2[i]],C2[i],Fphi2_5); \
            { CryptoPP::word32 tmp=t7; t7=t6; t6=t5; t5=t4; t4=t3; t3=t2; t2=t1; t1=t0; t0=tmp; }
        PASS2_ROUND(0);PASS2_ROUND(1);PASS2_ROUND(2);PASS2_ROUND(3);PASS2_ROUND(4);PASS2_ROUND(5);PASS2_ROUND(6);PASS2_ROUND(7);
        PASS2_ROUND(8);PASS2_ROUND(9);PASS2_ROUND(10);PASS2_ROUND(11);PASS2_ROUND(12);PASS2_ROUND(13);PASS2_ROUND(14);PASS2_ROUND(15);
        PASS2_ROUND(16);PASS2_ROUND(17);PASS2_ROUND(18);PASS2_ROUND(19);PASS2_ROUND(20);PASS2_ROUND(21);PASS2_ROUND(22);PASS2_ROUND(23);
        PASS2_ROUND(24);PASS2_ROUND(25);PASS2_ROUND(26);PASS2_ROUND(27);PASS2_ROUND(28);PASS2_ROUND(29);PASS2_ROUND(30);PASS2_ROUND(31);
        #undef PASS2_ROUND

        #define PASS3_ROUND(i) FF2(t7,t6,t5,t4,t3,t2,t1,t0,W[wp3[i]],C3[i],Fphi3_5); \
            { CryptoPP::word32 tmp=t7; t7=t6; t6=t5; t5=t4; t4=t3; t3=t2; t2=t1; t1=t0; t0=tmp; }
        PASS3_ROUND(0);PASS3_ROUND(1);PASS3_ROUND(2);PASS3_ROUND(3);PASS3_ROUND(4);PASS3_ROUND(5);PASS3_ROUND(6);PASS3_ROUND(7);
        PASS3_ROUND(8);PASS3_ROUND(9);PASS3_ROUND(10);PASS3_ROUND(11);PASS3_ROUND(12);PASS3_ROUND(13);PASS3_ROUND(14);PASS3_ROUND(15);
        PASS3_ROUND(16);PASS3_ROUND(17);PASS3_ROUND(18);PASS3_ROUND(19);PASS3_ROUND(20);PASS3_ROUND(21);PASS3_ROUND(22);PASS3_ROUND(23);
        PASS3_ROUND(24);PASS3_ROUND(25);PASS3_ROUND(26);PASS3_ROUND(27);PASS3_ROUND(28);PASS3_ROUND(29);PASS3_ROUND(30);PASS3_ROUND(31);
        #undef PASS3_ROUND

        #define PASS4_ROUND(i) FF2(t7,t6,t5,t4,t3,t2,t1,t0,W[wp4[i]],C4[i],Fphi4_5); \
            { CryptoPP::word32 tmp=t7; t7=t6; t6=t5; t5=t4; t4=t3; t3=t2; t2=t1; t1=t0; t0=tmp; }
        PASS4_ROUND(0);PASS4_ROUND(1);PASS4_ROUND(2);PASS4_ROUND(3);PASS4_ROUND(4);PASS4_ROUND(5);PASS4_ROUND(6);PASS4_ROUND(7);
        PASS4_ROUND(8);PASS4_ROUND(9);PASS4_ROUND(10);PASS4_ROUND(11);PASS4_ROUND(12);PASS4_ROUND(13);PASS4_ROUND(14);PASS4_ROUND(15);
        PASS4_ROUND(16);PASS4_ROUND(17);PASS4_ROUND(18);PASS4_ROUND(19);PASS4_ROUND(20);PASS4_ROUND(21);PASS4_ROUND(22);PASS4_ROUND(23);
        PASS4_ROUND(24);PASS4_ROUND(25);PASS4_ROUND(26);PASS4_ROUND(27);PASS4_ROUND(28);PASS4_ROUND(29);PASS4_ROUND(30);PASS4_ROUND(31);
        #undef PASS4_ROUND

        #define PASS5_ROUND(i) FF2(t7,t6,t5,t4,t3,t2,t1,t0,W[wp5[i]],C5[i],Fphi5_5); \
            { CryptoPP::word32 tmp=t7; t7=t6; t6=t5; t5=t4; t4=t3; t3=t2; t2=t1; t1=t0; t0=tmp; }
        PASS5_ROUND(0);PASS5_ROUND(1);PASS5_ROUND(2);PASS5_ROUND(3);PASS5_ROUND(4);PASS5_ROUND(5);PASS5_ROUND(6);PASS5_ROUND(7);
        PASS5_ROUND(8);PASS5_ROUND(9);PASS5_ROUND(10);PASS5_ROUND(11);PASS5_ROUND(12);PASS5_ROUND(13);PASS5_ROUND(14);PASS5_ROUND(15);
        PASS5_ROUND(16);PASS5_ROUND(17);PASS5_ROUND(18);PASS5_ROUND(19);PASS5_ROUND(20);PASS5_ROUND(21);PASS5_ROUND(22);PASS5_ROUND(23);
        PASS5_ROUND(24);PASS5_ROUND(25);PASS5_ROUND(26);PASS5_ROUND(27);PASS5_ROUND(28);PASS5_ROUND(29);PASS5_ROUND(30);PASS5_ROUND(31);
        #undef PASS5_ROUND
    }

    m_state[0] += t0; m_state[1] += t1; m_state[2] += t2; m_state[3] += t3;
    m_state[4] += t4; m_state[5] += t5; m_state[6] += t6; m_state[7] += t7;
}

template <unsigned int DIGEST_BITS, unsigned int PASSES>
void Haval<DIGEST_BITS, PASSES>::Tailor() {
    CryptoPP::word32 *f = m_state;
    if (DIGEST_BITS == 128) {
        f[0] += CryptoPP::rotrFixed((f[7]&0x000000FF)|(f[6]&0xFF000000)|(f[5]&0x00FF0000)|(f[4]&0x0000FF00), 8);
        f[1] += CryptoPP::rotrFixed((f[7]&0x0000FF00)|(f[6]&0x000000FF)|(f[5]&0xFF000000)|(f[4]&0x00FF0000), 16);
        f[2] += CryptoPP::rotrFixed((f[7]&0x00FF0000)|(f[6]&0x0000FF00)|(f[5]&0x000000FF)|(f[4]&0xFF000000), 24);
        f[3] += (f[7]&0xFF000000)|(f[6]&0x00FF0000)|(f[5]&0x0000FF00)|(f[4]&0x000000FF);
    } else if (DIGEST_BITS == 160) {
        f[0] += CryptoPP::rotrFixed((f[7]&0x3F)|(f[6]&(0x7F<<25))|(f[5]&(0x3F<<19)), 19);
        f[1] += CryptoPP::rotrFixed((f[7]&(0x3F<<6))|(f[6]&0x3F)|(f[5]&(0x7F<<25)), 25);
        f[2] += (f[7]&(0x7F<<12))|(f[6]&(0x3F<<6))|(f[5]&0x3F);
        f[3] += ((f[7]&(0x3F<<19))|(f[6]&(0x7F<<12))|(f[5]&(0x3F<<6)))>>6;
        f[4] += ((f[7]&(0x7F<<25))|(f[6]&(0x3F<<19))|(f[5]&(0x7F<<12)))>>12;
    } else if (DIGEST_BITS == 192) {
        f[0] += CryptoPP::rotrFixed((f[7]&0x1F)|(f[6]&(0x3F<<26)), 26);
        f[1] += (f[7]&(0x1F<<5))|(f[6]&0x1F);
        f[2] += ((f[7]&(0x3F<<10))|(f[6]&(0x1F<<5)))>>5;
        f[3] += ((f[7]&(0x1F<<16))|(f[6]&(0x3F<<10)))>>10;
        f[4] += ((f[7]&(0x1F<<21))|(f[6]&(0x1F<<16)))>>16;
        f[5] += ((f[7]&(0x3F<<26))|(f[6]&(0x1F<<21)))>>21;
    } else if (DIGEST_BITS == 224) {
        f[0] += (f[7]>>27)&0x1F; f[1] += (f[7]>>22)&0x1F; f[2] += (f[7]>>18)&0x0F;
        f[3] += (f[7]>>13)&0x1F; f[4] += (f[7]>>9)&0x0F; f[5] += (f[7]>>4)&0x1F; f[6] += f[7]&0x0F;
    }
}

template class Haval<128, 3>; template class Haval<160, 3>; template class Haval<192, 3>;
template class Haval<224, 3>; template class Haval<256, 3>;
template class Haval<128, 4>; template class Haval<160, 4>; template class Haval<192, 4>;
template class Haval<224, 4>; template class Haval<256, 4>;
template class Haval<128, 5>; template class Haval<160, 5>; template class Haval<192, 5>;
template class Haval<224, 5>; template class Haval<256, 5>;

#define REGISTER_HAVAL(bits, passes) \
    static HashAlgorithmRegistrar<HavalWrapper<Haval<bits, passes>>> \
        reg_haval_##bits##_##passes("HAVAL-" #bits "/Pass" #passes)

REGISTER_HAVAL(128, 3); REGISTER_HAVAL(160, 3); REGISTER_HAVAL(192, 3); REGISTER_HAVAL(224, 3); REGISTER_HAVAL(256, 3);
REGISTER_HAVAL(128, 4); REGISTER_HAVAL(160, 4); REGISTER_HAVAL(192, 4); REGISTER_HAVAL(224, 4); REGISTER_HAVAL(256, 4);
REGISTER_HAVAL(128, 5); REGISTER_HAVAL(160, 5); REGISTER_HAVAL(192, 5); REGISTER_HAVAL(224, 5); REGISTER_HAVAL(256, 5);

} // namespace impl
} // namespace core
