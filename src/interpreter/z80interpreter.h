#ifndef Z80INTERPRETER_H
#define Z80INTERPRETER_H

#include "../z80/z80.h"
#include <QString>
#include <QStringList>
#include <QVector>
#include <QRegExp>

namespace Interpreter
{
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

        Z80Interpreter(Z80::Z80 * cpu);

        virtual ~Z80Interpreter();

        bool hasCpu() const;

        Z80::Z80 * cpu() const;

        void setCpu(Z80::Z80 * cpu);

        void run();

        static void run(Z80::Z80 * cpu);

    protected:
        typedef QVector<Z80::UnsignedByte> Opcode;

        static const Opcode InvalidInstruction;

        QString readInput();

        static QStringList tokenise(const QString & input);

        /* returns true if the interpreter should continue running, false if it
         * should exit. */
        bool handleInput(const QString & input);

        /* dot-command methods */
        void handleDotCommand(const QStringList & tokens);

        void dotHelp() const;

        void dotShowOpcodes();

        void dotHideOpcodes();

        void dotShowCosts();

        void dotHideCosts();

        void dotAutoShowFlags(const QStringList & tokens);

        void dotStatus() const;

        void dotDumpFlags() const;

        void dotDumpMemory(const QStringList & tokens) const;

        void dotDumpMemory(int low, int len = 16) const;

        void dotDumpRegisters() const;

        void dotRegisterValue(const QStringList & tokens) const;

        void dotRegisterValue(const Z80::Z80::Register8 reg, const NumberFormats & fmt = AllFormats) const;

        void dotRegisterValue(const Z80::Z80::Register16 reg, const NumberFormats & fmt = AllFormats) const;

        /* Z80 instruction methods */
        void runOpcode(const Opcode & opcode);

        static Opcode assembleInstruction(const QStringList & tokens);

        static Opcode assembleADC(const QStringList & tokens);

        static Opcode assembleADD(const QStringList & tokens);

        static Opcode assembleAND(const QStringList & tokens);

        static Opcode assembleBIT(const QStringList & tokens);

        static Opcode assembleCALL(const QStringList & tokens);

        static Opcode assembleCCF(const QStringList & tokens);

        static Opcode assembleCP(const QStringList & tokens);

        static Opcode assembleCPD(const QStringList & tokens);

        static Opcode assembleCPDR(const QStringList & tokens);

        static Opcode assembleCPI(const QStringList & tokens);

        static Opcode assembleCPIR(const QStringList & tokens);

        static Opcode assembleCPL(const QStringList & tokens);

        static Opcode assembleDAA(const QStringList & tokens);

        static Opcode assembleDEC(const QStringList & tokens);

        static Opcode assembleDI(const QStringList & tokens);

        static Opcode assembleDJNZ(const QStringList & tokens);

        static Opcode assembleEI(const QStringList & tokens);

        static Opcode assembleEX(const QStringList & tokens);

        static Opcode assembleEXX(const QStringList & tokens);

        static Opcode assembleHALT(const QStringList & tokens);

        static Opcode assembleIM(const QStringList & tokens);

        static Opcode assembleIN(const QStringList & tokens);

        static Opcode assembleINC(const QStringList & tokens);

        static Opcode assembleIND(const QStringList & tokens);

        static Opcode assembleINDR(const QStringList & tokens);

        static Opcode assembleINI(const QStringList & tokens);

        static Opcode assembleINIR(const QStringList & tokens);

        static Opcode assembleJP(const QStringList & tokens);

        static Opcode assembleJR(const QStringList & tokens);

        static Opcode assembleLD(const QStringList & tokens);

        static Opcode assembleLDD(const QStringList & tokens);

        static Opcode assembleLDDR(const QStringList & tokens);

        static Opcode assembleLDI(const QStringList & tokens);

        static Opcode assembleLDIR(const QStringList & tokens);

        static Opcode assembleNEG(const QStringList & tokens);

        static Opcode assembleNOP(const QStringList & tokens);

        static Opcode assembleOR(const QStringList & tokens);

        static Opcode assembleOUT(const QStringList & tokens);

        static Opcode assembleOUTD(const QStringList & tokens);

        static Opcode assembleOTDR(const QStringList & tokens);

        static Opcode assembleOUTI(const QStringList & tokens);

        static Opcode assembleOTIR(const QStringList & tokens);

        static Opcode assemblePOP(const QStringList & tokens);

        static Opcode assemblePUSH(const QStringList & tokens);

        static Opcode assembleRES(const QStringList & tokens);

        static Opcode assembleRET(const QStringList & tokens);

        static Opcode assembleRETI(const QStringList & tokens);

        static Opcode assembleRETN(const QStringList & tokens);

        static Opcode assembleRLA(const QStringList & tokens);

        static Opcode assembleRL(const QStringList & tokens);

        static Opcode assembleRLCA(const QStringList & tokens);

        static Opcode assembleRLC(const QStringList & tokens);

        static Opcode assembleRLD(const QStringList & tokens);

        static Opcode assembleRRA(const QStringList & tokens);

        static Opcode assembleRR(const QStringList & tokens);

        static Opcode assembleRRCA(const QStringList & tokens);

        static Opcode assembleRRC(const QStringList & tokens);

        static Opcode assembleRRD(const QStringList & tokens);

        static Opcode assembleRST(const QStringList & tokens);

        static Opcode assembleSBC(const QStringList & tokens);

        static Opcode assembleSCF(const QStringList & tokens);

        static Opcode assembleSET(const QStringList & tokens);

        static Opcode assembleSLA(const QStringList & tokens);

        static Opcode assembleSRA(const QStringList & tokens);

        static Opcode assembleSLL(const QStringList & tokens);

        static Opcode assembleSRL(const QStringList & tokens);

        static Opcode assembleSUB(const QStringList & tokens);

        static Opcode assembleXOR(const QStringList & tokens);

    private:
        void discardCpu();

        Z80::Z80 * m_cpu;
        bool m_showOpcodes, m_showInstructionCost, m_autoShowFlags;
        QStringList m_inputHistory;
    };
}

#endif // Z80INTERPRETER_H
