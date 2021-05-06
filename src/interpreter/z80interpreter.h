#ifndef INTERPRETER_Z80INTERPRETER_H
#define INTERPRETER_Z80INTERPRETER_H

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

    /**
     * An interpreter for Z80 instructions.
     *
     * Given a line of user input, an instance of this class will assemble it as a Z80 machine code instruction and execute it. It provides a number of
     * "dot-commands" - commands to control the interpreter, all of which are prefixed with a single '.' dot character to differentiate them from Z80 assembler.
     */
    class Z80Interpreter
    {
    public:
        /**
         * Enumeration of supported number literal formats.
         */
        enum NumberFormat
        {
            HexFormat = 0x01,
            DecimalFormat = 0x02,
            OctalFormat = 0x04,
            BinaryFormat = 0x08,
            AllFormats = 0xff
        };

        typedef int NumberFormats;

        /**
         * Initialise a new interpreter with a CPU instance.
         *
         * @param cpu The Z80 CPU to use to execute instructions.
         */
        explicit Z80Interpreter(Z80Cpu * cpu);

        /**
         * Destructor.
         */
        virtual ~Z80Interpreter();

        [[nodiscard]] bool hasCpu() const;
        [[nodiscard]] Z80Cpu * cpu() const;

        void setCpu(Z80Cpu * cpu);
        void run();
        static void run(Z80Cpu * cpu);

    protected:
        /**
         * Type alias for a set of machine code bytes for a single instruction.
         */
        using Opcode = std::vector<UnsignedByte>;
        
        /**
         * Type alias for a stream of tokens read from the provided input.
         */
        using Tokens = std::vector<QString>;

        static const Opcode InvalidInstruction;

        /**
         * Fetch the input from the command-line.
         *
         * @return The input.
         */
        QString readInput();

        /**
         * Tokenise a string of input into a token stream.
         * 
         * @param input The string input.
         * 
         * @return The set of tokens.
         */
        static Tokens tokenise(const QString & input);

        // returns true if the interpreter should continue running, false if it should exit.
        bool handleInput(const QString & input);

        /**
         * Dispatcher for .dot commands.
         *
         * @param tokens The stream of tokens read from the initial input.
         */
        void handleDotCommand(const Tokens & tokens);

        /**
         * Helper to execute the .help command.
         */
        void dotHelp() const;

        /**
         * Helper to execute the .showopcodes command.
         */
        void dotShowOpcodes();

        /**
         * Helper to execute the .hideopcodes command.
         */
        void dotHideOpcodes();

        /**
         * Helper to execute the .showcosts command.
         */
        void dotShowCosts();

        /**
         * Helper to execute the .hidecosts command.
         */
        void dotHideCosts();

        /**
         * Helper to execute the .autoflags command.
         */
        void dotAutoShowFlags(const Tokens & tokens);

        /**
         * Helper to execute the .status command.
         */
        void dotStatus() const;

        /**
         * Helper to execute the .flags/.dumpflags command.
         */
        void dotDumpFlags() const;

        /**
         * Helper to execute the .ram/.dumpram command.
         */
        void dotDumpMemory(const Tokens & tokens) const;

        /**
         * Helper to execute the .ram/.dumpram command.
         */
        void dotDumpMemory(int low, int len = 16) const;

        /**
         * Helper to execute the .regs/.dumpregisters command.
         */
        void dotDumpRegisters() const;

        /**
         * Helper to execute the .rv/.regvalue/.registervalue command.
         */
        void dotRegisterValue(const Tokens & tokens) const;

        /**
         * Helper to execute the .rv/.regvalue/.registervalue command.
         */
        void dotRegisterValue(Register8 reg, const NumberFormats & fmt = AllFormats) const;

        /**
         * Helper to execute the .rv/.regvalue/.registervalue command.
         */
        void dotRegisterValue(Register16 reg, const NumberFormats & fmt = AllFormats) const;

        /**
         * Helper to run a single instruction on the Z80 CPU.
         * 
         * @param opcode The instruction to run.
         */
        void runOpcode(const Opcode & opcode);

        /**
         * Helper to dispatch a token stream for assembly.
         * 
         * @param tokens The token stream to assemble.
         * 
         * @return The assembled machine code.
         */
        static Opcode assembleInstruction(const Tokens & tokens);

        /**
         * Helper to assemble an ADC instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleADC(const Tokens & tokens);

        /**
         * Helper to assemble an ADC instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleADD(const Tokens & tokens);

        /**
         * Helper to assemble an AND instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleAND(const Tokens & tokens);

        /**
         * Helper to assemble a BIT instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleBIT(const Tokens & tokens);

        /**
         * Helper to assemble a CALL instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleCALL(const Tokens & tokens);

        /**
         * Helper to assemble a CCF instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleCCF(const Tokens & tokens);

        /**
         * Helper to assemble a CP instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleCP(const Tokens & tokens);

        /**
         * Helper to assemble a CPD instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleCPD(const Tokens & tokens);

        /**
         * Helper to assemble a CPDR instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleCPDR(const Tokens & tokens);

        /**
         * Helper to assemble a CPI instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleCPI(const Tokens & tokens);

        /**
         * Helper to assemble a CPIR instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleCPIR(const Tokens & tokens);

        /**
         * Helper to assemble a CPL instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleCPL(const Tokens & tokens);

        /**
         * Helper to assemble a DAA instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleDAA(const Tokens & tokens);

        /**
         * Helper to assemble a DEC instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleDEC(const Tokens & tokens);
        /**
         * Helper to assemble a DI instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */

        static Opcode assembleDI(const Tokens & tokens);
        /**
         * Helper to assemble a DJNZ instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */

        static Opcode assembleDJNZ(const Tokens & tokens);

        /**
         * Helper to assemble an EI instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleEI(const Tokens & tokens);

        /**
         * Helper to assemble an EX instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleEX(const Tokens & tokens);

        /**
         * Helper to assemble an EXX instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleEXX(const Tokens & tokens);

        /**
         * Helper to assemble a HALT instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleHALT(const Tokens & tokens);

        /**
         * Helper to assemble an IM instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleIM(const Tokens & tokens);

        /**
         * Helper to assemble an IN instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleIN(const Tokens & tokens);

        /**
         * Helper to assemble an INC instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleINC(const Tokens & tokens);

        /**
         * Helper to assemble an IND instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleIND(const Tokens & tokens);

        /**
         * Helper to assemble an INDR instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleINDR(const Tokens & tokens);

        /**
         * Helper to assemble an INI instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleINI(const Tokens & tokens);

        /**
         * Helper to assemble an INIR instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleINIR(const Tokens & tokens);

        /**
         * Helper to assemble a JP instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleJP(const Tokens & tokens);

        /**
         * Helper to assemble a JR instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleJR(const Tokens & tokens);

        /**
         * Helper to assemble a LD instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleLD(const Tokens & tokens);

        /**
         * Helper to assemble a LDD instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleLDD(const Tokens & tokens);

        /**
         * Helper to assemble a LDDR instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleLDDR(const Tokens & tokens);

        /**
         * Helper to assemble a LDI instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleLDI(const Tokens & tokens);

        /**
         * Helper to assemble a LDIR instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleLDIR(const Tokens & tokens);

        /**
         * Helper to assemble a NEG instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleNEG(const Tokens & tokens);

        /**
         * Helper to assemble a NOP instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleNOP(const Tokens & tokens);

        /**
         * Helper to assemble an OR instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleOR(const Tokens & tokens);

        /**
         * Helper to assemble an OUT instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleOUT(const Tokens & tokens);

        /**
         * Helper to assemble an OUTD instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleOUTD(const Tokens & tokens);

        /**
         * Helper to assemble an OTDR instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleOTDR(const Tokens & tokens);

        /**
         * Helper to assemble an OUTI instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleOUTI(const Tokens & tokens);

        /**
         * Helper to assemble an OTIR instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleOTIR(const Tokens & tokens);

        /**
         * Helper to assemble a POP instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assemblePOP(const Tokens & tokens);

        /**
         * Helper to assemble a PUSH instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assemblePUSH(const Tokens & tokens);

        /**
         * Helper to assemble a RES instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleRES(const Tokens & tokens);

        /**
         * Helper to assemble a RET instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleRET(const Tokens & tokens);

        /**
         * Helper to assemble a RETI instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleRETI(const Tokens & tokens);

        /**
         * Helper to assemble a RETN instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleRETN(const Tokens & tokens);

        /**
         * Helper to assemble a RLA instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleRLA(const Tokens & tokens);

        /**
         * Helper to assemble a RL instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleRL(const Tokens & tokens);

        /**
         * Helper to assemble a RLCA instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleRLCA(const Tokens & tokens);

        /**
         * Helper to assemble a RLC instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleRLC(const Tokens & tokens);

        /**
         * Helper to assemble a RLD instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleRLD(const Tokens & tokens);

        /**
         * Helper to assemble a RRA instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleRRA(const Tokens & tokens);

        /**
         * Helper to assemble a RR instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleRR(const Tokens & tokens);

        /**
         * Helper to assemble a RRCA instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleRRCA(const Tokens & tokens);

        /**
         * Helper to assemble a RRC instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleRRC(const Tokens & tokens);

        /**
         * Helper to assemble a RRD instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleRRD(const Tokens & tokens);

        /**
         * Helper to assemble a RST instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleRST(const Tokens & tokens);

        /**
         * Helper to assemble a SBC instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleSBC(const Tokens & tokens);

        /**
         * Helper to assemble a SCF instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleSCF(const Tokens & tokens);

        /**
         * Helper to assemble a SET instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleSET(const Tokens & tokens);

        /**
         * Helper to assemble a SLA instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleSLA(const Tokens & tokens);

        /**
         * Helper to assemble a SRA instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleSRA(const Tokens & tokens);

        /**
         * Helper to assemble a SLL instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleSLL(const Tokens & tokens);

        /**
         * Helper to assemble a SRL instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleSRL(const Tokens & tokens);

        /**
         * Helper to assemble a SUB instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleSUB(const Tokens & tokens);

        /**
         * Helper to assemble a XOR instruction.
         * 
         * @param tokens The token stream read from the input.
         * 
         * @return The machine code.
         */
        static Opcode assembleXOR(const Tokens & tokens);

    private:
        /**
         * Helper to deallocate and discard the CPU.
         */
        void discardCpu();
        
        /**
         * The Z80 CPU that is used to execute instructions.
         */
        Z80Cpu * m_cpu;
        
        /**
         * Whether the assembled machine code should be output for each instruction executed.
         */
        bool m_showOpcodes;

        /**
         * Whether the t-state cost and byte size should be output for each instruction executed.
         */
        bool m_showInstructionCost;
        
        /**
         * Whether or not the flags should be output after each instruction executed.
         */
        bool m_autoShowFlags;
    };
}

#endif // INTERPRETER_Z80INTERPRETER_H
