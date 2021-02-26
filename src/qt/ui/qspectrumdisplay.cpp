#include <cmath>

#include <QImage>
#include <QRgb>
#include <QPainter>

#include "qspectrumdisplay.h"

//#define QSPECTRUMDISPLAY_USEPAINTER

using namespace Spectrum;

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

QSpectrumDisplay::QSpectrumDisplay(QObject * parent)
: QObject(parent),
  m_image(fullWidth(), fullHeight(), QImage::Format_ARGB32)
{
}

QSpectrumDisplay::~QSpectrumDisplay() = default;

void QSpectrumDisplay::redrawDisplay(const uint8_t * displayMemory)
{
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
                    colourIndex = static_cast<std::size_t>(inkColour(attr));
                } else {
                    colourIndex = static_cast<std::size_t>(paperColour(attr));
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

    Q_EMIT displayUpdated(image());
}

void QSpectrumDisplay::setBorder(SpectrumDisplayDevice::Colour colour, bool bright)
{
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
    Q_EMIT displayUpdated(image());
}

constexpr int QSpectrumDisplay::fullWidth()
{
    return Width + BorderSize + BorderSize;
}

constexpr int QSpectrumDisplay::fullHeight()
{
    return Height + BorderSize + BorderSize;
}
