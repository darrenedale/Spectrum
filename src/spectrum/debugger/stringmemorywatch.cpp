//
// Created by darren on 05/05/2021.
//

#include <array>
#include <sstream>
#include "stringmemorywatch.h"

using namespace Spectrum::Debugger;

StringMemoryWatch::StringMemoryWatch(Spectrum::BaseSpectrum::MemoryType * memory, ::Z80::UnsignedWord address, MemoryWatch::WatchSize size)
: MemoryWatch(memory, address),
  m_size(static_cast<std::uint32_t>(size)),
  m_charset(CharacterEncoding::Spectrum),
  m_typeName()
{}

std::string StringMemoryWatch::typeName() const
{
    if (m_typeName.empty()) {
        std::ostringstream out;
        out << "String [" << size() << ']';
        m_typeName = out.str();
    }

    return m_typeName;
}

std::string StringMemoryWatch::displayValue() const
{
    std::vector<::Z80::UnsignedByte> str(size());
    memory()->readBytes(address(), str.size(), str.data());
    std::ostringstream out;

    switch (characterEncoding()) {
        case CharacterEncoding::Spectrum:
            for (const auto & byte : str) {
                appendSpectrumChar(out, byte);
            }
            break;

        case CharacterEncoding::Ascii:
            for (const auto & byte : str) {
                if (0x80 & byte) {
                    out << "\xef\xbf\xbd";  // � U+FFFD Replacement Character (0xef 0xbf 0xbd in UTF-8)
                } else {
                    out.put(static_cast<std::ostringstream::char_type>(byte));
                }
            }
            break;
    }

    return out.str();
}

void StringMemoryWatch::appendSpectrumChar(std::ostream & out, ::Z80::UnsignedByte ch)
{
    using namespace std::string_literals;

    // keyword "characters" from 165 - 255
    static const std::array<const std::string, 91> keywords = {
            "[RND]"s, "[INKEY$]"s, "[PI]"s, "[FN]"s, "[POINT]"s,
            "[SCREEN$]"s, "[ATTR]"s, "[AT]"s, "[TAB]"s, "[VAL$]"s, "[CODE]"s, "[VAL]"s, "[LEN]"s, "[SIN]"s, "[COS]"s,
            "[TAN]"s, "[ASN]"s, "[ACS]"s, "[ATN]"s, "[LN]"s, "[EXP]"s, "[INT]"s, "[SQR]"s, "[SGN]"s, "[ABS]"s,
            "[PEEK]"s, "[IN]"s, "[USR]"s, "[STR$]"s, "[CHR$]"s, "[NOT]"s, "[BIN]"s, "[OR]"s, "[AND]"s, "[<=]"s,
            "[>=]"s, "[<>]"s, "[LINE]"s, "[THEN]"s, "[TO]"s, "[STEP]"s, "[DEF FN]"s, "[CAT]"s, "[FORMAT]"s, "[MOVE]"s,
            "[ERASE]"s, "[OPEN #]"s, "[CLOSE #]"s, "[MERGE]"s, "[VERIFY]"s, "[BEEP]"s, "[CIRCLE]"s, "[INK]"s, "[PAPER]"s, "[FLASH]"s,
            "[BRIGHT]"s, "[INVERSE]"s, "[OVER]"s, "[OUT]"s, "[LPRINT]"s, "[LLIST]"s, "[STOP]"s, "[READ]"s, "[DATA]"s, "[RESTORE]"s,
            "[NEW]"s, "[BORDER]"s, "[CONTINUE]"s, "[DIM]"s, "[REM]"s, "[FOR]"s, "[GO TO]"s, "[GO SUB]"s, "[INPUT]"s, "[LOAD]"s,
            "[LIST]"s, "[LET]"s, "[PAUSE]"s, "[NEXT]"s, "[POKE]"s, "[PRINT]"s, "[PLOT]"s, "[RUN]"s, "[SAVE]"s, "[RANDOMIZE]"s,
            "[IF]"s, "[CLS]"s, "[DRAW]"s, "[CLEAR]"s, "[RETURN]"s, "[COPY]"s,
    };

    if (96 == ch) {
        // UK pounds sterling symbol
        out << "£";
    } else if (127 == ch) {
        // copyright symbol
        out << "©"; // U+00A9 Copyright Sign (0xc2 0xa9 in UTF-8)
    } else if (32 <= ch && 127 > ch) {
        // other characters from 32 - 126 inclusive are as ASCII
        out << static_cast<std::ostream::char_type>(ch);
    } else if (12 == ch) {
        // delete
        out << "⌫"; // U+232b Erase to the Left (0xe2 0x8c 0xab in UTF-8)
    } else if (13 == ch) {
        // enter
        out << "⏎"; // U+23ce Return Symbol (0xe2 0x8f 0x8e in UTF-8)
    } else if (128 == ch) {
        // Spectrum block graphic empty
        out << " ";
    } else if (129 == ch) {
        // Spectrum block graphic
        out << "\xe2\x96\x9d";  // U+259D Quadrant upper-right
    } else if (130 == ch) {
        // Spectrum block graphic
        out << "\xe2\x96\x98";  // U+2598 Quadrant upper-left
    } else if (131 == ch) {
        // Spectrum block graphic
        out << "\xe2\x96\x80";  // U+2580 Upper half-block
    } else if (132 == ch) {
        // Spectrum block graphic
        out << "\xe2\x96\x97";  // U+2597 Quadrant lower-right
    } else if (133 == ch) {
        // Spectrum block graphic
        out << "\xe2\x96\x90";  // U+2590 Right half-block
    } else if (134 == ch) {
        // Spectrum block graphic
        out << "\xe2\x96\x9a";  // U+259A Quadrant upper-left and lower-right
    } else if (135 == ch) {
        // Spectrum block graphic
        out << "\xe2\x96\x9c";  // U+259C Quadrant upper-left and upper-right and lower-right
    } else if (136 == ch) {
        // Spectrum block graphic
        out << "\xe2\x96\x96";  // U+2596 Quadrant lower-left
    } else if (137 == ch) {
        // Spectrum block graphic
        out << "\xe2\x96\x9e";  // U+259E Quadrant upper-right and lower-left
    } else if (138 == ch) {
        // Spectrum block graphic
        out << "\xe2\x96\x8c";  // U+258C Left half-block
    } else if (139 == ch) {
        // Spectrum block graphic
        out << "\xe2\x96\x9b";  // U+259B Quadrant upper-left and upper-right and lower-left
    } else if (140 == ch) {
        // Spectrum block graphic
        out << "\xe2\x96\x84";  // U+2584 Lower half-block
    } else if (141 == ch) {
        // Spectrum block graphic
        out << "\xe2\x96\x9f";  // U+259F Quadrant upper-right and lower-right and lower-left
    } else if (142 == ch) {
        // Spectrum block graphic
        out << "\xe2\x96\x99";  // U+2599 Quadrant upper-left and lower-left and lower-right
    } else if (143 == ch) {
        // Spectrum block graphic
        out << "\xe2\x96\x88";  // U+2588 Full block
    } else if (165 <= ch) {
        // Spectrum BASIC keyword chars
        out << keywords[ch - 165];
    } else {
        out << "�"; // U+FFFD Replacement Character (0xef 0xbf 0xbd in UTF-8)
    }
}
