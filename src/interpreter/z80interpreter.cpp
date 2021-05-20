/**
 * Simple Z80 interpreter. Type in assembly instructions and they will be run in a Z80 CPU.
 *
 * Limitations:
 * - obviously there is no PC since instructions are being run on-demand.
 * - this means there is no looping
 * - it also means that the block operations (LDIR, OTIR, etc.) don't work correctly because they rely on manipulating
 *   the PC to loop through the block (the instructions are assembled and executed, but at most one iteration of the
 *   loop will be executed)
 *
 * TODO:
 * - handle IX+n, IY+n operands
 * - handle port operands
 */

#include "z80interpreter.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <regex>
#include <cmath>
#include <cassert>
#include <readline/readline.h>
#include <readline/history.h>
#include "../z80/z80.h"
#include "../util/string.h"
#include "../util/debug.h"
#include "operand.h"

using namespace Interpreter;

namespace
{
    // opcodes that work with 8-bit registers use a pattern of 3 bits to select which register - these constants define those bit patterns
    constexpr const std::uint8_t RegbitsB = 0x00;
    constexpr const std::uint8_t RegbitsC = 0x01;
    constexpr const std::uint8_t RegbitsD = 0x02;
    constexpr const std::uint8_t RegbitsE = 0x03;
    constexpr const std::uint8_t RegbitsH = 0x04;
    constexpr const std::uint8_t RegbitsL = 0x05;
    constexpr const std::uint8_t RegbitsIndirectHl = 0x06;
    constexpr const std::uint8_t RegbitsIndirectIxIy = RegbitsIndirectHl;
    constexpr const std::uint8_t RegbitsA = 0x07;

    constexpr const std::array<const char *, 5> AffirmativeResponses = {"TRUE", "YES", "ON", "Y", "1"};
    constexpr const std::array<const char *, 5> NegativeResponses = {"FALSE", "NO", "OFF", "N", "0"};
}

using SignedByte = Z80::SignedByte;
using SignedWord = Z80::SignedWord;

using Util::trimmed;
using Util::trim;
using Util::upper_cased;
using Util::upper_case;

const Z80Interpreter::Opcode Z80Interpreter::InvalidInstruction;

Z80Interpreter::Z80Interpreter(std::unique_ptr<Z80::Z80> cpu)
: m_cpu(std::move(cpu)),
  m_showOpcodes(false),
  m_showInstructionCost(false),
  m_autoShowFlags(false)
{
}

Z80Interpreter::~Z80Interpreter() = default;

bool Z80Interpreter::hasCpu() const
{
    return static_cast<bool>(m_cpu);
}

Z80Cpu * Z80Interpreter::cpu() const
{
    return m_cpu.get();
}

void Z80Interpreter::setCpu(std::unique_ptr<Z80Cpu> cpu)
{
    m_cpu = std::move(cpu);
}

void Z80Interpreter::run()
{
    assert(hasCpu());
    std::cout << "Z80 interpreter\nDarren Edale, 2021\n\nType \".help\" for help.\n\n";
    std::cout << m_cpu->clockSpeedMHz() << "MHz Z80 CPU, " << m_cpu->memorySize() << " bytes of RAM\n";

    while (handleInput(readInput())) {
    }
}

void Z80Interpreter::run(std::unique_ptr<Z80Cpu> cpu)
{
    Z80Interpreter interpreter(std::move(cpu));
    interpreter.run();
}

std::string Z80Interpreter::readInput()
{
    auto * line = readline("> ");
    auto ret = std::string(line);

    if (line) {
        if (*line) {
            add_history(line);
        }

        free(line);
    }

    return ret;
}

Z80Interpreter::Tokens Z80Interpreter::tokenise(const std::string & input)
{
    // delimit tokens with any amount of whitespace or a single comma surrounded by any amount of whitespace
    // NOTE we must present the alternatives in this order because the ECMA grammar accepts the leftmost match of all the alternatives; the other option is to
    // use extended grammar which accepts the longest and then the leftmost as a tie-breaker but does not support non-capturing matches. Not doing one of these
    // would result in the delimiter " , " being matched by the " +" alternative, meaning the following token would be incorrect
    static const std::regex tokenBoundary("(?: *, *| +)", std::regex::optimize);
    Tokens tokens;
    auto it = std::sregex_iterator(input.cbegin(), input.cend(), tokenBoundary);

    if (it == std::sregex_iterator()) {
        // no delimiters, it's a single-token input line
        tokens.push_back(input);
    } else {
        // accepting iterators means we could migrate to tokens being string_view objects in future
        auto addToken = [&tokens](const std::string::const_iterator & begin, const std::string::const_iterator & end) {
            std::string str(begin, end);
            trim(str);
            tokens.push_back(str);
        };

        // the start location of the next token
        auto start = input.cbegin();

        // add all the delimited tokens
        while (it != std::sregex_iterator()) {
            addToken(start, input.cbegin() + it->position());
            start = input.cbegin() + it->position() + it->length();
            ++it;
        }

        // if there's a trailing token (i.e. delimited by the end of the input), add it.
        if (start != input.cend()) {
            addToken(start, input.cend());
        }
    }

    return tokens;
}

bool Z80Interpreter::handleInput(const std::string & input)
{
    Tokens tokens = tokenise(input);

    if (tokens.empty()) {
        return true;
    }

    if (".quit" == tokens.front() || ".exit" == tokens.front() || ".halt" == tokens.front()) {
        return false;
    }

    // NOTE tokens.front() cannot be empty
    if (tokens.front()[0] == '.') {
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
        std::cout << std::hex << std::setw(2) << std::setfill('0');

        for(UnsignedByte byte : opcode) {
            std::cout << " 0x" << static_cast<std::uint16_t>(byte);
        }

        std::cout << "\n";
    }

    auto cost = m_cpu->execute(opcode.data(), false);

    if (m_showInstructionCost) {
        std::cout << "instruction consumed " << cost.tStates << " CPU cycles and would occupy " << cost.size
                  << " byte(s) of RAM.\n";
    }

    if (m_autoShowFlags) {
        dotDumpFlags();
    }
}

/* dot-command methods */
void Z80Interpreter::handleDotCommand(const Tokens & tokens)
{
    if (tokens.empty()) {
        std::cerr << "handleDotCommand() given no tokens to interpret.\n";
        return;
    }

    if (tokens.front() == ".help") {
        dotHelp();
    } else if (tokens.front() == ".ram" || tokens.front() == ".dumpram") {
        dotDumpMemory(tokens);
    } else if (tokens.front() == ".regs" || tokens.front() == ".dumpregisters") {
        dotDumpRegisters();
    } else if (tokens.front() == ".status") {
        dotStatus();
    } else if (tokens.front() == ".rv" || tokens.front() == ".regvalue" || tokens.front() == ".registervalue") {
        dotRegisterValue(tokens);
    } else if (tokens.front() == ".showopcodes") {
        dotShowOpcodes();
    } else if (tokens.front() == ".hideopcodes") {
        dotHideOpcodes();
    } else if (tokens.front() == ".showcosts") {
        dotShowCosts();
    } else if (tokens.front() == ".hidecosts") {
        dotHideCosts();
    } else if (tokens.front() == ".flags" || tokens.front() == ".dumpflags") {
        dotDumpFlags();
    } else if (tokens.front() == ".autoflags" || tokens.front() == ".autoshowflags") {
        dotAutoShowFlags(tokens);
    } else {
        std::cout << "Unrecognised command \"" << tokens.front() << "\": try \".help\".\n";
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

void Z80Interpreter::dotAutoShowFlags(const Tokens & tokens)
{
    if (1 == tokens.size() || std::any_of(AffirmativeResponses.cbegin(), AffirmativeResponses.cend(), [token = std::move(upper_cased(tokens[1]))] (const auto & affirmativeResponse) -> bool {
        return token == affirmativeResponse;
    })) {
        m_autoShowFlags = true;
        std::cout << "automatically showing flags from now on.\n";
    } else if (std::any_of(NegativeResponses.cbegin(), NegativeResponses.cend(), [token = std::move(upper_cased(tokens[1]))] (const auto & negativeResponse) -> bool {
        return token == negativeResponse;
    })) {
        m_autoShowFlags = false;
        std::cout << "no longer automatically showing flags.\n";
    } else {
        std::cout << "unrecognised parameter \"" << tokens[1] << "\"\n";
    }
}

void Z80Interpreter::dotDumpFlags() const
{
    UnsignedByte f = m_cpu->fRegisterValue();

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

#include <charconv>

void Z80Interpreter::dotDumpMemory(const Tokens & tokens) const
{
    assert(!tokens.empty());
    int low = 0;
    int len = 16;

    if (1 < tokens.size()) {
        auto token = std::move(trimmed(tokens.at(1)));
        bool ok;

        if ('$' == token[0]) {
            auto [endPtr, error] = std::from_chars(token.data() + 1, token.data() + token.size(), low, 16);
            ok = error == std::errc() && endPtr == token.data() + token.size();
        } else if ("0x" == std::string_view(token.cbegin(), token.cbegin() + 2)) {
            auto [endPtr, error] = std::from_chars(token.data() + 2, token.data() + token.size(), low, 16);
            ok = error == std::errc() && endPtr == token.data() + token.size();
        } else {
            auto [endPtr, error] = std::from_chars(token.data(), token.data() + token.size(), low);
            ok = error == std::errc() && endPtr == token.data() + token.size();
        }

        if (!ok) {
            std::cout << tokens.front() << ": invalid start byte \"" << token << "\"\n";
            return;
        }
    }

    if (2 < tokens.size()) {
        auto token = std::move(trimmed(tokens.at(2)));
        bool ok;

        if ('$' == token[0]) {
            auto [endPtr, error] = std::from_chars(token.data() + 1, token.data() + token.size(), len, 16);
            ok = error == std::errc() && endPtr == token.data() + token.size();
        } else if ("0x" == std::string_view(token.cbegin(), token.cbegin() + 2)) {
            auto [endPtr, error] = std::from_chars(token.data() + 2, token.data() + token.size(), len, 16);
            ok = error == std::errc() && endPtr == token.data() + token.size();
        } else {
            auto [endPtr, error] = std::from_chars(token.data(), token.data() + token.size(), len);
            ok = error == std::errc() && endPtr == token.data() + token.size();
        }

        if (!ok) {
            std::cout << tokens.at(0) << ": invalid dump length \"" << token << "\"\n";
            return;
        }
    }

    dotDumpMemory(low, len);
}

void Z80Interpreter::dotDumpMemory(int low, int len) const
{
    assert(m_cpu);

    if (low < 0) {
        std::cout << "can't display memory below address 0";
        return;
    }

    if (len < 1) {
        std::cout << "memory range of 0 bytes requested - nothing to display.";
        return;
    }

    const auto * ram = m_cpu->memory();
    std::cout << "         ";
    std::cout << std::setfill('0') << std::hex;

    for (int i = 0; i < 16; ++i) {
        std::cout << " 0x" << std::setw(2) << i;
    }

    if (low % 16) {
        // there are some undisplayed values before low in the row, so insert extra spacing
        std::cout << "\n 0x" << std::setw(4) << static_cast<int>(std::floor(low / 16) * 16) << " :";

        for (int i = 0; i < (low % 16); ++i) {
            std::cout << "     ";
        }
    }

    for (int i = 0; i < len; ++i) {
        int addr = low + i;

        if (0 == (addr % 16)) {
            std::cout << "\n 0x" << std::setw(4) << addr << " :";
        }

        std::cout << " 0x" << std::setw(2) << static_cast<std::uint16_t>(ram->readByte(addr));
    }

    std::cout << "\n" << std::dec << std::setfill(' ');
}

void Z80Interpreter::dotStatus() const
{
    std::cout << "A  SZ5H3PNC  BC   DE   HL   IX   IY  A' SZ5H3PNC' BC'  DE'  HL'  SP  | IM  IFF1  IFF2\n"
              << std::hex << std::setfill('0')
              << std::setw(2) << static_cast<std::uint16_t>(m_cpu->aRegisterValue())
              << ' ' << (m_cpu->sFlag() ? '1' : '0') << (m_cpu->zFlag() ? '1' : '0') << (m_cpu->f5Flag() ? '1' : '0')
              << (m_cpu->hFlag() ? '1' : '0') << (m_cpu->f3Flag() ? '1' : '0') << (m_cpu->pFlag() ? '1' : '0')
              << (m_cpu->nFlag() ? '1' : '0') << (m_cpu->cFlag() ? '1' : '0')
              << ' ' << std::setw(4) << m_cpu->bcRegisterValue()
              << ' ' << std::setw(4) << m_cpu->deRegisterValue()
              << ' ' << std::setw(4) << m_cpu->hlRegisterValue()
              << ' ' << std::setw(4) << m_cpu->ixRegisterValue()
              << ' ' << std::setw(4) << m_cpu->iyRegisterValue()
              << ' ' << std::setw(2) << static_cast<std::uint16_t>(m_cpu->aShadowRegisterValue())
              << ' ' << (m_cpu->sShadowFlag() ? '1' : '0') << (m_cpu->zShadowFlag() ? '1' : '0')
              << (m_cpu->f5ShadowFlag() ? '1' : '0') << (m_cpu->hShadowFlag() ? '1' : '0')
              << (m_cpu->f3ShadowFlag() ? '1' : '0') << (m_cpu->pShadowFlag() ? '1' : '0')
              << (m_cpu->nShadowFlag() ? '1' : '0') << (m_cpu->cShadowFlag() ? '1' : '0')
              << ' ' << std::setw(4) << m_cpu->bcShadowRegisterValue()
              << ' ' << std::setw(4) << m_cpu->deShadowRegisterValue()
              << ' ' << std::setw(4) << m_cpu->hlShadowRegisterValue()
              << ' ' << std::setw(4) << m_cpu->sp()
              << " |  " << std::dec << std::setw(1) << static_cast<std::uint16_t>(m_cpu->interruptMode())
              << "    " << (m_cpu->iff1() ? '1' : '0')
              << "     " << (m_cpu->iff2() ? '1' : '0')
              << '\n'
              << std::setfill(' ');
}

void Z80Interpreter::dotDumpRegisters() const
{
    dotRegisterValue(Register8::A);
    dotRegisterValue(Register8::B);
    dotRegisterValue(Register8::C);
    dotRegisterValue(Register8::D);
    dotRegisterValue(Register8::E);
    dotRegisterValue(Register8::H);
    dotRegisterValue(Register8::L);
    dotRegisterValue(Register16::AF);
    dotRegisterValue(Register16::BC);
    dotRegisterValue(Register16::DE);
    dotRegisterValue(Register16::HL);
    dotRegisterValue(Register16::SP);
    dotRegisterValue(Register16::PC);
    dotRegisterValue(Register16::IX);
    dotRegisterValue(Register16::IY);
}

void Z80Interpreter::dotRegisterValue(const Tokens & tokens) const
{
    if (2 > tokens.size()) {
        std::cout << "you must specify which register's value you wish to display\n";
        return;
    }

    const auto registerToken = std::move(upper_cased(tokens[1]));
    
    if (registerToken == "A") {
        dotRegisterValue(Register8::A);
    } else if (registerToken == "B") {
        dotRegisterValue(Register8::B);
    } else if (registerToken == "C") {
        dotRegisterValue(Register8::C);
    } else if (registerToken == "D") {
        dotRegisterValue(Register8::D);
    } else if (registerToken == "E") {
        dotRegisterValue(Register8::E);
    } else if (registerToken == "H") {
        dotRegisterValue(Register8::H);
    } else if (registerToken == "L") {
        dotRegisterValue(Register8::L);
    } else if (registerToken == "F") {
        dotRegisterValue(Register8::F);
    } else if (registerToken == "AF") {
        dotRegisterValue(Register16::AF);
    } else if (registerToken == "BC") {
        dotRegisterValue(Register16::BC);
    } else if (registerToken == "DE") {
        dotRegisterValue(Register16::DE);
    } else if (registerToken == "HL") {
        dotRegisterValue(Register16::HL);
    } else if (registerToken == "SP") {
        dotRegisterValue(Register16::SP);
    } else if (registerToken == "PC") {
        dotRegisterValue(Register16::PC);
    } else if (registerToken == "IX") {
        dotRegisterValue(Register16::IX);
    } else if (registerToken == "IY") {
        dotRegisterValue(Register16::IY);
    } else if (registerToken == "A'") {
        dotRegisterValue(Register8::AShadow);
    } else if (registerToken == "B'") {
        dotRegisterValue(Register8::BShadow);
    } else if (registerToken == "C'") {
        dotRegisterValue(Register8::CShadow);
    } else if (registerToken == "D'") {
        dotRegisterValue(Register8::DShadow);
    } else if (registerToken == "E'") {
        dotRegisterValue(Register8::EShadow);
    } else if (registerToken == "H'") {
        dotRegisterValue(Register8::HShadow);
    } else if (registerToken == "L'") {
        dotRegisterValue(Register8::LShadow);
    } else if (registerToken == "F'") {
        dotRegisterValue(Register8::FShadow);
    } else if (registerToken == "AF'") {
        dotRegisterValue(Register16::AFShadow);
    } else if (registerToken == "BC'") {
        dotRegisterValue(Register16::BCShadow);
    } else if (registerToken == "DE'") {
        dotRegisterValue(Register16::DEShadow);
    } else if (registerToken == "HL'") {
        dotRegisterValue(Register16::HLShadow);
    } else {
        std::cout << "unrecognised register: \"" << tokens[1] << "\"\n";
    }
}

void Z80Interpreter::dotRegisterValue(const Register8 reg, const NumberFormats & fmt) const
{
    assert(m_cpu);
    UnsignedByte v = m_cpu->registerValue(reg);

    switch (reg) {
        case Register8::A:
            std::cout << "A  = ";
            break;
        case Register8::B:
            std::cout << "B  = ";
            break;
        case Register8::C:
            std::cout << "C  = ";
            break;
        case Register8::D:
            std::cout << "D  = ";
            break;
        case Register8::E:
            std::cout << "E  = ";
            break;
        case Register8::H:
            std::cout << "H  = ";
            break;
        case Register8::L:
            std::cout << "L  = ";
            break;
        case Register8::IXH:
            std::cout << "IXH  = ";
            break;
        case Register8::IXL:
            std::cout << "IXL  = ";
            break;
        case Register8::IYH:
            std::cout << "IYH  = ";
            break;
        case Register8::IYL:
            std::cout << "IYL  = ";
            break;
        case Register8::F:
            std::cout << "F  = ";
            break;
        case Register8::I:
            std::cout << "I  = ";
            break;
        case Register8::R:
            std::cout << "R  = ";
            break;
        case Register8::AShadow:
            std::cout << "A' = ";
            break;
        case Register8::BShadow:
            std::cout << "B' = ";
            break;
        case Register8::CShadow:
            std::cout << "C' = ";
            break;
        case Register8::DShadow:
            std::cout << "D' = ";
            break;
        case Register8::EShadow:
            std::cout << "E' = ";
            break;
        case Register8::HShadow:
            std::cout << "H' = ";
            break;
        case Register8::LShadow:
            std::cout << "L' = ";
            break;
        case Register8::FShadow:
            std::cout << "F' = ";
            break;
    }

    if (fmt & HexFormat) {
        std::cout << "  0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(v);
    }

    if (fmt & DecimalFormat) {
        std::cout << "  " << std::dec << std::setfill(' ') << std::setw(3) << static_cast<std::uint16_t>(v);
    }

    if (fmt & OctalFormat) {
        std::cout << "  " << std::oct << std::setfill(' ') << std::setw(3) << static_cast<std::uint16_t>(v);
    }

    if (fmt & BinaryFormat) {
        UnsignedByte mask = 0x80;
        std::cout << " ";

        for (int i = 0; i < 8; ++i) {
            std::cout << (v & mask ? '1' : '0');
            mask >>= 1;
        }

        std::cout << 'b';
    }

    std::cout << "\n" << std::dec << std::setfill(' ') << std::setw(0);
}

void Z80Interpreter::dotRegisterValue(const Register16 reg, const NumberFormats & fmt) const
{
    assert(m_cpu);
    UnsignedWord v = m_cpu->registerValue(reg);

    switch (reg) {
        case Register16::AF:
            std::cout << "AF  = ";
            break;
        case Register16::BC:
            std::cout << "BC  = ";
            break;
        case Register16::DE:
            std::cout << "DE  = ";
            break;
        case Register16::HL:
            std::cout << "HL  = ";
            break;
        case Register16::SP:
            std::cout << "SP  = ";
            break;
        case Register16::PC:
            std::cout << "PC  = ";
            break;
        case Register16::IX:
            std::cout << "IX  = ";
            break;
        case Register16::IY:
            std::cout << "IY  = ";
            break;
        case Register16::AFShadow:
            std::cout << "AF' = ";
            break;
        case Register16::BCShadow:
            std::cout << "BC' = ";
            break;
        case Register16::DEShadow:
            std::cout << "DE' = ";
            break;
        case Register16::HLShadow:
            std::cout << "HL' = ";
            break;
    }

    if (fmt & HexFormat) {
        std::cout << "  0x" << std::hex << std::setfill('0') << std::setw(4) << v;
    }

    if (fmt & DecimalFormat) {
        std::cout << "  " << std::dec << std::setfill(' ') << std::setw(6) << v;
    }

    if (fmt & OctalFormat) {
        std::cout << "  " << std::oct << std::setfill(' ') << std::setw(3) << v;
    }

    if (fmt & BinaryFormat) {
        UnsignedWord mask = 0x8000;
        std::cout << " ";

        for (int i = 0; i < 16; ++i) {
            std::cout << (v & mask ? '1' : '0');
            mask <<= 1;
        }

        std::cout << 'b';
    }

    std::cout << "\n" << std::dec << std::setw(0) << std::setfill(' ');
}

/* Z80 instruction methods */
Z80Interpreter::Opcode Z80Interpreter::assembleInstruction(const Tokens & tokens)
{
    if (tokens.empty()) {
        std::cerr << "assembleInstruction given no tokens\n";
        return Opcode();
    }

    const auto & token = std::move(upper_cased(tokens.front()));
    
    if (token == "ADC") {
        return assembleADC(tokens);
    } else if (token == "ADD") {
        return assembleADD(tokens);
    } else if (token == "AND") {
        return assembleAND(tokens);
    } else if (token == "BIT") {
        return assembleBIT(tokens);
    } else if (token == "CALL") {
        return assembleCALL(tokens);
    } else if (token == "CCF") {
        return assembleCCF(tokens);
    } else if (token == "CP") {
        return assembleCP(tokens);
    } else if (token == "CPD") {
        return assembleCPD(tokens);
    } else if (token == "CPDR") {
        return assembleCPDR(tokens);
    } else if (token == "CPI") {
        return assembleCPI(tokens);
    } else if (token == "CPIR") {
        return assembleCPIR(tokens);
    } else if (token == "CPL") {
        return assembleCPL(tokens);
    } else if (token == "DAA") {
        return assembleDAA(tokens);
    } else if (token == "DEC") {
        return assembleDEC(tokens);
    } else if (token == "DI") {
        return assembleDI(tokens);
    } else if (token == "DJNZ") {
        return assembleDJNZ(tokens);
    } else if (token == "EI") {
        return assembleEI(tokens);
    } else if (token == "EX") {
        return assembleEX(tokens);
    } else if (token == "EXX") {
        return assembleEXX(tokens);
    } else if (token == "HALT") {
        return assembleHALT(tokens);
    } else if (token == "IM") {
        return assembleIM(tokens);
    } else if (token == "IN") {
        return assembleIN(tokens);
    } else if (token == "INC") {
        return assembleINC(tokens);
    } else if (token == "IND") {
        return assembleIND(tokens);
    } else if (token == "INDR") {
        return assembleINDR(tokens);
    } else if (token == "INI") {
        return assembleINI(tokens);
    } else if (token == "INIR") {
        return assembleINIR(tokens);
    } else if (token == "JP") {
        return assembleJP(tokens);
    } else if (token == "JR") {
        return assembleJR(tokens);
    } else if (token == "LD") {
        return assembleLD(tokens);
    } else if (token == "LDD") {
        return assembleLDD(tokens);
    } else if (token == "LDDR") {
        return assembleLDDR(tokens);
    } else if (token == "LDI") {
        return assembleLDI(tokens);
    } else if (token == "LDIR") {
        return assembleLDIR(tokens);
    } else if (token == "NEG") {
        return assembleNEG(tokens);
    } else if (token == "NOP") {
        return assembleNOP(tokens);
    } else if (token == "OR") {
        return assembleOR(tokens);
    } else if (token == "OUT") {
        return assembleOUT(tokens);
    } else if (token == "OUTD") {
        return assembleOUTD(tokens);
    } else if (token == "OTDR") {
        return assembleOTDR(tokens);
    } else if (token == "OUTI") {
        return assembleOUTI(tokens);
    } else if (token == "OTIR") {
        return assembleOTIR(tokens);
    } else if (token == "POP") {
        return assemblePOP(tokens);
    } else if (token == "PUSH") {
        return assemblePUSH(tokens);
    } else if (token == "RES") {
        return assembleRES(tokens);
    } else if (token == "RET") {
        return assembleRET(tokens);
    } else if (token == "RETI") {
        return assembleRETI(tokens);
    } else if (token == "RETN") {
        return assembleRETN(tokens);
    } else if (token == "RLA") {
        return assembleRLA(tokens);
    } else if (token == "RL") {
        return assembleRL(tokens);
    } else if (token == "RLCA") {
        return assembleRLCA(tokens);
    } else if (token == "RLC") {
        return assembleRLC(tokens);
    } else if (token == "RLD") {
        return assembleRLD(tokens);
    } else if (token == "RRA") {
        return assembleRRA(tokens);
    } else if (token == "RR") {
        return assembleRR(tokens);
    } else if (token == "RRCA") {
        return assembleRRCA(tokens);
    } else if (token == "RRC") {
        return assembleRRC(tokens);
    } else if (token == "RRD") {
        return assembleRRD(tokens);
    } else if (token == "RST") {
        return assembleRST(tokens);
    } else if (token == "SBC") {
        return assembleSBC(tokens);
    } else if (token == "SCF") {
        return assembleSCF(tokens);
    } else if (token == "SET") {
        return assembleSET(tokens);
    } else if (token == "SLA") {
        return assembleSLA(tokens);
    } else if (token == "SRA") {
        return assembleSRA(tokens);
    } else if (token == "SLL") {
        return assembleSLL(tokens);
    } else if (token == "SRL") {
        return assembleSRL(tokens);
    } else if (token == "SUB") {
        return assembleSUB(tokens);
    } else if (token == "XOR") {
        return assembleXOR(tokens);
    }
    return InvalidInstruction;
}

Z80Interpreter::Opcode Z80Interpreter::assembleADC(const Tokens & tokens)
{
    auto c = tokens.size();
    Opcode ret;

    if (c < 3) {
        std::cout << "ADC requires two operands\n";
        return InvalidInstruction;
    }

    Operand op1(tokens.at(1));
    Operand op2(tokens.at(2));

    if (op1.isReg8() && op1.reg8() == Register8::A) {
        if (op2.isReg8()) {
            // ADC A,reg8
			// ADC A,(HL)
            switch (op2.reg8()) {
                case Register8::A:
                    ret.push_back(0x88 | RegbitsA);
                    break;
                case Register8::B:
                    ret.push_back(0x88 | RegbitsB);
                    break;
                case Register8::C:
                    ret.push_back(0x88 | RegbitsC);
                    break;
                case Register8::D:
                    ret.push_back(0x88 | RegbitsD);
                    break;
                case Register8::E:
                    ret.push_back(0x88 | RegbitsE);
                    break;
                case Register8::H:
                    ret.push_back(0x88 | RegbitsH);
                    break;
                case Register8::L:
                    ret.push_back(0x88 | RegbitsL);
                    break;
                default:
                    return InvalidInstruction;
            }
        } else if (op2.isByte()) {
            ret.push_back(0xce);
            ret.push_back(op2.byte());
        } else if (op2.isIndirectReg16() && op2.reg16() == Register16::HL) {
            ret.push_back(0x88 | RegbitsIndirectHl);
        } else if (op2.isIndirectReg16WithOffset()) {
            /* ACD A,(IX+d)
			 * ACD A,(IY+d) */
            if (op2.reg16() == Register16::IX) {
                ret.push_back(0xdd);
            } else if (op2.reg16() == Register16::IY) {
                ret.push_back(0xfd);
            } else {
                return InvalidInstruction;
            }

            ret.push_back(0x8e);
            ret.push_back(op2.offset());
        } else {
            return InvalidInstruction;
        }
    } else if (op1.isReg16() && op1.reg16() == Register16::HL && op2.isReg16()) {
        /* ADC HL,BC
			ADC HL,DE
			ADC HL,HL
			ADC HL,SP */
        ret.push_back(0xed);

        switch (op2.reg16()) {
            case Register16::BC:
                ret.push_back(0x4a);
                break;
            case Register16::DE:
                ret.push_back(0x5a);
                break;
            case Register16::HL:
                ret.push_back(0x6a);
                break;
            case Register16::SP:
                ret.push_back(0x7a);
                break;
            default:
                return InvalidInstruction;
        }
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleADD(const Tokens & tokens)
{
    auto c = tokens.size();
    Opcode ret;

    if (c < 3) {
        std::cout << "ADD requires two operands\n";
        return InvalidInstruction;
    }

    Operand op1(tokens.at(1));
    Operand op2(tokens.at(2));

    if (op1.isReg8() && op1.reg8() == Register8::A) {
        if (op2.isReg8()) {
            /* ADD A,reg8 */
            switch (op2.reg8()) {
                case Register8::A:
                    ret.push_back(0x80 | RegbitsA);
                    break;
                case Register8::B:
                    ret.push_back(0x80 | RegbitsB);
                    break;
                case Register8::C:
                    ret.push_back(0x80 | RegbitsC);
                    break;
                case Register8::D:
                    ret.push_back(0x80 | RegbitsD);
                    break;
                case Register8::E:
                    ret.push_back(0x80 | RegbitsE);
                    break;
                case Register8::H:
                    ret.push_back(0x80 | RegbitsH);
                    break;
                case Register8::L:
                    ret.push_back(0x80 | RegbitsL);
                    break;
                default:
                    return InvalidInstruction;
            }
        } else if (op2.isIndirectReg16() && op2.reg16() == Register16::HL) {
            /* ADD A,(HL) */
            ret.push_back(0x80 | RegbitsIndirectHl);
        } else if (op2.isIndirectReg16WithOffset()) {
            /* ADD A,(IX+d)
			 * ADD A,(IY+d) */
            if (op2.reg16() == Register16::IX) {
                ret.push_back(0xdd);
            } else if (op2.reg16() == Register16::IY) {
                ret.push_back(0xfd);
            } else {
                return InvalidInstruction;
            }

            ret.push_back(0x86);
            ret.push_back(op2.offset());
        } else if (op2.isByte()) {
            /* ADD A,n */
            ret.push_back(0xc6);
            ret.push_back(op2.byte());
        } else {
            return InvalidInstruction;
        }
    } else if (op1.isReg16() && op2.isReg16()) {
        /* ADD HL,BC; ADD HL,DE; ADD HL,HL; ADD HL,SP */
        /* ADD IX,BC; ADD IX,DE; ADD IX,HL; ADD IX,SP */
        /* ADD IY,BC; ADD IY,DE; ADD IY,HL; ADD IY,SP */
        if (op1.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        } else if (op1.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else if (op1.reg16() != Register16::HL) {
            return InvalidInstruction;
        }

        switch (op2.reg16()) {
            case Register16::BC:
                ret.push_back(0x09);
                break;
            case Register16::DE:
                ret.push_back(0x19);
                break;
            case Register16::HL:
                ret.push_back(0x29);
                break;
            case Register16::SP:
                ret.push_back(0x39);
                break;
            default:
                return InvalidInstruction;
        }
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleAND(const Tokens & tokens)
{
    auto c = tokens.size();
    Opcode ret;

    if (c < 2) {
        std::cout << "AND requires one operand\n";
        return InvalidInstruction;
    }

    Operand op1(tokens.at(1));

    if (op1.isReg8()) {
        /* AND A,reg8 */
        UnsignedByte opcode = 0xa0;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"     // only interested in the registers that are supported with the AND instruction
        switch (op1.reg8()) {
            case Register8::A:
                opcode |= RegbitsA;
                break;
            case Register8::B:
                opcode |= RegbitsB;
                break;
            case Register8::C:
                opcode |= RegbitsC;
                break;
            case Register8::D:
                opcode |= RegbitsD;
                break;
            case Register8::E:
                opcode |= RegbitsE;
                break;
            case Register8::H:
                opcode |= RegbitsH;
                break;
            case Register8::L:
                opcode |= RegbitsL;
                break;
        }
#pragma clang diagnostic pop

        ret.push_back(opcode);
    } else if ((op1.isIndirectReg16() && op1.reg16() == Register16::HL)) {
        ret.push_back(0xa6);
    } else if (op1.isByte()) {
        /* AND A,n */
        ret.push_back(0xe6);
        ret.push_back(op1.byte());
    } else if (op1.isIndirectReg16WithOffset()) {
        /* AND A,(IX+d)
		 * AND A,(IY+d) */
        if (op1.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        } else if (op1.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.push_back(0x8e);
        ret.push_back(op1.offset());
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleBIT(const Tokens & tokens)
{
    auto c = tokens.size();
    Opcode ret;

    if (c < 3) {
        std::cout << "BIT requires two operands\n";
        return InvalidInstruction;
    }

    Operand op1(tokens.at(1));
    Operand op2(tokens.at(2));

    if (!op1.isBitIndex()) {
        return InvalidInstruction;
    }
    UnsignedByte opcode = 0x40;
    opcode += (op1.bitIndex() << 3);

    if (op2.isReg8() || (op2.isIndirectReg16() && op2.reg16() == Register16::HL)) {
        /* BIT b,reg8
			BIT b,(HL) */

        ret.push_back(0xcb);

        if (op2.isIndirectReg16() && op2.reg16() == Register16::HL) {
            opcode |= RegbitsIndirectHl;
        } else if (op2.reg8() == Register8::A) {
            opcode |= RegbitsA;
        } else if (op2.reg8() == Register8::B) {
            opcode |= RegbitsB;
        } else if (op2.reg8() == Register8::C) {
            opcode |= RegbitsC;
        } else if (op2.reg8() == Register8::D) {
            opcode |= RegbitsD;
        } else if (op2.reg8() == Register8::E) {
            opcode |= RegbitsE;
        } else if (op2.reg8() == Register8::H) {
            opcode |= RegbitsH;
        } else if (op2.reg8() == Register8::L) {
            opcode |= RegbitsL;
        } else {
            return InvalidInstruction;
        }

        ret.push_back(opcode);
    } else if (op2.isIndirectReg16WithOffset()) {
        /* BIT b,(IX+d)
		 * BIT b,(IY+d) */
        if (op2.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        } else if (op2.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.push_back(0xcb);
        ret.push_back(op2.offset());
        ret.push_back(opcode | RegbitsIndirectIxIy);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleCALL(const Tokens & tokens)
{
    auto c = tokens.size();
    Opcode ret;

    if (2 > c) {
        std::cout << "CALL requires at least one operand\n";
        return InvalidInstruction;
    }

    if (2 == c) {
        Operand op(tokens.at(1));

        if (!op.isWord()) {
            std::cout << "unconditional CALL requires a 16-bit direct address as its only operand\n";
            return InvalidInstruction;
        }

        ret.push_back(0xcd);
        ret.push_back(op.wordLowByte());
        ret.push_back(op.wordHighByte());
    } else if (3 == c) {
        Operand op1(tokens.at(1));
        Operand op2(tokens.at(2));

        if (!op1.isCondition()) {
            std::cout << "conditional CALL requires a valid call condition as its first operand\n";
            return InvalidInstruction;
        }

        if (!op2.isWord()) {
            std::cout << "conditional CALL requires a 16-bit direct address as its second operand\n";
            return InvalidInstruction;
        }

        switch (op1.condition()) {
            case Operand::ConditionType::NonZero:
                ret.push_back(0xc4);
                break;
            case Operand::ConditionType::Zero:
                ret.push_back(0xcc);
                break;
            case Operand::ConditionType::NoCarry:
                ret.push_back(0xd4);
                break;
            case Operand::ConditionType::Carry:
                ret.push_back(0xdc);
                break;
            case Operand::ConditionType::ParityOdd:
                ret.push_back(0xe4);
                break;
            case Operand::ConditionType::ParityEven:
                ret.push_back(0xec);
                break;
            case Operand::ConditionType::Plus:
                ret.push_back(0xf4);
                break;
            case Operand::ConditionType::Minus:
                ret.push_back(0xfc);
                break;
            default:
                return InvalidInstruction;
        }

        ret.push_back(op2.wordLowByte());
        ret.push_back(op2.wordHighByte());
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleCCF(const Tokens &)
{
    Opcode ret;
    ret.push_back(0x3f);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleCP(const Tokens & tokens)
{
    auto c = tokens.size();
    Opcode ret;

    if (c < 2) {
        std::cout << "CP requires one operand\n";
        return InvalidInstruction;
    }

    Operand op(tokens.at(1));

    if (op.isReg8()) {
        /* CP reg8 */
        UnsignedByte opcode = 0xb8;

        switch (op.reg8()) {
            case Register8::A:
                opcode |= RegbitsA;
                break;
            case Register8::B:
                opcode |= RegbitsB;
                break;
            case Register8::C:
                opcode |= RegbitsC;
                break;
            case Register8::D:
                opcode |= RegbitsD;
                break;
            case Register8::E:
                opcode |= RegbitsE;
                break;
            case Register8::H:
                opcode |= RegbitsH;
                break;
            case Register8::L:
                opcode |= RegbitsL;
                break;
            default:
                return InvalidInstruction;
        }

        ret.push_back(opcode);
    } else if ((op.isIndirectReg16() && op.reg16() == Register16::HL)) {
        /* CP (HL) */
        ret.push_back(0xbe);
    } else if (op.isIndirectReg16WithOffset()) {
        /* CP (IX+d)
		 * CP (IY+d) */
        if (op.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        } else if (op.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.push_back(0xbe);
        ret.push_back(op.offset());
    } else if (op.isByte()) {
        /* CP n */
        ret.push_back(0xfe);
        ret.push_back(op.byte());
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleCPD(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0xa9);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleCPDR(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0xb9);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleCPI(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0xa1);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleCPIR(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0xb1);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleCPL(const Tokens &)
{
    Opcode ret;
    ret.push_back(0x2f);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleDAA(const Tokens &)
{
    Opcode ret;
    ret.push_back(0x27);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleDEC(const Tokens & tokens)
{
    Opcode ret;

    if (2 > tokens.size()) {
        std::cout << "DEC instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Operand op(tokens.at(1));

    if (op.isReg8()) {
        /* DEC reg8 */
        switch (op.reg8()) {
            case Register8::A:
                ret.push_back(0x3d);
                break;
            case Register8::B:
                ret.push_back(0x05);
                break;
            case Register8::C:
                ret.push_back(0x0d);
                break;
            case Register8::D:
                ret.push_back(0x15);
                break;
            case Register8::E:
                ret.push_back(0x1d);
                break;
            case Register8::H:
                ret.push_back(0x25);
                break;
            case Register8::L:
                ret.push_back(0x2d);
                break;
            default:
                return InvalidInstruction;
        }
    } else if (op.isReg16()) {
        switch (op.reg16()) {
            case Register16::BC:
                ret.push_back(0x0b);
                break;
            case Register16::DE:
                ret.push_back(0x1b);
                break;
            case Register16::HL:
                ret.push_back(0x2b);
                break;
            case Register16::SP:
                ret.push_back(0x3b);
                break;
            case Register16::IX:
                ret.push_back(0xdd);
                ret.push_back(0x2b);
                break;
            case Register16::IY:
                ret.push_back(0xfd);
                ret.push_back(0x2b);
                break;
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16() && op.reg16() == Register16::HL) {
        /* DEC (HL) */
        ret.push_back(0x35);
    } else if (op.isIndirectReg16WithOffset()) {
        /* DEC (IX+d) */
        /* DEC (IY+d) */
        if (op.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        } else if (op.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.push_back(0x35);
        ret.push_back(op.offset());
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleDI(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xf3);
    return ret;
}

/* need to handle relative offset operand type before we can assemble DJNZ */
Z80Interpreter::Opcode Z80Interpreter::assembleDJNZ(const Tokens & tokens)
{
    Z80Interpreter::Opcode ret;

    if (2 > tokens.size()) {
        std::cout << "DJNZ instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Operand op(tokens.at(1));
    if (!op.isOffset()) {
        return InvalidInstruction;
    }
    ret.push_back(0x10);
    ret.push_back(op.offset());
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleEI(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xfb);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleEX(const Tokens & tokens)
{
    Opcode ret;
    auto c = tokens.size();

    if (3 > c) {
        std::cout << "EX requires two operands.\n";
        return InvalidInstruction;
    }

    Operand op1(tokens.at(1));
    Operand op2(tokens.at(2));

    if (op1.isIndirectReg16() && op1.reg16() == Register16::SP && op2.isReg16()) {
        switch (op2.reg16()) {
            default:
                return InvalidInstruction;
            case Register16::HL:
                ret.push_back(0xe3);
                break;
            case Register16::IX:
                ret.push_back(0xdd);
                ret.push_back(0xe3);
                break;
            case Register16::IY:
                ret.push_back(0xfd);
                ret.push_back(0xe3);
                break;
        }
    } else if (op1.isReg16() && op2.isReg16()) {
        if (op1.reg16() == Register16::AF && op2.reg16() == Register16::AFShadow) {
            ret.push_back(0x08);
        } else if (op1.reg16() == Register16::DE && op2.reg16() == Register16::HL) {
            ret.push_back(0xeb);
        } else {
            return InvalidInstruction;
        }
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleEXX(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xd9);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleHALT(const Tokens &)
{
    Opcode ret;
    ret.push_back(0x76);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleIM(const Tokens & tokens)
{
    Opcode ret;

    if (2 > tokens.size()) {
        std::cout << "IM requires one operand.\n";
        return InvalidInstruction;
    }

    ret.push_back(0xed);

    if ("0" == tokens[1]) {
        ret.push_back(0x46);
    } else if ("1" == tokens[1]) {
        ret.push_back(0x56);
    } else if ("2" == tokens[1]) {
        ret.push_back(0x5e);
    } else {
        std::cout << "Invalid interrupt mode (must be 0, 1 or 2).\n";
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleIN(const Tokens & tokens)
{
    Opcode ret;
    auto c = tokens.size();

    if (2 == c) {
        Operand op(tokens.at(1));

        if (op.isIndirectReg8() && op.reg8() == Register8::C) {
            ret.push_back(0xed);
            ret.push_back(0x70);
        } else {
            return InvalidInstruction;
        }
    } else if (3 == c) {
        Operand op1(tokens.at(1));
        Operand op2(tokens.at(2));

        /* TODO need IndirectByte operand type (perhaps it's IndirectPort?) - (N) */
//		if(op1.isReg8() && op1.reg8() == ) {
//		}
        if (op1.isReg8() && op2.isIndirectReg8() && op2.reg8() == Register8::C) {
            switch (op1.reg8()) {
                case Register8::B:
                    ret.push_back(0xed);
                    ret.push_back(0x40);
                    break;
                case Register8::C:
                    ret.push_back(0xed);
                    ret.push_back(0x48);
                    break;
                case Register8::D:
                    ret.push_back(0xed);
                    ret.push_back(0x50);
                    break;
                case Register8::E:
                    ret.push_back(0xed);
                    ret.push_back(0x58);
                    break;
                case Register8::H:
                    ret.push_back(0xed);
                    ret.push_back(0x60);
                    break;
                case Register8::L:
                    ret.push_back(0xed);
                    ret.push_back(0x68);
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

Z80Interpreter::Opcode Z80Interpreter::assembleINC(const Tokens & tokens)
{
    Opcode ret;
    auto c = tokens.size();

    if (2 != c) {
        std::cout << "INC instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Operand op(tokens.at(1));

    if (op.isReg8()) {
        /* INC reg8 */
        switch (op.reg8()) {
            case Register8::A:
                ret.push_back(0x3c);
                break;
            case Register8::B:
                ret.push_back(0x04);
                break;
            case Register8::C:
                ret.push_back(0x0c);
                break;
            case Register8::D:
                ret.push_back(0x14);
                break;
            case Register8::E:
                ret.push_back(0x1c);
                break;
            case Register8::H:
                ret.push_back(0x24);
                break;
            case Register8::L:
                ret.push_back(0x2c);
                break;
            default:
                return InvalidInstruction;
        }
    } else if (op.isReg16()) {
        /* INC reg16 */
        switch (op.reg16()) {
            case Register16::BC:
                ret.push_back(0x03);
                break;
            case Register16::DE:
                ret.push_back(0x13);
                break;
            case Register16::HL:
                ret.push_back(0x23);
                break;
            case Register16::SP:
                ret.push_back(0x33);
                break;
            case Register16::IX:
                ret.push_back(0xdd);
                ret.push_back(0x23);
                break;
            case Register16::IY:
                ret.push_back(0xfd);
                ret.push_back(0x23);
                break;
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16()) {
        /* INC (HL) */
        switch (op.reg16()) {
            case Register16::HL:
                ret.push_back(0x34);
                break;
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16WithOffset()) {
        /* INC (IX+n) */
        /* INC (IY+n) */
        switch (op.reg16()) {
            case Register16::IX:
                ret.push_back(0xdd);
                break;
            case Register16::IY:
                ret.push_back(0xfd);
                break;
            default:
                return InvalidInstruction;
        }

        ret.push_back(0x34);
        ret.push_back(op.offset());
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleIND(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0xaa);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleINDR(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0xba);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleINI(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0xa2);
    return ret;
}


Z80Interpreter::Opcode Z80Interpreter::assembleINIR(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0xb2);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleJP(const Tokens & tokens)
{
    auto c = tokens.size();
    Opcode ret;

    if (2 > c) {
        std::cout << "JP requires at least one operand\n";
        return InvalidInstruction;
    }

    if (2 == c) {
        Operand op(tokens.at(1));

        if (op.isWord()) {
            ret.push_back(0xc3);
            ret.push_back(op.wordLowByte());
            ret.push_back(op.wordHighByte());
        } else if (op.isIndirectReg16()) {
            switch (op.reg16()) {
                case Register16::HL:
                    ret.push_back(0xe9);
                    break;
                case Register16::IX: {
                    ret.push_back(0xdd);
                    ret.push_back(0xe9);
                    break;
                }
                case Register16::IY: {
                    ret.push_back(0xfd);
                    ret.push_back(0xe9);
                    break;
                }
                default:
                    return InvalidInstruction;
            }
        } else {
            return InvalidInstruction;
        }
    } else if (3 == c) {
        Operand op1(tokens.at(1));
        Operand op2(tokens.at(2));

        if (!op1.isCondition()) {
            std::cout << "conditional JP requires a valid call condition as its first operand\n";
            return InvalidInstruction;
        }

        if (!op2.isWord()) {
            std::cout << "conditional JP requires a 16-bit direct address as its second operand\n";
            return InvalidInstruction;
        }

        switch (op1.condition()) {
            case Operand::ConditionType::NonZero:
                ret.push_back(0xc2);
                break;
            case Operand::ConditionType::Zero:
                ret.push_back(0xca);
                break;
            case Operand::ConditionType::NoCarry:
                ret.push_back(0xd2);
                break;
            case Operand::ConditionType::Carry:
                ret.push_back(0xda);
                break;
            case Operand::ConditionType::ParityOdd:
                ret.push_back(0xe2);
                break;
            case Operand::ConditionType::ParityEven:
                ret.push_back(0xea);
                break;
            case Operand::ConditionType::Plus:
                ret.push_back(0xf2);
                break;
            case Operand::ConditionType::Minus:
                ret.push_back(0xfa);
                break;
            default:
                return InvalidInstruction;
        }

        ret.push_back(op2.wordLowByte());
        ret.push_back(op2.wordHighByte());
    } else {
        return InvalidInstruction;
    }

    return ret;
}


Z80Interpreter::Opcode Z80Interpreter::assembleJR(const Tokens & tokens)
{
    auto c = tokens.size();
    Opcode ret;

    if (2 > c) {
        std::cout << "JR requires at least one operand\n";
        return InvalidInstruction;
    }

    if (2 == c) {
        Operand op(tokens.at(1));

        if (op.isByte()) {
            ret.push_back(0x18);
            ret.push_back(op.byte());
        } else {
            return InvalidInstruction;
        }
    } else if (3 == c) {
        Operand op1(tokens.at(1));
        Operand op2(tokens.at(2));

        if (!op1.isCondition()) {
            std::cout << "conditional JR requires a valid condition as its first operand\n";
            return InvalidInstruction;
        }

        if (!op2.isByte()) {
            std::cout << "conditional JR requires an 8-bit jump offset as its second operand\n";
            return InvalidInstruction;
        }

        switch (op1.condition()) {
            case Operand::ConditionType::NonZero:
                ret.push_back(0x20);
                break;
            case Operand::ConditionType::Zero:
                ret.push_back(0x28);
                break;
            case Operand::ConditionType::NoCarry:
                ret.push_back(0x30);
                break;
            case Operand::ConditionType::Carry:
                ret.push_back(0x38);
                break;
            default:
                return InvalidInstruction;
        }

        ret.push_back(op2.byte());
    } else {
        return InvalidInstruction;
    }

    return ret;
}


Z80Interpreter::Opcode Z80Interpreter::assembleLD(const Tokens & tokens)
{
    auto c = tokens.size();
    Opcode ret;

    if (c < 3) {
        std::cout << "LD requires two operands\n";
        return InvalidInstruction;
    }

    Operand op1(tokens.at(1));
    Operand op2(tokens.at(2));

    /* TODO support IXH and IYH */
    if (op1.isReg8() && (op2.isReg8() || (op2.isIndirectReg16() && op2.reg16() == Register16::HL))) {
        /* LD reg8,reg8
			LD reg8,(HL) */
        UnsignedByte opcode;

        switch (op1.reg8()) {
            case Register8::A:
                opcode = 0x78;
                break;
            case Register8::B:
                opcode = 0x40;
                break;
            case Register8::C:
                opcode = 0x48;
                break;
            case Register8::D:
                opcode = 0x50;
                break;
            case Register8::E:
                opcode = 0x58;
                break;
            case Register8::H:
                opcode = 0x60;
                break;
            case Register8::L:
                opcode = 0x68;
                break;
            default:
                return InvalidInstruction;
        }

        if (op2.isIndirectReg16() && op2.reg16() == Register16::HL) {
            opcode |= RegbitsIndirectHl;
        } else if (op2.reg8() == Register8::A) {
            opcode |= RegbitsA;
        } else if (op2.reg8() == Register8::B) {
            opcode |= RegbitsB;
        } else if (op2.reg8() == Register8::C) {
            opcode |= RegbitsC;
        } else if (op2.reg8() == Register8::D) {
            opcode |= RegbitsD;
        } else if (op2.reg8() == Register8::E) {
            opcode |= RegbitsE;
        } else if (op2.reg8() == Register8::H) {
            opcode |= RegbitsH;
        } else if (op2.reg8() == Register8::L) {
            opcode |= RegbitsL;
        }

        ret.push_back(opcode);
    } else if (op1.isReg8() && op2.isIndirectReg16WithOffset()) {
        /* LD reg8,(IX+n) */
        /* LD reg8,(IY+n) */
        switch (op2.reg16()) {
            case Register16::IX:
                ret.push_back(0xdd);
                break;
            case Register16::IY:
                ret.push_back(0xfd);
                break;
            default:
                return InvalidInstruction;
        }

        switch (op1.reg8()) {
            case Register8::A:
                ret.push_back(0x7e);
                break;
            case Register8::B:
                ret.push_back(0x46);
                break;
            case Register8::C:
                ret.push_back(0x4e);
                break;
            case Register8::D:
                ret.push_back(0x56);
                break;
            case Register8::E:
                ret.push_back(0x5e);
                break;
            case Register8::H:
                ret.push_back(0x66);
                break;
            case Register8::L:
                ret.push_back(0x6e);
                break;
            default:
                return InvalidInstruction;
        }

        ret.push_back(op2.offset());
    } else if (op1.isReg8() && op2.isByte()) {
        /* LD reg8,n */
        if (op1.reg8() == Register8::A) {
            ret.push_back(0x3e);
        } else if (op1.reg8() == Register8::B) {
            ret.push_back(0x06);
        } else if (op1.reg8() == Register8::C) {
            ret.push_back(0x0e);
        } else if (op1.reg8() == Register8::D) {
            ret.push_back(0x16);
        } else if (op1.reg8() == Register8::E) {
            ret.push_back(0x1e);
        } else if (op1.reg8() == Register8::H) {
            ret.push_back(0x26);
        } else if (op1.reg8() == Register8::L) {
            ret.push_back(0x2e);
        } else if (op1.reg8() == Register8::IXH) {
            ret.push_back(0xdd);
            ret.push_back(0x26);
        } else if (op1.reg8() == Register8::IXL) {
            ret.push_back(0xdd);
            ret.push_back(0x2e);
        } else if (op1.reg8() == Register8::IYH) {
            ret.push_back(0xfd);
            ret.push_back(0x26);
        } else if (op1.reg8() == Register8::IYL) {
            ret.push_back(0xfd);
            ret.push_back(0x2e);
        }

        ret.push_back(op2.byte());
    } else if (op1.isReg8() && op1.reg8() == Register8::A) {
        /* LD A,I
			LD A,R
			LD A,(reg16)   - (not including (HL), which is handled above)
			LD A,(nn) */
        if (op2.isReg8()) {
            if (op2.reg8() == Register8::I) {
                ret.push_back(0xed);
                ret.push_back(0x57);
            }
            if (op2.reg8() == Register8::R) {
                ret.push_back(0xed);
                ret.push_back(0x5f);
            } else {
                return InvalidInstruction;
            }
        } else if (op2.isIndirectReg16()) {
            if (op2.reg16() == Register16::BC) {
                ret.push_back(0x0a);
            } else if (op2.reg16() == Register16::DE) {
                ret.push_back(0x1a);
            } else {
                return InvalidInstruction;
            }
        } else if (op2.isIndirectAddress()) {
            ret.push_back(0x3a);
            ret.push_back(op2.wordLowByte());
            ret.push_back(op2.wordHighByte());
        } else {
            return InvalidInstruction;
        }
    } else if (op1.isReg16()) {
        if (op2.isIndirectAddress()) {
            /* LD reg16,(nn) */
            if (op1.reg16() == Register16::BC) {
                ret.push_back(0xed);
                ret.push_back(0x4b);
            } else if (op1.reg16() == Register16::DE) {
                ret.push_back(0xed);
                ret.push_back(0x5b);
            } else if (op1.reg16() == Register16::HL) {
                ret.push_back(0x2a);
            } else if (op1.reg16() == Register16::SP) {
                ret.push_back(0x7b);
            } else if (op1.reg16() == Register16::IX) {
                ret.push_back(0xdd);
                ret.push_back(0x2a);
            } else if (op1.reg16() == Register16::IY) {
                ret.push_back(0xfd);
                ret.push_back(0x2a);
            }
            ret.push_back(op2.wordLowByte());
            ret.push_back(op2.wordHighByte());
        } else if (op2.isWord()) {
            /* LD reg16,nn */
            if (op1.reg16() == Register16::BC) {
                ret.push_back(0x01);
            } else if (op1.reg16() == Register16::DE) {
                ret.push_back(0x11);
            } else if (op1.reg16() == Register16::HL) {
                ret.push_back(0x21);
            } else if (op1.reg16() == Register16::SP) {
                ret.push_back(0x31);
            } else if (op1.reg16() == Register16::IX) {
                ret.push_back(0xdd);
                ret.push_back(0x21);
            } else if (op1.reg16() == Register16::IY) {
                ret.push_back(0xfd);
                ret.push_back(0x21);
            } else {
                return InvalidInstruction;
            }
            ret.push_back(op2.wordLowByte());
            ret.push_back(op2.wordHighByte());
        } else if (op1.isReg16() && op1.reg16() == Register16::SP && op2.isReg16()) {
            if (op2.reg16() == Register16::HL) {
                ret.push_back(0xf9);
            } else if (op2.reg16() == Register16::IX) {
                ret.push_back(0xdd);
                ret.push_back(0xf9);
            } else if (op2.reg16() == Register16::IY) {
                ret.push_back(0xfd);
                ret.push_back(0xf9);
            } else {
                return InvalidInstruction;
            }
        }
    } else if (op1.isIndirectAddress()) {
        /* LD (nn),reg16
			LD (nn),A */
        if (op2.isReg8() && op2.reg8() == Register8::A) {
            ret.push_back(0x32);
        } else if (op2.isReg16() && op2.reg16() == Register16::BC) {
            ret.push_back(0xed);
            ret.push_back(0x43);
        } else if (op2.isReg16() && op2.reg16() == Register16::DE) {
            ret.push_back(0xed);
            ret.push_back(0x32);
        } else if (op2.isReg16() && op2.reg16() == Register16::HL) {
            ret.push_back(0x22);
        } else if (op2.isReg16() && op2.reg16() == Register16::IX) {
            ret.push_back(0xdd);
            ret.push_back(0x22);
        } else if (op2.isReg16() && op2.reg16() == Register16::IY) {
            ret.push_back(0xfd);
            ret.push_back(0x22);
        } else if (op2.isReg16() && op2.reg16() == Register16::SP) {
            ret.push_back(0xed);
            ret.push_back(0x73);
        } else {
            return InvalidInstruction;
        }

        ret.push_back(op2.wordLowByte());
        ret.push_back(op2.wordHighByte());
    } else if (op1.isIndirectReg16()) {
        /* LD (HL),reg8
			LD (HL),n
			LD (BC),A
			LD (DE),A */
        if (op1.reg16() == Register16::HL) {
            UnsignedByte opcode = 0x70;

            if (op2.isReg8()) {
                if (op2.reg8() == Register8::A) {
                    ret.push_back(opcode | RegbitsA);
                } else if (op2.reg8() == Register8::B) {
                    ret.push_back(opcode | RegbitsB);
                } else if (op2.reg8() == Register8::C) {
                    ret.push_back(opcode | RegbitsC);
                } else if (op2.reg8() == Register8::D) {
                    ret.push_back(opcode | RegbitsD);
                } else if (op2.reg8() == Register8::E) {
                    ret.push_back(opcode | RegbitsE);
                } else if (op2.reg8() == Register8::H) {
                    ret.push_back(opcode | RegbitsH);
                } else if (op2.reg8() == Register8::L) {
                    ret.push_back(opcode | RegbitsL);
                } else {
                    return InvalidInstruction;
                }
            } else if (op2.isByte()) {
                ret.push_back(0x36);
                ret.push_back(op2.byte());
            } else {
                return InvalidInstruction;
            }
        } else if (op1.reg16() == Register16::BC && op2.isReg8() && op2.reg8() == Register8::A) {
            ret.push_back(0x02);
        } else if (op1.reg16() == Register16::DE && op2.isReg8() && op2.reg8() == Register8::A) {
            ret.push_back(0x12);
        } else {
            return InvalidInstruction;
        }
    } else if (op1.isIndirectReg16WithOffset()) {
        /* LD (IX+d),reg8
			LD (IX+d),n
			LD (IY+d),n
			LD (IY+d),n */
        if (op1.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        } else if (op1.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else {
            return InvalidInstruction;
        }

        UnsignedByte opcode = 0x70;

        if (op2.isReg8()) {
            if (op2.reg8() == Register8::A) {
                ret.push_back(opcode | RegbitsA);
            } else if (op2.reg8() == Register8::B) {
                ret.push_back(opcode | RegbitsB);
            } else if (op2.reg8() == Register8::C) {
                ret.push_back(opcode | RegbitsC);
            } else if (op2.reg8() == Register8::D) {
                ret.push_back(opcode | RegbitsD);
            } else if (op2.reg8() == Register8::E) {
                ret.push_back(opcode | RegbitsE);
            } else if (op2.reg8() == Register8::H) {
                ret.push_back(opcode | RegbitsH);
            } else if (op2.reg8() == Register8::L) {
                ret.push_back(opcode | RegbitsL);
            } else {
                return InvalidInstruction;
            }

            ret.push_back(op1.offset());
        } else if (op2.isByte()) {
            ret.push_back(0x36);

            /* is this the right way round? */
            ret.push_back(op2.byte());
            ret.push_back(op1.offset());
        } else {
            return InvalidInstruction;
        }
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleLDD(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0xa8);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleLDDR(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0xb8);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleLDI(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0xa0);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleLDIR(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0xb0);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleNEG(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0x44);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleNOP(const Tokens &)
{
    Opcode ret;
    ret.push_back(0x00);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleOR(const Tokens & tokens)
{
    auto c = tokens.size();
    Opcode ret;

    if (c < 2) {
        std::cout << "OR requires one operand\n";
        return InvalidInstruction;
    }

    Operand op(tokens.at(1));

    if (op.isReg8()) {
        /* OR A,reg8 */
        UnsignedByte opcode = 0xb0;

        switch (op.reg8()) {
            case Register8::A:
                opcode |= RegbitsA;
                break;
            case Register8::B:
                opcode |= RegbitsB;
                break;
            case Register8::C:
                opcode |= RegbitsC;
                break;
            case Register8::D:
                opcode |= RegbitsD;
                break;
            case Register8::E:
                opcode |= RegbitsE;
                break;
            case Register8::H:
                opcode |= RegbitsH;
                break;
            case Register8::L:
                opcode |= RegbitsL;
                break;
            default:
                return InvalidInstruction;
        }

        ret.push_back(opcode);
    } else if (op.isIndirectReg16() && op.reg16() == Register16::HL) {
        /* OR A,(HL) */
        ret.push_back(0xb6);
    } else if (op.isIndirectReg16WithOffset()) {
        /* OR A,(IX+n) */
        /* OR A,(IY+n) */
        if (op.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        } else if (op.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.push_back(0xb6);
        ret.push_back(op.offset());
    } else if (op.isByte()) {
        /* OR A,n */
        ret.push_back(0xf6);
        ret.push_back(op.byte());
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleOUT(const Tokens & tokens)
{
    Opcode ret;
    auto c = tokens.size();

    if (3 == c) {
        Operand op1(tokens.at(1));
        Operand op2(tokens.at(2));

        /* TODO need IndirectByte operand type (perhaps it's IndirectPort?) - (N) */
//		if(op1.isReg8() && op1.reg8() == ) {
//		}
        if (op1.isIndirectReg8() && op1.reg8() == Register8::C) {
            if (op2.isReg8()) {
                switch (op2.reg8()) {
                    case Register8::B:
                        ret.push_back(0xed);
                        ret.push_back(0x41);
                        break;
                    case Register8::C:
                        ret.push_back(0xed);
                        ret.push_back(0x49);
                        break;
                    case Register8::D:
                        ret.push_back(0xed);
                        ret.push_back(0x51);
                        break;
                    case Register8::E:
                        ret.push_back(0xed);
                        ret.push_back(0x59);
                        break;
                    case Register8::H:
                        ret.push_back(0xed);
                        ret.push_back(0x61);
                        break;
                    case Register8::L:
                        ret.push_back(0xed);
                        ret.push_back(0x69);
                        break;
                    default:
                        return InvalidInstruction;
                }
            } else if (op2.isByte() && 0 == op2.byte()) {
                ret.push_back(0xed);
                ret.push_back(0x71);
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

Z80Interpreter::Opcode Z80Interpreter::assembleOUTD(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0xab);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleOTDR(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0xbb);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleOUTI(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0xa3);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleOTIR(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0xb3);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assemblePOP(const Tokens & tokens)
{
    auto c = tokens.size();
    Opcode ret;

    if (c < 2) {
        std::cout << "POP requires one operand\n";
        return InvalidInstruction;
    }

    Operand op(tokens.at(1));
    if (!op.isReg16()) {
        return InvalidInstruction;
    }

    if (op.reg16() == Register16::AF) {
        ret.push_back(0xf1);
    } else if (op.reg16() == Register16::BC) {
        ret.push_back(0xc1);
    } else if (op.reg16() == Register16::DE) {
        ret.push_back(0xd1);
    } else if (op.reg16() == Register16::HL) {
        ret.push_back(0xe1);
    } else if (op.reg16() == Register16::IX) {
        ret.push_back(0xdd);
        ret.push_back(0xe1);
    } else if (op.reg16() == Register16::IY) {
        ret.push_back(0xfd);
        ret.push_back(0xe1);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assemblePUSH(const Tokens & tokens)
{
    auto c = tokens.size();
    Opcode ret;

    if (c < 2) {
        std::cout << "PUSH requires one operand\n";
        return InvalidInstruction;
    }

    Operand op(tokens.at(1));
    if (!op.isReg16()) {
        return InvalidInstruction;
    }

    if (op.reg16() == Register16::AF) {
        ret.push_back(0xf5);
    } else if (op.reg16() == Register16::BC) {
        ret.push_back(0xc5);
    } else if (op.reg16() == Register16::DE) {
        ret.push_back(0xd5);
    } else if (op.reg16() == Register16::HL) {
        ret.push_back(0xe5);
    } else if (op.reg16() == Register16::IX) {
        ret.push_back(0xdd);
        ret.push_back(0xe5);
    } else if (op.reg16() == Register16::IY) {
        ret.push_back(0xfd);
        ret.push_back(0xe5);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRES(const Tokens & tokens)
{
    auto c = tokens.size();
    Opcode ret;

    if (c < 3) {
        std::cout << "RES requires two operands\n";
        return InvalidInstruction;
    }

    Operand op1(tokens.at(1));
    Operand op2(tokens.at(2));

    if (!op1.isBitIndex()) {
        return InvalidInstruction;
    }
    UnsignedByte opcode = 0x80;
    opcode += (op1.bitIndex() << 3);

    if (op2.isReg8() || (op2.isIndirectReg16() && op2.reg16() == Register16::HL)) {
        /* RES b,reg8
			RES b,(HL) */

        ret.push_back(0xcb);

        if (op2.isIndirectReg16() && op2.reg16() == Register16::HL) {
            opcode |= RegbitsIndirectHl;
        } else if (op2.reg8() == Register8::A) {
            opcode |= RegbitsA;
        } else if (op2.reg8() == Register8::B) {
            opcode |= RegbitsB;
        } else if (op2.reg8() == Register8::C) {
            opcode |= RegbitsC;
        } else if (op2.reg8() == Register8::D) {
            opcode |= RegbitsD;
        } else if (op2.reg8() == Register8::E) {
            opcode |= RegbitsE;
        } else if (op2.reg8() == Register8::H) {
            opcode |= RegbitsH;
        } else if (op2.reg8() == Register8::L) {
            opcode |= RegbitsL;
        } else {
            return InvalidInstruction;
        }

        ret.push_back(opcode);
    } else if (op2.isIndirectReg16WithOffset()) {
        /* RES b,(IX+n) */
        /* RES b,(IY+n) */
        if (op2.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        } else if (op2.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.push_back(0xcb);
        ret.push_back(op2.offset());
        ret.push_back(opcode | RegbitsIndirectIxIy);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRET(const Tokens & tokens)
{
    auto c = tokens.size();
    Opcode ret;

    if (1 == c) {
        ret.push_back(0xc9);
    } else if (2 == c) {
        Operand op(tokens.at(1));

        if (!op.isCondition()) {
            std::cout << "conditional RET requires a valid condition as its operand\n";
            return InvalidInstruction;
        }

        switch (op.condition()) {
            case Operand::ConditionType::NonZero:
                ret.push_back(0xc0);
                break;
            case Operand::ConditionType::Zero:
                ret.push_back(0xc8);
                break;
            case Operand::ConditionType::NoCarry:
                ret.push_back(0xd0);
                break;
            case Operand::ConditionType::Carry:
                ret.push_back(0xd8);
                break;
            case Operand::ConditionType::ParityOdd:
                ret.push_back(0xe0);
                break;
            case Operand::ConditionType::ParityEven:
                ret.push_back(0xe8);
                break;
            case Operand::ConditionType::Plus:
                ret.push_back(0xf0);
                break;
            case Operand::ConditionType::Minus:
                ret.push_back(0xf8);
                break;
            default:
                return InvalidInstruction;
        }
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRETI(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0x4d);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRETN(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0x45);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRLA(const Tokens &)
{
    Opcode ret;
    ret.push_back(0x17);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRL(const Tokens & tokens)
{
    auto c = tokens.size();

    if (2 != c) {
        std::cout << "RL instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Opcode ret;
    Operand op(tokens.at(1));

    if (op.isReg8()) {
        ret.push_back(0xcb);

        switch (op.reg8()) {
            case Register8::A:
                ret.push_back(0x10 | RegbitsA);
            case Register8::B:
                ret.push_back(0x10 | RegbitsB);
            case Register8::C:
                ret.push_back(0x10 | RegbitsC);
            case Register8::D:
                ret.push_back(0x10 | RegbitsD);
            case Register8::E:
                ret.push_back(0x10 | RegbitsE);
            case Register8::H:
                ret.push_back(0x10 | RegbitsH);
            case Register8::L:
                ret.push_back(0x10 | RegbitsL);
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16() && op.reg16() == Register16::HL) {
        ret.push_back(0xcb);
        ret.push_back(0x10 | RegbitsIndirectHl);
    } else if (op.isIndirectReg16WithOffset()) {
        if (op.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        }
        if (op.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.push_back(op.offset());
        ret.push_back(0x16);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRLCA(const Tokens &)
{
    Opcode ret;
    ret.push_back(0x07);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRLC(const Tokens & tokens)
{
    auto c = tokens.size();

    if (2 != c) {
        std::cout << "RLC instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Opcode ret;
    Operand op(tokens.at(1));

    if (op.isReg8()) {
        ret.push_back(0xcb);

        switch (op.reg8()) {
            case Register8::A:
                ret.push_back(0x00 | RegbitsA);
            case Register8::B:
                ret.push_back(0x00 | RegbitsB);
            case Register8::C:
                ret.push_back(0x00 | RegbitsC);
            case Register8::D:
                ret.push_back(0x00 | RegbitsD);
            case Register8::E:
                ret.push_back(0x00 | RegbitsE);
            case Register8::H:
                ret.push_back(0x00 | RegbitsH);
            case Register8::L:
                ret.push_back(0x00 | RegbitsL);
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16() && op.reg16() == Register16::HL) {
        ret.push_back(0xcb);
        ret.push_back(0x00 | RegbitsIndirectHl);
    } else if (op.isIndirectReg16WithOffset()) {
        if (op.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        }
        if (op.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.push_back(op.offset());
        ret.push_back(0x06);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRLD(const Tokens &)
{
    Opcode ret;
    ret.push_back(0xed);
    ret.push_back(0x6f);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRRA(const Tokens &)
{
    Opcode ret;
    ret.push_back(0x1f);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRR(const Tokens & tokens)
{
    auto c = tokens.size();

    if (2 != c) {
        std::cout << "RR instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Opcode ret;
    Operand op(tokens.at(1));

    if (op.isReg8()) {
        ret.push_back(0xcb);

        switch (op.reg8()) {
            case Register8::A:
                ret.push_back(0x18 | RegbitsA);
            case Register8::B:
                ret.push_back(0x18 | RegbitsB);
            case Register8::C:
                ret.push_back(0x18 | RegbitsC);
            case Register8::D:
                ret.push_back(0x18 | RegbitsD);
            case Register8::E:
                ret.push_back(0x18 | RegbitsE);
            case Register8::H:
                ret.push_back(0x18 | RegbitsH);
            case Register8::L:
                ret.push_back(0x18 | RegbitsL);
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16() && op.reg16() == Register16::HL) {
        ret.push_back(0xcb);
        ret.push_back(0x18 | RegbitsIndirectHl);
    } else if (op.isIndirectReg16WithOffset()) {
        if (op.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        }
        if (op.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.push_back(op.offset());
        ret.push_back(0x1e);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRRCA(const Tokens &)
{
    Opcode ret;
    ret.push_back(0x0f);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRRC(const Tokens & tokens)
{
    auto c = tokens.size();

    if (2 != c) {
        std::cout << "RRC instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Opcode ret;
    Operand op(tokens.at(1));

    if (op.isReg8()) {
        ret.push_back(0xcb);

        switch (op.reg8()) {
            case Register8::A:
                ret.push_back(0x08 | RegbitsA);
            case Register8::B:
                ret.push_back(0x08 | RegbitsB);
            case Register8::C:
                ret.push_back(0x08 | RegbitsC);
            case Register8::D:
                ret.push_back(0x08 | RegbitsD);
            case Register8::E:
                ret.push_back(0x08 | RegbitsE);
            case Register8::H:
                ret.push_back(0x08 | RegbitsH);
            case Register8::L:
                ret.push_back(0x08 | RegbitsL);
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16() && op.reg16() == Register16::HL) {
        ret.push_back(0xcb);
        ret.push_back(0x08 | RegbitsIndirectHl);
    } else if (op.isIndirectReg16WithOffset()) {
        if (op.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        }
        if (op.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.push_back(op.offset());
        ret.push_back(0x0e);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleRRD(const Tokens &)
{
    return {{0xed, 0x67}};
}

Z80Interpreter::Opcode Z80Interpreter::assembleRST(const Tokens & tokens)
{
    static constexpr const char * const invalidResetLocation = "RST instruction's operand must be one of the reset addresses $00, $08, $10, $18, $20, $28, $30, $38.";
    auto c = tokens.size();

    if (2 > tokens.size()) {
        std::cout << "RST instruction requires at least one operand.\n";
        return InvalidInstruction;
    }

    Operand op(tokens[1]);

    if (!op.isByte()) {
        std::cout << invalidResetLocation << '\n';
        return InvalidInstruction;
    }

    switch (op.byte()) {
        case 0x00:
            return {std::initializer_list<Opcode::value_type>{0b11000111}};
            break;

        case 0x08:
            return {std::initializer_list<Opcode::value_type>{0b11001111}};
            break;

        case 0x10:
            return {std::initializer_list<Opcode::value_type>{0b11010111}};
            break;

        case 0x18:
            return {std::initializer_list<Opcode::value_type>{0b11011111}};
            break;

        case 0x20:
            return {std::initializer_list<Opcode::value_type>{0b11100111}};
            break;

        case 0x28:
            return {std::initializer_list<Opcode::value_type>{0b11101111}};
            break;

        case 0x30:
            return {std::initializer_list<Opcode::value_type>{0b11110111}};
            break;

        case 0x38:
            return {std::initializer_list<Opcode::value_type>{0b11111111}};
            break;

        default:
            std::cout << invalidResetLocation << '\n';
            return InvalidInstruction;
    }
}

Z80Interpreter::Opcode Z80Interpreter::assembleSBC(const Tokens & tokens)
{
    auto c = tokens.size();

    if (2 > tokens.size()) {
        std::cout << "SBC instruction requires at least one operand.\n";
        return InvalidInstruction;
    }

    Opcode ret;

    if (2 == c) {
        Operand op(tokens.at(1));

        if (op.isReg8()) {
            switch (op.reg8()) {
                case Register8::A:
                    ret.push_back(0x98 | RegbitsA);
                    break;
                case Register8::B:
                    ret.push_back(0x98 | RegbitsB);
                    break;
                case Register8::C:
                    ret.push_back(0x98 | RegbitsC);
                    break;
                case Register8::D:
                    ret.push_back(0x98 | RegbitsD);
                    break;
                case Register8::E:
                    ret.push_back(0x98 | RegbitsE);
                    break;
                case Register8::H:
                    ret.push_back(0x98 | RegbitsH);
                    break;
                case Register8::L:
                    ret.push_back(0x98 | RegbitsL);
                    break;
                default:
                    return InvalidInstruction;
            }
        } else if (op.isIndirectReg16() && op.reg16() == Register16::HL) {
            ret.push_back(0xcb);
            ret.push_back(0x98 | RegbitsIndirectHl);
        } else {
            return InvalidInstruction;
        }
    } else if (3 == c) {
        Operand op1(tokens.at(1));
        Operand op2(tokens.at(2));

        if (op1.isReg8() && op1.reg8() == Register8::A) {
            if (op2.isByte()) {
                ret.push_back(0xde);
                ret.push_back(op2.byte());
            } else if (op2.isIndirectReg16WithOffset()) {
                if (op2.reg16() == Register16::IX) {
                    ret.push_back(0xdd);
                }
                if (op2.reg16() == Register16::IY) {
                    ret.push_back(0xfd);
                } else {
                    return InvalidInstruction;
                }

                ret.push_back(0x9e);
                ret.push_back(op2.offset());
            } else {
                return InvalidInstruction;
            }
        } else if (op1.isReg16() && op1.reg16() == Register16::HL && op2.isReg16()) {
            ret.push_back(0xed);

            switch (op2.reg16()) {
                case Register16::BC:
                    ret.push_back(0x42);
                    break;
                case Register16::DE:
                    ret.push_back(0x52);
                    break;
                case Register16::HL:
                    ret.push_back(0x62);
                    break;
                case Register16::SP:
                    ret.push_back(0x72);
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

Z80Interpreter::Opcode Z80Interpreter::assembleSCF(const Tokens &)
{
    Opcode ret;
    ret.push_back(0x37);
    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleSET(const Tokens & tokens)
{
    auto c = tokens.size();
    Opcode ret;

    if (c < 3) {
        std::cout << "SET instruction requires two operands\n";
        return InvalidInstruction;
    }

    Operand op1(tokens.at(1));
    Operand op2(tokens.at(2));

    if (!op1.isBitIndex()) {
        return InvalidInstruction;
    }
    UnsignedByte opcode = 0xc0;
    opcode += (op1.bitIndex() << 3);

    if (op2.isReg8() || (op2.isIndirectReg16() && op2.reg16() == Register16::HL)) {
        /* SET b,reg8
			SET b,(HL) */
        ret.push_back(0xcb);

        if (op2.isIndirectReg16() && op2.reg16() == Register16::HL) {
            opcode |= RegbitsIndirectHl;
        } else if (op2.reg8() == Register8::A) {
            opcode |= RegbitsA;
        } else if (op2.reg8() == Register8::B) {
            opcode |= RegbitsB;
        } else if (op2.reg8() == Register8::C) {
            opcode |= RegbitsC;
        } else if (op2.reg8() == Register8::D) {
            opcode |= RegbitsD;
        } else if (op2.reg8() == Register8::E) {
            opcode |= RegbitsE;
        } else if (op2.reg8() == Register8::H) {
            opcode |= RegbitsH;
        } else if (op2.reg8() == Register8::L) {
            opcode |= RegbitsL;
        } else {
            return InvalidInstruction;
        }

        ret.push_back(opcode);
    } else if (op2.isIndirectReg16WithOffset()) {
        if (op2.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        }
        if (op2.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.push_back(0xcb);
        ret.push_back(op2.offset());
        ret.push_back(opcode | RegbitsIndirectIxIy);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleSLA(const Tokens & tokens)
{
    auto c = tokens.size();

    if (2 != c) {
        std::cout << "SLA instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Opcode ret;
    Operand op(tokens.at(1));

    if (op.isReg8()) {
        ret.push_back(0xcb);

        switch (op.reg8()) {
            case Register8::A:
                ret.push_back(0x20 | RegbitsA);
                break;
            case Register8::B:
                ret.push_back(0x20 | RegbitsB);
                break;
            case Register8::C:
                ret.push_back(0x20 | RegbitsC);
                break;
            case Register8::D:
                ret.push_back(0x20 | RegbitsD);
                break;
            case Register8::E:
                ret.push_back(0x20 | RegbitsE);
                break;
            case Register8::H:
                ret.push_back(0x20 | RegbitsH);
                break;
            case Register8::L:
                ret.push_back(0x20 | RegbitsL);
                break;
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16() && op.reg16() == Register16::HL) {
        ret.push_back(0xcb);
        ret.push_back(0x20 | RegbitsIndirectHl);
    } else if (op.isIndirectReg16WithOffset()) {
        if (op.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        }
        if (op.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.push_back(0xcb);
        ret.push_back(op.offset());
        ret.push_back(0x20 | RegbitsIndirectIxIy);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleSRA(const Tokens & tokens)
{
    auto c = tokens.size();

    if (2 != c) {
        std::cout << "SRA instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Opcode ret;
    Operand op(tokens.at(1));

    if (op.isReg8()) {
        ret.push_back(0xcb);

        switch (op.reg8()) {
            case Register8::A:
                ret.push_back(0x28 | RegbitsA);
                break;
            case Register8::B:
                ret.push_back(0x28 | RegbitsB);
                break;
            case Register8::C:
                ret.push_back(0x28 | RegbitsC);
                break;
            case Register8::D:
                ret.push_back(0x28 | RegbitsD);
                break;
            case Register8::E:
                ret.push_back(0x28 | RegbitsE);
                break;
            case Register8::H:
                ret.push_back(0x28 | RegbitsH);
                break;
            case Register8::L:
                ret.push_back(0x28 | RegbitsL);
                break;
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16() && op.reg16() == Register16::HL) {
        ret.push_back(0xcb);
        ret.push_back(0x28 | RegbitsIndirectHl);
    } else if (op.isIndirectReg16WithOffset()) {
        if (op.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        }
        if (op.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.push_back(0xcb);
        ret.push_back(op.offset());
        ret.push_back(0x28 | RegbitsIndirectIxIy);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleSLL(const Tokens & tokens)
{
    auto c = tokens.size();

    if (2 != c) {
        std::cout << "SLL instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Opcode ret;
    Operand op(tokens.at(1));

    if (op.isReg8()) {
        ret.push_back(0xcb);

        switch (op.reg8()) {
            case Register8::A:
                ret.push_back(0x30 | RegbitsA);
                break;
            case Register8::B:
                ret.push_back(0x30 | RegbitsB);
                break;
            case Register8::C:
                ret.push_back(0x30 | RegbitsC);
                break;
            case Register8::D:
                ret.push_back(0x30 | RegbitsD);
                break;
            case Register8::E:
                ret.push_back(0x30 | RegbitsE);
                break;
            case Register8::H:
                ret.push_back(0x30 | RegbitsH);
                break;
            case Register8::L:
                ret.push_back(0x30 | RegbitsL);
                break;
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16() && op.reg16() == Register16::HL) {
        ret.push_back(0xcb);
        ret.push_back(0x30 | RegbitsIndirectHl);
    } else if (op.isIndirectReg16WithOffset()) {
        if (op.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        }
        if (op.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.push_back(0xcb);
        ret.push_back(op.offset());
        ret.push_back(0x30 | RegbitsIndirectIxIy);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleSRL(const Tokens & tokens)
{
    auto c = tokens.size();

    if (2 != c) {
        std::cout << "SRL instruction requires one operand.\n";
        return InvalidInstruction;
    }

    Opcode ret;
    Operand op(tokens.at(1));

    if (op.isReg8()) {
        ret.push_back(0xcb);

        switch (op.reg8()) {
            case Register8::A:
                ret.push_back(0x38 | RegbitsA);
                break;
            case Register8::B:
                ret.push_back(0x38 | RegbitsB);
                break;
            case Register8::C:
                ret.push_back(0x38 | RegbitsC);
                break;
            case Register8::D:
                ret.push_back(0x38 | RegbitsD);
                break;
            case Register8::E:
                ret.push_back(0x38 | RegbitsE);
                break;
            case Register8::H:
                ret.push_back(0x38 | RegbitsH);
                break;
            case Register8::L:
                ret.push_back(0x38 | RegbitsL);
                break;
            default:
                return InvalidInstruction;
        }
    } else if (op.isIndirectReg16() && op.reg16() == Register16::HL) {
        ret.push_back(0xcb);
        ret.push_back(0x38 | RegbitsIndirectHl);
    } else if (op.isIndirectReg16WithOffset()) {
        if (op.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        }
        if (op.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.push_back(0xcb);
        ret.push_back(op.offset());
        ret.push_back(0x38 | RegbitsIndirectIxIy);
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleSUB(const Tokens & tokens)
{
    auto c = tokens.size();
    Opcode ret;

    if (c < 2) {
        std::cout << "SUB requires one operand\n";
        return InvalidInstruction;
    }

    Operand op(tokens.at(1));

    if (op.isReg8() || (op.isIndirectReg16() && op.reg16() == Register16::HL)) {
        /* SUB reg8
			SUB (HL) */
        UnsignedByte opcode = 0x90;

        if (op.isIndirectReg16() && op.reg16() == Register16::HL) {
            opcode |= RegbitsIndirectHl;
        } else if (op.reg8() == Register8::A) {
            opcode |= RegbitsA;
        } else if (op.reg8() == Register8::B) {
            opcode |= RegbitsB;
        } else if (op.reg8() == Register8::C) {
            opcode |= RegbitsC;
        } else if (op.reg8() == Register8::D) {
            opcode |= RegbitsD;
        } else if (op.reg8() == Register8::E) {
            opcode |= RegbitsE;
        } else if (op.reg8() == Register8::H) {
            opcode |= RegbitsH;
        } else if (op.reg8() == Register8::L) {
            opcode |= RegbitsL;
        } else {
            return InvalidInstruction;
        }

        ret.push_back(opcode);
    } else if (op.isIndirectReg16WithOffset()) {
        if (op.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        }
        if (op.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.push_back(0x90 | RegbitsIndirectIxIy);
        ret.push_back(op.offset());
    } else if (op.isByte()) {
        /* SUB n */
        ret.push_back(0xd6);
        ret.push_back(op.byte());
    } else {
        return InvalidInstruction;
    }

    return ret;
}

Z80Interpreter::Opcode Z80Interpreter::assembleXOR(const Tokens & tokens)
{
    auto c = tokens.size();
    Opcode ret;

    if (c < 2) {
        std::cout << "XOR instruction requires one operand\n";
        return InvalidInstruction;
    }

    Operand op(tokens.at(1));

    if (op.isReg8()) {
        /* XOR A,reg8 */
        UnsignedByte opcode = 0xa8;

        switch (op.reg8()) {
            case Register8::A:
                opcode |= RegbitsA;
                break;
            case Register8::B:
                opcode |= RegbitsB;
                break;
            case Register8::C:
                opcode |= RegbitsC;
                break;
            case Register8::D:
                opcode |= RegbitsD;
                break;
            case Register8::E:
                opcode |= RegbitsE;
                break;
            case Register8::H:
                opcode |= RegbitsH;
                break;
            case Register8::L:
                opcode |= RegbitsL;
                break;
            default:
                return InvalidInstruction;
        }

        ret.push_back(opcode);
    } else if (op.isIndirectReg16() && op.reg16() == Register16::HL) {
        /* XOR A,(HL) */
        ret.push_back(0xae);
    } else if (op.isIndirectReg16WithOffset()) {
        /* XOR A,(IX+n) */
        /* XOR A,(IY+n) */
        if (op.reg16() == Register16::IX) {
            ret.push_back(0xdd);
        } else if (op.reg16() == Register16::IY) {
            ret.push_back(0xfd);
        } else {
            return InvalidInstruction;
        }

        ret.push_back(0xae);
        ret.push_back(op.offset());
    } else if (op.isByte()) {
        /* XOR A,n */
        ret.push_back(0xee);
        ret.push_back(op.byte());
    } else {
        return InvalidInstruction;
    }

    return ret;
}
