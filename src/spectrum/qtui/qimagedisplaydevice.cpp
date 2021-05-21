#include <cmath>
#include <iostream>
#include <chrono>
#include <QImage>
#include <QRgb>
#include <QPainter>
#include "qimagedisplaydevice.h"
#include "../../util/debug.h"

using namespace Spectrum::QtUi;

namespace
{
    constexpr const int DefaultRefreshRate = 50;

    constexpr const QRgb colourMap[16] = {
            qRgb(0x00, 0x00, 0x00),
            qRgb(0x00, 0x00, 0xcd),
            qRgb(0xcd, 0x00, 0x00),
            qRgb(0xcd, 0x00, 0xcd),
            qRgb(0x00, 0xcd, 0x00),
            qRgb(0x00, 0xcd, 0xcd),
            qRgb(0xcd, 0xcd, 0x00),
            qRgb(0xcd, 0xcd, 0xcd),

            qRgb(0x00, 0x00, 0x00),
            qRgb(0x00, 0x00, 0xff),
            qRgb(0xff, 0x00, 0x00),
            qRgb(0xff, 0x00, 0xff),
            qRgb(0x00, 0xff, 0x00),
            qRgb(0x00, 0xff, 0xff),
            qRgb(0xff, 0xff, 0x00),
            qRgb(0xff, 0xff, 0xff)
    };

    constexpr const QRgb monochromeMap[16] = {
            qRgb(0x00, 0x00, 0x00),
            qRgb(0x1d, 0x1d, 0x1d),
            qRgb(0x3a, 0x3a, 0x3a),
            qRgb(0x57, 0x57, 0x57),
            qRgb(0x75, 0x75, 0x75),
            qRgb(0x92, 0x92, 0x92),
            qRgb(0xaf, 0xaf, 0xaf),
            qRgb(0xcd, 0xcd, 0xcd),

            qRgb(0x00, 0x00, 0x00),
            qRgb(0x24, 0x24, 0x24),
            qRgb(0x48, 0x48, 0x48),
            qRgb(0x6d, 0x6d, 0x6d),
            qRgb(0x91, 0x91, 0x91),
            qRgb(0xb6, 0xb6, 0xb6),
            qRgb(0xda, 0xda, 0xda),
            qRgb(0xff, 0xff, 0xff),
    };
}

QImageDisplayDevice::QImageDisplayDevice(int frameSkip)
: m_image(fullWidth(), fullHeight(), QImage::Format_ARGB32),
  m_border(Colour::White),
  m_frameCounter(0),
  m_frameSkip(frameSkip + 1),
  m_colourMode(ColourMode::Colour),
  m_bwForeground(DefaultBlackAndWhiteForeground),
  m_bwBackground(DefaultBlackAndWhiteBackground)
{
    assert (m_frameSkip > 0);
}

void QImageDisplayDevice::redrawDisplay(const DisplayFile & displayMemory)
{
    using namespace std::chrono_literals;

    ++m_frameCounter;

    // m_frameSkip will be 1 (its lowest possible value) if frame skipping is disabled
    if (1 == m_frameSkip || m_frameCounter % m_frameSkip) {
        renderFrame(displayMemory);
    }
}

void QImageDisplayDevice::renderFrame(const DisplayFile & displayMemory)
{
    bool flashInvert = m_frameCounter & 0x10;
    auto * data = reinterpret_cast<QRgb *>(image().bits());

    if (ColourMode::BlackAndWhite == m_colourMode) {
        // 32 bytes per scanline, 192 scanlines
        for (std::uint8_t y = 0; y < Height; ++y) {
            for (std::uint8_t xByte = 0; xByte < 32; ++xByte) {
                // address translation algorithm: https://zxasm.wordpress.com/2016/05/28/zx-spectrum-screen-memory-layout/
                std::uint8_t yBits = (y & 0b11000000) | ((y & 0b00111000) >> 3) | ((y & 0b00000111) << 3);
                std::uint16_t addr = static_cast<std::uint16_t>(xByte & 0b00011111) | (static_cast<std::uint16_t>(yBits) << 5);
                std::uint8_t attr = displayMemory[AttributesOffset + static_cast<std::size_t>(std::floor(y / 8)) * 32 + xByte];
                std::uint8_t mask = 0b10000000;
                QRgb ink = m_bwForeground;
                QRgb paper = m_bwBackground;

                if (flashInvert && isFlashing(attr)) {
                    ink = m_bwBackground;
                    paper = m_bwForeground;
                }

                for (int bit = 0; bit < 8; ++bit) {
                    data[((BorderSize + y) * fullWidth()) + BorderSize + (xByte * 8) + bit] = (displayMemory[addr] & mask ? ink : paper);
                    mask >>= 1;
                }
            }
        }
    } else {
        const auto * palette = colourMap;

        if (ColourMode::Monochrome == m_colourMode) {
            palette = monochromeMap;
        }

        // 32 bytes per scanline, 192 scanlines
        for (std::uint8_t y = 0; y < Height; ++y) {
            for (std::uint8_t xByte = 0; xByte < 32; ++xByte) {
                // address translation algorithm: https://zxasm.wordpress.com/2016/05/28/zx-spectrum-screen-memory-layout/
                std::uint8_t yBits = (y & 0b11000000) | ((y & 0b00111000) >> 3) | ((y & 0b00000111) << 3);
                std::uint16_t addr =
                        static_cast<std::uint16_t>(xByte & 0b00011111) | (static_cast<std::uint16_t>(yBits) << 5);
                std::uint8_t attr = displayMemory[AttributesOffset + static_cast<std::size_t>(std::floor(y / 8)) * 32 +
                                                  xByte];
                std::uint8_t mask = 0b10000000;
                std::size_t colourIndex;
                std::uint8_t ink;
                std::uint8_t paper;

                if (flashInvert && isFlashing(attr)) {
                    ink = static_cast<std::size_t>(paperColour(attr));
                    paper = static_cast<std::size_t>(inkColour(attr));
                } else {
                    ink = static_cast<std::size_t>(inkColour(attr));
                    paper = static_cast<std::size_t>(paperColour(attr));
                }

                for (int bit = 0; bit < 8; ++bit) {
                    if (displayMemory[addr] & mask) {
                        colourIndex = ink;
                    } else {
                        colourIndex = paper;
                    }

                    if (isBright(attr)) {
                        colourIndex += 8;
                    }

                    data[((BorderSize + y) * fullWidth()) + BorderSize + (xByte * 8) + bit] = palette[colourIndex];
                    mask >>= 1;
                }
            }
        }
    }
}

void QImageDisplayDevice::setBorder(Colour colour, bool bright)
{
    m_border = colour;

    QPainter painter(&image());
    auto idx = static_cast<std::size_t>(colour);

    if (bright) {
        idx += 8;
    }

    auto fill = QBrush(colourMap[idx]);
    painter.fillRect(0, 0, Width + BorderSize + BorderSize, BorderSize, fill);
    painter.fillRect(0, BorderSize, BorderSize, Height, fill);
    painter.fillRect(Width + BorderSize, BorderSize, BorderSize, Height, fill);
    painter.fillRect(0, Height + BorderSize, Width + BorderSize + BorderSize, BorderSize, fill);
    painter.end();
}

Spectrum::Colour QImageDisplayDevice::border() const
{
    return m_border;
}
