#ifndef Z80INTERPRETER_H
#define Z80INTERPRETER_H

#include <vector>

#include <QString>

#include "../z80/types.h"

namespace Z80
{
    class Z80;
}

namespace Interpreter
{
    using UnsignedByte = Z80::UnsignedByte;
    using UnsignedWord = Z80::UnsignedWord;
    using Register8 = Z80::Register8;
    using Register16 = Z80::Register16;
    using Z80Cpu = Z80::Z80;

    class Z80Interpreter
    {
    public:
        enum NumberFormat
        {
            HexFormat = 0x01,
            DecimalFormat = 0x02,
            OctalFormat = 0x04,
            BinaryFormat = 0x08,
            AllFormats = 0xff
        };

        typedef int NumberFormats;

        explicit Z80Interpreter(Z80Cpu * cpu);
        virtual ~Z80Interpreter();

        [[nodiscard]] bool hasCpu() const;
        [[nodiscard]] Z80Cpu * cpu() const;

        void setCpu(Z80Cpu * cpu);
        void run();
        static void run(Z80Cpu * cpu);

    protected:
        using Opcode = std::vector<UnsignedByte>;
        using Tokens = std::vector<QString>;

        static const Opcode InvalidInstruction;

        QString readInput();

        static Tokens tokenise(const QString & input);

        // returns true if the interpreter should continue running, false if it should exit.
        bool handleInput(const QString & input);

        // dot-command methods
        void handleDotCommand(const Tokens & tokens);

        void dotHelp() const;

        void dotShowOpcodes();

        void dotHideOpcodes();

        void dotShowCosts();

        void dotHideCosts();

        void dotAutoShowFlags(const Tokens & tokens);

        void dotStatus() const;

        void dotDumpFlags() const;

        void dotDumpMemory(const Tokens & tokens) const;

        void dotDumpMemory(int low, int len = 16) const;

        void dotDumpRegisters() const;

        void dotRegisterValue(const Tokens & tokens) const;

        void dotRegisterValue(Register8 reg, const NumberFormats & fmt = AllFormats) const;

        void dotRegisterValue(Register16 reg, const NumberFormats & fmt = AllFormats) const;

        // Z80 instruction methods
        void runOpcode(const Opcode & opcode);

        static Opcode assembleInstruction(const Tokens & tokens);

        static Opcode assembleADC(const Tokens & tokens);

        static Opcode assembleADD(const Tokens & tokens);

        static Opcode assembleAND(const Tokens & tokens);

        static Opcode assembleBIT(const Tokens & tokens);

        static Opcode assembleCALL(const Tokens & tokens);

        static Opcode assembleCCF(const Tokens & tokens);

        static Opcode assembleCP(const Tokens & tokens);

        static Opcode assembleCPD(const Tokens & tokens);

        static Opcode assembleCPDR(const Tokens & tokens);

        static Opcode assembleCPI(const Tokens & tokens);

        static Opcode assembleCPIR(const Tokens & tokens);

        static Opcode assembleCPL(const Tokens & tokens);

        static Opcode assembleDAA(const Tokens & tokens);

        static Opcode assembleDEC(const Tokens & tokens);

        static Opcode assembleDI(const Tokens & tokens);

        static Opcode assembleDJNZ(const Tokens & tokens);

        static Opcode assembleEI(const Tokens & tokens);

        static Opcode assembleEX(const Tokens & tokens);

        static Opcode assembleEXX(const Tokens & tokens);

        static Opcode assembleHALT(const Tokens & tokens);

        static Opcode assembleIM(const Tokens & tokens);

        static Opcode assembleIN(const Tokens & tokens);

        static Opcode assembleINC(const Tokens & tokens);

        static Opcode assembleIND(const Tokens & tokens);

        static Opcode assembleINDR(const Tokens & tokens);

        static Opcode assembleINI(const Tokens & tokens);

        static Opcode assembleINIR(const Tokens & tokens);

        static Opcode assembleJP(const Tokens & tokens);

        static Opcode assembleJR(const Tokens & tokens);

        static Opcode assembleLD(const Tokens & tokens);

        static Opcode assembleLDD(const Tokens & tokens);

        static Opcode assembleLDDR(const Tokens & tokens);

        static Opcode assembleLDI(const Tokens & tokens);

        static Opcode assembleLDIR(const Tokens & tokens);

        static Opcode assembleNEG(const Tokens & tokens);

        static Opcode assembleNOP(const Tokens & tokens);

        static Opcode assembleOR(const Tokens & tokens);

        static Opcode assembleOUT(const Tokens & tokens);

        static Opcode assembleOUTD(const Tokens & tokens);

        static Opcode assembleOTDR(const Tokens & tokens);

        static Opcode assembleOUTI(const Tokens & tokens);

        static Opcode assembleOTIR(const Tokens & tokens);

        static Opcode assemblePOP(const Tokens & tokens);

        static Opcode assemblePUSH(const Tokens & tokens);

        static Opcode assembleRES(const Tokens & tokens);

        static Opcode assembleRET(const Tokens & tokens);

        static Opcode assembleRETI(const Tokens & tokens);

        static Opcode assembleRETN(const Tokens & tokens);

        static Opcode assembleRLA(const Tokens & tokens);

        static Opcode assembleRL(const Tokens & tokens);

        static Opcode assembleRLCA(const Tokens & tokens);

        static Opcode assembleRLC(const Tokens & tokens);

        static Opcode assembleRLD(const Tokens & tokens);

        static Opcode assembleRRA(const Tokens & tokens);

        static Opcode assembleRR(const Tokens & tokens);

        static Opcode assembleRRCA(const Tokens & tokens);

        static Opcode assembleRRC(const Tokens & tokens);

        static Opcode assembleRRD(const Tokens & tokens);

        static Opcode assembleRST(const Tokens & tokens);

        static Opcode assembleSBC(const Tokens & tokens);

        static Opcode assembleSCF(const Tokens & tokens);

        static Opcode assembleSET(const Tokens & tokens);

        static Opcode assembleSLA(const Tokens & tokens);

        static Opcode assembleSRA(const Tokens & tokens);

        static Opcode assembleSLL(const Tokens & tokens);

        static Opcode assembleSRL(const Tokens & tokens);

        static Opcode assembleSUB(const Tokens & tokens);

        static Opcode assembleXOR(const Tokens & tokens);

    private:
        void discardCpu();

        Z80Cpu * m_cpu;
        bool m_showOpcodes;
        bool m_showInstructionCost;
        bool m_autoShowFlags;
    };
}

#endif // Z80INTERPRETER_H
