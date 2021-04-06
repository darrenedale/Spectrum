//
// Created by darren on 18/03/2021.
//

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <QHelpEvent>
#include <QToolTip>
#include <QScrollBar>
#include <QPainter>

#include "disassemblywidget.h"

using namespace Spectrum::QtUi;

using Disassembler = ::Z80::Assembly::Disassembler;
using UnsignedWord = ::Z80::UnsignedWord;

namespace
{
    constexpr const int Margin = 5;
    constexpr const int MinimumLines = 20;
    constexpr const int FontPointSize = 8;

    /**
     * Internal view widget that sits behind the DisassemblyWidget's scroll area.
     *
     * NOTE while we could assume that this is a private helper widget for DisassemblyWidget and therefore make
     * everything public so that DisassemblyWidget can read and write it as it sees fit, it is exposed to the outside
     * world by the QScrollView::widget() method, so we need to make DisassemblyWidget a friend and keep to our usual
     * visiblity rules.
     */
    class DisassemblyView
    : public QWidget
    {
        friend class Spectrum::QtUi::DisassemblyWidget;

    public:
        DisassemblyView(const Disassembler & disassembler, QWidget * parent)
        : QWidget(parent),
          m_disassembler(disassembler)
        {
            auto font = this->font();
            font.setPointSize(FontPointSize);
            font.setFixedPitch(true);
            setFont(font);
            auto metrics = fontMetrics();
            m_addressWidth = metrics.horizontalAdvance(QStringLiteral("0x0000"));
            m_mnemonicWidth = metrics.horizontalAdvance(QStringLiteral("WWWW 0x0000,0x0000"));
            m_gutter = 2 * metrics.horizontalAdvance(QStringLiteral("W"));
            m_lineHeight = metrics.height();
            setMinimumSize((2 * Margin) + m_addressWidth + m_gutter + m_mnemonicWidth, (2 * Margin) + (MinimumLines * m_lineHeight));
            updateMnemonics();
        }

        /**
         * The height, in pixels, of a line of disassembly.
         *
         * @return
         */
        [[nodiscard]] int lineHeight() const
        {
            return m_lineHeight;
        }

        /**
         * The y-coordinate of the top of the line of disassembly for a given address.
         *
         * Note that this method works for all addresses in the provided memory space, including those that are within
         * an instruction's machine code byte sequence. That is, if there is a 3-byte instruction at 0x0010, requesting
         * the y-coordinate for address 0x0011 or 0x0012 will return the y-coordinate for the instruction whose machine
         * code starts at 0x0010.
         *
         * @param address
         * @return
         */
        std::optional<int> addressYCoordinate(UnsignedWord address)
        {
            int line = 0;

            while (line < m_mnemonics.size() && m_mnemonics[line].address < address) {
                ++line;
            }

            if (line >= m_mnemonics.size()) {
                std::cerr << "address 0x" << std::hex << std::setfill('0') << std::setw(4) << address << " not found in disassembly\n" << std::dec << std::setfill(' ');
                return {};
            }

            if (m_mnemonics[line].address > address) {
                --line;
            }

            return line * m_lineHeight;
        }

        /**
         * Updates the stored mnemonics from a given address onward.
         *
         * This is usually called to ensure that the disassembly from the current PC is correct. When jumps occur, they
         * could jump to a location that was previously incorrectly disassembled because memory content that is actually
         * just data was disassembled as if it were machine code. When this happens, there is no guarantee that the
         * disassembly will encounter the first byte of after the data as an opcode - for example, the last byte of data
         * might be disassembled as a multi-byte instruction - in which case the disassembly will be incorrect. We can't
         * tell beforehand what a program considers data and what it considers code, so we have to redo the disassembly
         * when the PC changes.
         *
         * @param fromAddress
         */
        void updateMnemonics(UnsignedWord fromAddress = 0x0000)
        {
            // fetch fresh disassembly of machine code from given address
            auto mnemonics = m_disassembler.disassembleFrom(fromAddress);

            {
                // discard any mnemonics previously disassembled from that address onward
                auto mnemonic = std::find_if(m_mnemonics.begin(), m_mnemonics.end(),
                                             [fromAddress](const AddressedMnemonic & mnemonic) {
                                                 return mnemonic.address >= fromAddress;
                                             });

                if (mnemonic != m_mnemonics.end()) {
                    m_mnemonics.erase(mnemonic, m_mnemonics.end());
                }
            }

            // store the freshly disassembled mnemonics in our collection
            m_mnemonics.reserve(m_mnemonics.size() + mnemonics.size());

            for (auto & mnemonic : mnemonics) {
                m_mnemonics.push_back(AddressedMnemonic {
                    mnemonic.instruction,
                    std::move(mnemonic.operands),
                    mnemonic.size,
                    fromAddress,
                });

                fromAddress += mnemonic.size;
            }

            // work out the new height of the view
            setFixedHeight((m_mnemonics.size() * m_lineHeight) + (2 * Margin));
        }

    protected:
        /**
         * Qt event handler.
         *
         * TODO show tooltip
         *
         * @param event
         * @return
         */
        bool event(QEvent * event) override
        {
            if (QEvent::Type::ToolTip == event->type()) {

            }

            return QWidget::event(event);
        }

        /**
         * Paint the widget.
         *
         * @param event
         */
        void paintEvent(QPaintEvent * event) override
        {
            QPainter painter(this);
            auto y = Margin;
            const auto width = this->width();
            const auto defaultPen = painter.pen();
            const auto gutter = 2 * fontMetrics().horizontalAdvance(QLatin1Char('W'));
            auto textRect = QRect(Margin, y, width, m_lineHeight);

            for (const auto & mnemonic : m_mnemonics) {
                y += m_lineHeight;

                // only paint the required area
                if (!event->region().intersects(QRect(0, y - m_lineHeight, width, m_lineHeight))) {
                    continue;
                }

                textRect.moveTop(y - m_lineHeight);

                if (showPcIndicator && mnemonic.address == pc) {
                    textRect.moveLeft(Margin + m_addressWidth);
                    textRect.setWidth(m_gutter);
                    painter.setPen(qRgb(0x80, 0xe0, 0x80));
                    painter.drawText(textRect, Qt::AlignCenter, QStringLiteral(">"));
                }

                textRect.moveLeft(Margin);
                textRect.setWidth(m_addressWidth);
                painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, QStringLiteral("0x%1").arg(mnemonic.address, 4, 16, FillChar));

                textRect.moveLeft(Margin + m_addressWidth + gutter);
                textRect.setWidth(m_mnemonicWidth);
                painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, QString::fromStdString(std::to_string(mnemonic)));

                if (showPcIndicator && mnemonic.address == pc) {
                    painter.setPen(defaultPen);
                }
            }

            painter.end();
        }

    private:
        /**
         * Extend the Mnemonic structure to include the address at which the instruction is located.
         *
         * We require the address for display and for querying the location of an address in the view.
         */
        struct AddressedMnemonic
        : public ::Z80::Assembly::Mnemonic
        {
            UnsignedWord address = 0x0000;
        };

        using Mnemonics = std::vector<AddressedMnemonic>;
        static constexpr const QChar FillChar = QChar('0');

        // NOTE these are read and written directly by DisassemblyWidget
        bool showPcIndicator = true;
        UnsignedWord pc = 0x0000;

        // NOTE these should be considered implementation details even by DisassemblyWidget
        Disassembler m_disassembler;
        Mnemonics m_mnemonics;
        int m_addressWidth;
        int m_gutter;
        int m_mnemonicWidth;
        int m_lineHeight;
    };
}

DisassemblyWidget::DisassemblyWidget(BaseSpectrum::MemoryType * memory, QWidget * parent)
: QScrollArea(parent)
{
    QScrollArea::setWidget(new DisassemblyView(Disassembler(memory), this));
}

DisassemblyWidget::~DisassemblyWidget() = default;

void DisassemblyWidget::updateMnemonics(UnsignedWord fromAddress)
{
    dynamic_cast<DisassemblyView *>(widget())->updateMnemonics(fromAddress);
}

void DisassemblyWidget::scrollToAddress(UnsignedWord address)
{
    auto y = dynamic_cast<DisassemblyView *>(widget())->addressYCoordinate(address);

    if (!y) {
        return;
    }

    auto * scrollBar = verticalScrollBar();

    if (scrollBar->value() > *y || scrollBar->value() + height() < *y) {
        scrollBar->setValue(*y);
    }
}

void DisassemblyWidget::setPc(UnsignedWord pc)
{
    auto * view = dynamic_cast<DisassemblyView *>(widget());
    view->pc = pc;
    view->update();
}

UnsignedWord DisassemblyWidget::pc() const
{
    return dynamic_cast<DisassemblyView *>(widget())->pc;
}

bool DisassemblyWidget::pcIndicatorEnabled() const
{
    return dynamic_cast<DisassemblyView *>(widget())->showPcIndicator;
}

void DisassemblyWidget::enablePcIndicator(bool enabled)
{
    auto * view = dynamic_cast<DisassemblyView *>(widget());

    if (enabled != view->showPcIndicator) {
        view->showPcIndicator = enabled;
        view->update();
    }
}

void DisassemblyWidget::setMemory(BaseSpectrum::MemoryType * memory)
{
    QScrollArea::setWidget(new DisassemblyView(Disassembler(memory), this));
}
