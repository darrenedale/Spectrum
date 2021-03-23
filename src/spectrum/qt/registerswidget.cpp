//
// Created by darren on 23/03/2021.
//

#include <iostream>
#include <iomanip>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "registerswidget.h"

using namespace Spectrum::Qt;

using ::Z80::UnsignedWord;
using ::Z80::Register16;

RegistersWidget::RegistersWidget(QWidget * parent)
: QWidget(parent),
  m_af(QStringLiteral("AF")),
  m_bc(QStringLiteral("BC")),
  m_de(QStringLiteral("DE")),
  m_hl(QStringLiteral("HL")),
  m_ix(QStringLiteral("IX")),
  m_iy(QStringLiteral("IY")),
  m_flags()
{
    auto * widgetLayout = new QVBoxLayout();

    widgetLayout->setSpacing(2);
    widgetLayout->addWidget(&m_af);
    widgetLayout->addWidget(&m_bc);
    widgetLayout->addWidget(&m_de);
    widgetLayout->addWidget(&m_hl);
    widgetLayout->addWidget(&m_ix);
    widgetLayout->addWidget(&m_iy);

    auto * flagsLayout = new QHBoxLayout();
    auto * flagsLabel = new QLabel("Flags");
    flagsLabel->setBuddy(&m_flags);
    flagsLabel->setFont(m_flags.font());
    flagsLayout->addWidget(flagsLabel);
    flagsLayout->addWidget(&m_flags);
    flagsLayout->addStretch(10);
    widgetLayout->addLayout(flagsLayout);
    setLayout(widgetLayout);
    
    auto registerChangedEmitter = [this](Register16 reg) {
        return [this, reg](UnsignedWord value) {
            Q_EMIT registerChanged(reg, value);
        };
    };

    QFont widgetFont = m_af.font();
    widgetFont.setPointSizeF(widgetFont.pointSizeF() * 0.85);

    for (auto * widget : {&m_af, &m_bc, &m_de, &m_hl, &m_ix, &m_iy,}) {
        widget->setFont(widgetFont);
    }

    m_flags.setFont(widgetFont);

    connect(&m_af, &RegisterPairWidget::valueChanged, registerChangedEmitter(Register16::AF));
    connect(&m_bc, &RegisterPairWidget::valueChanged, registerChangedEmitter(Register16::BC));
    connect(&m_de, &RegisterPairWidget::valueChanged, registerChangedEmitter(Register16::DE));
    connect(&m_hl, &RegisterPairWidget::valueChanged, registerChangedEmitter(Register16::HL));
    connect(&m_ix, &RegisterPairWidget::valueChanged, registerChangedEmitter(Register16::IX));
    connect(&m_iy, &RegisterPairWidget::valueChanged, registerChangedEmitter(Register16::IY));
}

RegistersWidget::~RegistersWidget() = default;

void RegistersWidget::setRegister(Register16 reg, UnsignedWord value)
{
    switch (reg) {
        case Register16::AF:
            m_af.setValue(value);
            m_flags.setAllFlags(value & 0xff);
            break;

        case Register16::BC:
            m_bc.setValue(value);
            break;

        case Register16::DE:
            m_de.setValue(value);
            break;

        case Register16::HL:
            m_hl.setValue(value);
            break;

        case Register16::IX:
            m_ix.setValue(value);
            break;

        case Register16::IY:
            m_iy.setValue(value);
            break;
            
        default:
            std::cerr << "Only registers AF, BC, DE, HL, IX an IY are present in this widget.\n";
            break;
    }
}

UnsignedWord RegistersWidget::registerValue(Register16 reg)
{
    switch (reg) {
        case Register16::AF:
            return m_af.value();

        case Register16::BC:
            return m_bc.value();

        case Register16::DE:
            return m_de.value();

        case Register16::HL:
            return m_hl.value();

        case Register16::IX:
            return m_ix.value();

        case Register16::IY:
            return m_iy.value();

        default:
            std::cerr << "Only registers AF, BC, DE, HL, IX an IY are present in this widget.\n";
            return 0;
    }
}

void RegistersWidget::setRegisters(const Z80::Registers & registers)
{
    m_af.setValue(registers.af);
    m_bc.setValue(registers.bc);
    m_de.setValue(registers.de);
    m_hl.setValue(registers.hl);
    m_ix.setValue(registers.ix);
    m_iy.setValue(registers.iy);
    m_flags.setAllFlags(registers.f);
}
