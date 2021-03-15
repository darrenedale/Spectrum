#include <cmath>
#include <iostream>

#include <QImage>
#include <QRgb>
#include <QPainter>

#include "qimagedisplaydevice.h"

using namespace Spectrum::Qt;

namespace
{
    constexpr const std::size_t BorderSize = 32;

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
}

QImageDisplayDevice::QImageDisplayDevice()
: m_image(fullWidth(), fullHeight(), QImage::Format_ARGB32),
  m_border(Colour::White),
  m_frameCounter(0)
{
}

void QImageDisplayDevice::redrawDisplay(const uint8_t * displayMemory)
{
    ++m_frameCounter;
    bool flashInvert = m_frameCounter & 0x20;

#if defined(QSPECTRUMDISPLAY_USEPAINTER)
    QPainter painter(&image());
#else
    auto * data = reinterpret_cast<QRgb *>(image().bits());
#endif

    // 32 bytes per scanline, 192 scanlines
    for (std::uint8_t y = 0; y < Height; ++y) {
        for (std::uint8_t xByte = 0; xByte < 32; ++xByte) {
            // address translation algorithm: https://zxasm.wordpress.com/2016/05/28/zx-spectrum-screen-memory-layout/
            std::uint8_t yBits = (y & 0b11000000) | ((y & 0b00111000) >> 3) | ((y & 0b00000111) << 3);
            std::uint16_t addr = static_cast<std::uint16_t>(xByte & 0b00011111) | (static_cast<std::uint16_t>(yBits) << 5);
            std::uint8_t attr = displayMemory[AttributesOffset + static_cast<std::size_t>(std::floor(y / 8)) * 32 + xByte];
            std::uint8_t mask = 0b10000000;
            std::size_t colourIndex;
            std::uint8_t ink;
            std::uint8_t paper;

            if (flashInvert && isFlashing(attr)) {
                ink = static_cast<std::size_t>(isFlashing(attr) && flashInvert ? paperColour(attr) :  inkColour(attr));
                paper = static_cast<std::size_t>(isFlashing(attr) && flashInvert ? inkColour(attr) :  paperColour(attr));
            } else {
                ink = static_cast<std::size_t>(isFlashing(attr) && flashInvert ? paperColour(attr) :  inkColour(attr));
                paper = static_cast<std::size_t>(isFlashing(attr) && flashInvert ? inkColour(attr) :  paperColour(attr));
            }

            for (int bit = 0; bit < 8; ++bit) {
#if defined(QSPECTRUMDISPLAY_USEPAINTER)
                if (displayMemory[addr] & mask) {
                    painter.setPen(colourMap[0]);
                } else {
                    painter.setPen(colourMap[7]);
                }

                painter.drawPoint(static_cast<int>(xByte * 8 + bit), static_cast<int>(y));
#else
                if (displayMemory[addr] & mask) {
                    colourIndex = ink;
                } else {
                    colourIndex = paper;
                }

                if (isBright(attr)) {
                    colourIndex += 8;
                }

                data[((BorderSize + y) * fullWidth()) + BorderSize + (xByte * 8) + bit] = colourMap[colourIndex];
#endif
                mask >>= 1;
            }
        }
    }

#if defined(QSPECTRUMDISPLAY_USEPAINTER)
    painter.end();
#endif
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

constexpr int QImageDisplayDevice::fullWidth()
{
    return Width + BorderSize + BorderSize;
}

constexpr int QImageDisplayDevice::fullHeight()
{
    return Height + BorderSize + BorderSize;
}

Spectrum::Colour QImageDisplayDevice::border() const
{
    return m_border;
}
