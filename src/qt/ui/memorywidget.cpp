//
// Created by darren on 06/03/2021.
//

#include <QPainter>
#include <QPaintEvent>
#include <QFont>

#include "memorywidget.h"

using namespace Spectrum;

namespace
{
    constexpr const int Margin = 5;
    constexpr const int FontPointSize = 8;

    class MemoryView
    : public QWidget
    {
    public:
        MemoryView(Z80::UnsignedByte * memory, QWidget * parent)
        : QWidget(parent),
          m_memory(memory)
        {
            auto font = this->font();
            font.setPointSize(FontPointSize);
            setFont(font);
            auto metrics = fontMetrics();
            m_addressWidth = metrics.horizontalAdvance(QStringLiteral("0x0000     "));
            m_cellWidth = metrics.horizontalAdvance(QStringLiteral("00  "));
            m_cellHeight = metrics.height();
            setMinimumSize(m_addressWidth + (16 * m_cellWidth) + (2 * Margin), ((0xffff / 16) * m_cellHeight) + (2 * Margin));
        }

        [[nodiscard]] int rowHeight() const
        {
            return m_cellHeight;
        }

    protected:
        void paintEvent(QPaintEvent * event) override
        {
            if (!m_memory) {
                return;
            }

            QPainter painter(this);
            int y = Margin;
            int width = this->width();

            for (int addr = 0; addr < 0xffff; addr += 16) {
                y += m_cellHeight;

                // only paint the required area
                if (!event->region().intersects(QRect(0, y - m_cellHeight, width, m_cellHeight))) {
                    continue;
                }

                painter.drawText(Margin, y, QStringLiteral("0x%1").arg(addr, 4, 16, FillChar));

                int x = m_addressWidth;
                for (std::uint8_t byte = 0; byte < 16; ++byte) {
                    painter.drawText(x, y, QStringLiteral("%1").arg(*(m_memory + addr + byte), 2, 16, FillChar));
                    x += m_cellWidth;
                }
            }

            painter.end();
        }

    private:
        static constexpr const QChar FillChar = QChar('0');
        Z80::UnsignedByte * m_memory;
        int m_cellHeight;
        int m_addressWidth;
        int m_cellWidth;
    };
}

MemoryWidget::MemoryWidget(Z80::UnsignedByte * memory, QWidget * parent)
: QScrollArea(parent),
  m_memory(memory)
{
    QScrollArea::setWidget(new MemoryView(memory, this));
}

void MemoryWidget::scrollToAddress(Z80::UnsignedWord addr)
{
    auto y = (addr / 16) * dynamic_cast<MemoryView *>(widget())->rowHeight();
    ensureVisible(0, y);
}

MemoryWidget::~MemoryWidget() = default;
