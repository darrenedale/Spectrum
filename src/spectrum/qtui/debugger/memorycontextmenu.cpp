//
// Created by darren on 04/05/2021.
//

#include "memorycontextmenu.h"

using namespace Spectrum::QtUi::Debugger;

MemoryContextMenu::MemoryContextMenu(::Z80::UnsignedWord address, QWidget * parent)
: QMenu(parent),
  m_address(address),
  m_sectionTitle(nullptr)
{
    m_sectionTitle = addSection(QStringLiteral("0x%1").arg(address, 4, 16, QLatin1Char('0')));

    addAction(tr("Poke..."), this, &MemoryContextMenu::onPokeTriggered);

    auto * action = addAction(tr("Break on PC"), this, &MemoryContextMenu::onBreakAtProgramCounterTriggered);
    action->setToolTip(tr("Break when the PC reaches 0x%1.").arg(address, 4, 16, QLatin1Char('0')));

    action = addAction(tr("Break on change (word)"), this, &MemoryContextMenu::onBreakOnChangeTriggered<::Z80::UnsignedWord>);
    action->setToolTip(tr("Break when the 16-bit value at 0x%1 changes.").arg(address, 4, 16, QLatin1Char('0')));

    action = addAction(tr("Break on change (byte)"), this, &MemoryContextMenu::onBreakOnChangeTriggered<::Z80::UnsignedByte>);
    action->setToolTip(tr("Break when the 8-bit value at 0x%1 changes.").arg(address, 4, 16, QLatin1Char('0')));

    addSeparator();

    action = addAction(tr("Watch (word)"), this, &MemoryContextMenu::onWatchIntegerTriggered<::Z80::UnsignedWord>);
    action->setToolTip(tr("Watch the 16-bit value in memory at 0x%1.").arg(address, 4, 16, QLatin1Char('0')));

    action = addAction(tr("Watch (byte)"), this, &MemoryContextMenu::onWatchIntegerTriggered<::Z80::UnsignedByte>);
    action->setToolTip(tr("Watch the 8-bit value in memory at 0x%1.").arg(address, 4, 16, QLatin1Char('0')));

    action = addAction(tr("Watch (string)"), this, &MemoryContextMenu::onWatchStringTriggered);
    action->setToolTip(tr("Watch the string value in memory at 0x%1.").arg(address, 4, 16, QLatin1Char('0')));
}

void MemoryContextMenu::onPokeTriggered()
{
    Q_EMIT poke(m_address);
}

void MemoryContextMenu::onBreakAtProgramCounterTriggered()
{
    Q_EMIT breakAtProgramCounter(m_address);
}

void MemoryContextMenu::onWatchStringTriggered()
{
    Q_EMIT watchString(m_address);
}

template<class int_t>
void MemoryContextMenu::onBreakOnChangeTriggered()
{
    if constexpr (1 == sizeof(int_t)) {
        Q_EMIT breakOnByteChange(m_address);
    } else if constexpr (2 == sizeof(int_t)) {
        Q_EMIT breakOnWordChange(m_address);
    } else {
        // can only reach here if a call is made with an int type template arg for which no signal exists
        assert(false);
    }
}

template<class int_t>
void MemoryContextMenu::onWatchIntegerTriggered()
{
    if constexpr (1 == sizeof(int_t)) {
        Q_EMIT watchByte(m_address);
    } else if constexpr (2 == sizeof(int_t)) {
        Q_EMIT watchWord(m_address);
    } else {
        // can only reach here if a call is made with an int type template arg for which no signal exists
        assert(false);
    }
}

void MemoryContextMenu::setAddress(::Z80::UnsignedWord address)
{
    m_address = address;
    m_sectionTitle->setText(QStringLiteral("0x%1").arg(address, 4, 16, QLatin1Char('0')));
}
