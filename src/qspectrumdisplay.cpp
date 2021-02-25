#include "qspectrumdisplay.h"
#include <QImage>
#include <QRgb>
#include <QPainter>

#define QSPECTRUMDISPLAY_USEPAINTER

using namespace Spectrum;

#define SPECTRUM_ATTR_INK_MASK 0x07
#define SPECTRUM_ATTR_PAPER_MASK 0x38
#define SPECTRUM_ATTR_BRIGHT_MASK 0x40
#define SPECTRUM_ATTR_FLASH_MASK 0x80

/* get the index into the colour map for an attribute's ink colour. this
 * accommodates the BRIGHT flag */
#define SPECTRUM_ATTR_INK(attr) ((attr & SPECTRUM_ATTR_INK_MASK) | ((SPECTRUM_ATTR_BRIGHT(attr)) ? 0x04 : 0x00))

/* get the index into the colour map for an attribute's paper colour. this
 * accommodates the BRIGHT flag */
#define SPECTRUM_ATTR_PAPER(attr) (((attr & SPECTRUM_ATTR_PAPER_MASK) >> 3)  | ((SPECTRUM_ATTR_BRIGHT(attr)) ? 0x04 : 0x00))

/* true if the bright flag is set, false otherwise */
#define SPECTRUM_ATTR_BRIGHT(attr) (0 != (attr & SPECTRUM_ATTR_BRIGHT_MASK))

/* true if the flash flag is set, false otherwise */
#define SPECTRUM_ATTR_FLASH(attr) (0 != (attr & SPECTRUM_ATTR_FLASH_MASK))

namespace
{
    QRgb colourMap[16] = {
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

QSpectrumDisplay::QSpectrumDisplay(QWidget * parent)
: ImageWidget(parent)
{
    setImage(QImage(256, 192, QImage::Format_ARGB32));
    setMinimumSize(256, 192);
}

QSpectrumDisplay::~QSpectrumDisplay() = default;

void QSpectrumDisplay::redrawDisplay(const uint8_t * displayMemory)
{
#if defined(QSPECTRUMDISPLAY_USEPAINTER)
    QPainter painter(&image());
#else
    auto * data = static_cast<QRgb *>(image->bits());
#endif

    // 32 bytes per scanline, 192 scanlines
    for (int y = 0; y < 192; y += 8) {
        for (int yOffset = 0; yOffset < 8; ++yOffset) {
            for (int b = 0; b < 32; ++b) {
                unsigned char mask = 0x80;

                for (int i = 0; i < 8; ++i) {
                    // just black and white for now
#if defined(QSPECTRUMDISPLAY_USEPAINTER)
                    if (displayMemory[(y + (yOffset * 8)) * 256 + b] & mask) {
                        painter.setPen(colourMap[0]);
                    } else {
                        painter.setPen(colourMap[7]);
                    }

                    painter.drawPoint(b * 8 + i, y + yOffset);
#else
                    if(displayMemory[(y + (yOffset * 8)) * 256 + b] & mask) {
                        data[(y + yOffset) * 256 + b * 8 + i] = s_colourMap[0];
                    } else {
                        data[(y + yOffset) * 256 + b * 8 + i] = s_colourMap[7];
                    }
#endif
                    mask >>= 1;
                }
            }
        }
    }

#if defined(QSPECTRUMDISPLAY_USEPAINTER)
    painter.end();
#endif
    update();
}

int QSpectrumDisplay::heightForWidth(int w) const
{
    return (w * 192) / 256;
}
