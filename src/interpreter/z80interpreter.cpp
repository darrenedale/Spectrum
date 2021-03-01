#include "z80interpreter.h"

/* TODO:
	- assembleRST()
	- handle IX+n, IY+n operands
	- handle port operands */

#include <readline/readline.h>
#include <readline/history.h>
#include <QString>
#include <QStringList>
#include <QTextStream>
// TODO finish migrating from QRegExp to QRegularExpression
#include <QRegExp>
#include <QRegularExpression>
#include <QDebug>
#include <iostream>
#include <cmath>
#include <cstdio>

#define RegbitsB 0x00
#define RegbitsC 0x01
#define RegbitsD 0x02
#define RegbitsE 0x03
#define RegbitsH 0x04
#define RegbitsL 0x05
#define RegbitsIndirectHl 0x06
#define RegbitsIndirectIxIy RegbitsIndirectHl
#define RegbitsA 0x07

using namespace Interpreter;

const Z80Interpreter::Opcode Z80Interpreter::InvalidInstruction;

class Z80Operand
{
public:
    enum OperandType
    {
        InvalidOperand = 0,
        NumberLiteral,
        IndirectAddress,
        Register16,
        Register8,
        IndirectReg16,
        IndirectReg8,
        IndirectReg16WithOffset,
        Condition,
        Port
    };

    enum ConditionType
    {
        InvalidCondition = 0,
        Z,
        Zero = Z,
        NZ,
        NonZero = NZ,
        C,
        Carry = C,
        NC,
        NoCarry = NC,
        PO,
        ParityOdd = PO,
        PE,
        ParityEven = PE,
        P,
        Plus = P,
        M,
        Minus = M
    };

    inline explicit Z80Operand(const QString & op)
    : m_string(op.trimmed().toUpper()),
      m_number(0),
      m_reg16(Z80::Z80::Register16::HL),
      m_reg8(Z80::Z80::Register8::A),
      m_type(InvalidOperand),
      m_condition(InvalidCondition)
    {
        parse();
    }

    inline OperandType type() const
    {
        return m_type;
    }

    inline bool isValid() const
    {
        return m_type != InvalidOperand && !(m_type == Condition && m_condition == InvalidCondition);
    }

    inline bool isNumber() const
    {
        return isByte() || isWord();
    }

    inline bool isRegister() const
    {
        return isReg16() || isReg8();
    }

    inline bool isReg16() const
    {
        return m_type == Register16;
    }

    inline bool isReg8() const
    {
        return m_type == Register8;
    }

    inline bool isBitIndex() const
    {
        return m_type == NumberLiteral && m_number >= 0 && m_number <= 7;
    }

    inline bool isByte() const
    {
        return m_type == NumberLiteral && m_number >= 0 && m_number <= 256;
    }

    inline bool isSignedByte() const
    {
        return m_type == NumberLiteral && m_number >= -128 && m_number <= 127;
    }

    inline bool isWord() const
    {
        return m_type == NumberLiteral && m_number >= 0 && m_number <= 65535;
    }

    inline bool isSignedWord() const
    {
        return m_type == NumberLiteral && m_number >= -32767 && m_number <= 32768;
    }

    inline bool isOffset() const
    {
        return isSignedByte();
    }

    inline bool isIndirectAddress()
    {
        return m_type == IndirectAddress;
    }

    inline bool isIndirectReg16()
    {
        return m_type == IndirectReg16;
    }

    inline bool isIndirectReg16WithOffset()
    {
        return m_type == IndirectReg16WithOffset;
    }

    inline bool isIndirectReg8()
    {
        return m_type == IndirectReg8;
    }

    inline bool isCondition()
    {
        /* "C" can be either a register or the condition Carry, so while it is stored internally as RegC, report it externally as the C condition also */
        return m_type == Condition || (m_type == Register8 && m_reg8 == Z80::Z80::Register8::C);
    }

    inline bool isPort()
    {
        return m_type == Port;
    }

    inline ConditionType condition()
    {
        return m_condition;
    }

    inline const QString & string() const
    {
        return m_string;
    }

    inline QString operand() const
    {
        return m_string;
    }

    inline Z80::Z80::UnsignedByte bitIndex() const
    {
        return Z80::Z80::UnsignedByte(m_number & 0x07);
    }

    inline Z80::Z80::UnsignedByte byte() const
    {
        return Z80::Z80::UnsignedByte(m_number & 0xff);
    }

    inline Z80::Z80::UnsignedWord word() const
    {
        return Z80::Z80::UnsignedWord(m_number & 0xffff);
    }

    inline Z80::Z80::SignedByte signedByte()
    {
        return Z80::Z80::SignedByte(m_number & 0xff);
    }

    inline Z80::Z80::SignedWord signedWord()
    {
        return Z80::Z80::SignedWord(m_number & 0xffff);
    }

    inline Z80::Z80::SignedByte offset()
    {
        return signedByte();
    }

    inline Z80::Z80::UnsignedByte wordLowByte() const
    {
        Z80::Z80::UnsignedWord nn = Z80::Z80::hostToZ80ByteOrder(word());
        return ((nn & 0xff00) >> 8);
    }

    inline Z80::Z80::UnsignedByte wordHighByte() const
    {
        Z80::Z80::UnsignedWord nn = Z80::Z80::hostToZ80ByteOrder(word());
        return nn & 0x00ff;
    }

    inline Z80::Z80::Register16 reg16() const
    {
        return m_reg16;
    }

    inline Z80::Z80::Register8 reg8() const
    {
        return m_reg8;
    }

    inline Z80::Z80::UnsignedWord address() const
    {
        return Z80::Z80::UnsignedWord(m_number & 0xffff);
    }

private:
    void parse()
    {
        static QRegularExpression s_decimalIndirectAddress("^\\(([0-9]|[1-9][0-9]+)\\)$");
        static QRegularExpression s_decimalLiteral("^[+-]{0,1}([0-9]|[1-9][0-9]+)$");
        static QRegExp s_indirectReg8("\\((B|C|D|E|H|L|A|F|IXH|IXL|IYH|IYL|I|R|B'|C'|D'|E'|H'|L'|A'|F')\\)");
        static QRegExp s_indrectReg16Offset("\\((IX|IY) *([+\\-]) *([0-9]+|0[0-9]+|[01]+B|0X[0-9A-F]+|[0-9A-F]+H)\\)");
        static QRegExp s_indrectReg16("\\((BC|DE|HL|AF|SP|PC|IX|IY|BC'|DE'|HL'|AF')\\)");

        m_type = InvalidOperand;
        m_condition = InvalidCondition;
        m_number = 0;
        m_reg8 = Z80::Z80::Register8::A;
        m_reg16 = Z80::Z80::Register16::HL;

        /* number literals */
        if (s_decimalLiteral.match(m_string).hasMatch()) {
            /* decimal number */
            m_type = NumberLiteral;
            m_number = m_string.toInt(nullptr, 10);
        } else if (QRegularExpression("^[0-9]+D$").match(m_string).hasMatch()) {
            /* decimal number */
            m_type = NumberLiteral;
            m_number = m_string.midRef(0, m_string.length() - 1).toInt();
        } else if (QRegularExpression("^[0-9]+O$").match(m_string).hasMatch()) {
            /* octal number */
            m_type = NumberLiteral;
            m_number = m_string.midRef(0, m_string.length() - 1).toInt(nullptr, 8);
        } else if (QRegularExpression("^0[0-9]+$").match(m_string).hasMatch()) {
            /* octal number */
            m_type = NumberLiteral;
            m_number = m_string.midRef(1).toInt(nullptr, 8);
        } else if (QRegularExpression("^%[01]+$").match(m_string).hasMatch()) {
            /* binary number */
            m_type = NumberLiteral;
            m_number = m_string.midRef(1).toInt(nullptr, 2);
        } else if (QRegularExpression("^[01]+B$").match(m_string).hasMatch()) {
            /* binary number */
            m_type = NumberLiteral;
            m_number = m_string.midRef(0, m_string.length() - 1).toInt(nullptr, 2);
        } else if (QRegularExpression("^0b[01]+$").match(m_string).hasMatch()) {
            /* binary number */
            m_type = NumberLiteral;
            m_number = m_string.midRef(2).toInt(nullptr, 2);
        } else if (QRegularExpression("^\\$[0-9A-F]+$").match(m_string).hasMatch()) {
            /* hex number */
            m_type = NumberLiteral;
            m_number = m_string.midRef(1).toInt(nullptr, 16);
        } else if (QRegularExpression("^0X[0-9A-F]+$").match(m_string).hasMatch()) {
            /* hex number */
            m_type = NumberLiteral;
            m_number = m_string.midRef(2).toInt(nullptr, 16);
        } else if (QRegularExpression("^([0-9][A-F]*|[A-F]*[0-9]+[0-9A-F]*)+H$").match(m_string).hasMatch()) {
            /* hex number */
            m_type = NumberLiteral;
            m_number = m_string.leftRef(m_string.length() - 1).toInt(nullptr, 16);
        }

            /* indirect addresses */
        else if (s_decimalIndirectAddress.match(m_string).hasMatch()) {
            /* decimal number */
            m_type = IndirectAddress;
            m_number = m_string.toInt(nullptr, 10);
        } else if (QRegularExpression("^\\(0[0-9]+\\)$").match(m_string).hasMatch()) {
            /* octal number */
            m_type = IndirectAddress;
            m_number = m_string.toInt(nullptr, 8);
        } else if (QRegularExpression("^\\([01]+B\\)$").match(m_string).hasMatch()) {
            /* binary number */
            m_type = IndirectAddress;
            m_number = m_string.toInt(nullptr, 2);
        } else if (QRegularExpression("^\\(0X[0-9A-F]+\\)$").match(m_string).hasMatch()) {
            /* hex number */
            m_type = IndirectAddress;
            m_number = m_string.midRef(2).toInt(nullptr, 16);
        } else if (QRegularExpression("^\\([0-9A-F]+H\\)$").match(m_string).hasMatch()) {
            /* hex number */
            m_type = IndirectAddress;
            m_number = m_string.leftRef(m_string.length() - 1).toInt(nullptr, 16);
        } else if (s_indrectReg16.exactMatch(m_string)) {
            m_type = IndirectReg16;

            if ("BC" == s_indrectReg16.cap(1)) {
                m_reg16 = Z80::Z80::Register16::BC;
            } else if ("DE" == s_indrectReg16.cap(1)) {
                m_reg16 = Z80::Z80::Register16::DE;
            } else if ("HL" == s_indrectReg16.cap(1)) {
                m_reg16 = Z80::Z80::Register16::HL;
            } else if ("AF" == s_indrectReg16.cap(1)) {
                m_reg16 = Z80::Z80::Register16::AF;
            } else if ("IX" == s_indrectReg16.cap(1)) {
                m_reg16 = Z80::Z80::Register16::IX;
            } else if ("IY" == s_indrectReg16.cap(1)) {
                m_reg16 = Z80::Z80::Register16::IY;
            } else if ("SP" == s_indrectReg16.cap(1)) {
                m_reg16 = Z80::Z80::Register16::SP;
            } else if ("PC" == s_indrectReg16.cap(1)) {
                m_reg16 = Z80::Z80::Register16::PC;
            } else if ("BC'" == s_indrectReg16.cap(1)) {
                m_reg16 = Z80::Z80::Register16::BCShadow;
            } else if ("DE'" == s_indrectReg16.cap(1)) {
                m_reg16 = Z80::Z80::Register16::DEShadow;
            } else if ("HL'" == s_indrectReg16.cap(1)) {
                m_reg16 = Z80::Z80::Register16::HLShadow;
            } else if ("AF'" == s_indrectReg16.cap(1)) {
                m_reg16 = Z80::Z80::Register16::AFShadow;
            }
        } else if (s_indrectReg16Offset.exactMatch((m_string))) {
            m_type = IndirectReg16WithOffset;

            if ("IX" == s_indrectReg16Offset.cap(1)) {
                m_reg16 = Z80::Z80::Register16::IX;
            } else if ("IY" == s_indrectReg16Offset.cap(1)) {
                m_reg16 = Z80::Z80::Register16::IY;
            } else {
                m_type = InvalidOperand;
            }

            bool neg = ("-" == s_indrectReg16Offset.cap(2));
            QString offsetString = s_indrectReg16Offset.cap(3);
            int d = 0xffff;    /* an invalid value */

            if (offsetString.startsWith("0X")) {
                d = offsetString.midRef(2).toInt(nullptr, 16);
            } else if (offsetString.endsWith("H")) {
                d = offsetString.leftRef(offsetString.length() - 1).toInt(nullptr, 16);
            } else if (offsetString.endsWith("B")) {
                d = offsetString.leftRef(offsetString.length() - 1).toInt(nullptr, 2);
            } else if (offsetString.startsWith("0") && offsetString.length() > 1) {
                d = offsetString.toInt(nullptr, 8);
            } else {
                d = offsetString.toInt(nullptr, 10);
            }

            if (neg) {
                d = 0 - d;
            }
            if (d < -128 || d > 127) {
                m_type = InvalidOperand;
            } else {
                m_number = d;
            }
        } else if ("BC" == m_string) {
            m_type = Register16;
            m_reg16 = Z80::Z80::Register16::BC;
        } else if ("DE" == m_string) {
            m_type = Register16;
            m_reg16 = Z80::Z80::Register16::DE;
        } else if ("HL" == m_string) {
            m_type = Register16;
            m_reg16 = Z80::Z80::Register16::HL;
        } else if ("AF" == m_string) {
            m_type = Register16;
            m_reg16 = Z80::Z80::Register16::AF;
        } else if ("IX" == m_string) {
            m_type = Register16;
            m_reg16 = Z80::Z80::Register16::IX;
        } else if ("IY" == m_string) {
            m_type = Register16;
            m_reg16 = Z80::Z80::Register16::IY;
        } else if ("SP" == m_string) {
            m_type = Register16;
            m_reg16 = Z80::Z80::Register16::SP;
        } else if ("PC" == m_string) {
            m_type = Register16;
            m_reg16 = Z80::Z80::Register16::PC;
        } else if ("BC'" == m_string) {
            m_type = Register16;
            m_reg16 = Z80::Z80::Register16::BCShadow;
        } else if ("DE'" == m_string) {
            m_type = Register16;
            m_reg16 = Z80::Z80::Register16::DEShadow;
        } else if ("HL'" == m_string) {
            m_type = Register16;
            m_reg16 = Z80::Z80::Register16::HLShadow;
        } else if ("AF'" == m_string) {
            m_type = Register16;
            m_reg16 = Z80::Z80::Register16::AFShadow;
        } else if (s_indirectReg8.exactMatch(m_string)) {
            m_type = IndirectReg8;

            if ("B" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::B;
            } else if ("C" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::C;
                m_condition = Carry;
            } else if ("D" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::D;
            } else if ("E" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::E;
            } else if ("H" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::H;
            } else if ("L" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::L;
            } else if ("IXH" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::IXH;
            } else if ("IXL" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::IXL;
            } else if ("IYH" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::IYH;
            } else if ("IYL" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::IYL;
            } else if ("A" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::A;
            } else if ("F" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::F;
            } else if ("I" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::I;
            } else if ("R" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::R;
            } else if ("B'" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::BShadow;
            } else if ("C'" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::CShadow;
            } else if ("D'" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::DShadow;
            } else if ("E'" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::EShadow;
            } else if ("H'" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::HShadow;
            } else if ("L'" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::LShadow;
            } else if ("A'" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::AShadow;
            } else if ("F'" == s_indirectReg8.cap(1)) {
                m_reg8 = Z80::Z80::Register8::FShadow;
            }
        } else if ("B" == m_string) {
            m_type = Register8;
            m_reg8 = Z80::Z80::Register8::B;
        } else if ("C" == m_string) {
            m_type = Register8;
            m_reg8 = Z80::Z80::Register8::C;
        } else if ("D" == m_string) {
            m_type = Register8;
            m_reg8 = Z80::Z80::Register8::D;
        } else if ("E" == m_string) {
            m_type = Register8;
            m_reg8 = Z80::Z80::Register8::E;
        } else if ("H" == m_string) {
            m_type = Register8;
            m_reg8 = Z80::Z80::Register8::H;
        } else if ("L" == m_string) {
            m_type = Register8;
            m_reg8 = Z80::Z80::Register8::L;
        } else if ("A" == m_string) {
            m_type = Register8;
            m_reg8 = Z80::Z80::Register8::A;
        } else if ("F" == m_string) {
            m_type = Register8;
            m_reg8 = Z80::Z80::Register8::F;
        } else if ("I" == m_string) {
            m_type = Register8;
            m_reg8 = Z80::Z80::Register8::I;
        } else if ("R" == m_string) {
            m_type = Register8;
            m_reg8 = Z80::Z80::Register8::R;
        } else if ("B'" == m_string) {
            m_type = Register8;
            m_reg8 = Z80::Z80::Register8::BShadow;
        } else if ("C'" == m_string) {
            m_type = Register8;
            m_reg8 = Z80::Z80::Register8::CShadow;
        } else if ("D'" == m_string) {
            m_type = Register8;
            m_reg8 = Z80::Z80::Register8::DShadow;
        } else if ("E'" == m_string) {
            m_type = Register8;
            m_reg8 = Z80::Z80::Register8::EShadow;
        } else if ("H'" == m_string) {
            m_type = Register8;
            m_reg8 = Z80::Z80::Register8::HShadow;
        } else if ("L'" == m_string) {
            m_type = Register8;
            m_reg8 = Z80::Z80::Register8::LShadow;
        } else if ("A'" == m_string) {
            m_type = Register8;
            m_reg8 = Z80::Z80::Register8::AShadow;
        } else if ("F'" == m_string) {
            m_type = Register8;
            m_reg8 = Z80::Z80::Register8::FShadow;
        } else if ("Z" == m_string) {
            m_type = Condition;
            m_condition = Zero;
        } else if ("NZ" == m_string) {
            m_type = Condition;
            m_condition = NonZero;
        } else if ("NC" == m_string) {
            m_type = Condition;
            m_condition = NoCarry;
        } else if ("PO" == m_string) {
            m_type = Condition;
            m_condition = ParityOdd;
        } else if ("PE" == m_string) {
            m_type = Condition;
            m_condition = ParityEven;
        } else if ("P" == m_string) {
            m_type = Condition;
            m_condition = Plus;
        } else if ("M" == m_string) {
            m_type = Condition;
            m_condition = Minus;
        }
    }

    QString m_string;
    int m_number;
    Z80::Z80::Register16 m_reg16;
    Z80::Z80::Register8 m_reg8;
    OperandType m_type;
    ConditionType m_condition;
};

Z80Interpreter::Z80Interpreter(Z80::Z80 * cpu)
        : m_cpu(cpu),
          m_showOpcodes(false),
          m_showInstructionCost(false),
          m_autoShowFlags(false)
{
}

Z80Interpreter::~Z80Interpreter()
{
    discardCpu();
}

void Z80Interpreter::discardCpu()
{
    if (m_cpu) {
        delete m_cpu;
    }
    m_cpu = 0;
}

bool Z80Interpreter::hasCpu() const
{
    return (0 != m_cpu);
}

Z80::Z80 * Z80Interpreter::cpu() const
{
    return m_cpu;
}

void Z80Interpreter::setCpu(Z80::Z80 * cpu)
{
    discardCpu();
    m_cpu = cpu;
}

void Z80Interpreter::run()
{
    std::cout << "Z80 interpreter\nDarren Hatherley, 2012\n\nType \".help\" for help.\n\n";

    if (!hasCpu()) {
        std::cout << "No CPU. Exiting.";
        return;
    }

    std::cout << m_cpu->clockSpeedMHz() << "MHz Z80 CPU, " << m_cpu->ramSize() << " bytes of RAM\n";

    while (handleInput(readInput())) {
    };
}

void Z80Interpreter::run(Z80::Z80 * cpu)
{
    Z80Interpreter interpreter(cpu);
    interpreter.run();
}

QString Z80Interpreter::readInput()
{
    char * myLine = readline("> ");
    QString ret = QString::fromUtf8(myLine);

    if (myLine) {
        if (*myLine) {
            add_history(myLine);
        }
        free(myLine);
    }

    return ret;
}

QStringList Z80Interpreter::tokenise(const QString & input)
{
    QStringList ret, tmp = input.split(QRegExp("[ ,]+"));

            foreach(QString s, tmp) {
            ret.append(s.trimmed());
        }

    return ret;
}

bool Z80Interpreter::handleInput(const QString & input)
{
    QStringList tokens = tokenise(input);

    if (0 == tokens.size()) {
        return true;
    }

    if (tokens.at(0) == ".quit") {
        return false;
    }
    if (tokens.at(0) == ".exit") {
        return false;
    }
    if (tokens.at(0) == ".halt") {
        return false;
    }

    if ('.' == tokens.at(0).at(0)) {
        handleDotCommand(tokens);
    } else {
        runOpcode(assembleInstruction(tokens));
    }

    return true;
}

void Z80Interpreter::runOpcode(const Opcode & opcode)
{
    if (!hasCpu()) {
        std::cout << "no cpu available to execute instruction\n";
        return;
    }

    if (InvalidInstruction == opcode) {
        std::cout << "instruction could not be assembled.\n";
        return;
    }

    if (m_showOpcodes) {
        std::cout << "opcode:";

                foreach(Z80::Z80::UnsignedByte b, opcode) {
                printf(" 0x%02x", b);
            }

        std::cout << "\n";
    }

    int cycles, size;
    m_cpu->execute(opcode.data(), false, &cycles, &size);

    if (m_showInstructionCost) {
        std::cout << "instruction consumed " << cycles << " CPU cycles and would occupy " << size
                  << " byte(s) of RAM.\n";
    }

    if (m_autoShowFlags) {
        dotDumpFlags();
    }
}

/* dot-command methods */
void Z80Interpreter::handleDotCommand(const QStringList & tokens)
{
    if (1 > tokens.count()) {
        qDebug() << "handleDotCommand() given no tokens to interpret.\n";
        return;
    }

    if (tokens.at(0) == ".help") {
        dotHelp();
    } else if (tokens.at(0) == ".ram") {
        dotDumpMemory(tokens);
    } else if (tokens.at(0) == ".dumpram") {
        dotDumpMemory(tokens);
    } else if (tokens.at(0) == ".regs") {
        dotDumpRegisters();
    } else if (tokens.at(0) == ".dumpregisters") {
        dotDumpRegisters();
    } else if (tokens.at(0) == ".status") {
        dotStatus();
    } else if (tokens.at(0) == ".rv") {
        dotRegisterValue(tokens);
    } else if (tokens.at(0) == ".regvalue") {
        dotRegisterValue(tokens);
    } else if (tokens.at(0) == ".registervalue") {
        dotRegisterValue(tokens);
    } else if (tokens.at(0) == ".showopcodes") {
        dotShowOpcodes();
    } else if (tokens.at(0) == ".hideopcodes") {
        dotHideOpcodes();
    } else if (tokens.at(0) == ".showcosts") {
        dotShowCosts();
    } else if (tokens.at(0) == ".hidecosts") {
        dotHideCosts();
    } else if (tokens.at(0) == ".flags") {
        dotDumpFlags();
    } else if (tokens.at(0) == ".dumpflags") {
        dotDumpFlags();
    } else if (tokens.at(0) == ".autoflags") {
        dotAutoShowFlags(tokens);
    } else if (tokens.at(0) == ".autoshowflags") {
        dotAutoShowFlags(tokens);
    } else {
        std::cout << "Unrecognised command \"" << qPrintable(tokens.at(0)) << "\": try \".help\".\n";
    }
}

void Z80Interpreter::dotHelp() const
{
    std::cout <<
              "Enter a Z80 instruction to execute that instruction. All instructions except\n"
              "IN and OUT and the IX and IY register instructions are supported.\n\n"

              "Special commands starting with a dot (.) are available to examine the state\n"
              "of the Z80 CPU and it's memory, and to control the interpreter. These are\n"
              "listed below:\n\n"

              ".halt\n"
              ".quit\n"
              ".exit\n"
              "          Exit the interpreter.\n\n"

              ".ram <start> [<len> = 16]\n"
              ".dumpram <start> [<len> = 16]\n"
              "          Dump the contents of the Z80 memory from <start> for <len> bytes.\n"
              "          <len> is optional and defaults to 16. <start> is not optional.\n\n"

              ".regs\n"
              ".dumpregisters\n"
              "          Dump the current state of the Z80 registers. This includes the\n"
              "          main registers, the shadow registers, the interrupt flip-flops,\n"
              "          and the IX and IY registers.\n\n"

              ".status\n"
              "          Shows the status of the Z80 (registers and flags) in a compact\n"
              "          format.\n\n"

              ".rv <reg>\n"
              ".regvalue <reg>\n"
              ".registervalue <reg>\n"
              "          Dump the value currenlty held in a register. <reg> can be any of.\n"
              "          the Z80's 8-bit registers or 16-bit register pairs, including the\n"
              "          shadow registers.\n\n"

              ".showopcodes\n"
              "          Show the opcode for each executed instruction.\n\n"

              ".hideopcodes\n"
              "          Don't show the opcode for each executed instruction.\n\n"

              ".showcosts\n"
              "          Show the number of clock cycles consumed and the amount of RAM\n"
              "          that would be occupied by each executed instruction.\n\n"

              ".hidecosts\n"
              "          Don't show the number of clock cycles consumed and the amount of\n"        "          RAM that would be occupied by each executed instruction.\n\n"

              ".flags\n"
              ".dumpflags\n"
              "          Dump the current state of the Z80 flags register.\n\n"

              ".autoflags\n"
              ".autoshowflags\n"
              "          Automatically dump the state of the Z80 flags register after each\n"
              "          executed instruction.\n";
}

void Z80Interpreter::dotShowCosts()
{
    m_showInstructionCost = true;
    std::cout << "showing instruction costs from now on.\n";
}

void Z80Interpreter::dotHideCosts()
{
    m_showInstructionCost = false;
    std::cout << "not showing instruction costs from now on.\n";
}

void Z80Interpreter::dotShowOpcodes()
{
    m_showOpcodes = true;
    std::cout << "showing opcodes from now on.\n";
}

void Z80Interpreter::dotHideOpcodes()
{
    m_showOpcodes = false;
    std::cout << "not showing opcodes from now on.\n";
}

void Z80Interpreter::dotAutoShowFlags(const QStringList & tokens)
{
    if (1 == tokens.count() || tokens.at(1).toUpper() == "TRUE" || tokens.at(1).toUpper() == "YES" ||
        tokens.at(1) == "1" || tokens.at(1).toUpper() == "ON" || tokens.at(1).toUpper() == "Y") {
        m_autoShowFlags = true;
        std::cout << "automatically showing flags from now on.\n";
    } else if (tokens.at(1).toUpper() == "FALSE" || tokens.at(1).toUpper() == "NO" || tokens.at(1) == "0" ||
               tokens.at(1).toUpper() == "OFF" || tokens.at(1).toUpper() == "N") {
        m_autoShowFlags = false;
        std::cout << "no longer automatically showing flags.\n";
    } else {
        std::cout << "unrecognised parameter \"" << qPrintable(tokens.at(1)) << "\"\n";
    }
}

void Z80Interpreter::dotDumpFlags() const
{
    Z80::Z80::UnsignedByte f = m_cpu->fRegisterValue();

    std::cout << " S Z H 5 P 3 N C\n";
    std::cout << " " << (f & 0x80 ? '1' : '0');
    std::cout << " " << (f & 0x40 ? '1' : '0');
    std::cout << " " << (f & 0x20 ? '1' : '0');
    std::cout << " " << (f & 0x10 ? '1' : '0');
    std::cout << " " << (f & 0x04 ? '1' : '0');
    std::cout << " " << (f & 0x02 ? '1' : '0');
    std::cout << " " << (f & 0x02 ? '1' : '0');
    std::cout << " " << (f & 0x01 ? '1' : '0');
    std::cout << "\n";
}

void Z80Interpreter::dotDumpMemory(const QStringList & tokens) const
{
    Q_ASSERT(0 < tokens.count());
    int low = 0;
    int len = 16;

    if (1 < tokens.count()) {
        /* TODO support hex (0x) values */
        bool ok;
        low = tokens.at(1).toUInt(&ok);

        if (!ok) {
            std::cout << qPrintable(tokens.at(0)) << ": invalid start byte \"" << qPrintable(tokens.at(1)) << "\"\n";
            return;
        }
    }

    if (2 < tokens.count()) {
        /* TODO support hex (0x) values */
        bool ok;
        len = tokens.at(2).toUInt(&ok);

        if (!ok) {
            std::cout << qPrintable(tokens.at(0)) << ": invalid dump length \"" << qPrintable(tokens.at(1)) << "\"\n";
            return;
        }
    }

    dotDumpMemory(low, len);
}

void Z80Interpreter::dotDumpMemory(int low, int len) const
{
    Q_ASSERT(m_cpu);

    if (low < 0) {
        std::cout << "can't display memory below address 0";
        return;
    }

    if (len < 1) {
        std::cout << "memory range of 0 bytes requested - nothing to display.";
        return;
    }

    const Z80::Z80::UnsignedByte * ram = m_cpu->memory();
    std::cout << "         ";

    for (int i = 0; i < 16; ++i) {
        printf(" 0x%02x", i);
    }

    if (low % 16) {
        /* there are some undisplayed values before low in the row, so insert extra spacing */
        printf("\n 0x%04x :", int(std::floor(low / 16) * 16));

        for (int i = 0; i < (low % 16); ++i) {
            std::cout << "     ";
        }
    }

    for (int i = 0; i < len; ++i) {
        int addr = low + i;

        if (0 == (addr % 16)) {
            printf("\n 0x%04x :", addr);
        }
        printf(" 0x%02x", (unsigned int) (ram[addr]));
    }

    std::cout << "\n";
}

void Z80Interpreter::dotStatus() const
{
    std::cout << "A  CZPSNH  BC   DE   HL   IX   IY  A' CZPSNH' BC'  DE'  HL'  SP  | IM  IFF1  IFF2\n";
    printf("%02x %c%c%c%c%c%c %04x %04x %04x %04x %04x %02x %c%c%c%c%c%c %04x %04x %04x %04x    %1d    %1d     %1d\n",
           m_cpu->aRegisterValue(), (m_cpu->cFlag() ? '1' : '0'), (m_cpu->zFlag() ? '1' : '0'),
           (m_cpu->pFlag() ? '1' : '0'), (m_cpu->sFlag() ? '1' : '0'), (m_cpu->nFlag() ? '1' : '0'),
           (m_cpu->hFlag() ? '1' : '0'), m_cpu->bcRegisterValue(), m_cpu->deRegisterValue(), m_cpu->hlRegisterValue(),
           m_cpu->ixRegisterValue(), m_cpu->iyRegisterValue(), m_cpu->afShadowRegisterValue(), '0', '0', '0', '0', '0',
           '0', m_cpu->bcShadowRegisterValue(), m_cpu->deShadowRegisterValue(), m_cpu->hlShadowRegisterValue(),
           m_cpu->sp(), m_cpu->interruptMode(), m_cpu->iff1() ? 1 : 0, m_cpu->iff2() ? 1 : 0);
}

void Z80Interpreter::dotDumpRegisters() const
{
    dotRegisterValue(Z80::Z80::Register8::A);
    dotRegisterValue(Z80::Z80::Register8::B);
    dotRegisterValue(Z80::Z80::Register8::C);
    dotRegisterValue(Z80::Z80::Register8::D);
    dotRegisterValue(Z80::Z80::Register8::E);
    dotRegisterValue(Z80::Z80::Register8::H);
    dotRegisterValue(Z80::Z80::Register8::L);
    dotRegisterValue(Z80::Z80::Register16::AF);
    dotRegisterValue(Z80::Z80::Register16::BC);
    dotRegisterValue(Z80::Z80::Register16::DE);
    dotRegisterValue(Z80::Z80::Register16::HL);
    dotRegisterValue(Z80::Z80::Register16::SP);
    dotRegisterValue(Z80::Z80::Register16::PC);
    dotRegisterValue(Z80::Z80::Register16::IX);
    dotRegisterValue(Z80::Z80::Register16::IY);
}

void Z80Interpreter::dotRegisterValue(const QStringList & tokens) const
{
    if (2 > tokens.count()) {
        std::cout << "you must specify which register's value you wish to display\n";
        return;
    }

    if (tokens.at(1).toUpper() == "A") {
        dotRegisterValue(Z80::Z80::Register8::A);
    } else if (tokens.at(1).toUpper() == "B") {
        dotRegisterValue(Z80::Z80::Register8::B);
    } else if (tokens.at(1).toUpper() == "C") {
        dotRegisterValue(Z80::Z80::Register8::C);
    } else if (tokens.at(1).toUpper() == "D") {
        dotRegisterValue(Z80::Z80::Register8::D);
    } else if (tokens.at(1).toUpper() == "E") {
        dotRegisterValue(Z80::Z80::Register8::E);
    } else if (tokens.at(1).toUpper() == "H") {
        dotRegisterValue(Z80::Z80::Register8::H);
    } else if (tokens.at(1).toUpper() == "L") {
        dotRegisterValue(Z80::Z80::Register8::L);
    } else if (tokens.at(1).toUpper() == "F") {
        dotRegisterValue(Z80::Z80::Register8::F);
    } else if (tokens.at(1).toUpper() == "AF") {
        dotRegisterValue(Z80::Z80::Register16::AF);
    } else if (tokens.at(1).toUpper() == "BC") {
        dotRegisterValue(Z80::Z80::Register16::BC);
    } else if (tokens.at(1).toUpper() == "DE") {
        dotRegisterValue(Z80::Z80::Register16::DE);
    } else if (tokens.at(1).toUpper() == "HL") {
        dotRegisterValue(Z80::Z80::Register16::HL);
    } else if (tokens.at(1).toUpper() == "SP") {
        dotRegisterValue(Z80::Z80::Register16::SP);
    } else if (tokens.at(1).toUpper() == "PC") {
        dotRegisterValue(Z80::Z80::Register16::PC);
    } else if (tokens.at(1).toUpper() == "IX") {
        dotRegisterValue(Z80::Z80::Register16::IX);
    } else if (tokens.at(1).toUpper() == "IY") {
        dotRegisterValue(Z80::Z80::Register16::IY);
    } else if (tokens.at(1).toUpper() == "A'") {
        dotRegisterValue(Z80::Z80::Register8::AShadow);
    } else if (tokens.at(1).toUpper() == "B'") {
        dotRegisterValue(Z80::Z80::Register8::BShadow);
    } else if (tokens.at(1).toUpper() == "C'") {
        dotRegisterValue(Z80::Z80::Register8::CShadow);
    } else if (tokens.at(1).toUpper() == "D'") {
        dotRegisterValue(Z80::Z80::Register8::DShadow);
    } else if (tokens.at(1).toUpper() == "E'") {
        dotRegisterValue(Z80::Z80::Register8::EShadow);
    } else if (tokens.at(1).toUpper() == "H'") {
        dotRegisterValue(Z80::Z80::Register8::HShadow);
    } else if (tokens.at(1).toUpper() == "L'") {
        dotRegisterValue(Z80::Z80::Register8::LShadow);
    } else if (tokens.at(1).toUpper() == "F'") {
        dotRegisterValue(Z80::Z80::Register8::FShadow);
    } else if (tokens.at(1).toUpper() == "AF'") {
        dotRegisterValue(Z80::Z80::Register16::AFShadow);
    } else if (tokens.at(1).toUpper() == "BC'") {
        dotRegisterValue(Z80::Z80::Register16::BCShadow);
    } else if (tokens.at(1).toUpper() == "DE'") {
        dotRegisterValue(Z80::Z80::Register16::DEShadow);
    } else if (tokens.at(1).toUpper() == "HL'") {
        dotRegisterValue(Z80::Z80::Register16::HLShadow);
    } else {
        std::cout << "unrecognised register: \"" << qPrintable(tokens.at(1)) << "\"\n";
    }
}

void Z80Interpreter::dotRegisterValue(const Z80::Z80::Register8 reg, const NumberFormats & fmt) const
{
    Q_ASSERT(m_cpu);
    Z80::Z80::UnsignedByte v = m_cpu->registerValue(reg);

    switch (reg) {
        case Z80::Z80::Register8::A:
            std::cout << "A  = ";
            break;
        case Z80::Z80::Register8::B:
            std::cout << "B  = ";
            break;
        case Z80::Z80::Register8::C:
            std::cout << "C  = ";
            break;
        case Z80::Z80::Register8::D:
            std::cout << "D  = ";
            break;
        case Z80::Z80::Register8::E:
            std::cout << "E  = ";
            break;
        case Z80::Z80::Register8::H:
            std::cout << "H  = ";
            break;
        case Z80::Z80::Register8::L:
            std::cout << "L  = ";
            break;
        case Z80::Z80::Register8::IXH:
            std::cout << "IXH  = ";
            break;
        case Z80::Z80::Register8::IXL:
            std::cout << "IXL  = ";
            break;
        case Z80::Z80::Register8::IYH:
            std::cout << "IYH  = ";
            break;
        case Z80::Z80::Register8::IYL:
            std::cout << "IYL  = ";
            break;
        case Z80::Z80::Register8::F:
            std::cout << "F  = ";
            break;
        case Z80::Z80::Register8::I:
            std::cout << "I  = ";
            break;
        case Z80::Z80::Register8::R:
            std::cout << "R  = ";
            break;
        case Z80::Z80::Register8::AShadow:
            std::cout << "A' = ";
            break;
        case Z80::Z80::Register8::BShadow:
            std::cout << "B' = ";
            break;
        case Z80::Z80::Register8::CShadow:
            std::cout << "C' = ";
            break;
        case Z80::Z80::Register8::DShadow:
            std::cout << "D' = ";
            break;
        case Z80::Z80::Register8::EShadow:
            std::cout << "E' = ";
            break;
        case Z80::Z80::Register8::HShadow:
            std::cout << "H' = ";
            break;
        case Z80::Z80::Register8::LShadow:
            std::cout << "L' = ";
            break;
        case Z80::Z80::Register8::FShadow:
            std::cout << "F' = ";
            break;
    }

    if (fmt & HexFormat) {
        printf("  0x%02x", v);
    }
    if (fmt & DecimalFormat) {
        printf("  %3d", v);
    }
    if (fmt & OctalFormat) {
        printf("  %3o", v);
    }

    if (fmt & BinaryFormat) {
        Z80::Z80::UnsignedByte mask = 0x80;
        std::cout << " ";

        for (int i = 0; i < 8; ++i) {
            std::cout << (v & mask ? '1' : '0');
            mask >>= 1;
        }

        std::cout << 'b';
    }

    std::cout << "\n";
}

void Z80Interpreter::dotRegisterValue(const Z80::Z80::Register16 reg, const NumberFormats & fmt) const
{
    Q_ASSERT(m_cpu);
    Z80::Z80::UnsignedWord v = m_cpu->registerValue(reg);

    switch (reg) {
        case Z80::Z80::Register16::AF:
            std::cout << "AF  = ";
            break;
        case Z80::Z80::Register16::BC:
            std::cout << "BC  = ";
            break;
        case Z80::Z80::Register16::DE:
            std::cout << "DE  = ";
            break;
        case Z80::Z80::Register16::HL:
            std::cout << "HL  = ";
            break;
        case Z80::Z80::Register16::SP:
            std::cout << "SP  = ";
            break;
        case Z80::Z80::Register16::PC:
            std::cout << "PC  = ";
            break;
        case Z80::Z80::Register16::IX:
            std::cout << "IX  = ";
            break;
        case Z80::Z80::Register16::IY:
            std::cout << "IY  = ";
            break;
        case Z80::Z80::Register16::AFShadow:
            std::cout << "AF' = ";
            break;
        case Z80::Z80::Register16::BCShadow:
            std::cout << "BC' = ";
            break;
        case Z80::Z80::Register16::DEShadow:
            std::cout << "DE' = ";
            break;
        case Z80::Z80::Register16::HLShadow:
            std::cout << "HL' = ";
            break;
    }

    if (fmt && HexFormat) {
        printf("  0x%04x", v);
    }
    if (fmt && DecimalFormat) {
        printf("  %6d", v);
    }
    if (fmt && OctalFormat) {
        printf("  0%3o", v);
    }

    if (fmt && BinaryFormat) {
        Z80::Z80::UnsignedWord mask = 0x8000;
        std::cout << " ";

        for (int i = 0; i < 16; ++i) {
            std::cout << (v & mask ? '1' : '0');
            mask <<= 1;
        }

        std::cout << 'b';
    }

    std::cout << "\n";
}

/* Z80 instruction methods */
Z80Interpreter::Opcode Z80Interpreter::assembleInstruction(const QStringList & tokens)
{
    if (1 > tokens.count()) {
        qDebug() << "assembleInstruction given no tokens";
        return Opcode();
    }

    if (tokens.at(0).toUpper() == "ADC") {
        return assembleADC(tokens);
    } else if (tokens.at(0).toUpper() == "ADC") {
        return assembleADC(tokens);
    } else if (tokens.at(0).toUpper() == "ADD") {
        return assembleADD(tokens);
    } else if (tokens.at(0).toUpper() == "AND") {
        return assembleAND(tokens);
    } else if (tokens.at(0).toUpper() == "BIT") {
        return assembleBIT(tokens);
    } else if (tokens.at(0).toUpper() == "CALL") {
        return assembleCALL(tokens);
    } else if (tokens.at(0).toUpper() == "CCF") {
        return assembleCCF(tokens);
    } else if (tokens.at(0).toUpper() == "CP") {
        return assembleCP(tokens);
    } else if (tokens.at(0).toUpper() == "CPD") {
        return assembleCPD(tokens);
    } else if (tokens.at(0).toUpper() == "CPDR") {
        return assembleCPDR(tokens);
    } else if (tokens.at(0).toUpper() == "CPI") {
        return assembleCPI(tokens);
    } else if (tokens.at(0).toUpper() == "CPIR") {
        return assembleCPIR(tokens);
    } else if (tokens.at(0).toUpper() == "CPL") {
        return assembleCPL(tokens);
    } else if (tokens.at(0).toUpper() == "DAA") {
        return assembleDAA(tokens);
    } else if (tokens.at(0).toUpper() == "DEC") {
        return assembleDEC(tokens);
    } else if (tokens.at(0).toUpper() == "DI") {
        return assembleDI(tokens);
    } else if (tokens.at(0).toUpper() == "DJNZ") {
        return assembleDJNZ(tokens);
    } else if (tokens.at(0).toUpper() == "EI") {
        return assembleEI(tokens);
    } else if (tokens.at(0).toUpper() == "EX") {
        return assembleEX(tokens);
    } else if (tokens.at(0).toUpper() == "EXX") {
        return assembleEXX(tokens);
    } else if (tokens.at(0).toUpper() == "HALT") {
        return assembleHALT(tokens);
    } else if (tokens.at(0).toUpper() == "IM") {
        return assembleIM(tokens);
    } else if (tokens.at(0).toUpper() == "IN") {
        return assembleIN(tokens);
    } else if (tokens.at(0).toUpper() == "INC") {
        return assembleINC(tokens);
    } else if (tokens.at(0).toUpper() == "IND") {
        return assembleIND(tokens);
    } else if (tokens.at(0).toUpper() == "INDR") {
        return assembleINDR(tokens);
    } else if (tokens.at(0).toUpper() == "INI") {
        return assembleINI(tokens);
    } else if (tokens.at(0).toUpper() == "INIR") {
        return assembleINIR(tokens);
    } else if (tokens.at(0).toUpper() == "JP") {
        return assembleJP(tokens);
    } else if (tokens.at(0).toUpper() == "JR") {
        return assembleJR(tokens);
    } else if (tokens.at(0).toUpper() == "LD") {
        return assembleLD(tokens);
    } else if (tokens.at(0).toUpper() == "LDD") {
        return assembleLDD(tokens);
    } else if (tokens.at(0).toUpper() == "LDDR") {
        return assembleLDDR(tokens);
    } else if (tokens.at(0).toUpper() == "LDI") {
        return assembleLDI(tokens);
    } else if (tokens.at(0).toUpper() == "LDIR") {
        return assembleLDIR(tokens);
    } else if (tokens.at(0).toUpper() == "NEG") {
        return assembleNEG(tokens);
    } else if (tokens.at(0).toUpper() == "NOP") {
        return assembleNOP(tokens);
    } else if (tokens.at(0).toUpper() == "OR") {
        return assembleOR(tokens);
    } else if (tokens.at(0).toUpper() == "OUT") {
        return assembleOUT(tokens);
    } else if (tokens.at(0).toUpper() == "OUTD") {
        return assembleOUTD(tokens);
    } else if (tokens.at(0).toUpper() == "OTDR") {
        return assembleOTDR(tokens);
    } else if (tokens.at(0).toUpper() == "OUTI") {
        return assembleOUTI(tokens);
    } else if (tokens.at(0).toUpper() == "OTIR") {
        return assembleOTIR(tokens);
    } else if (tokens.at(0).toUpper() == "POP") {
        return assemblePOP(tokens);
    } else if (tokens.at(0).toUpper() == "PUSH") {
        return assemblePUSH(tokens);
    } else if (tokens.at(0).toUpper() == "RES") {
        return assembleRES(tokens);
    } else if (tokens.at(0).toUpper() == "RET") {
        return assembleRET(tokens);
    } else if (tokens.at(0).toUpper() == "RETI") {
        return assembleRETI(tokens);
    } else if (tokens.at(0).toUpper() == "RETN") {
        return assembleRETN(tokens);
    } else if (tokens.at(0).toUpper() == "RLA") {
        return assembleRLA(tokens);
    } else if (tokens.at(0).toUpper() == "RL") {
        return assembleRL(tokens);
    } else if (tokens.at(0).toUpper() == "RLCA") {
        return assembleRLCA(tokens);
    } else if (tokens.at(0).toUpper() == "RLC") {
        return assembleRLC(tokens);
    } else if (tokens.at(0).toUpper() == "RLD") {
        return assembleRLD(tokens);
    } else if (tokens.at(0).toUpper() == "RRA") {
        return assembleRRA(tokens);
    } else if (tokens.at(0).toUpper() == "RR") {
        return assembleRR(tokens);
    } else if (tokens.at(0).toUpper() == "RRCA") {
        return assembleRRCA(tokens);
    } else if (tokens.at(0).toUpper() == "RRC") {
        return assembleRRC(tokens);
    } else if (tokens.at(0).toUpper() == "RRD") {
        return assembleRRD(tokens);
    } else if (tokens.at(0).toUpper() == "RST") {
        return assembleRST(tokens);
    } else if (tokens.at(0).toUpper() == "SBC") {
        return assembleSBC(tokens);
    } else if (tokens.at(0).toUpper() == "SCF") {
        return assembleSCF(tokens);
    } else if (tokens.at(0).toUpper() == "SET") {
        return assembleSET(tokens);
    } else if (tokens.at(0).toUpper() == "SLA") {
        return assembleSLA(tokens);
    } else if (tokens.at(0).toUpper() == "SRA") {
        return assembleSRA(tokens);
    } else if (tokens.at(0).toUpper() == "SLL") {
        return assembleSLL(tokens);
    } else if (tokens.at(0).toUpper() == "SRL") {
        return assembleSRL(tokens);
    } else if (tokens.at(0).toUpper() == "SUB") {
        return assembleSUB(tokens);
    } else if (tokens.at(0).toUpper() == "XOR") {
        return assembleXOR(tokens);
    }
    return InvalidInstruction;
}

Z80Interpreter::Opcode Z80Interpreter::assembleADC(const QStringList & tokens)
{
    int c = tokens.count();
    Opcode ret;

    if (c < 3) {
        std::cout << "ADC requires two operands\n";
        return InvalidInstruction;
    }

    Z80Operand op1(tokens.at(1));
    Z80Operand op2(tokens.at(2));

    if (op1.isReg8() && op1.reg8() == Z80::Z80::Register8::A) {
        if (op2.isReg8()) {
            /* ADC A,reg8
				ADC A,(HL) */
            switch (op2.reg8()) {
                case Z80::Z80::Register8::A:
                    ret.append(0x88 | RegbitsA);
                    break;
                case Z80::Z80::Register8::B:
                    ret.append(0x88 | RegbitsB);
                    break;
                case Z80::Z80::Register8::C:
                    ret.append(0x88 | RegbitsC);
                    break;
                case Z80::Z80::Register8::D:
                    ret.append(0x88 | RegbitsD);
                    break;
                case Z80::Z80::Register8::E:
                    ret.append(0x88 | RegbitsE);
                    break;
                case Z80::Z80::Register8::H:
                    ret.append(0x88 | RegbitsH);
                    break;
                case Z80::Z80::Register8::L:
                    ret.append(0x88 | RegbitsL);
                    break;
                default:
                    return InvalidInstruction;
            }
        } else if (op2.isByte()) {
            ret.append(0xce);
            ret.append(op2.byte());
        } else if (op2.isIndirectReg16() && op2.reg16() == Z80::Z80::Register16::HL) {
            ret.append(0x88 | RegbitsIndirectHl);
        } else if (op2.isIndirectReg16WithOffset()) {
            /* ACD A,(IX+d)
			 * ACD A,(IY+d) */
            if (op2.reg16() == Z80::Z80::Register16::IX) {
                ret.append(0xdd);
            } else if (op2.reg16() == Z80::Z80::Register16::IY) {
                ret.append(0xfd);
            } else {
                return InvalidInstruction;
            }

            ret.append(0x8e);
            ret.append(op2.offset());
        } else {
            return InvalidInstruction;
        }
    } else if (op1.isReg16() && op1.reg16() == Z80::Z80::Register16::HL && op2.isReg16()) {
        /* ADC HL,BC
			ADC HL,DE
			ADC HL,HL
			ADC HL,SP */
        ret.append(0xed);

        switch (op2.reg16()) {
            case Z80::Z80::Register16::BC:
                ret.append(0x4a);
                break;
            case Z80::Z80::Register16::DE:
                ret.append(0x5a);
                break;
            case Z80::Z80::Register16::HL:
                ret.append(0x6a);
                break;
            case Z80::Z80::Register16::SP:
                ret.append(0x7a);
                break;
            default:
                return InvalidInstruction;
        }
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleADD(const QStringList & tokens)
{
    int c = tokens.count();
    Opcode ret;

    if (c < 3) {
        std::cout << "ADD requires two operands\n";
        return InvalidInstruction;
    }

    Z80Operand op1(tokens.at(1));
    Z80Operand op2(tokens.at(2));

    if (op1.isReg8() && op1.reg8() == Z80::Z80::Register8::A) {
        if (op2.isReg8()) {
            /* ADD A,reg8 */
            switch (op2.reg8()) {
                case Z80::Z80::Register8::A:
                    ret.append(0x80 | RegbitsA);
                    break;
                case Z80::Z80::Register8::B:
                    ret.append(0x80 | RegbitsB);
                    break;
                case Z80::Z80::Register8::C:
                    ret.append(0x80 | RegbitsC);
                    break;
                case Z80::Z80::Register8::D:
                    ret.append(0x80 | RegbitsD);
                    break;
                case Z80::Z80::Register8::E:
                    ret.append(0x80 | RegbitsE);
                    break;
                case Z80::Z80::Register8::H:
                    ret.append(0x80 | RegbitsH);
                    break;
                case Z80::Z80::Register8::L:
                    ret.append(0x80 | RegbitsL);
                    break;
                default:
                    return InvalidInstruction;
            }
        } else if (op2.isIndirectReg16() && op2.reg16() == Z80::Z80::Register16::HL) {
            /* ADD A,(HL) */
            ret.append(0x80 | RegbitsIndirectHl);
        } else if (op2.isIndirectReg16WithOffset()) {
            /* ADD A,(IX+d)
			 * ADD A,(IY+d) */
            if (op2.reg16() == Z80::Z80::Register16::IX) {
                ret.append(0xdd);
            } else if (op2.reg16() == Z80::Z80::Register16::IY) {
                ret.append(0xfd);
            } else {
                return InvalidInstruction;
            }

            ret.append(0x86);
            ret.append(op2.offset());
        } else if (op2.isByte()) {
            /* ADD A,n */
            ret.append(0xc6);
            ret.append(op2.byte());
        } else {
            return InvalidInstruction;
        }
    } else if (op1.isReg16() && op2.isReg16()) {
        /* ADD HL,BC; ADD HL,DE; ADD HL,HL; ADD HL,SP */
        /* ADD IX,BC; ADD IX,DE; ADD IX,HL; ADD IX,SP */
        /* ADD IY,BC; ADD IY,DE; ADD IY,HL; ADD IY,SP */
        if (op1.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        } else if (op1.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else if (op1.reg16() != Z80::Z80::Register16::HL) {
            return InvalidInstruction;
        }

        switch (op2.reg16()) {
            case Z80::Z80::Register16::BC:
                ret.append(0x09);
                break;
            case Z80::Z80::Register16::DE:
                ret.append(0x19);
                break;
            case Z80::Z80::Register16::HL:
                ret.append(0x29);
                break;
            case Z80::Z80::Register16::SP:
                ret.append(0x39);
                break;
            default:
                return InvalidInstruction;
        }
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleAND(const QStringList & tokens)
{
    int c = tokens.count();
    Opcode ret;

    if (c < 2) {
        std::cout << "AND requires one operand\n";
        return InvalidInstruction;
    }

    Z80Operand op1(tokens.at(1));

    if (op1.isReg8()) {
        /* AND A,reg8 */
        Z80::Z80::UnsignedByte opcode = 0xa0;

        switch (op1.reg8()) {
            case Z80::Z80::Register8::A:
                opcode |= RegbitsA;
                break;
            case Z80::Z80::Register8::B:
                opcode |= RegbitsB;
                break;
            case Z80::Z80::Register8::C:
                opcode |= RegbitsC;
                break;
            case Z80::Z80::Register8::D:
                opcode |= RegbitsD;
                break;
            case Z80::Z80::Register8::E:
                opcode |= RegbitsE;
                break;
            case Z80::Z80::Register8::H:
                opcode |= RegbitsH;
                break;
            case Z80::Z80::Register8::L:
                opcode |= RegbitsL;
                break;
        }

        ret.append(opcode);
    } else if ((op1.isIndirectReg16() && op1.reg16() == Z80::Z80::Register16::HL)) {
        ret.append(0xa6);
    } else if (op1.isByte()) {
        /* AND A,n */
        ret.append(0xe6);
        ret.append(op1.byte());
    } else if (op1.isIndirectReg16WithOffset()) {
        /* AND A,(IX+d)
		 * AND A,(IY+d) */
        if (op1.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        } else if (op1.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.append(0x8e);
        ret.append(op1.offset());
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleBIT(const QStringList & tokens)
{
    int c = tokens.count();
    Opcode ret;

    if (c < 3) {
        std::cout << "BIT requires two operands\n";
        return InvalidInstruction;
    }

    Z80Operand op1(tokens.at(1));
    Z80Operand op2(tokens.at(2));

    if (!op1.isBitIndex()) {
        return InvalidInstruction;
    }
    Z80::Z80::UnsignedByte opcode = 0x40;
    opcode += (op1.bitIndex() << 3);

    if (op2.isReg8() || (op2.isIndirectReg16() && op2.reg16() == Z80::Z80::Register16::HL)) {
        /* BIT b,reg8
			BIT b,(HL) */

        ret.append(0xcb);

        if (op2.isIndirectReg16() && op2.reg16() == Z80::Z80::Register16::HL) {
            opcode |= RegbitsIndirectHl;
        } else if (op2.reg8() == Z80::Z80::Register8::A) {
            opcode |= RegbitsA;
        } else if (op2.reg8() == Z80::Z80::Register8::B) {
            opcode |= RegbitsB;
        } else if (op2.reg8() == Z80::Z80::Register8::C) {
            opcode |= RegbitsC;
        } else if (op2.reg8() == Z80::Z80::Register8::D) {
            opcode |= RegbitsD;
        } else if (op2.reg8() == Z80::Z80::Register8::E) {
            opcode |= RegbitsE;
        } else if (op2.reg8() == Z80::Z80::Register8::H) {
            opcode |= RegbitsH;
        } else if (op2.reg8() == Z80::Z80::Register8::L) {
            opcode |= RegbitsL;
        } else {
            return InvalidInstruction;
        }

        ret.append(opcode);
    } else if (op2.isIndirectReg16WithOffset()) {
        /* BIT b,(IX+d)
		 * BIT b,(IY+d) */
        if (op2.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        } else if (op2.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.append(0xcb);
        ret.append(op2.offset());
        ret.append(opcode | RegbitsIndirectIxIy);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleCALL(const QStringList & tokens)
{
    int c = tokens.count();
    Opcode ret;

    if (2 > c) {
        std::cout << "CALL requires at least one operand\n";
        return InvalidInstruction;
    }

    if (2 == c) {
        Z80Operand op(tokens.at(1));

        if (!op.isWord()) {
            std::cout << "unconditional CALL requires a 16-bit direct address as its only operand\n";
            return InvalidInstruction;
        }

        ret.append(0xcd);
        ret.append(op.wordLowByte());
        ret.append(op.wordHighByte());
    } else if (3 == c) {
        Z80Operand op1(tokens.at(1));
        Z80Operand op2(tokens.at(2));

        if (!op1.isCondition()) {
            std::cout << "conditional CALL requires a valid call condition as its first operand\n";
            return InvalidInstruction;
        }

        if (!op2.isWord()) {
            std::cout << "conditional CALL requires a 16-bit direct address as its second operand\n";
            return InvalidInstruction;
        }

        switch (op1.condition()) {
            case Z80Operand::NonZero:
                ret.append(0xc4);
                break;
            case Z80Operand::Zero:
                ret.append(0xcc);
                break;
            case Z80Operand::NoCarry:
                ret.append(0xd4);
                break;
            case Z80Operand::Carry:
                ret.append(0xdc);
                break;
            case Z80Operand::ParityOdd:
                ret.append(0xe4);
                break;
            case Z80Operand::ParityEven:
                ret.append(0xec);
                break;
            case Z80Operand::Plus:
                ret.append(0xf4);
                break;
            case Z80Operand::Minus:
                ret.append(0xfc);
                break;
            default:
                return InvalidInstruction;
        }

        ret.append(op2.wordLowByte());
        ret.append(op2.wordHighByte());
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleCCF(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0x3f);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleCP(const QStringList & tokens)
{
    int c = tokens.count();
    Opcode ret;

    if (c < 2) {
        std::cout << "CP requires one operand\n";
        return InvalidInstruction;
    }

    Z80Operand op(tokens.at(1));

    if (op.isReg8()) {
        /* CP reg8 */
        Z80::Z80::UnsignedByte opcode = 0xb8;

        switch (op.reg8()) {
            case Z80::Z80::Register8::A:
                opcode |= RegbitsA;
                break;
            case Z80::Z80::Register8::B:
                opcode |= RegbitsB;
                break;
            case Z80::Z80::Register8::C:
                opcode |= RegbitsC;
                break;
            case Z80::Z80::Register8::D:
                opcode |= RegbitsD;
                break;
            case Z80::Z80::Register8::E:
                opcode |= RegbitsE;
                break;
            case Z80::Z80::Register8::H:
                opcode |= RegbitsH;
                break;
            case Z80::Z80::Register8::L:
                opcode |= RegbitsL;
                break;
            default:
                return InvalidInstruction;
        }

        ret.append(opcode);
    } else if ((op.isIndirectReg16() && op.reg16() == Z80::Z80::Register16::HL)) {
        /* CP (HL) */
        ret.append(0xbe);
    } else if (op.isIndirectReg16WithOffset()) {
        /* CP (IX+d)
		 * CP (IY+d) */
        if (op.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        } else if (op.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.append(0xbe);
        ret.append(op.offset());
    } else if (op.isByte()) {
        /* CP n */
        ret.append(0xfe);
        ret.append(op.byte());
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleCPD(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0xa9);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleCPDR(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0xb9);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleCPI(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0xa1);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleCPIR(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0xb1);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleCPL(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0x2f);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleDAA(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0x27);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleDEC(const QStringList & tokens)
{
    Opcode ret;

    if (2 > tokens.count()) {
        std::cout << "DEC instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Z80Operand op(tokens.at(1));

    if (op.isReg8()) {
        /* DEC reg8 */
        switch (op.reg8()) {
            case Z80::Z80::Register8::A:
                ret.append(0x3d);
                break;
            case Z80::Z80::Register8::B:
                ret.append(0x05);
                break;
            case Z80::Z80::Register8::C:
                ret.append(0x0d);
                break;
            case Z80::Z80::Register8::D:
                ret.append(0x15);
                break;
            case Z80::Z80::Register8::E:
                ret.append(0x1d);
                break;
            case Z80::Z80::Register8::H:
                ret.append(0x25);
                break;
            case Z80::Z80::Register8::L:
                ret.append(0x2d);
                break;
            default:
                return InvalidInstruction;
        }
    } else if (op.isReg16()) {
        switch (op.reg16()) {
            case Z80::Z80::Register16::BC:
                ret.append(0x0b);
                break;
            case Z80::Z80::Register16::DE:
                ret.append(0x1b);
                break;
            case Z80::Z80::Register16::HL:
                ret.append(0x2b);
                break;
            case Z80::Z80::Register16::SP:
                ret.append(0x3b);
                break;
            case Z80::Z80::Register16::IX:
                ret.append(0xdd);
                ret.append(0x2b);
                break;
            case Z80::Z80::Register16::IY:
                ret.append(0xfd);
                ret.append(0x2b);
                break;
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16() && op.reg16() == Z80::Z80::Register16::HL) {
        /* DEC (HL) */
        ret.append(0x35);
    } else if (op.isIndirectReg16WithOffset()) {
        /* DEC (IX+d) */
        /* DEC (IY+d) */
        if (op.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        } else if (op.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.append(0x35);
        ret.append(op.offset());
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleDI(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xf3);
    return ret;
}

/* need to handle relative offset operand type before we can assemble DJNZ */
Z80Interpreter::Opcode Z80Interpreter::assembleDJNZ(const QStringList & tokens)
{
    Z80Interpreter::Opcode ret;

    if (2 > tokens.count()) {
        std::cout << "DJNZ instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Z80Operand op(tokens.at(1));
    if (!op.isOffset()) {
        return InvalidInstruction;
    }
    ret.append(0x10);
    ret.append(op.offset());
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleEI(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xfb);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleEX(const QStringList & tokens)
{
    Opcode ret;
    int c = tokens.count();

    if (3 > c) {
        std::cout << "EX requires two operands.\n";
        return InvalidInstruction;
    }

    Z80Operand op1(tokens.at(1));
    Z80Operand op2(tokens.at(2));

    if (op1.isIndirectReg16() && op1.reg16() == Z80::Z80::Register16::SP && op2.isReg16()) {
        switch (op2.reg16()) {
            default:
                return InvalidInstruction;
            case Z80::Z80::Register16::HL:
                ret.append(0xe3);
                break;
            case Z80::Z80::Register16::IX:
                ret.append(0xdd);
                ret.append(0xe3);
                break;
            case Z80::Z80::Register16::IY:
                ret.append(0xfd);
                ret.append(0xe3);
                break;
        }
    } else if (op1.isReg16() && op2.isReg16()) {
        if (op1.reg16() == Z80::Z80::Register16::AF && op2.reg16() == Z80::Z80::Register16::AFShadow) {
            ret.append(0x08);
        } else if (op1.reg16() == Z80::Z80::Register16::DE && op2.reg16() == Z80::Z80::Register16::HL) {
            ret.append(0xeb);
        } else {
            return InvalidInstruction;
        }
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleEXX(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xd9);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleHALT(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0x76);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleIM(const QStringList & tokens)
{
    Opcode ret;

    if (2 > tokens.count()) {
        std::cout << "IM requires one operand.\n";
        return InvalidInstruction;
    }

    ret.append(0xed);

    if ("0" == tokens.at(1).toUpper()) {
        ret.append(0x46);
    } else if ("1" == tokens.at(1).toUpper()) {
        ret.append(0x56);
    } else if ("2" == tokens.at(1).toUpper()) {
        ret.append(0x5e);
    } else {
        std::cout << "Invalid interrupt mode (must be 0, 1 or 2).\n";
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleIN(const QStringList & tokens)
{
    Opcode ret;
    int c = tokens.count();

    if (2 == c) {
        Z80Operand op(tokens.at(1));

        if (op.isIndirectReg8() && op.reg8() == Z80::Z80::Register8::C) {
            ret.append(0xed);
            ret.append(0x70);
        } else {
            return InvalidInstruction;
        }
    } else if (3 == c) {
        Z80Operand op1(tokens.at(1));
        Z80Operand op2(tokens.at(2));

        /* TODO need IndirectByte operand type (perhaps it's IndirectPort?) - (N) */
//		if(op1.isReg8() && op1.reg8() == ) {
//		}
        if (op1.isReg8() && op2.isIndirectReg8() && op2.reg8() == Z80::Z80::Register8::C) {
            switch (op1.reg8()) {
                case Z80::Z80::Register8::B:
                    ret.append(0xed);
                    ret.append(0x40);
                    break;
                case Z80::Z80::Register8::C:
                    ret.append(0xed);
                    ret.append(0x48);
                    break;
                case Z80::Z80::Register8::D:
                    ret.append(0xed);
                    ret.append(0x50);
                    break;
                case Z80::Z80::Register8::E:
                    ret.append(0xed);
                    ret.append(0x58);
                    break;
                case Z80::Z80::Register8::H:
                    ret.append(0xed);
                    ret.append(0x60);
                    break;
                case Z80::Z80::Register8::L:
                    ret.append(0xed);
                    ret.append(0x68);
                    break;
                default:
                    return InvalidInstruction;
            }
        } else {
            return InvalidInstruction;
        }
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleINC(const QStringList & tokens)
{
    Opcode ret;
    int c = tokens.count();

    if (2 != c) {
        std::cout << "INC instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Z80Operand op(tokens.at(1));

    if (op.isReg8()) {
        /* INC reg8 */
        switch (op.reg8()) {
            case Z80::Z80::Register8::A:
                ret.append(0x3c);
                break;
            case Z80::Z80::Register8::B:
                ret.append(0x04);
                break;
            case Z80::Z80::Register8::C:
                ret.append(0x0c);
                break;
            case Z80::Z80::Register8::D:
                ret.append(0x14);
                break;
            case Z80::Z80::Register8::E:
                ret.append(0x1c);
                break;
            case Z80::Z80::Register8::H:
                ret.append(0x24);
                break;
            case Z80::Z80::Register8::L:
                ret.append(0x2c);
                break;
            default:
                return InvalidInstruction;
        }
    } else if (op.isReg16()) {
        /* INC reg16 */
        switch (op.reg16()) {
            case Z80::Z80::Register16::BC:
                ret.append(0x03);
                break;
            case Z80::Z80::Register16::DE:
                ret.append(0x13);
                break;
            case Z80::Z80::Register16::HL:
                ret.append(0x23);
                break;
            case Z80::Z80::Register16::SP:
                ret.append(0x33);
                break;
            case Z80::Z80::Register16::IX:
                ret.append(0xdd);
                ret.append(0x23);
                break;
            case Z80::Z80::Register16::IY:
                ret.append(0xfd);
                ret.append(0x23);
                break;
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16()) {
        /* INC (HL) */
        switch (op.reg16()) {
            case Z80::Z80::Register16::HL:
                ret.append(0x34);
                break;
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16WithOffset()) {
        /* INC (IX+n) */
        /* INC (IY+n) */
        switch (op.reg16()) {
            case Z80::Z80::Register16::IX:
                ret.append(0xdd);
                break;
            case Z80::Z80::Register16::IY:
                ret.append(0xfd);
                break;
            default:
                return InvalidInstruction;
        }

        ret.append(0x34);
        ret.append(op.offset());
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleIND(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0xaa);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleINDR(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0xba);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleINI(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0xa2);
    return ret;
}


Z80Interpreter::Opcode Z80Interpreter::assembleINIR(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0xb2);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleJP(const QStringList & tokens)
{
    int c = tokens.count();
    Opcode ret;

    if (2 > c) {
        std::cout << "JP requires at least one operand\n";
        return InvalidInstruction;
    }

    if (2 == c) {
        Z80Operand op(tokens.at(1));

        if (op.isWord()) {
            ret.append(0xc3);
            ret.append(op.wordLowByte());
            ret.append(op.wordHighByte());
        } else if (op.isIndirectReg16()) {
            switch (op.reg16()) {
                case Z80::Z80::Register16::HL:
                    ret.append(0xe9);
                    break;
                case Z80::Z80::Register16::IX: {
                    ret.append(0xdd);
                    ret.append(0xe9);
                    break;
                }
                case Z80::Z80::Register16::IY: {
                    ret.append(0xfd);
                    ret.append(0xe9);
                    break;
                }
                default:
                    return InvalidInstruction;
            }
        } else {
            return InvalidInstruction;
        }
    } else if (3 == c) {
        Z80Operand op1(tokens.at(1));
        Z80Operand op2(tokens.at(2));

        if (!op1.isCondition()) {
            std::cout << "conditional JP requires a valid call condition as its first operand\n";
            return InvalidInstruction;
        }

        if (!op2.isWord()) {
            std::cout << "conditional JP requires a 16-bit direct address as its second operand\n";
            return InvalidInstruction;
        }

        switch (op1.condition()) {
            case Z80Operand::NonZero:
                ret.append(0xc2);
                break;
            case Z80Operand::Zero:
                ret.append(0xca);
                break;
            case Z80Operand::NoCarry:
                ret.append(0xd2);
                break;
            case Z80Operand::Carry:
                ret.append(0xda);
                break;
            case Z80Operand::ParityOdd:
                ret.append(0xe2);
                break;
            case Z80Operand::ParityEven:
                ret.append(0xea);
                break;
            case Z80Operand::Plus:
                ret.append(0xf2);
                break;
            case Z80Operand::Minus:
                ret.append(0xfa);
                break;
            default:
                return InvalidInstruction;
        }

        ret.append(op2.wordLowByte());
        ret.append(op2.wordHighByte());
    } else {
        return InvalidInstruction;
    }

    return ret;
}


Z80Interpreter::Opcode Z80Interpreter::assembleJR(const QStringList & tokens)
{
    int c = tokens.count();
    Opcode ret;

    if (2 > c) {
        std::cout << "JR requires at least one operand\n";
        return InvalidInstruction;
    }

    if (2 == c) {
        Z80Operand op(tokens.at(1));

        if (op.isByte()) {
            ret.append(0x18);
            ret.append(op.byte());
        } else {
            return InvalidInstruction;
        }
    } else if (3 == c) {
        Z80Operand op1(tokens.at(1));
        Z80Operand op2(tokens.at(2));

        if (!op1.isCondition()) {
            std::cout << "conditional JR requires a valid condition as its first operand\n";
            return InvalidInstruction;
        }

        if (!op2.isByte()) {
            std::cout << "conditional JR requires an 8-bit jump offset as its second operand\n";
            return InvalidInstruction;
        }

        switch (op1.condition()) {
            case Z80Operand::NonZero:
                ret.append(0x20);
                break;
            case Z80Operand::Zero:
                ret.append(0x28);
                break;
            case Z80Operand::NoCarry:
                ret.append(0x30);
                break;
            case Z80Operand::Carry:
                ret.append(0x38);
                break;
            default:
                return InvalidInstruction;
        }

        ret.append(op2.byte());
    } else {
        return InvalidInstruction;
    }

    return ret;
}


Z80Interpreter::Opcode Z80Interpreter::assembleLD(const QStringList & tokens)
{
    int c = tokens.count();
    Opcode ret;

    if (c < 3) {
        std::cout << "LD requires two operands\n";
        return InvalidInstruction;
    }

    Z80Operand op1(tokens.at(1));
    Z80Operand op2(tokens.at(2));

    /* TODO support IXH and IYH */
    if (op1.isReg8() && (op2.isReg8() || (op2.isIndirectReg16() && op2.reg16() == Z80::Z80::Register16::HL))) {
        /* LD reg8,reg8
			LD reg8,(HL) */
        Z80::Z80::UnsignedByte opcode;

        switch (op1.reg8()) {
            case Z80::Z80::Register8::A:
                opcode = 0x78;
                break;
            case Z80::Z80::Register8::B:
                opcode = 0x40;
                break;
            case Z80::Z80::Register8::C:
                opcode = 0x48;
                break;
            case Z80::Z80::Register8::D:
                opcode = 0x50;
                break;
            case Z80::Z80::Register8::E:
                opcode = 0x58;
                break;
            case Z80::Z80::Register8::H:
                opcode = 0x60;
                break;
            case Z80::Z80::Register8::L:
                opcode = 0x68;
                break;
            default:
                return InvalidInstruction;
        }

        if (op2.isIndirectReg16() && op2.reg16() == Z80::Z80::Register16::HL) {
            opcode |= RegbitsIndirectHl;
        } else if (op2.reg8() == Z80::Z80::Register8::A) {
            opcode |= RegbitsA;
        } else if (op2.reg8() == Z80::Z80::Register8::B) {
            opcode |= RegbitsB;
        } else if (op2.reg8() == Z80::Z80::Register8::C) {
            opcode |= RegbitsC;
        } else if (op2.reg8() == Z80::Z80::Register8::D) {
            opcode |= RegbitsD;
        } else if (op2.reg8() == Z80::Z80::Register8::E) {
            opcode |= RegbitsE;
        } else if (op2.reg8() == Z80::Z80::Register8::H) {
            opcode |= RegbitsH;
        } else if (op2.reg8() == Z80::Z80::Register8::L) {
            opcode |= RegbitsL;
        }

        ret.append(opcode);
    } else if (op1.isReg8() && op2.isIndirectReg16WithOffset()) {
        /* LD reg8,(IX+n) */
        /* LD reg8,(IY+n) */
        switch (op2.reg16()) {
            case Z80::Z80::Register16::IX:
                ret.append(0xdd);
                break;
            case Z80::Z80::Register16::IY:
                ret.append(0xfd);
                break;
            default:
                return InvalidInstruction;
        }

        switch (op1.reg8()) {
            case Z80::Z80::Register8::A:
                ret.append(0x7e);
                break;
            case Z80::Z80::Register8::B:
                ret.append(0x46);
                break;
            case Z80::Z80::Register8::C:
                ret.append(0x4e);
                break;
            case Z80::Z80::Register8::D:
                ret.append(0x56);
                break;
            case Z80::Z80::Register8::E:
                ret.append(0x5e);
                break;
            case Z80::Z80::Register8::H:
                ret.append(0x66);
                break;
            case Z80::Z80::Register8::L:
                ret.append(0x6e);
                break;
            default:
                return InvalidInstruction;
        }

        ret.append(op2.offset());
    } else if (op1.isReg8() && op2.isByte()) {
        /* LD reg8,n */
        if (op1.reg8() == Z80::Z80::Register8::A) {
            ret.append(0x3e);
        } else if (op1.reg8() == Z80::Z80::Register8::B) {
            ret.append(0x06);
        } else if (op1.reg8() == Z80::Z80::Register8::C) {
            ret.append(0x0e);
        } else if (op1.reg8() == Z80::Z80::Register8::D) {
            ret.append(0x16);
        } else if (op1.reg8() == Z80::Z80::Register8::E) {
            ret.append(0x1e);
        } else if (op1.reg8() == Z80::Z80::Register8::H) {
            ret.append(0x26);
        } else if (op1.reg8() == Z80::Z80::Register8::L) {
            ret.append(0x2e);
        } else if (op1.reg8() == Z80::Z80::Register8::IXH) {
            ret.append(0xdd);
            ret.append(0x26);
        } else if (op1.reg8() == Z80::Z80::Register8::IXL) {
            ret.append(0xdd);
            ret.append(0x2e);
        } else if (op1.reg8() == Z80::Z80::Register8::IYH) {
            ret.append(0xfd);
            ret.append(0x26);
        } else if (op1.reg8() == Z80::Z80::Register8::IYL) {
            ret.append(0xfd);
            ret.append(0x2e);
        }

        ret.append(op2.byte());
    } else if (op1.isReg8() && op1.reg8() == Z80::Z80::Register8::A) {
        /* LD A,I
			LD A,R
			LD A,(reg16)   - (not including (HL), which is handled above)
			LD A,(nn) */
        if (op2.isReg8()) {
            if (op2.reg8() == Z80::Z80::Register8::I) {
                ret.append(0xed);
                ret.append(0x57);
            }
            if (op2.reg8() == Z80::Z80::Register8::R) {
                ret.append(0xed);
                ret.append(0x5f);
            } else {
                return InvalidInstruction;
            }
        } else if (op2.isIndirectReg16()) {
            if (op2.reg16() == Z80::Z80::Register16::BC) {
                ret.append(0x0a);
            } else if (op2.reg16() == Z80::Z80::Register16::DE) {
                ret.append(0x1a);
            } else {
                return InvalidInstruction;
            }
        } else if (op2.isIndirectAddress()) {
            ret.append(0x3a);
            ret.append(op2.wordLowByte());
            ret.append(op2.wordHighByte());
        } else {
            return InvalidInstruction;
        }
    } else if (op1.isReg16()) {
        if (op2.isIndirectAddress()) {
            /* LD reg16,(nn) */
            if (op1.reg16() == Z80::Z80::Register16::BC) {
                ret.append(0xed);
                ret.append(0x4b);
            } else if (op1.reg16() == Z80::Z80::Register16::DE) {
                ret.append(0xed);
                ret.append(0x5b);
            } else if (op1.reg16() == Z80::Z80::Register16::HL) {
                ret.append(0x2a);
            } else if (op1.reg16() == Z80::Z80::Register16::SP) {
                ret.append(0x7b);
            } else if (op1.reg16() == Z80::Z80::Register16::IX) {
                ret.append(0xdd);
                ret.append(0x2a);
            } else if (op1.reg16() == Z80::Z80::Register16::IY) {
                ret.append(0xfd);
                ret.append(0x2a);
            }
            ret.append(op2.wordLowByte());
            ret.append(op2.wordHighByte());
        } else if (op2.isWord()) {
            /* LD reg16,nn */
            if (op1.reg16() == Z80::Z80::Register16::BC) {
                ret.append(0x01);
            } else if (op1.reg16() == Z80::Z80::Register16::DE) {
                ret.append(0x11);
            } else if (op1.reg16() == Z80::Z80::Register16::HL) {
                ret.append(0x21);
            } else if (op1.reg16() == Z80::Z80::Register16::SP) {
                ret.append(0x31);
            } else if (op1.reg16() == Z80::Z80::Register16::IX) {
                ret.append(0xdd);
                ret.append(0x21);
            } else if (op1.reg16() == Z80::Z80::Register16::IY) {
                ret.append(0xfd);
                ret.append(0x21);
            } else {
                return InvalidInstruction;
            }
            ret.append(op2.wordLowByte());
            ret.append(op2.wordHighByte());
        } else if (op1.isReg16() && op1.reg16() == Z80::Z80::Register16::SP && op2.isReg16()) {
            if (op2.reg16() == Z80::Z80::Register16::HL) {
                ret.append(0xf9);
            } else if (op2.reg16() == Z80::Z80::Register16::IX) {
                ret.append(0xdd);
                ret.append(0xf9);
            } else if (op2.reg16() == Z80::Z80::Register16::IY) {
                ret.append(0xfd);
                ret.append(0xf9);
            } else {
                return InvalidInstruction;
            }
        }
    } else if (op1.isIndirectAddress()) {
        /* LD (nn),reg16
			LD (nn),A */
        if (op2.isReg8() && op2.reg8() == Z80::Z80::Register8::A) {
            ret.append(0x32);
        } else if (op2.isReg16() && op2.reg16() == Z80::Z80::Register16::BC) {
            ret.append(0xed);
            ret.append(0x43);
        } else if (op2.isReg16() && op2.reg16() == Z80::Z80::Register16::DE) {
            ret.append(0xed);
            ret.append(0x32);
        } else if (op2.isReg16() && op2.reg16() == Z80::Z80::Register16::HL) {
            ret.append(0x22);
        } else if (op2.isReg16() && op2.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
            ret.append(0x22);
        } else if (op2.isReg16() && op2.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
            ret.append(0x22);
        } else if (op2.isReg16() && op2.reg16() == Z80::Z80::Register16::SP) {
            ret.append(0xed);
            ret.append(0x73);
        } else {
            return InvalidInstruction;
        }

        ret.append(op2.wordLowByte());
        ret.append(op2.wordHighByte());
    } else if (op1.isIndirectReg16()) {
        /* LD (HL),reg8
			LD (HL),n
			LD (BC),A
			LD (DE),A */
        if (op1.reg16() == Z80::Z80::Register16::HL) {
            Z80::Z80::UnsignedByte opcode = 0x70;

            if (op2.isReg8()) {
                if (op2.reg8() == Z80::Z80::Register8::A) {
                    ret.append(opcode | RegbitsA);
                } else if (op2.reg8() == Z80::Z80::Register8::B) {
                    ret.append(opcode | RegbitsB);
                } else if (op2.reg8() == Z80::Z80::Register8::C) {
                    ret.append(opcode | RegbitsC);
                } else if (op2.reg8() == Z80::Z80::Register8::D) {
                    ret.append(opcode | RegbitsD);
                } else if (op2.reg8() == Z80::Z80::Register8::E) {
                    ret.append(opcode | RegbitsE);
                } else if (op2.reg8() == Z80::Z80::Register8::H) {
                    ret.append(opcode | RegbitsH);
                } else if (op2.reg8() == Z80::Z80::Register8::L) {
                    ret.append(opcode | RegbitsL);
                } else {
                    return InvalidInstruction;
                }
            } else if (op2.isByte()) {
                ret.append(0x36);
                ret.append(op2.byte());
            } else {
                return InvalidInstruction;
            }
        } else if (op1.reg16() == Z80::Z80::Register16::BC && op2.isReg8() && op2.reg8() == Z80::Z80::Register8::A) {
            ret.append(0x02);
        } else if (op1.reg16() == Z80::Z80::Register16::DE && op2.isReg8() && op2.reg8() == Z80::Z80::Register8::A) {
            ret.append(0x12);
        } else {
            return InvalidInstruction;
        }
    } else if (op1.isIndirectReg16WithOffset()) {
        /* LD (IX+d),reg8
			LD (IX+d),n
			LD (IY+d),n
			LD (IY+d),n */
        if (op1.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        } else if (op1.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else {
            return InvalidInstruction;
        }

        Z80::Z80::UnsignedByte opcode = 0x70;

        if (op2.isReg8()) {
            if (op2.reg8() == Z80::Z80::Register8::A) {
                ret.append(opcode | RegbitsA);
            } else if (op2.reg8() == Z80::Z80::Register8::B) {
                ret.append(opcode | RegbitsB);
            } else if (op2.reg8() == Z80::Z80::Register8::C) {
                ret.append(opcode | RegbitsC);
            } else if (op2.reg8() == Z80::Z80::Register8::D) {
                ret.append(opcode | RegbitsD);
            } else if (op2.reg8() == Z80::Z80::Register8::E) {
                ret.append(opcode | RegbitsE);
            } else if (op2.reg8() == Z80::Z80::Register8::H) {
                ret.append(opcode | RegbitsH);
            } else if (op2.reg8() == Z80::Z80::Register8::L) {
                ret.append(opcode | RegbitsL);
            } else {
                return InvalidInstruction;
            }

            ret.append(op1.offset());
        } else if (op2.isByte()) {
            ret.append(0x36);

            /* is this the right way round? */
            ret.append(op2.byte());
            ret.append(op1.offset());
        } else {
            return InvalidInstruction;
        }
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleLDD(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0xa8);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleLDDR(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0xb8);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleLDI(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0xa0);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleLDIR(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0xb0);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleNEG(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0x44);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleNOP(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0x00);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleOR(const QStringList & tokens)
{
    int c = tokens.count();
    Opcode ret;

    if (c < 2) {
        std::cout << "OR requires one operand\n";
        return InvalidInstruction;
    }

    Z80Operand op(tokens.at(1));

    if (op.isReg8()) {
        /* OR A,reg8 */
        Z80::Z80::UnsignedByte opcode = 0xb0;

        switch (op.reg8()) {
            case Z80::Z80::Register8::A:
                opcode |= RegbitsA;
                break;
            case Z80::Z80::Register8::B:
                opcode |= RegbitsB;
                break;
            case Z80::Z80::Register8::C:
                opcode |= RegbitsC;
                break;
            case Z80::Z80::Register8::D:
                opcode |= RegbitsD;
                break;
            case Z80::Z80::Register8::E:
                opcode |= RegbitsE;
                break;
            case Z80::Z80::Register8::H:
                opcode |= RegbitsH;
                break;
            case Z80::Z80::Register8::L:
                opcode |= RegbitsL;
                break;
            default:
                return InvalidInstruction;
        }

        ret.append(opcode);
    } else if (op.isIndirectReg16() && op.reg16() == Z80::Z80::Register16::HL) {
        /* OR A,(HL) */
        ret.append(0xb6);
    } else if (op.isIndirectReg16WithOffset()) {
        /* OR A,(IX+n) */
        /* OR A,(IY+n) */
        if (op.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        } else if (op.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.append(0xb6);
        ret.append(op.offset());
    } else if (op.isByte()) {
        /* OR A,n */
        ret.append(0xf6);
        ret.append(op.byte());
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleOUT(const QStringList & tokens)
{
    Opcode ret;
    int c = tokens.count();

    if (3 == c) {
        Z80Operand op1(tokens.at(1));
        Z80Operand op2(tokens.at(2));

        /* TODO need IndirectByte operand type (perhaps it's IndirectPort?) - (N) */
//		if(op1.isReg8() && op1.reg8() == ) {
//		}
        if (op1.isIndirectReg8() && op1.reg8() == Z80::Z80::Register8::C) {
            if (op2.isReg8()) {
                switch (op2.reg8()) {
                    case Z80::Z80::Register8::B:
                        ret.append(0xed);
                        ret.append(0x41);
                        break;
                    case Z80::Z80::Register8::C:
                        ret.append(0xed);
                        ret.append(0x49);
                        break;
                    case Z80::Z80::Register8::D:
                        ret.append(0xed);
                        ret.append(0x51);
                        break;
                    case Z80::Z80::Register8::E:
                        ret.append(0xed);
                        ret.append(0x59);
                        break;
                    case Z80::Z80::Register8::H:
                        ret.append(0xed);
                        ret.append(0x61);
                        break;
                    case Z80::Z80::Register8::L:
                        ret.append(0xed);
                        ret.append(0x69);
                        break;
                    default:
                        return InvalidInstruction;
                }
            } else if (op2.isByte() && 0 == op2.byte()) {
                ret.append(0xed);
                ret.append(0x71);
            } else {
                return InvalidInstruction;
            }
        } else {
            return InvalidInstruction;
        }
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleOUTD(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0xab);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleOTDR(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0xbb);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleOUTI(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0xa3);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleOTIR(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0xb3);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assemblePOP(const QStringList & tokens)
{
    int c = tokens.count();
    Opcode ret;

    if (c < 2) {
        std::cout << "POP requires one operand\n";
        return InvalidInstruction;
    }

    Z80Operand op(tokens.at(1));
    if (!op.isReg16()) {
        return InvalidInstruction;
    }

    if (op.reg16() == Z80::Z80::Register16::AF) {
        ret.append(0xf1);
    } else if (op.reg16() == Z80::Z80::Register16::BC) {
        ret.append(0xc1);
    } else if (op.reg16() == Z80::Z80::Register16::DE) {
        ret.append(0xd1);
    } else if (op.reg16() == Z80::Z80::Register16::HL) {
        ret.append(0xe1);
    } else if (op.reg16() == Z80::Z80::Register16::IX) {
        ret.append(0xdd);
        ret.append(0xe1);
    } else if (op.reg16() == Z80::Z80::Register16::IY) {
        ret.append(0xfd);
        ret.append(0xe1);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assemblePUSH(const QStringList & tokens)
{
    int c = tokens.count();
    Opcode ret;

    if (c < 2) {
        std::cout << "PUSH requires one operand\n";
        return InvalidInstruction;
    }

    Z80Operand op(tokens.at(1));
    if (!op.isReg16()) {
        return InvalidInstruction;
    }

    if (op.reg16() == Z80::Z80::Register16::AF) {
        ret.append(0xf5);
    } else if (op.reg16() == Z80::Z80::Register16::BC) {
        ret.append(0xc5);
    } else if (op.reg16() == Z80::Z80::Register16::DE) {
        ret.append(0xd5);
    } else if (op.reg16() == Z80::Z80::Register16::HL) {
        ret.append(0xe5);
    } else if (op.reg16() == Z80::Z80::Register16::IX) {
        ret.append(0xdd);
        ret.append(0xe5);
    } else if (op.reg16() == Z80::Z80::Register16::IY) {
        ret.append(0xfd);
        ret.append(0xe5);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRES(const QStringList & tokens)
{
    int c = tokens.count();
    Opcode ret;

    if (c < 3) {
        std::cout << "RES requires two operands\n";
        return InvalidInstruction;
    }

    Z80Operand op1(tokens.at(1));
    Z80Operand op2(tokens.at(2));

    if (!op1.isBitIndex()) {
        return InvalidInstruction;
    }
    Z80::Z80::UnsignedByte opcode = 0x80;
    opcode += (op1.bitIndex() << 3);

    if (op2.isReg8() || (op2.isIndirectReg16() && op2.reg16() == Z80::Z80::Register16::HL)) {
        /* RES b,reg8
			RES b,(HL) */

        ret.append(0xcb);

        if (op2.isIndirectReg16() && op2.reg16() == Z80::Z80::Register16::HL) {
            opcode |= RegbitsIndirectHl;
        } else if (op2.reg8() == Z80::Z80::Register8::A) {
            opcode |= RegbitsA;
        } else if (op2.reg8() == Z80::Z80::Register8::B) {
            opcode |= RegbitsB;
        } else if (op2.reg8() == Z80::Z80::Register8::C) {
            opcode |= RegbitsC;
        } else if (op2.reg8() == Z80::Z80::Register8::D) {
            opcode |= RegbitsD;
        } else if (op2.reg8() == Z80::Z80::Register8::E) {
            opcode |= RegbitsE;
        } else if (op2.reg8() == Z80::Z80::Register8::H) {
            opcode |= RegbitsH;
        } else if (op2.reg8() == Z80::Z80::Register8::L) {
            opcode |= RegbitsL;
        } else {
            return InvalidInstruction;
        }

        ret.append(opcode);
    } else if (op2.isIndirectReg16WithOffset()) {
        /* RES b,(IX+n) */
        /* RES b,(IY+n) */
        if (op2.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        } else if (op2.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.append(0xcb);
        ret.append(op2.offset());
        ret.append(opcode | RegbitsIndirectIxIy);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRET(const QStringList & tokens)
{
    int c = tokens.count();
    Opcode ret;

    if (1 == c) {
        ret.append(0xc9);
    } else if (2 == c) {
        Z80Operand op(tokens.at(1));

        if (!op.isCondition()) {
            std::cout << "conditional RET requires a valid condition as its operand\n";
            return InvalidInstruction;
        }

        switch (op.condition()) {
            case Z80Operand::NonZero:
                ret.append(0xc0);
                break;
            case Z80Operand::Zero:
                ret.append(0xc8);
                break;
            case Z80Operand::NoCarry:
                ret.append(0xd0);
                break;
            case Z80Operand::Carry:
                ret.append(0xd8);
                break;
            case Z80Operand::ParityOdd:
                ret.append(0xe0);
                break;
            case Z80Operand::ParityEven:
                ret.append(0xe8);
                break;
            case Z80Operand::Plus:
                ret.append(0xf0);
                break;
            case Z80Operand::Minus:
                ret.append(0xf8);
                break;
            default:
                return InvalidInstruction;
        }
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRETI(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0x4d);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRETN(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0x45);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRLA(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0x17);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRL(const QStringList & tokens)
{
    int c = tokens.count();

    if (2 != c) {
        std::cout << "RL instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Opcode ret;
    Z80Operand op(tokens.at(1));

    if (op.isReg8()) {
        ret.append(0xcb);

        switch (op.reg8()) {
            case Z80::Z80::Register8::A:
                ret.append(0x10 | RegbitsA);
            case Z80::Z80::Register8::B:
                ret.append(0x10 | RegbitsB);
            case Z80::Z80::Register8::C:
                ret.append(0x10 | RegbitsC);
            case Z80::Z80::Register8::D:
                ret.append(0x10 | RegbitsD);
            case Z80::Z80::Register8::E:
                ret.append(0x10 | RegbitsE);
            case Z80::Z80::Register8::H:
                ret.append(0x10 | RegbitsH);
            case Z80::Z80::Register8::L:
                ret.append(0x10 | RegbitsL);
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16() && op.reg16() == Z80::Z80::Register16::HL) {
        ret.append(0xcb);
        ret.append(0x10 | RegbitsIndirectHl);
    } else if (op.isIndirectReg16WithOffset()) {
        if (op.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        }
        if (op.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.append(op.offset());
        ret.append(0x16);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRLCA(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0x07);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRLC(const QStringList & tokens)
{
    int c = tokens.count();

    if (2 != c) {
        std::cout << "RLC instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Opcode ret;
    Z80Operand op(tokens.at(1));

    if (op.isReg8()) {
        ret.append(0xcb);

        switch (op.reg8()) {
            case Z80::Z80::Register8::A:
                ret.append(0x00 | RegbitsA);
            case Z80::Z80::Register8::B:
                ret.append(0x00 | RegbitsB);
            case Z80::Z80::Register8::C:
                ret.append(0x00 | RegbitsC);
            case Z80::Z80::Register8::D:
                ret.append(0x00 | RegbitsD);
            case Z80::Z80::Register8::E:
                ret.append(0x00 | RegbitsE);
            case Z80::Z80::Register8::H:
                ret.append(0x00 | RegbitsH);
            case Z80::Z80::Register8::L:
                ret.append(0x00 | RegbitsL);
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16() && op.reg16() == Z80::Z80::Register16::HL) {
        ret.append(0xcb);
        ret.append(0x00 | RegbitsIndirectHl);
    } else if (op.isIndirectReg16WithOffset()) {
        if (op.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        }
        if (op.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.append(op.offset());
        ret.append(0x06);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRLD(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0x6f);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRRA(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0x1f);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRR(const QStringList & tokens)
{
    int c = tokens.count();

    if (2 != c) {
        std::cout << "RR instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Opcode ret;
    Z80Operand op(tokens.at(1));

    if (op.isReg8()) {
        ret.append(0xcb);

        switch (op.reg8()) {
            case Z80::Z80::Register8::A:
                ret.append(0x18 | RegbitsA);
            case Z80::Z80::Register8::B:
                ret.append(0x18 | RegbitsB);
            case Z80::Z80::Register8::C:
                ret.append(0x18 | RegbitsC);
            case Z80::Z80::Register8::D:
                ret.append(0x18 | RegbitsD);
            case Z80::Z80::Register8::E:
                ret.append(0x18 | RegbitsE);
            case Z80::Z80::Register8::H:
                ret.append(0x18 | RegbitsH);
            case Z80::Z80::Register8::L:
                ret.append(0x18 | RegbitsL);
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16() && op.reg16() == Z80::Z80::Register16::HL) {
        ret.append(0xcb);
        ret.append(0x18 | RegbitsIndirectHl);
    } else if (op.isIndirectReg16WithOffset()) {
        if (op.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        }
        if (op.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.append(op.offset());
        ret.append(0x1e);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRRCA(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0x0f);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRRC(const QStringList & tokens)
{
    int c = tokens.count();

    if (2 != c) {
        std::cout << "RRC instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Opcode ret;
    Z80Operand op(tokens.at(1));

    if (op.isReg8()) {
        ret.append(0xcb);

        switch (op.reg8()) {
            case Z80::Z80::Register8::A:
                ret.append(0x08 | RegbitsA);
            case Z80::Z80::Register8::B:
                ret.append(0x08 | RegbitsB);
            case Z80::Z80::Register8::C:
                ret.append(0x08 | RegbitsC);
            case Z80::Z80::Register8::D:
                ret.append(0x08 | RegbitsD);
            case Z80::Z80::Register8::E:
                ret.append(0x08 | RegbitsE);
            case Z80::Z80::Register8::H:
                ret.append(0x08 | RegbitsH);
            case Z80::Z80::Register8::L:
                ret.append(0x08 | RegbitsL);
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16() && op.reg16() == Z80::Z80::Register16::HL) {
        ret.append(0xcb);
        ret.append(0x08 | RegbitsIndirectHl);
    } else if (op.isIndirectReg16WithOffset()) {
        if (op.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        }
        if (op.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.append(op.offset());
        ret.append(0x0e);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRRD(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0xed);
    ret.append(0x67);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRST(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    return Opcode();
}

Z80Interpreter::Opcode Z80Interpreter::assembleSBC(const QStringList & tokens)
{
    int c = tokens.count();

    if (2 > tokens.count()) {
        std::cout << "SBC instruction requires at least one operand.\n";
        return InvalidInstruction;
    }

    Opcode ret;

    if (2 == c) {
        Z80Operand op(tokens.at(1));

        if (op.isReg8()) {
            switch (op.reg8()) {
                case Z80::Z80::Register8::A:
                    ret.append(0x98 | RegbitsA);
                    break;
                case Z80::Z80::Register8::B:
                    ret.append(0x98 | RegbitsB);
                    break;
                case Z80::Z80::Register8::C:
                    ret.append(0x98 | RegbitsC);
                    break;
                case Z80::Z80::Register8::D:
                    ret.append(0x98 | RegbitsD);
                    break;
                case Z80::Z80::Register8::E:
                    ret.append(0x98 | RegbitsE);
                    break;
                case Z80::Z80::Register8::H:
                    ret.append(0x98 | RegbitsH);
                    break;
                case Z80::Z80::Register8::L:
                    ret.append(0x98 | RegbitsL);
                    break;
                default:
                    return InvalidInstruction;
            }
        } else if (op.isIndirectReg16() && op.reg16() == Z80::Z80::Register16::HL) {
            ret.append(0xcb);
            ret.append(0x98 | RegbitsIndirectHl);
        } else {
            return InvalidInstruction;
        }
    } else if (3 == c) {
        Z80Operand op1(tokens.at(1));
        Z80Operand op2(tokens.at(2));

        if (op1.isReg8() && op1.reg8() == Z80::Z80::Register8::A) {
            if (op2.isByte()) {
                ret.append(0xde);
                ret.append(op2.byte());
            } else if (op2.isIndirectReg16WithOffset()) {
                if (op2.reg16() == Z80::Z80::Register16::IX) {
                    ret.append(0xdd);
                }
                if (op2.reg16() == Z80::Z80::Register16::IY) {
                    ret.append(0xfd);
                } else {
                    return InvalidInstruction;
                }

                ret.append(0x9e);
                ret.append(op2.offset());
            } else {
                return InvalidInstruction;
            }
        } else if (op1.isReg16() && op1.reg16() == Z80::Z80::Register16::HL && op2.isReg16()) {
            ret.append(0xed);

            switch (op2.reg16()) {
                case Z80::Z80::Register16::BC:
                    ret.append(0x42);
                    break;
                case Z80::Z80::Register16::DE:
                    ret.append(0x52);
                    break;
                case Z80::Z80::Register16::HL:
                    ret.append(0x62);
                    break;
                case Z80::Z80::Register16::SP:
                    ret.append(0x72);
                    break;
                default:
                    return InvalidInstruction;
            }
        } else {
            return InvalidInstruction;
        }
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleSCF(const QStringList & tokens)
{
    Q_UNUSED(tokens);
    Opcode ret;
    ret.append(0x37);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleSET(const QStringList & tokens)
{
    int c = tokens.count();
    Opcode ret;

    if (c < 3) {
        std::cout << "SET instruction requires two operands\n";
        return InvalidInstruction;
    }

    Z80Operand op1(tokens.at(1));
    Z80Operand op2(tokens.at(2));

    if (!op1.isBitIndex()) {
        return InvalidInstruction;
    }
    Z80::Z80::UnsignedByte opcode = 0xc0;
    opcode += (op1.bitIndex() << 3);

    if (op2.isReg8() || (op2.isIndirectReg16() && op2.reg16() == Z80::Z80::Register16::HL)) {
        /* SET b,reg8
			SET b,(HL) */
        ret.append(0xcb);

        if (op2.isIndirectReg16() && op2.reg16() == Z80::Z80::Register16::HL) {
            opcode |= RegbitsIndirectHl;
        } else if (op2.reg8() == Z80::Z80::Register8::A) {
            opcode |= RegbitsA;
        } else if (op2.reg8() == Z80::Z80::Register8::B) {
            opcode |= RegbitsB;
        } else if (op2.reg8() == Z80::Z80::Register8::C) {
            opcode |= RegbitsC;
        } else if (op2.reg8() == Z80::Z80::Register8::D) {
            opcode |= RegbitsD;
        } else if (op2.reg8() == Z80::Z80::Register8::E) {
            opcode |= RegbitsE;
        } else if (op2.reg8() == Z80::Z80::Register8::H) {
            opcode |= RegbitsH;
        } else if (op2.reg8() == Z80::Z80::Register8::L) {
            opcode |= RegbitsL;
        } else {
            return InvalidInstruction;
        }

        ret.append(opcode);
    } else if (op2.isIndirectReg16WithOffset()) {
        if (op2.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        }
        if (op2.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.append(0xcb);
        ret.append(op2.offset());
        ret.append(opcode | RegbitsIndirectIxIy);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleSLA(const QStringList & tokens)
{
    int c = tokens.count();

    if (2 != c) {
        std::cout << "SLA instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Opcode ret;
    Z80Operand op(tokens.at(1));

    if (op.isReg8()) {
        ret.append(0xcb);

        switch (op.reg8()) {
            case Z80::Z80::Register8::A:
                ret.append(0x20 | RegbitsA);
                break;
            case Z80::Z80::Register8::B:
                ret.append(0x20 | RegbitsB);
                break;
            case Z80::Z80::Register8::C:
                ret.append(0x20 | RegbitsC);
                break;
            case Z80::Z80::Register8::D:
                ret.append(0x20 | RegbitsD);
                break;
            case Z80::Z80::Register8::E:
                ret.append(0x20 | RegbitsE);
                break;
            case Z80::Z80::Register8::H:
                ret.append(0x20 | RegbitsH);
                break;
            case Z80::Z80::Register8::L:
                ret.append(0x20 | RegbitsL);
                break;
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16() && op.reg16() == Z80::Z80::Register16::HL) {
        ret.append(0xcb);
        ret.append(0x20 | RegbitsIndirectHl);
    } else if (op.isIndirectReg16WithOffset()) {
        if (op.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        }
        if (op.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.append(0xcb);
        ret.append(op.offset());
        ret.append(0x20 | RegbitsIndirectIxIy);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleSRA(const QStringList & tokens)
{
    int c = tokens.count();

    if (2 != c) {
        std::cout << "SRA instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Opcode ret;
    Z80Operand op(tokens.at(1));

    if (op.isReg8()) {
        ret.append(0xcb);

        switch (op.reg8()) {
            case Z80::Z80::Register8::A:
                ret.append(0x28 | RegbitsA);
                break;
            case Z80::Z80::Register8::B:
                ret.append(0x28 | RegbitsB);
                break;
            case Z80::Z80::Register8::C:
                ret.append(0x28 | RegbitsC);
                break;
            case Z80::Z80::Register8::D:
                ret.append(0x28 | RegbitsD);
                break;
            case Z80::Z80::Register8::E:
                ret.append(0x28 | RegbitsE);
                break;
            case Z80::Z80::Register8::H:
                ret.append(0x28 | RegbitsH);
                break;
            case Z80::Z80::Register8::L:
                ret.append(0x28 | RegbitsL);
                break;
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16() && op.reg16() == Z80::Z80::Register16::HL) {
        ret.append(0xcb);
        ret.append(0x28 | RegbitsIndirectHl);
    } else if (op.isIndirectReg16WithOffset()) {
        if (op.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        }
        if (op.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.append(0xcb);
        ret.append(op.offset());
        ret.append(0x28 | RegbitsIndirectIxIy);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleSLL(const QStringList & tokens)
{
    int c = tokens.count();

    if (2 != c) {
        std::cout << "SLL instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Opcode ret;
    Z80Operand op(tokens.at(1));

    if (op.isReg8()) {
        ret.append(0xcb);

        switch (op.reg8()) {
            case Z80::Z80::Register8::A:
                ret.append(0x30 | RegbitsA);
                break;
            case Z80::Z80::Register8::B:
                ret.append(0x30 | RegbitsB);
                break;
            case Z80::Z80::Register8::C:
                ret.append(0x30 | RegbitsC);
                break;
            case Z80::Z80::Register8::D:
                ret.append(0x30 | RegbitsD);
                break;
            case Z80::Z80::Register8::E:
                ret.append(0x30 | RegbitsE);
                break;
            case Z80::Z80::Register8::H:
                ret.append(0x30 | RegbitsH);
                break;
            case Z80::Z80::Register8::L:
                ret.append(0x30 | RegbitsL);
                break;
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16() && op.reg16() == Z80::Z80::Register16::HL) {
        ret.append(0xcb);
        ret.append(0x30 | RegbitsIndirectHl);
    } else if (op.isIndirectReg16WithOffset()) {
        if (op.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        }
        if (op.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.append(0xcb);
        ret.append(op.offset());
        ret.append(0x30 | RegbitsIndirectIxIy);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleSRL(const QStringList & tokens)
{
    int c = tokens.count();

    if (2 != c) {
        std::cout << "SRL instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Opcode ret;
    Z80Operand op(tokens.at(1));

    if (op.isReg8()) {
        ret.append(0xcb);

        switch (op.reg8()) {
            case Z80::Z80::Register8::A:
                ret.append(0x38 | RegbitsA);
                break;
            case Z80::Z80::Register8::B:
                ret.append(0x38 | RegbitsB);
                break;
            case Z80::Z80::Register8::C:
                ret.append(0x38 | RegbitsC);
                break;
            case Z80::Z80::Register8::D:
                ret.append(0x38 | RegbitsD);
                break;
            case Z80::Z80::Register8::E:
                ret.append(0x38 | RegbitsE);
                break;
            case Z80::Z80::Register8::H:
                ret.append(0x38 | RegbitsH);
                break;
            case Z80::Z80::Register8::L:
                ret.append(0x38 | RegbitsL);
                break;
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16() && op.reg16() == Z80::Z80::Register16::HL) {
        ret.append(0xcb);
        ret.append(0x38 | RegbitsIndirectHl);
    } else if (op.isIndirectReg16WithOffset()) {
        if (op.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        }
        if (op.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.append(0xcb);
        ret.append(op.offset());
        ret.append(0x38 | RegbitsIndirectIxIy);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleSUB(const QStringList & tokens)
{
    int c = tokens.count();
    Opcode ret;

    if (c < 2) {
        std::cout << "SUB requires one operand\n";
        return InvalidInstruction;
    }

    Z80Operand op(tokens.at(1));

    if (op.isReg8() || (op.isIndirectReg16() && op.reg16() == Z80::Z80::Register16::HL)) {
        /* SUB reg8
			SUB (HL) */
        Z80::Z80::UnsignedByte opcode = 0x90;

        if (op.isIndirectReg16() && op.reg16() == Z80::Z80::Register16::HL) {
            opcode |= RegbitsIndirectHl;
        } else if (op.reg8() == Z80::Z80::Register8::A) {
            opcode |= RegbitsA;
        } else if (op.reg8() == Z80::Z80::Register8::B) {
            opcode |= RegbitsB;
        } else if (op.reg8() == Z80::Z80::Register8::C) {
            opcode |= RegbitsC;
        } else if (op.reg8() == Z80::Z80::Register8::D) {
            opcode |= RegbitsD;
        } else if (op.reg8() == Z80::Z80::Register8::E) {
            opcode |= RegbitsE;
        } else if (op.reg8() == Z80::Z80::Register8::H) {
            opcode |= RegbitsH;
        } else if (op.reg8() == Z80::Z80::Register8::L) {
            opcode |= RegbitsL;
        } else {
            return InvalidInstruction;
        }

        ret.append(opcode);
    } else if (op.isIndirectReg16WithOffset()) {
        if (op.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        }
        if (op.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.append(0x90 | RegbitsIndirectIxIy);
        ret.append(op.offset());
    } else if (op.isByte()) {
        /* SUB n */
        ret.append(0xd6);
        ret.append(op.byte());
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleXOR(const QStringList & tokens)
{
    int c = tokens.count();
    Opcode ret;

    if (c < 2) {
        std::cout << "XOR instruction requires one operand\n";
        return InvalidInstruction;
    }

    Z80Operand op(tokens.at(1));

    if (op.isReg8()) {
        /* XOR A,reg8 */
        Z80::Z80::UnsignedByte opcode = 0xa8;

        switch (op.reg8()) {
            case Z80::Z80::Register8::A:
                opcode |= RegbitsA;
                break;
            case Z80::Z80::Register8::B:
                opcode |= RegbitsB;
                break;
            case Z80::Z80::Register8::C:
                opcode |= RegbitsC;
                break;
            case Z80::Z80::Register8::D:
                opcode |= RegbitsD;
                break;
            case Z80::Z80::Register8::E:
                opcode |= RegbitsE;
                break;
            case Z80::Z80::Register8::H:
                opcode |= RegbitsH;
                break;
            case Z80::Z80::Register8::L:
                opcode |= RegbitsL;
                break;
            default:
                return InvalidInstruction;
        }

        ret.append(opcode);
    } else if (op.isIndirectReg16() && op.reg16() == Z80::Z80::Register16::HL) {
        /* XOR A,(HL) */
        ret.append(0xae);
    } else if (op.isIndirectReg16WithOffset()) {
        /* XOR A,(IX+n) */
        /* XOR A,(IY+n) */
        if (op.reg16() == Z80::Z80::Register16::IX) {
            ret.append(0xdd);
        } else if (op.reg16() == Z80::Z80::Register16::IY) {
            ret.append(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.append(0xae);
        ret.append(op.offset());
    } else if (op.isByte()) {
        /* XOR A,n */
        ret.append(0xee);
        ret.append(op.byte());
    } else {
        return InvalidInstruction;
    }

    return ret;
}
