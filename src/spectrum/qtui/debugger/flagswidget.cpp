//
// Created by darren on 05/03/2021.
//

#include <QHBoxLayout>

#include "flagswidget.h"

using namespace Spectrum::QtUi::Debugger;

FlagsWidget::FlagsWidget(QWidget * parent)
: FlagsWidget(0x00, parent)
{}

FlagsWidget::FlagsWidget(Z80::UnsignedByte flags, QWidget * parent)
: QWidget(parent)
{
    m_flagS.setText(QStringLiteral("S"));
    m_flagZ.setText(QStringLiteral("Z"));
    m_flag5.setText(QStringLiteral("5"));
    m_flagH.setText(QStringLiteral("H"));
    m_flag3.setText(QStringLiteral("3"));
    m_flagPV.setText(QStringLiteral("PV"));
    m_flagN.setText(QStringLiteral("N"));
    m_flagC.setText(QStringLiteral("C"));
    for (auto * button : {&m_flagS, &m_flagZ, &m_flag5, &m_flagH, &m_flag3, &m_flagPV, &m_flagN, &m_flagC,}) {
        button->setCheckable(true);
    }

    createLayout();
    setAllFlags(flags);
}

FlagsWidget::~FlagsWidget() = default;

void FlagsWidget::createLayout()
{
    auto * layout = new QHBoxLayout();

    for (auto * button : {&m_flagS, &m_flagZ, &m_flag5, &m_flagH, &m_flag3, &m_flagPV, &m_flagN, &m_flagC,}) {
        layout->addWidget(button);
    }

    setLayout(layout);
}

void FlagsWidget::setAllFlags(Z80::UnsignedByte flags)
{
    Z80::UnsignedByte mask = 0x80;
    
    for (auto * button : {&m_flagS, &m_flagZ, &m_flag5, &m_flagH, &m_flag3, &m_flagPV, &m_flagN, &m_flagC,}) {
        button->setChecked(flags & mask);
        mask >>= 1;
    }

    // TODO emit changed/set/cleared for individual flags
    Q_EMIT flagsChanged(flags);
}

Z80::UnsignedByte FlagsWidget::allFlags() const
{
    Z80::UnsignedByte mask = 0x80;
    Z80::UnsignedByte flags = 0x00;

    for (auto * button : {&m_flagS, &m_flagZ, &m_flag5, &m_flagH, &m_flag3, &m_flagPV, &m_flagN, &m_flagC,}) {
        if (button->isChecked()) {
            flags |= mask;
        }
        
        mask >>= 1;
    }
    
    return flags;
}

void FlagsWidget::setFlagS(bool set)
{
    if (set == m_flagS.isChecked()) {
        return;
    }
    
    m_flagS.setChecked(set);

    Q_EMIT flagsChanged(allFlags());
    Q_EMIT flagSChanged(set);

    if (set) {
        Q_EMIT flagSSet();
    } else {
        Q_EMIT flagSCleared();
    }
}

void FlagsWidget::setFlagZ(bool set)
{
    if (set == m_flagZ.isChecked()) {
        return;
    }
    
    m_flagZ.setChecked(set);

    Q_EMIT flagsChanged(allFlags());
    Q_EMIT flagZChanged(set);

    if (set) {
        Q_EMIT flagZSet();
    } else {
        Q_EMIT flagZCleared();
    }
}

void FlagsWidget::setFlag5(bool set)
{
    if (set == m_flag5.isChecked()) {
        return;
    }
    
    m_flag5.setChecked(set);

    Q_EMIT flagsChanged(allFlags());
    Q_EMIT flag5Changed(set);

    if (set) {
        Q_EMIT flag5Set();
    } else {
        Q_EMIT flag5Cleared();
    }
}

void FlagsWidget::setFlagH(bool set)
{
    if (set == m_flagH.isChecked()) {
        return;
    }

    m_flagH.setChecked(set);

    Q_EMIT flagsChanged(allFlags());
    Q_EMIT flagHChanged(set);

    if (set) {
        Q_EMIT flagHSet();
    } else {
        Q_EMIT flagHCleared();
    }
}

void FlagsWidget::setFlag3(bool set)
{
    if (set == m_flag3.isChecked()) {
        return;
    }

    m_flag3.setChecked(set);

    Q_EMIT flagsChanged(allFlags());
    Q_EMIT flag3Changed(set);

    if (set) {
        Q_EMIT flag3Set();
    } else {
        Q_EMIT flag3Cleared();
    }
}

void FlagsWidget::setFlagPV(bool set)
{
    if (set == m_flagPV.isChecked()) {
        return;
    }

    m_flagPV.setChecked(set);

    Q_EMIT flagsChanged(allFlags());
    Q_EMIT flagPVChanged(set);

    if (set) {
        Q_EMIT flagPVSet();
    } else {
        Q_EMIT flagPVCleared();
    }
}

void FlagsWidget::setFlagN(bool set)
{
    if (set == m_flagN.isChecked()) {
        return;
    }

    m_flagN.setChecked(set);

    Q_EMIT flagsChanged(allFlags());
    Q_EMIT flagNChanged(set);

    if (set) {
        Q_EMIT flagNSet();
    } else {
        Q_EMIT flagNCleared();
    }
}

void FlagsWidget::setFlagC(bool set)
{
    if (set == m_flagC.isChecked()) {
        return;
    }

    m_flagC.setChecked(set);

    Q_EMIT flagsChanged(allFlags());
    Q_EMIT flagCChanged(set);

    if (set) {
        Q_EMIT flagCSet();
    } else {
        Q_EMIT flagCCleared();
    }
}
