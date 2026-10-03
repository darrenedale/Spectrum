#ifndef Z80O_OPCODES_PLAIN_H
#define Z80O_OPCODES_PLAIN_H

#include "../types.h"

namespace Z80::Opcodes
{
    /* plain opcodes */
    constexpr UnsignedByte Z80_Plain_Nop = 0x00;
    constexpr UnsignedByte Z80_Plain_Ld_Bc_Nn = 0x01;
    constexpr UnsignedByte Z80_Plain_Ld_IndirectBc_A = 0x02;
    constexpr UnsignedByte Z80_Plain_Inc_Bc = 0x03;
    constexpr UnsignedByte Z80_Plain_Inc_B = 0x04;
    constexpr UnsignedByte Z80_Plain_Dec_B = 0x05;
    constexpr UnsignedByte Z80_Plain_Ld_B_N = 0x06;
    constexpr UnsignedByte Z80_Plain_Rlca = 0x07;

    constexpr UnsignedByte Z80_Plain_Ex_Af_AfShadow = 0x08;
    constexpr UnsignedByte Z80_Plain_Add_Hl_Bc = 0x09;
    constexpr UnsignedByte Z80_Plain_Ld_A_IndirectBc = 0x0a;
    constexpr UnsignedByte Z80_Plain_Dec_Bc = 0x0b;
    constexpr UnsignedByte Z80_Plain_Inc_C = 0x0c;
    constexpr UnsignedByte Z80_Plain_Dec_C = 0x0d;
    constexpr UnsignedByte Z80_Plain_Ld_C_N = 0x0e;
    constexpr UnsignedByte Z80_Plain_Rrca = 0x0f;

    constexpr UnsignedByte Z80_Plain_Djnz_d = 0x10;
    constexpr UnsignedByte Z80_Plain_Ld_De_Nn = 0x11;
    constexpr UnsignedByte Z80_Plain_Ld_IndirectDe_A = 0x12;
    constexpr UnsignedByte Z80_Plain_Inc_De = 0x13;
    constexpr UnsignedByte Z80_Plain_Inc_D = 0x14;
    constexpr UnsignedByte Z80_Plain_Dec_D = 0x15;
    constexpr UnsignedByte Z80_Plain_Ld_D_N = 0x16;
    constexpr UnsignedByte Z80_Plain_Rla = 0x17;

    constexpr UnsignedByte Z80_Plain_Jr_d = 0x18;
    constexpr UnsignedByte Z80_Plain_Add_Hl_De = 0x19;
    constexpr UnsignedByte Z80_Plain_Ld_A_IndirectDe = 0x1a;
    constexpr UnsignedByte Z80_Plain_Dec_De = 0x1b;
    constexpr UnsignedByte Z80_Plain_Inc_E = 0x1c;
    constexpr UnsignedByte Z80_Plain_Dec_E = 0x1d;
    constexpr UnsignedByte Z80_Plain_Ld_E_N = 0x1e;
    constexpr UnsignedByte Z80_Plain_Rra = 0x1f;

    constexpr UnsignedByte Z80_Plain_Jr_Nz_d = 0x20;
    constexpr UnsignedByte Z80_Plain_Ld_Hl_Nn = 0x21;
    constexpr UnsignedByte Z80_Plain_Ld_IndirectNn_Hl = 0x22;
    constexpr UnsignedByte Z80_Plain_Inc_Hl = 0x23;
    constexpr UnsignedByte Z80_Plain_Inc_H = 0x24;
    constexpr UnsignedByte Z80_Plain_Dec_H = 0x25;
    constexpr UnsignedByte Z80_Plain_Ld_H_N = 0x26;
    constexpr UnsignedByte Z80_Plain_Daa = 0x27;

    constexpr UnsignedByte Z80_Plain_Jr_Z_d = 0x28;
    constexpr UnsignedByte Z80_Plain_Add_Hl_Hl = 0x29;
    constexpr UnsignedByte Z80_Plain_Ld_Hl_IndirectNn = 0x2a;
    constexpr UnsignedByte Z80_Plain_Dec_Hl = 0x2b;
    constexpr UnsignedByte Z80_Plain_Inc_L = 0x2c;
    constexpr UnsignedByte Z80_Plain_Dec_L = 0x2d;
    constexpr UnsignedByte Z80_Plain_Ld_L_N = 0x2e;
    constexpr UnsignedByte Z80_Plain_Cpl = 0x2f;

    constexpr UnsignedByte Z80_Plain_Jr_Nc_d = 0x30;
    constexpr UnsignedByte Z80_Plain_Ld_Sp_Nn = 0x31;
    constexpr UnsignedByte Z80_Plain_Ld_IndirectNn_A = 0x32;
    constexpr UnsignedByte Z80_Plain_Inc_Sp = 0x33;
    constexpr UnsignedByte Z80_Plain_Inc_IndirectHl = 0x34;
    constexpr UnsignedByte Z80_Plain_Dec_IndirectHl = 0x35;
    constexpr UnsignedByte Z80_Plain_Ld_IndirectHl_N = 0x36;
    constexpr UnsignedByte Z80_Plain_Scf = 0x37;

    constexpr UnsignedByte Z80_Plain_Jr_C_d = 0x38;
    constexpr UnsignedByte Z80_Plain_Add_Hl_Sp = 0x39;
    constexpr UnsignedByte Z80_Plain_Ld_A_IndirectNn = 0x3a;
    constexpr UnsignedByte Z80_Plain_Dec_Sp = 0x3b;
    constexpr UnsignedByte Z80_Plain_Inc_A = 0x3c;
    constexpr UnsignedByte Z80_Plain_Dec_A = 0x3d;
    constexpr UnsignedByte Z80_Plain_Ld_A_N = 0x3e;
    constexpr UnsignedByte Z80_Plain_Ccf = 0x3f;

    constexpr UnsignedByte Z80_Plain_Ld_B_B = 0x40;
    constexpr UnsignedByte Z80_Plain_Ld_B_C = 0x41;
    constexpr UnsignedByte Z80_Plain_Ld_B_D = 0x42;
    constexpr UnsignedByte Z80_Plain_Ld_B_E = 0x43;
    constexpr UnsignedByte Z80_Plain_Ld_B_H = 0x44;
    constexpr UnsignedByte Z80_Plain_Ld_B_L = 0x45;
    constexpr UnsignedByte Z80_Plain_Ld_B_IndirectHl = 0x46;
    constexpr UnsignedByte Z80_Plain_Ld_B_A = 0x47;

    constexpr UnsignedByte Z80_Plain_Ld_C_B = 0x48;
    constexpr UnsignedByte Z80_Plain_Ld_C_C = 0x49;
    constexpr UnsignedByte Z80_Plain_Ld_C_D = 0x4a;
    constexpr UnsignedByte Z80_Plain_Ld_C_E = 0x4b;
    constexpr UnsignedByte Z80_Plain_Ld_C_H = 0x4c;
    constexpr UnsignedByte Z80_Plain_Ld_C_L = 0x4d;
    constexpr UnsignedByte Z80_Plain_Ld_C_IndirectHl = 0x4e;
    constexpr UnsignedByte Z80_Plain_Ld_C_A = 0x4f;

    constexpr UnsignedByte Z80_Plain_Ld_D_B = 0x50;
    constexpr UnsignedByte Z80_Plain_Ld_D_C = 0x51;
    constexpr UnsignedByte Z80_Plain_Ld_D_D = 0x52;
    constexpr UnsignedByte Z80_Plain_Ld_D_E = 0x53;
    constexpr UnsignedByte Z80_Plain_Ld_D_H = 0x54;
    constexpr UnsignedByte Z80_Plain_Ld_D_L = 0x55;
    constexpr UnsignedByte Z80_Plain_Ld_D_IndirectHl = 0x56;
    constexpr UnsignedByte Z80_Plain_Ld_D_A = 0x57;

    constexpr UnsignedByte Z80_Plain_Ld_E_B = 0x58;
    constexpr UnsignedByte Z80_Plain_Ld_E_C = 0x59;
    constexpr UnsignedByte Z80_Plain_Ld_E_D = 0x5a;
    constexpr UnsignedByte Z80_Plain_Ld_E_E = 0x5b;
    constexpr UnsignedByte Z80_Plain_Ld_E_H = 0x5c;
    constexpr UnsignedByte Z80_Plain_Ld_E_L = 0x5d;
    constexpr UnsignedByte Z80_Plain_Ld_E_IndirectHl = 0x5e;
    constexpr UnsignedByte Z80_Plain_Ld_E_A = 0x5f;

    constexpr UnsignedByte Z80_Plain_Ld_H_B = 0x60;
    constexpr UnsignedByte Z80_Plain_Ld_H_C = 0x61;
    constexpr UnsignedByte Z80_Plain_Ld_H_D = 0x62;
    constexpr UnsignedByte Z80_Plain_Ld_H_E = 0x63;
    constexpr UnsignedByte Z80_Plain_Ld_H_H = 0x64;
    constexpr UnsignedByte Z80_Plain_Ld_H_L = 0x65;
    constexpr UnsignedByte Z80_Plain_Ld_H_IndirectHl = 0x66;
    constexpr UnsignedByte Z80_Plain_Ld_H_A = 0x67;

    constexpr UnsignedByte Z80_Plain_Ld_L_B = 0x68;
    constexpr UnsignedByte Z80_Plain_Ld_L_C = 0x69;
    constexpr UnsignedByte Z80_Plain_Ld_L_D = 0x6a;
    constexpr UnsignedByte Z80_Plain_Ld_L_E = 0x6b;
    constexpr UnsignedByte Z80_Plain_Ld_L_H = 0x6c;
    constexpr UnsignedByte Z80_Plain_Ld_L_L = 0x6d;
    constexpr UnsignedByte Z80_Plain_Ld_L_IndirectHl = 0x6e;
    constexpr UnsignedByte Z80_Plain_Ld_L_A = 0x6f;

    constexpr UnsignedByte Z80_Plain_Ld_IndirectHl_B = 0x70;
    constexpr UnsignedByte Z80_Plain_Ld_IndirectHl_C = 0x71;
    constexpr UnsignedByte Z80_Plain_Ld_IndirectHl_D = 0x72;
    constexpr UnsignedByte Z80_Plain_Ld_IndirectHl_E = 0x73;
    constexpr UnsignedByte Z80_Plain_Ld_IndirectHl_H = 0x74;
    constexpr UnsignedByte Z80_Plain_Ld_IndirectHl_L = 0x75;
    constexpr UnsignedByte Z80_Plain_Halt = 0x76;
    constexpr UnsignedByte Z80_Plain_Ld_IndirectHl_A = 0x77;

    constexpr UnsignedByte Z80_Plain_Ld_A_B = 0x78;
    constexpr UnsignedByte Z80_Plain_Ld_A_C = 0x79;
    constexpr UnsignedByte Z80_Plain_Ld_A_D = 0x7a;
    constexpr UnsignedByte Z80_Plain_Ld_A_E = 0x7b;
    constexpr UnsignedByte Z80_Plain_Ld_A_H = 0x7c;
    constexpr UnsignedByte Z80_Plain_Ld_A_L = 0x7d;
    constexpr UnsignedByte Z80_Plain_Ld_A_IndirectHl = 0x7e;
    constexpr UnsignedByte Z80_Plain_Ld_A_A = 0x7f;

    constexpr UnsignedByte Z80_Plain_Add_A_B = 0x80;
    constexpr UnsignedByte Z80_Plain_Add_A_C = 0x81;
    constexpr UnsignedByte Z80_Plain_Add_A_D = 0x82;
    constexpr UnsignedByte Z80_Plain_Add_A_E = 0x83;
    constexpr UnsignedByte Z80_Plain_Add_A_H = 0x84;
    constexpr UnsignedByte Z80_Plain_Add_A_L = 0x85;
    constexpr UnsignedByte Z80_Plain_Add_A_IndirectHl = 0x86;
    constexpr UnsignedByte Z80_Plain_Add_A_A = 0x87;

    constexpr UnsignedByte Z80_Plain_Adc_A_B = 0x88;
    constexpr UnsignedByte Z80_Plain_Adc_A_C = 0x89;
    constexpr UnsignedByte Z80_Plain_Adc_A_D = 0x8a;
    constexpr UnsignedByte Z80_Plain_Adc_A_E = 0x8b;
    constexpr UnsignedByte Z80_Plain_Adc_A_H = 0x8c;
    constexpr UnsignedByte Z80_Plain_Adc_A_L = 0x8d;
    constexpr UnsignedByte Z80_Plain_Adc_A_IndirectHl = 0x8e;
    constexpr UnsignedByte Z80_Plain_Adc_A_A = 0x8f;

    constexpr UnsignedByte Z80_Plain_Sub_B = 0x90;
    constexpr UnsignedByte Z80_Plain_Sub_C = 0x91;
    constexpr UnsignedByte Z80_Plain_Sub_D = 0x92;
    constexpr UnsignedByte Z80_Plain_Sub_E = 0x93;
    constexpr UnsignedByte Z80_Plain_Sub_H = 0x94;
    constexpr UnsignedByte Z80_Plain_Sub_L = 0x95;
    constexpr UnsignedByte Z80_Plain_Sub_IndirectHl = 0x96;
    constexpr UnsignedByte Z80_Plain_Sub_A = 0x97;

    constexpr UnsignedByte Z80_Plain_Sbc_A_B = 0x98;
    constexpr UnsignedByte Z80_Plain_Sbc_A_C = 0x99;
    constexpr UnsignedByte Z80_Plain_Sbc_A_D = 0x9a;
    constexpr UnsignedByte Z80_Plain_Sbc_A_E = 0x9b;
    constexpr UnsignedByte Z80_Plain_Sbc_A_H = 0x9c;
    constexpr UnsignedByte Z80_Plain_Sbc_A_L = 0x9d;
    constexpr UnsignedByte Z80_Plain_Sbc_A_IndirectHl = 0x9e;
    constexpr UnsignedByte Z80_Plain_Sbc_A_A = 0x9f;

    constexpr UnsignedByte Z80_Plain_And_B = 0xa0;
    constexpr UnsignedByte Z80_Plain_And_C = 0xa1;
    constexpr UnsignedByte Z80_Plain_And_D = 0xa2;
    constexpr UnsignedByte Z80_Plain_And_E = 0xa3;
    constexpr UnsignedByte Z80_Plain_And_H = 0xa4;
    constexpr UnsignedByte Z80_Plain_And_L = 0xa5;
    constexpr UnsignedByte Z80_Plain_And_IndirectHl = 0xa6;
    constexpr UnsignedByte Z80_Plain_And_A = 0xa7;

    constexpr UnsignedByte Z80_Plain_Xor_B = 0xa8;
    constexpr UnsignedByte Z80_Plain_Xor_C = 0xa9;
    constexpr UnsignedByte Z80_Plain_Xor_D = 0xaa;
    constexpr UnsignedByte Z80_Plain_Xor_E = 0xab;
    constexpr UnsignedByte Z80_Plain_Xor_H = 0xac;
    constexpr UnsignedByte Z80_Plain_Xor_L = 0xad;
    constexpr UnsignedByte Z80_Plain_Xor_IndirectHl = 0xae;
    constexpr UnsignedByte Z80_Plain_Xor_A = 0xaf;

    constexpr UnsignedByte Z80_Plain_Or_B = 0xb0;
    constexpr UnsignedByte Z80_Plain_Or_C = 0xb1;
    constexpr UnsignedByte Z80_Plain_Or_D = 0xb2;
    constexpr UnsignedByte Z80_Plain_Or_E = 0xb3;
    constexpr UnsignedByte Z80_Plain_Or_H = 0xb4;
    constexpr UnsignedByte Z80_Plain_Or_L = 0xb5;
    constexpr UnsignedByte Z80_Plain_Or_IndirectHl = 0xb6;
    constexpr UnsignedByte Z80_Plain_Or_A = 0xb7;

    constexpr UnsignedByte Z80_Plain_Cp_B = 0xb8;
    constexpr UnsignedByte Z80_Plain_Cp_C = 0xb9;
    constexpr UnsignedByte Z80_Plain_Cp_D = 0xba;
    constexpr UnsignedByte Z80_Plain_Cp_E = 0xbb;
    constexpr UnsignedByte Z80_Plain_Cp_H = 0xbc;
    constexpr UnsignedByte Z80_Plain_Cp_L = 0xbd;
    constexpr UnsignedByte Z80_Plain_Cp_IndirectHl = 0xbe;
    constexpr UnsignedByte Z80_Plain_Cp_A = 0xbf;

    constexpr UnsignedByte Z80_Plain_Ret_Nz = 0xc0;
    constexpr UnsignedByte Z80_Plain_Pop_Bc = 0xc1;
    constexpr UnsignedByte Z80_Plain_Jp_Nz_Nn = 0xc2;
    constexpr UnsignedByte Z80_Plain_Jp_Nn = 0xc3;
    constexpr UnsignedByte Z80_Plain_Call_Nz_Nn = 0xc4;
    constexpr UnsignedByte Z80_Plain_Push_Bc = 0xc5;
    constexpr UnsignedByte Z80_Plain_Add_A_N = 0xc6;
    constexpr UnsignedByte Z80_Plain_Rst_00 = 0xc7;

    constexpr UnsignedByte Z80_Plain_Ret_Z = 0xc8;
    constexpr UnsignedByte Z80_Plain_Ret = 0xc9;
    constexpr UnsignedByte Z80_Plain_Jp_Z_Nn = 0xca;
    constexpr UnsignedByte Z80_Plain_Prefix_Cb = 0xcb;
    constexpr UnsignedByte Z80_Plain_Call_Z_Nn = 0xcc;
    constexpr UnsignedByte Z80_Plain_Call_Nn = 0xcd;
    constexpr UnsignedByte Z80_Plain_Adc_A_N = 0xce;
    constexpr UnsignedByte Z80_Plain_Rst_08 = 0xcf;

    constexpr UnsignedByte Z80_Plain_Ret_Nc = 0xd0;
    constexpr UnsignedByte Z80_Plain_Pop_De = 0xd1;
    constexpr UnsignedByte Z80_Plain_Jp_Nc_Nn = 0xd2;
    constexpr UnsignedByte Z80_Plain_Out_IndirectN_A = 0xd3;
    constexpr UnsignedByte Z80_Plain_Call_Nc_Nn = 0xd4;
    constexpr UnsignedByte Z80_Plain_Push_De = 0xd5;
    constexpr UnsignedByte Z80_Plain_Sub_N = 0xd6;
    constexpr UnsignedByte Z80_Plain_Rst_10 = 0xd7;

    constexpr UnsignedByte Z80_Plain_Ret_C = 0xd8;
    constexpr UnsignedByte Z80_Plain_Exx = 0xd9;
    constexpr UnsignedByte Z80_Plain_Jp_C_Nn = 0xda;
    constexpr UnsignedByte Z80_Plain_In_A_IndirectN = 0xdb;
    constexpr UnsignedByte Z80_Plain_Call_C_Nn = 0xdc;
    constexpr UnsignedByte Z80_Plain_Prefix_Dd = 0xdd;
    constexpr UnsignedByte Z80_Plain_Sbc_A_N = 0xde;
    constexpr UnsignedByte Z80_Plain_Rst_18 = 0xdf;

    constexpr UnsignedByte Z80_Plain_Ret_Po = 0xe0;
    constexpr UnsignedByte Z80_Plain_Pop_Hl = 0xe1;
    constexpr UnsignedByte Z80_Plain_Jp_Po_Nn = 0xe2;
    constexpr UnsignedByte Z80_Plain_Ex_IndirectSp_Hl = 0xe3;
    constexpr UnsignedByte Z80_Plain_Call_Po_Nn = 0xe4;
    constexpr UnsignedByte Z80_Plain_Push_Hl = 0xe5;
    constexpr UnsignedByte Z80_Plain_And_N = 0xe6;
    constexpr UnsignedByte Z80_Plain_Rst_20 = 0xe7;

    constexpr UnsignedByte Z80_Plain_Ret_Pe = 0xe8;
    constexpr UnsignedByte Z80_Plain_Jp_IndirectHl = 0xe9;
    constexpr UnsignedByte Z80_Plain_Jp_Pe_Nn = 0xea;
    constexpr UnsignedByte Z80_Plain_Ex_De_Hl = 0xeb;
    constexpr UnsignedByte Z80_Plain_Call_Pe_Nn = 0xec;
    constexpr UnsignedByte Z80_Plain_Prefix_Ed = 0xed;
    constexpr UnsignedByte Z80_Plain_Xor_N = 0xee;
    constexpr UnsignedByte Z80_Plain_Rst_28 = 0xef;

    constexpr UnsignedByte Z80_Plain_Ret_P = 0xf0;
    constexpr UnsignedByte Z80_Plain_Pop_Af = 0xf1;
    constexpr UnsignedByte Z80_Plain_Jp_P_Nn = 0xf2;
    constexpr UnsignedByte Z80_Plain_Di = 0xf3;
    constexpr UnsignedByte Z80_Plain_Call_P_Nn = 0xf4;
    constexpr UnsignedByte Z80_Plain_Push_Af = 0xf5;
    constexpr UnsignedByte Z80_Plain_Or_N = 0xf6;
    constexpr UnsignedByte Z80_Plain_Rst_30 = 0xf7;
    constexpr UnsignedByte Z80_Plain_Ret_M = 0xf8;
    constexpr UnsignedByte Z80_Plain_Ld_Sp_Hl = 0xf9;
    constexpr UnsignedByte Z80_Plain_Jp_M_Nn = 0xfa;
    constexpr UnsignedByte Z80_Plain_Ei = 0xfb;
    constexpr UnsignedByte Z80_Plain_Call_M_Nn = 0xfc;
    constexpr UnsignedByte Z80_Plain_Prefix_Fd = 0xfd;
    constexpr UnsignedByte Z80_Plain_Cp_N = 0xfe;
    constexpr UnsignedByte Z80_Plain_Rst_38 = 0xff;
}

#endif // Z80O_OPCODES_PLAIN_H
