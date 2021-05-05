//
// Created by darren on 05/03/2021.
//

#include <QHBoxLayout>
#include "flagswidget.h"
#include "../../../util/debug.h"

using namespace Spectrum::QtUi::Debugger;

namespace
{
    // bit indices for the individual flags
    constexpr const int CFlagBit = 0;
    constexpr const int NFlagBit = 1;
    constexpr const int PVFlagBit = 2;
    constexpr const int Undoc3FlagBit = 3;
    constexpr const int HFlagBit = 4;
    constexpr const int Undoc5FlagBit = 5;
    constexpr const int ZFlagBit = 6;
    constexpr const int SFlagBit = 7;
}

FlagsWidget::FlagsWidget(QWidget * parent)
: FlagsWidget(0x00, parent)
{}

FlagsWidget::FlagsWidget(Z80::UnsignedByte flags, QWidget * parent)
: QWidget(parent),
  m_flagS(),
  m_flagZ(),
  m_flag5(),
  m_flagH(),
  m_flag3(),
  m_flagPV(),
  m_flagN(),
  m_flagC(),
  m_flagData{{
        {&m_flagC, QStringLiteral("C"), &FlagsWidget::flagCChanged, &FlagsWidget::flagCSet, &FlagsWidget::flagCCleared},      // bit 0 is the carry flag
        {&m_flagN, QStringLiteral("N"), &FlagsWidget::flagNChanged, &FlagsWidget::flagNSet, &FlagsWidget::flagNCleared},      // bit 1 is the negation flag
        {&m_flagPV, QStringLiteral("PV"), &FlagsWidget::flagPVChanged, &FlagsWidget::flagPVSet, &FlagsWidget::flagPVCleared}, // bit 2 is the parity/overflow flag
        {&m_flag3, QStringLiteral("3"), &FlagsWidget::flag3Changed, &FlagsWidget::flag3Set, &FlagsWidget::flag3Cleared},      // bit 3 is the undocumented 3 flag
        {&m_flagH, QStringLiteral("H"), &FlagsWidget::flagHChanged, &FlagsWidget::flagHSet, &FlagsWidget::flagHCleared},      // bit 4 is the half-carry flag
        {&m_flag5, QStringLiteral("5"), &FlagsWidget::flag5Changed, &FlagsWidget::flag5Set, &FlagsWidget::flag5Cleared},      // bit 5 is the undocumented 5 flag
        {&m_flagZ, QStringLiteral("Z"), &FlagsWidget::flagZChanged, &FlagsWidget::flagZSet, &FlagsWidget::flagZCleared},      // bit 6 is the zero flag
        {&m_flagS, QStringLiteral("S"), &FlagsWidget::flagSChanged, &FlagsWidget::flagSSet, &FlagsWidget::flagSCleared},      // bit 7 is sign flag
    }}
{
    for (int flagBit = 0; flagBit < 8; ++flagBit) {
        auto * button = m_flagData[flagBit].button;
        button->setText(m_flagData[flagBit].label);
        button->setCheckable(true);
        connect(button, &QToolButton::toggled, [this, flagBit](bool set) {
            emitFlagChangeSignals(flagBit, set);
        });
    }

    createLayout();
    setAllFlags(flags);
}

FlagsWidget::~FlagsWidget() = default;

void FlagsWidget::createLayout()
{
    auto * layout = new QHBoxLayout();

    // NOTE the widgets are in the reverse of the display order in m_flagData so we don't use that
    for (auto * button : {&m_flagS, &m_flagZ, &m_flag5, &m_flagH, &m_flag3, &m_flagPV, &m_flagN, &m_flagC,}) {
        layout->addWidget(button);
    }

    setLayout(layout);
}

void FlagsWidget::setAllFlags(Z80::UnsignedByte flags)
{
    Z80::UnsignedByte mask = 0x80;
    int flagIdx = 7;

    while (0 <= flagIdx) {
        auto * const button = m_flagData[flagIdx].button;
        // stash before and after state for the flag so that we know what signals to emit
        auto before = button->isChecked();
        auto after = static_cast<bool>(flags & mask);

        // only update and signal if the state has changed
        if (before != after) {
            button->setChecked(after);
            emitFlagChangeSignals(flagIdx, after);
        }

        --flagIdx;
        mask >>= 1;
    }

    Q_EMIT flagsChanged(flags);
}

Z80::UnsignedByte FlagsWidget::allFlags() const
{
    Z80::UnsignedByte mask = 0x01;
    Z80::UnsignedByte flags = 0x00;

    for (const auto & flagData : m_flagData) {
        if (flagData.button->isChecked()) {
            flags |= mask;
        }
        
        mask <<= 1;
    }
    
    return flags;
}

template <int flagBit>
void FlagsWidget::setFlag(bool set)
{
    // implementation template for setFlag() for any given flag
    static_assert(0 <= flagBit && 7 >= flagBit, "Invalid flag bit in instantiation of template FlagsWidget::setFlag<int>()");
    auto * button = m_flagData[flagBit].button;

    // short-circuit if there's no change
    if (set == button->isChecked()) {
        return;
    }

    button->setChecked(set);
    Q_EMIT flagsChanged(allFlags());
    emitFlagChangeSignals(flagBit, set);
}

void FlagsWidget::emitFlagChangeSignals(int flagBit, bool set)
{
    // emit the changed signal for this flag
    Q_EMIT (this->*(m_flagData[flagBit].changedSignal))(set);

    // emit the set or cleared signal for this flag, based on the new state
    if (set) {
        Q_EMIT (this->*(m_flagData[flagBit].setSignal))();
    } else {
        Q_EMIT (this->*(m_flagData[flagBit].clearedSignal))();
    }

}

void FlagsWidget::setFlagS(bool set)
{
    // delegate to tepmlate with appropriate flag bit index
    setFlag<SFlagBit>(set);
}

void FlagsWidget::setFlagZ(bool set)
{
    // delegate to tepmlate with appropriate flag bit index
    setFlag<ZFlagBit>(set);
}

void FlagsWidget::setFlag5(bool set)
{
    // delegate to tepmlate with appropriate flag bit index
    setFlag<Undoc5FlagBit>(set);
}

void FlagsWidget::setFlagH(bool set)
{
    // delegate to tepmlate with appropriate flag bit index
    setFlag<HFlagBit>(set);
}

void FlagsWidget::setFlag3(bool set)
{
    // delegate to tepmlate with appropriate flag bit index
    setFlag<Undoc3FlagBit>(set);
}

void FlagsWidget::setFlagPV(bool set)
{
    // delegate to tepmlate with appropriate flag bit index
    setFlag<PVFlagBit>(set);
}

void FlagsWidget::setFlagN(bool set)
{
    // delegate to tepmlate with appropriate flag bit index
    setFlag<NFlagBit>(set);
}

void FlagsWidget::setFlagC(bool set)
{
    // delegate to tepmlate with appropriate flag bit index
    setFlag<CFlagBit>(set);
}
