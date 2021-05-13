//
// Created by darren on 06/03/2021.
//

#include <unordered_map>
#include <iomanip>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QToolTip>
#include <QTimer>
#include "memoryview.h"
#include "application.h"

using namespace Spectrum::QtUi;

namespace
{
    constexpr const int Margin = 5;
    constexpr const int FontPointSize = 8;

    class MemoryViewInternal
    : public QWidget
    {
    public:
        MemoryViewInternal(Spectrum::BaseSpectrum::MemoryType * memory, QWidget * parent)
        : QWidget(parent),
          m_memory(memory)
        {
            auto font = this->font();
            font.setPointSize(FontPointSize);
            font.setFixedPitch(true);
            setFont(font);
            auto metrics = fontMetrics();
            m_addressWidth = 20 + metrics.horizontalAdvance(QStringLiteral("0x0000"));
            m_cellWidth = 10 + metrics.horizontalAdvance(QStringLiteral("00"));
            m_cellHeight = metrics.height();
            setMinimumSize(m_addressWidth + (16 * m_cellWidth) + (2 * Margin), ((0xffff / 16) * m_cellHeight) + (2 * Margin));
        }

        [[nodiscard]] int rowHeight() const
        {
            return m_cellHeight;
        }

        void setMemory(Spectrum::BaseSpectrum::MemoryType * memory)
        {
            m_memory = memory;
            update();
        }

        std::optional<Z80::UnsignedWord> addressAt(const QPoint & pos) const
        {
            if (m_addressWidth > pos.x()) {
                return {};
            }

            return (((pos.y() - Margin) / m_cellHeight) * 16) + ((pos.x() - Margin - m_addressWidth) / m_cellWidth);
        }

        void setHighlight(Z80::UnsignedWord address, const QColor & foreground, const QColor & background)
        {
            m_highlights.insert_or_assign(address, ByteHighlight{address, foreground, background});
            auto y = (address / 16) * m_cellHeight;
            update(QRect(0, y, width(), y + m_cellHeight));
        }

        void removeHighlight(Z80::UnsignedWord address)
        {
            auto highlight = m_highlights.find(address);

            if (highlight == m_highlights.cend()) {
                return;
            }

            m_highlights.erase(highlight);
            auto y = (address / 16) * m_cellHeight;
            update(QRect(0, y, width(), y + m_cellHeight));
        }

        void clearHighlights()
        {
            m_highlights.clear();
            update();
        }

    protected:
        bool event(QEvent * event) override
        {
            if (QEvent::Type::ToolTip == event->type()) {
                auto address = addressAt(dynamic_cast<QHelpEvent *>(event)->pos());

                if (!address) {
                    return false;
                }

                auto byteValue = m_memory->readByte(*address);
                auto wordValue = ::Z80::z80ToHostByteOrder(m_memory->readWord<::Z80::UnsignedWord>(*address));

                // {addr}   (0x%1)
                // 8-bit    (%2)
                // Hex      (%3: 0x%4)
                // Dec      (%5: %6)
                // Bin      (%7: %8)
                // Oct      (%9: 0%10)
                // 16-bit   (%11)
                // Hex      (%3: %12)
                // Dec      (%5: %13)
                // Oct      (%9: %14)
                QToolTip::showText(
                    dynamic_cast<QHelpEvent *>(event)->globalPos(),
                    QStringLiteral("<p><strong>0x%1</strong></p><p><strong>%2</strong><br>%3: 0x%4<br>%5: %6<br>%7: %8<br>%9: 0%10</p><p><strong>%11</strong><br>%3: 0x%12<br>%5: %13<br>%9: 0%14</p>")
                    .arg(*address, 4, 16, QChar('0'))       // %1
                    .arg(tr("8-bit"), tr("Hex"))         // %2, %3
                    .arg(byteValue, 2, 16, QChar('0'))      // %4
                    .arg(tr("Dec"))                         // %5
                    .arg(byteValue)                            // %6
                    .arg(tr("Bin"))                         // %7
                    .arg(byteValue, 8, 2, QChar('0'))       // %8
                    .arg(tr("Oct"))                         // %9
                    .arg(byteValue, 0, 8)                      // %10
                    .arg(tr("16-bit"))                      // %11
                    .arg(wordValue, 4, 16, QChar('0'))      // %12
                    .arg(wordValue)                            // %13
                    .arg(wordValue, 0, 8)                      // %14
                );

                return true;
            }

            return QWidget::event(event);
        }

        void paintEvent(QPaintEvent * event) override
        {
            if (!m_memory) {
                return;
            }

            QPainter painter(this);
            int y = Margin;
            int width = this->width();
            const auto endHighlight = m_highlights.cend();
            const auto defaultPen = painter.pen();
            QRect cellRect;

            for (int addr = 0; addr < 0xffff; addr += 16) {
                y += m_cellHeight;

                // only paint the required area
                if (!event->region().intersects(QRect(0, y - m_cellHeight, width, m_cellHeight))) {
                    continue;
                }

                cellRect = QRect(Margin, y - m_cellHeight, m_addressWidth, m_cellHeight);
                painter.drawText(cellRect, Qt::AlignVCenter | Qt::AlignLeft, QStringLiteral("0x%1").arg(addr, 4, 16, FillChar));

                int x = m_addressWidth;

                for (std::uint8_t byte = 0; byte < 16; ++byte) {
                    const auto highlight = m_highlights.find(addr + byte);
                    const bool highlighted = endHighlight != highlight;
                    cellRect = QRect(x, y - m_cellHeight, m_cellWidth, m_cellHeight);

                    if (highlighted) {
                        painter.fillRect(cellRect, QBrush(highlight->second.background));
                        painter.setPen(highlight->second.foreground);
                    }

                    painter.drawText(cellRect, Qt::AlignCenter, QStringLiteral("%1").arg(m_memory->readByte(addr + byte), 2, 16, FillChar));

                    if (highlighted) {
                        painter.setPen(defaultPen);
                    }

                    x += m_cellWidth;
                }
            }

            painter.end();
        }

    private:
        struct ByteHighlight
        {
            int address;
            QColor foreground;
            QColor background;
        };

        static constexpr const QChar FillChar = QChar('0');
        Spectrum::BaseSpectrum::MemoryType * m_memory;
        int m_cellHeight;
        int m_addressWidth;
        int m_cellWidth;
        std::unordered_map<int, ByteHighlight> m_highlights;
    };
}

MemoryView::MemoryView(Spectrum::BaseSpectrum::MemoryType * memory, QWidget * parent)
: QScrollArea(parent),
  m_memory(memory),
  m_lastSearch()
{
    QScrollArea::setWidget(new MemoryViewInternal(memory, this));
}

void MemoryView::scrollToAddress(::Z80::UnsignedWord addr)
{
    auto y = (addr / 16) * dynamic_cast<MemoryViewInternal *>(widget())->rowHeight();
    ensureVisible(0, y);
}

void MemoryView::setHighlight(::Z80::UnsignedWord address, const QColor & foreground, const QColor & background)
{
    dynamic_cast<MemoryViewInternal *>(widget())->setHighlight(address, foreground, background);
}

void MemoryView::removeHighlight(::Z80::UnsignedWord address)
{
    dynamic_cast<MemoryViewInternal *>(widget())->removeHighlight(address);
}

void MemoryView::clearHighlights()
{
    dynamic_cast<MemoryViewInternal *>(widget())->clearHighlights();
}

std::optional<Z80::UnsignedWord> MemoryView::addressAt(const QPoint & pos) const
{
    auto * view = dynamic_cast<MemoryViewInternal *>(widget());
    return view->addressAt(view->mapFromParent(pos));
}

void MemoryView::setMemory(BaseSpectrum::MemoryType * memory)
{
    m_memory = memory;
    auto * view = dynamic_cast<MemoryViewInternal *>(widget());
    view->setMemory(memory);
    verticalScrollBar()->setValue(0);
    update();
}

void MemoryView::find(::Z80::UnsignedWord value, std::optional<::Z80::UnsignedWord> fromAddress)
{
    Util::debug << "Searching for unsigned word\n";
    if constexpr (::Z80::HostByteOrder != ::Z80::Z80ByteOrder) {
        value = Util::swapByteOrder(value);
    }

    find(QByteArray(reinterpret_cast<const char *>(&value), sizeof(::Z80::UnsignedWord)), fromAddress);
}

void MemoryView::find(::Z80::SignedWord value, std::optional<::Z80::UnsignedWord> fromAddress)
{
    Util::debug << "Searching for unsigned word\n";
    if constexpr (::Z80::HostByteOrder != ::Z80::Z80ByteOrder) {
        value = Util::swapByteOrder(value);
    }

    find(QByteArray(reinterpret_cast<const char *>(&value), sizeof(::Z80::UnsignedWord)), fromAddress);
}

void MemoryView::find(const QByteArray & value, std::optional<::Z80::UnsignedWord> fromAddress)
{
    if (!fromAddress) {
        if (m_lastSearch && m_lastSearch->foundAt) {
            fromAddress = *(m_lastSearch->foundAt) + 1;
        } else {
            fromAddress = 0;
        }
    }

    const auto endAddress = m_memory->addressableSize() - value.size();
    std::vector<::Z80::UnsignedByte> searchBuffer(value.size());

    for (::Z80::UnsignedWord address = *fromAddress; address < endAddress; ++address) {
        m_memory->readBytes(address, searchBuffer.size(), searchBuffer.data());

        if (0 == std::memcmp(value.data(), searchBuffer.data(), searchBuffer.size())) {
            if (m_lastSearch && m_lastSearch->foundAt) {
                removeHighlight(*(m_lastSearch->foundAt));
            }

            m_lastSearch = {.query = value, .foundAt = address,};
            scrollToAddress(address);
            setHighlight(address, qRgb(0x44, 0x44, 0x44), qRgb(0x88, 0xcc, 0x88));
            QTimer::singleShot(5000, [this, address]() {
                removeHighlight(address);
            });
            return;
        }
    }

    m_lastSearch = {.query = value, .foundAt = {},};
    Application::showNotification(tr("Search value not found."));
}

MemoryView::~MemoryView() = default;
