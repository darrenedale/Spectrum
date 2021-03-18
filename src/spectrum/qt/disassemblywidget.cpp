//
// Created by darren on 18/03/2021.
//

#include <iostream>
#include <QHelpEvent>
#include <QToolTip>
#include <QPainter>

#include "disassemblywidget.h"

using namespace Spectrum::Qt;

using Disassembler = ::Z80::Assembly::Disassembler;
using UnsignedWord = ::Z80::UnsignedWord;

namespace
{
    constexpr const int Margin = 5;
    constexpr const int MinimumLines = 20;
    constexpr const int FontPointSize = 8;

    class DisassemblyView
    : public QWidget
    {
    public:
        DisassemblyView(Disassembler & disassembler, QWidget * parent)
        : QWidget(parent),
          m_disassembler(disassembler),
          m_firstOpcodeAddress(0x0000),
          m_instructionCount(-1)
        {
            auto font = this->font();
            font.setPointSize(FontPointSize);
            font.setFixedPitch(true);
            setFont(font);
            auto metrics = fontMetrics();
            m_addressWidth = 20 + metrics.horizontalAdvance(QStringLiteral("0x0000"));
            m_cellHeight = metrics.height();
            setMinimumSize(3 * m_addressWidth + (2 * Margin), (MinimumLines * m_cellHeight) + (2 * Margin));
            std::cout << "disassembly view minimum size: " << minimumSize().width() << " x " << minimumSize().height() << '\n';
            updateMnemonics();
        }

        [[nodiscard]] int rowHeight() const
        {
            return m_cellHeight;
        }

        void setFirstAddress(UnsignedWord address)
        {
            m_firstOpcodeAddress = address;
            updateMnemonics();
            update();
        }

        UnsignedWord firstAddress() const
        {
            return m_firstOpcodeAddress;
        }

        void setInstructionCount(int count)
        {
            m_instructionCount = count;
            updateMnemonics();
            update();
        }

        [[nodiscard]] inline int instructionCount() const
        {
            return m_instructionCount;
        }

    protected:
        void updateMnemonics()
        {
            m_mnemonics = m_disassembler.disassembleFrom(firstAddress(), instructionCount());
            setFixedHeight((m_mnemonics.size() * m_cellHeight) + (2 * Margin));
            std::cout << m_mnemonics.size() << " mnemonics to display\n";
        }

        bool event(QEvent * event) override
        {
            if (QEvent::Type::ToolTip == event->type()) {

            }

            return QWidget::event(event);
        }

        void paintEvent(QPaintEvent * event) override
        {
            QPainter painter(this);
            auto y = Margin;
            const auto width = this->width();
//            const auto defaultPen = painter.pen();
            auto address = firstAddress();
            QRect lineRect;

            for (const auto & mnemonic : m_mnemonics) {
                y += m_cellHeight;

                // only paint the required area
                if (!event->region().intersects(QRect(0, y - m_cellHeight, width, m_cellHeight))) {
                    address += mnemonic.size;
                    continue;
                }

                lineRect = QRect(Margin, y - m_cellHeight, width - (2 * Margin), m_cellHeight);
                painter.drawText(lineRect, Qt::AlignVCenter | Qt::AlignLeft, QStringLiteral("0x%1").arg(address, 4, 16, FillChar));
                lineRect.setLeft(Margin + m_addressWidth);
                painter.drawText(lineRect, Qt::AlignVCenter | Qt::AlignLeft, QString::fromStdString(std::to_string(mnemonic)));
                address += mnemonic.size;
            }

            painter.end();
        }

    private:
        using Mnemonics = std::vector<::Z80::Assembly::Mnemonic>;

        static constexpr const QChar FillChar = QChar('0');
        Disassembler & m_disassembler;
        UnsignedWord m_firstOpcodeAddress;
        int m_instructionCount;
        Mnemonics m_mnemonics;
        int m_addressWidth;
        int m_cellHeight;
    };
}

DisassemblyWidget::DisassemblyWidget(Z80::UnsignedByte * memory, QWidget * parent)
: QScrollArea(parent),
  m_disassembler(memory, 0xffff)
{
    QScrollArea::setWidget(new DisassemblyView(m_disassembler, this));
}

void DisassemblyWidget::setFirstAddress(Z80::UnsignedWord address)
{
    dynamic_cast<DisassemblyView *>(widget())->setFirstAddress(address);
}

DisassemblyWidget::~DisassemblyWidget() = default;
