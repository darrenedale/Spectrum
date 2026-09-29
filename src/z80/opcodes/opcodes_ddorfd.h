#ifndef Z80_OPCODES_DDORFD_H
#define Z80_OPCODES_DDORFD_H

#include "../types.h"

namespace Z80::Opcodes
{
    /* a large number of these opcodes replicate precisely plain opcodes. in these
     * cases, the executor for 0xdd or 0xfd opcodes should call the plain opcode
     * executor and add to the returned byte size. this saves code duplication, but
     * care will need to be taken that an infinite recursion does not take place
     * where the two execution methods continually call one another */

    constexpr UnsignedByte Z80_DdOrFd_Nop = 0x00;
    constexpr UnsignedByte Z80_DdOrFd_Ld_Bc_Nn = 0x01;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IndirectBc_A = 0x02;
    constexpr UnsignedByte Z80_DdOrFd_Inc_Bc = 0x03;
    constexpr UnsignedByte Z80_DdOrFd_Inc_B = 0x04;
    constexpr UnsignedByte Z80_DdOrFd_Dec_B = 0x05;
    constexpr UnsignedByte Z80_DdOrFd_Ld_B_N = 0x06;
    constexpr UnsignedByte Z80_DdOrFd_Rlca = 0x07;

    constexpr UnsignedByte Z80_DdOrFd_Ex_Af_AfShadow = 0x08;
    constexpr UnsignedByte Z80_DdOrFd_Add_IxOrIy_Bc = 0x09;
    constexpr UnsignedByte Z80_DdOrFd_Ld_A_IndirectBc = 0x0a;
    constexpr UnsignedByte Z80_DdOrFd_Dec_Bc = 0x0b;
    constexpr UnsignedByte Z80_DdOrFd_Inc_C = 0x0c;
    constexpr UnsignedByte Z80_DdOrFd_Dec_C = 0x0d;
    constexpr UnsignedByte Z80_DdOrFd_Ld_C_N = 0x0e;
    constexpr UnsignedByte Z80_DdOrFd_Rrca = 0x0f;

    constexpr UnsignedByte Z80_DdOrFd_Djnz_d = 0x10;
    constexpr UnsignedByte Z80_DdOrFd_Ld_De_Nn = 0x11;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IndirectDe_A = 0x12;
    constexpr UnsignedByte Z80_DdOrFd_Inc_De = 0x13;
    constexpr UnsignedByte Z80_DdOrFd_Inc_D = 0x14;
    constexpr UnsignedByte Z80_DdOrFd_Dec_D = 0x15;
    constexpr UnsignedByte Z80_DdOrFd_Ld_D_N = 0x16;
    constexpr UnsignedByte Z80_DdOrFd_Rla = 0x17;

    constexpr UnsignedByte Z80_DdOrFd_Jr_d = 0x18;
    constexpr UnsignedByte Z80_DdOrFd_Add_IxOrIy_De = 0x19;
    constexpr UnsignedByte Z80_DdOrFd_Ld_A_IndirectDe = 0x1a;
    constexpr UnsignedByte Z80_DdOrFd_Dec_De = 0x1b;
    constexpr UnsignedByte Z80_DdOrFd_Inc_E = 0x1c;
    constexpr UnsignedByte Z80_DdOrFd_Dec_E = 0x1d;
    constexpr UnsignedByte Z80_DdOrFd_Ld_E_N = 0x1e;
    constexpr UnsignedByte Z80_DdOrFd_Rra = 0x1f;

    constexpr UnsignedByte Z80_DdOrFd_Jr_Nz_d = 0x20;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IxOrIy_Nn = 0x21;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IndirectNn_IxOrIy = 0x22;
    constexpr UnsignedByte Z80_DdOrFd_Inc_IxOrIy = 0x23;
    constexpr UnsignedByte Z80_DdOrFd_Inc_IxhOrIyh = 0x24;
    constexpr UnsignedByte Z80_DdOrFd_Dec_IxhOrIyh = 0x25;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IxhOrIyh_N = 0x26;
    constexpr UnsignedByte Z80_DdOrFd_Daa = 0x27;

    constexpr UnsignedByte Z80_DdOrFd_Jr_Z_d = 0x28;
    constexpr UnsignedByte Z80_DdOrFd_Add_IxOrIy_IxOrIy = 0x29;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IxOrIy_IndirectNn = 0x2a;
    constexpr UnsignedByte Z80_DdOrFd_Dec_IxOrIy = 0x2b;
    constexpr UnsignedByte Z80_DdOrFd_Inc_IxlOrIyl = 0x2c;
    constexpr UnsignedByte Z80_DdOrFd_Dec_IxlOrIyl = 0x2d;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IxlOrIyl_N = 0x2e;
    constexpr UnsignedByte Z80_DdOrFd_Cpl = 0x2f;

    constexpr UnsignedByte Z80_DdOrFd_Jr_Nc_d = 0x30;
    constexpr UnsignedByte Z80_DdOrFd_Ld_Sp_Nn = 0x31;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IndirectNn_A = 0x32;
    constexpr UnsignedByte Z80_DdOrFd_Inc_Sp = 0x33;
    constexpr UnsignedByte Z80_DdOrFd_Inc_IndirectIxdOrIyd = 0x34;

    constexpr UnsignedByte Z80_DdOrFd_Dec_IndirectIxdOrIyd = 0x35;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IndirectIxdOrIyd_N = 0x36;
    constexpr UnsignedByte Z80_DdOrFd_Scf = 0x37;

    constexpr UnsignedByte Z80_DdOrFd_Jr_C_d = 0x38;
    constexpr UnsignedByte Z80_DdOrFd_Add_IxOrIy_Sp = 0x39;
    constexpr UnsignedByte Z80_DdOrFd_Ld_A_IndirectNn = 0x3a;
    constexpr UnsignedByte Z80_DdOrFd_Dec_Sp = 0x3b;
    constexpr UnsignedByte Z80_DdOrFd_Inc_A = 0x3c;
    constexpr UnsignedByte Z80_DdOrFd_Dec_A = 0x3d;
    constexpr UnsignedByte Z80_DdOrFd_Ld_A_N = 0x3e;
    constexpr UnsignedByte Z80_DdOrFd_Ccf = 0x3f;

    constexpr UnsignedByte Z80_DdOrFd_Ld_B_B = 0x40;
    constexpr UnsignedByte Z80_DdOrFd_Ld_B_C = 0x41;
    constexpr UnsignedByte Z80_DdOrFd_Ld_B_D = 0x42;
    constexpr UnsignedByte Z80_DdOrFd_Ld_B_E = 0x43;
    constexpr UnsignedByte Z80_DdOrFd_Ld_B_IxhOrIyh = 0x44;
    constexpr UnsignedByte Z80_DdOrFd_Ld_B_IxlOrIyl = 0x45;
    constexpr UnsignedByte Z80_DdOrFd_Ld_B_IndirectIxdOrIyd = 0x46;
    constexpr UnsignedByte Z80_DdOrFd_Ld_B_A = 0x47;

    constexpr UnsignedByte Z80_DdOrFd_Ld_C_B = 0x48;
    constexpr UnsignedByte Z80_DdOrFd_Ld_C_C = 0x49;
    constexpr UnsignedByte Z80_DdOrFd_Ld_C_D = 0x4a;
    constexpr UnsignedByte Z80_DdOrFd_Ld_C_E = 0x4b;
    constexpr UnsignedByte Z80_DdOrFd_Ld_C_IxhOrIyh = 0x4c;
    constexpr UnsignedByte Z80_DdOrFd_Ld_C_IxlOrIyl = 0x4d;
    constexpr UnsignedByte Z80_DdOrFd_Ld_C_IndirectIxdOrIyd = 0x4e;
    constexpr UnsignedByte Z80_DdOrFd_Ld_C_A = 0x4f;

    constexpr UnsignedByte Z80_DdOrFd_Ld_D_B = 0x50;
    constexpr UnsignedByte Z80_DdOrFd_Ld_D_C = 0x51;
    constexpr UnsignedByte Z80_DdOrFd_Ld_D_D = 0x52;
    constexpr UnsignedByte Z80_DdOrFd_Ld_D_E = 0x53;
    constexpr UnsignedByte Z80_DdOrFd_Ld_D_IxhOrIyh = 0x54;
    constexpr UnsignedByte Z80_DdOrFd_Ld_D_IxlOrIyl = 0x55;
    constexpr UnsignedByte Z80_DdOrFd_Ld_D_IndirectIxdOrIyd = 0x56;
    constexpr UnsignedByte Z80_DdOrFd_Ld_D_A = 0x57;

    constexpr UnsignedByte Z80_DdOrFd_Ld_E_B = 0x58;
    constexpr UnsignedByte Z80_DdOrFd_Ld_E_C = 0x59;
    constexpr UnsignedByte Z80_DdOrFd_Ld_E_D = 0x5a;
    constexpr UnsignedByte Z80_DdOrFd_Ld_E_E = 0x5b;
    constexpr UnsignedByte Z80_DdOrFd_Ld_E_IxhOrIyh = 0x5c;
    constexpr UnsignedByte Z80_DdOrFd_Ld_E_IxlOrIyl = 0x5d;
    constexpr UnsignedByte Z80_DdOrFd_Ld_E_IndirectIxdOrIyd = 0x5e;
    constexpr UnsignedByte Z80_DdOrFd_Ld_E_A = 0x5f;

    constexpr UnsignedByte Z80_DdOrFd_Ld_IxhOrIyh_B = 0x60;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IxhOrIyh_C = 0x61;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IxhOrIyh_D = 0x62;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IxhOrIyh_E = 0x63;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IxhOrIyh_IxhOrIyh = 0x64;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IxhOrIyh_IxlOrIyl = 0x65;
    constexpr UnsignedByte Z80_DdOrFd_Ld_H_IndirectIxdOrIyd = 0x66;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IxhOrIyh_A = 0x67;

    constexpr UnsignedByte Z80_DdOrFd_Ld_IxlOrIyl_B = 0x68;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IxlOrIyl_C = 0x69;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IxlOrIyl_D = 0x6a;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IxlOrIyl_E = 0x6b;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IxlOrIyl_IxhOrIyh = 0x6c;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IxlOrIyl_IxlOrIyl = 0x6d;
    constexpr UnsignedByte Z80_DdOrFd_Ld_L_IndirectIxdOrIyd = 0x6e;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IxlOrIyl_A = 0x6f;

    constexpr UnsignedByte Z80_DdOrFd_Ld_IndirectIxdOrIyd_B = 0x70;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IndirectIxdOrIyd_C = 0x71;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IndirectIxdOrIyd_D = 0x72;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IndirectIxdOrIyd_E = 0x73;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IndirectIxdOrIyd_H = 0x74;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IndirectIxdOrIyd_L = 0x75;
    constexpr UnsignedByte Z80_DdOrFd_Halt = 0x76;
    constexpr UnsignedByte Z80_DdOrFd_Ld_IndirectIxdOrIyd_A = 0x77;

    constexpr UnsignedByte Z80_DdOrFd_Ld_A_B = 0x78;
    constexpr UnsignedByte Z80_DdOrFd_Ld_A_C = 0x79;
    constexpr UnsignedByte Z80_DdOrFd_Ld_A_D = 0x7a;
    constexpr UnsignedByte Z80_DdOrFd_Ld_A_E = 0x7b;
    constexpr UnsignedByte Z80_DdOrFd_Ld_A_IxhOrIyh = 0x7c;
    constexpr UnsignedByte Z80_DdOrFd_Ld_A_IxlOrIyl = 0x7d;
    constexpr UnsignedByte Z80_DdOrFd_Ld_A_IndirectIxdOrIyd = 0x7e;
    constexpr UnsignedByte Z80_DdOrFd_Ld_A_A = 0x7f;

    constexpr UnsignedByte Z80_DdOrFd_Add_A_B = 0x80;
    constexpr UnsignedByte Z80_DdOrFd_Add_A_C = 0x81;
    constexpr UnsignedByte Z80_DdOrFd_Add_A_D = 0x82;
    constexpr UnsignedByte Z80_DdOrFd_Add_A_E = 0x83;
    constexpr UnsignedByte Z80_DdOrFd_Add_A_IxhOrIyh = 0x84;
    constexpr UnsignedByte Z80_DdOrFd_Add_A_IxlOrIyl = 0x85;
    constexpr UnsignedByte Z80_DdOrFd_Add_A_IndirectIxdOrIyd = 0x86;
    constexpr UnsignedByte Z80_DdOrFd_Add_A_A = 0x87;

    constexpr UnsignedByte Z80_DdOrFd_Adc_A_B = 0x88;
    constexpr UnsignedByte Z80_DdOrFd_Adc_A_C = 0x89;
    constexpr UnsignedByte Z80_DdOrFd_Adc_A_D = 0x8a;
    constexpr UnsignedByte Z80_DdOrFd_Adc_A_E = 0x8b;
    constexpr UnsignedByte Z80_DdOrFd_Adc_A_IxhOrIyh = 0x8c;
    constexpr UnsignedByte Z80_DdOrFd_Adc_A_IxlOrIyl = 0x8d;
    constexpr UnsignedByte Z80_DdOrFd_Adc_A_IndirectIxdOrIyd = 0x8e;
    constexpr UnsignedByte Z80_DdOrFd_Adc_A_A = 0x8f;

    constexpr UnsignedByte Z80_DdOrFd_Sub_B = 0x90;
    constexpr UnsignedByte Z80_DdOrFd_Sub_C = 0x91;
    constexpr UnsignedByte Z80_DdOrFd_Sub_D = 0x92;
    constexpr UnsignedByte Z80_DdOrFd_Sub_E = 0x93;
    constexpr UnsignedByte Z80_DdOrFd_Sub_IxhOrIyh = 0x94;
    constexpr UnsignedByte Z80_DdOrFd_Sub_IxlOrIyl = 0x95;
    constexpr UnsignedByte Z80_DdOrFd_Sub_IndirectIxdOrIyd = 0x96;
    constexpr UnsignedByte Z80_DdOrFd_Sub_A = 0x97;

    constexpr UnsignedByte Z80_DdOrFd_Sbc_A_B = 0x98;
    constexpr UnsignedByte Z80_DdOrFd_Sbc_A_C = 0x99;
    constexpr UnsignedByte Z80_DdOrFd_Sbc_A_D = 0x9a;
    constexpr UnsignedByte Z80_DdOrFd_Sbc_A_E = 0x9b;
    constexpr UnsignedByte Z80_DdOrFd_Sbc_A_IxhOrIyh = 0x9c;
    constexpr UnsignedByte Z80_DdOrFd_Sbc_A_IxlOrIyl = 0x9d;
    constexpr UnsignedByte Z80_DdOrFd_Sbc_A_IndirectIxdOrIyd = 0x9e;
    constexpr UnsignedByte Z80_DdOrFd_Sbc_A_A = 0x9f;

    constexpr UnsignedByte Z80_DdOrFd_And_B = 0xa0;
    constexpr UnsignedByte Z80_DdOrFd_And_C = 0xa1;
    constexpr UnsignedByte Z80_DdOrFd_And_D = 0xa2;
    constexpr UnsignedByte Z80_DdOrFd_And_E = 0xa3;
    constexpr UnsignedByte Z80_DdOrFd_And_IxhOrIyh = 0xa4;
    constexpr UnsignedByte Z80_DdOrFd_And_IxlOrIyl = 0xa5;
    constexpr UnsignedByte Z80_DdOrFd_And_IndirectIxdOrIyd = 0xa6;
    constexpr UnsignedByte Z80_DdOrFd_And_A = 0xa7;

    constexpr UnsignedByte Z80_DdOrFd_Xor_B = 0xa8;
    constexpr UnsignedByte Z80_DdOrFd_Xor_C = 0xa9;
    constexpr UnsignedByte Z80_DdOrFd_Xor_D = 0xaa;
    constexpr UnsignedByte Z80_DdOrFd_Xor_E = 0xab;
    constexpr UnsignedByte Z80_DdOrFd_Xor_IxhOrIyh = 0xac;
    constexpr UnsignedByte Z80_DdOrFd_Xor_IxlOrIyl = 0xad;
    constexpr UnsignedByte Z80_DdOrFd_Xor_IndirectIxdOrIyd = 0xae;
    constexpr UnsignedByte Z80_DdOrFd_Xor_A = 0xaf;

    constexpr UnsignedByte Z80_DdOrFd_Or_B = 0xb0;
    constexpr UnsignedByte Z80_DdOrFd_Or_C = 0xb1;
    constexpr UnsignedByte Z80_DdOrFd_Or_D = 0xb2;
    constexpr UnsignedByte Z80_DdOrFd_Or_E = 0xb3;
    constexpr UnsignedByte Z80_DdOrFd_Or_IxhOrIyh = 0xb4;
    constexpr UnsignedByte Z80_DdOrFd_Or_IxlOrIyl = 0xb5;
    constexpr UnsignedByte Z80_DdOrFd_Or_IndirectIxdOrIyd = 0xb6;
    constexpr UnsignedByte Z80_DdOrFd_Or_A = 0xb7;

    constexpr UnsignedByte Z80_DdOrFd_Cp_B = 0xb8;
    constexpr UnsignedByte Z80_DdOrFd_Cp_C = 0xb9;
    constexpr UnsignedByte Z80_DdOrFd_Cp_D = 0xba;
    constexpr UnsignedByte Z80_DdOrFd_Cp_E = 0xbb;
    constexpr UnsignedByte Z80_DdOrFd_Cp_IxhOrIyh = 0xbc;
    constexpr UnsignedByte Z80_DdOrFd_Cp_IxlOrIyl = 0xbd;
    constexpr UnsignedByte Z80_DdOrFd_Cp_IndirectIxdOrIyd = 0xbe;
    constexpr UnsignedByte Z80_DdOrFd_Cp_A = 0xbf;

    constexpr UnsignedByte Z80_DdOrFd_Ret_Nz = 0xc0;
    constexpr UnsignedByte Z80_DdOrFd_Pop_Bc = 0xc1;
    constexpr UnsignedByte Z80_DdOrFd_Jp_Nz_Nn = 0xc2;
    constexpr UnsignedByte Z80_DdOrFd_Jp_Nn = 0xc3;
    constexpr UnsignedByte Z80_DdOrFd_Call_Nz_Nn = 0xc4;
    constexpr UnsignedByte Z80_DdOrFd_Push_Bc = 0xc5;
    constexpr UnsignedByte Z80_DdOrFd_Add_A_N = 0xc6;
    constexpr UnsignedByte Z80_DdOrFd_Rst_00 = 0xc7;

    constexpr UnsignedByte Z80_DdOrFd_Ret_Z = 0xc8;
    constexpr UnsignedByte Z80_DdOrFd_Ret = 0xc9;
    constexpr UnsignedByte Z80_DdOrFd_Jp_Z_Nn = 0xca;
    constexpr UnsignedByte Z80_DdOrFd_Prefix_Cb = 0xcb;
    constexpr UnsignedByte Z80_DdOrFd_Call_Z_Nn = 0xcc;
    constexpr UnsignedByte Z80_DdOrFd_Call_Nn = 0xcd;
    constexpr UnsignedByte Z80_DdOrFd_Adc_A_N = 0xce;
    constexpr UnsignedByte Z80_DdOrFd_Rst_08 = 0xcf;

    constexpr UnsignedByte Z80_DdOrFd_Ret_Nc = 0xd0;
    constexpr UnsignedByte Z80_DdOrFd_Pop_De = 0xd1;
    constexpr UnsignedByte Z80_DdOrFd_Jp_Nc_Nn = 0xd2;
    constexpr UnsignedByte Z80_DdOrFd_Out_IndirectN_A = 0xd3;
    constexpr UnsignedByte Z80_DdOrFd_Call_Nc_Nn = 0xd4;
    constexpr UnsignedByte Z80_DdOrFd_Push_De = 0xd5;
    constexpr UnsignedByte Z80_DdOrFd_Sub_N = 0xd6;
    constexpr UnsignedByte Z80_DdOrFd_Rst_10 = 0xd7;

    constexpr UnsignedByte Z80_DdOrFd_Ret_C = 0xd8;
    constexpr UnsignedByte Z80_DdOrFd_Exx = 0xd9;
    constexpr UnsignedByte Z80_DdOrFd_Jp_C_Nn = 0xda;
    constexpr UnsignedByte Z80_DdOrFd_In_A_IndirectN = 0xdb;
    constexpr UnsignedByte Z80_DdOrFd_Call_C_Nn = 0xdc;
    constexpr UnsignedByte Z80_DdOrFd_Prefix_Dd = 0xdd;
    constexpr UnsignedByte Z80_DdOrFd_Sbc_A_N = 0xde;
    constexpr UnsignedByte Z80_DdOrFd_Rst_18 = 0xdf;

    constexpr UnsignedByte Z80_DdOrFd_Ret_Po = 0xe0;
    constexpr UnsignedByte Z80_DdOrFd_Pop_IxOrIy = 0xe1;
    constexpr UnsignedByte Z80_DdOrFd_Jp_Po_Nn = 0xe2;
    constexpr UnsignedByte Z80_DdOrFd_Ex_IndirectSp_IxOrIy = 0xe3;
    constexpr UnsignedByte Z80_DdOrFd_Call_Po_Nn = 0xe4;
    constexpr UnsignedByte Z80_DdOrFd_Push_IxOrIy = 0xe5;
    constexpr UnsignedByte Z80_DdOrFd_And_N = 0xe6;
    constexpr UnsignedByte Z80_DdOrFd_Rst_20 = 0xe7;

    constexpr UnsignedByte Z80_DdOrFd_Ret_Pe = 0xe8;
    constexpr UnsignedByte Z80_DdOrFd_Jp_IxOrIy = 0xe9;
    constexpr UnsignedByte Z80_DdOrFd_Jp_Pe_Nn = 0xea;
    constexpr UnsignedByte Z80_DdOrFd_Ex_De_Hl = 0xeb;
    constexpr UnsignedByte Z80_DdOrFd_Call_Pe_Nn = 0xec;
    constexpr UnsignedByte Z80_DdOrFd_Prefix_Ed = 0xed;
    constexpr UnsignedByte Z80_DdOrFd_Xor_N = 0xee;
    constexpr UnsignedByte Z80_DdOrFd_Rst_28 = 0xef;

    constexpr UnsignedByte Z80_DdOrFd_Ret_P = 0xf0;
    constexpr UnsignedByte Z80_DdOrFd_Pop_Af = 0xf1;
    constexpr UnsignedByte Z80_DdOrFd_Jp_P_Nn = 0xf2;
    constexpr UnsignedByte Z80_DdOrFd_Di = 0xf3;
    constexpr UnsignedByte Z80_DdOrFd_Call_P_Nn = 0xf4;
    constexpr UnsignedByte Z80_DdOrFd_Push_Af = 0xf5;
    constexpr UnsignedByte Z80_DdOrFd_Or_N = 0xf6;
    constexpr UnsignedByte Z80_DdOrFd_Rst_30 = 0xf7;

    constexpr UnsignedByte Z80_DdOrFd_Ret_M = 0xf8;
    constexpr UnsignedByte Z80_DdOrFd_Ld_Sp_IxOrIy = 0xf9;
    constexpr UnsignedByte Z80_DdOrFd_Jp_M_Nn = 0xfa;
    constexpr UnsignedByte Z80_DdOrFd_Ei = 0xfb;
    constexpr UnsignedByte Z80_DdOrFd_Call_M_Nn = 0xfc;
    constexpr UnsignedByte Z80_DdOrFd_Prefix_Fd = 0xfd;
    constexpr UnsignedByte Z80_DdOrFd_Cp_N = 0xfe;
    constexpr UnsignedByte Z80_DdOrFd_Rst_38 = 0xff;
}

#endif // Z80_OPCODES_DDORFD_H
