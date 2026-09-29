#ifndef Z80_OPCODES_DDORFD_CB_H
#define Z80_OPCODES_DDORFD_CB_H

#include "../types.h"

namespace Z80::Opcodes
{
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_B = 0x00;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_C = 0x01;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_D = 0x02;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_E = 0x03;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_H = 0x04;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_L = 0x05;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd = 0x06;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_A = 0x07;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_B = 0x08;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_C = 0x09;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_D = 0x0a;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_E = 0x0b;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_H = 0x0c;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_L = 0x0d;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd = 0x0e;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_A = 0x0f;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_B = 0x10;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_C = 0x11;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_D = 0x12;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_E = 0x13;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_H = 0x14;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_L = 0x15;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd = 0x16;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_A = 0x17;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_B = 0x18;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_C = 0x19;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_D = 0x1a;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_E = 0x1b;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_H = 0x1c;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_L = 0x1d;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd = 0x1e;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_A = 0x1f;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_B = 0x20;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_C = 0x21;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_D = 0x22;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_E = 0x23;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_H = 0x24;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_L = 0x25;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd = 0x26;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_A = 0x27;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_B = 0x28;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_C = 0x29;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_D = 0x2a;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_E = 0x2b;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_H = 0x2c;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_L = 0x2d;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd = 0x2e;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_A = 0x2f;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_B = 0x30;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_C = 0x31;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_D = 0x32;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_E = 0x33;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_H = 0x34;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_L = 0x35;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd = 0x36;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_A = 0x37;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_B = 0x38;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_C = 0x39;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_D = 0x3a;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_E = 0x3b;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_H = 0x3c;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_L = 0x3d;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd = 0x3e;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_A = 0x3f;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_B = 0x40;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_C = 0x41;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_D = 0x42;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_E = 0x43;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_H = 0x44;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_L = 0x45;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd = 0x46;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_A = 0x47;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_B = 0x48;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_C = 0x49;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_D = 0x4a;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_E = 0x4b;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_H = 0x4c;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_L = 0x4d;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd = 0x4e;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_A = 0x4f;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_B = 0x50;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_C = 0x51;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_D = 0x52;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_E = 0x53;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_H = 0x54;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_L = 0x55;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd = 0x56;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_A = 0x57;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_B = 0x58;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_C = 0x59;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_D = 0x5a;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_E = 0x5b;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_H = 0x5c;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_L = 0x5d;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd = 0x5e;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_A = 0x5f;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_B = 0x60;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_C = 0x61;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_D = 0x62;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_E = 0x63;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_H = 0x64;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_L = 0x65;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd = 0x66;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_A = 0x67;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_B = 0x68;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_C = 0x69;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_D = 0x6a;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_E = 0x6b;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_H = 0x6c;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_L = 0x6d;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd = 0x6e;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_A = 0x6f;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_B = 0x70;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_C = 0x71;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_D = 0x72;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_E = 0x73;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_H = 0x74;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_L = 0x75;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd = 0x76;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_A = 0x77;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_B = 0x78;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_C = 0x79;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_D = 0x7a;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_E = 0x7b;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_H = 0x7c;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_L = 0x7d;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd = 0x7e;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_A = 0x7f;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_B = 0x80;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_C = 0x81;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_D = 0x82;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_E = 0x83;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_H = 0x84;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_L = 0x85;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd = 0x86;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_A = 0x87;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_B = 0x88;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_C = 0x89;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_D = 0x8a;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_E = 0x8b;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_H = 0x8c;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_L = 0x8d;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd = 0x8e;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_A = 0x8f;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_B = 0x90;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_C = 0x91;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_D = 0x92;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_E = 0x93;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_H = 0x94;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_L = 0x95;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd = 0x96;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_A = 0x97;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_B = 0x98;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_C = 0x99;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_D = 0x9a;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_E = 0x9b;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_H = 0x9c;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_L = 0x9d;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd = 0x9e;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_A = 0x9f;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_B = 0xa0;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_C = 0xa1;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_D = 0xa2;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_E = 0xa3;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_H = 0xa4;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_L = 0xa5;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd = 0xa6;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_A = 0xa7;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_B = 0xa8;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_C = 0xa9;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_D = 0xaa;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_E = 0xab;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_H = 0xac;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_L = 0xad;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd = 0xae;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_A = 0xaf;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_B = 0xb0;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_C = 0xb1;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_D = 0xb2;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_E = 0xb3;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_H = 0xb4;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_L = 0xb5;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd = 0xb6;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_A = 0xb7;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_B = 0xb8;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_C = 0xb9;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_D = 0xba;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_E = 0xbb;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_H = 0xbc;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_L = 0xbd;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd = 0xbe;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_A = 0xbf;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_B = 0xc0;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_C = 0xc1;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_D = 0xc2;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_E = 0xc3;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_H = 0xc4;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_L = 0xc5;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd = 0xc6;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_A = 0xc7;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_B = 0xc8;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_C = 0xc9;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_D = 0xca;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_E = 0xcb;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_H = 0xcc;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_L = 0xcd;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd = 0xce;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_A = 0xcf;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_B = 0xd0;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_C = 0xd1;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_D = 0xd2;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_E = 0xd3;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_H = 0xd4;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_L = 0xd5;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd = 0xd6;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_A = 0xd7;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_B = 0xd8;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_C = 0xd9;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_D = 0xda;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_E = 0xdb;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_H = 0xdc;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_L = 0xdd;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd = 0xde;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_A = 0xdf;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_B = 0xe0;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_C = 0xe1;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_D = 0xe2;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_E = 0xe3;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_H = 0xe4;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_L = 0xe5;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd = 0xe6;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_A = 0xe7;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_B = 0xe8;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_C = 0xe9;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_D = 0xea;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_E = 0xeb;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_H = 0xec;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_L = 0xed;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd = 0xee;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_A = 0xef;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_B = 0xf0;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_C = 0xf1;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_D = 0xf2;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_E = 0xf3;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_H = 0xf4;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_L = 0xf5;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd = 0xf6;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_A = 0xf7;

    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_B = 0xf8;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_C = 0xf9;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_D = 0xfa;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_E = 0xfb;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_H = 0xfc;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_L = 0xfd;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd = 0xfe;
    constexpr UnsignedByte Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_A = 0xff;
}

#endif // Z80_OPCODES_DDORFD_CB_H
