//
// Created by darren on 23/03/2021.
//

#include <QVBoxLayout>
#include <QHBoxLayout>
#include "shadowregisterswidget.h"
#include "../../../util/debug.h"

using namespace Spectrum::QtUi::Debugger;

using ::Z80::UnsignedWord;
using ::Z80::Register16;

ShadowRegistersWidget::ShadowRegistersWidget(QWidget * parent)
: QWidget(parent),
  m_afShadow(QStringLiteral("AF")),
  m_bcShadow(QStringLiteral("BC")),
  m_deShadow(QStringLiteral("DE")),
  m_hlShadow(QStringLiteral("HL")),
  m_shadowFlags()
{
    auto * widgetLayout = new QVBoxLayout();

    widgetLayout->setSpacing(2);
    widgetLayout->addWidget(&m_afShadow);
    widgetLayout->addWidget(&m_bcShadow);
    widgetLayout->addWidget(&m_deShadow);
    widgetLayout->addWidget(&m_hlShadow);

    auto * flagsLayout = new QHBoxLayout();
    auto * flagsLabel = new QLabel("Flags");
    flagsLabel->setBuddy(&m_shadowFlags);
    flagsLabel->setFont(m_shadowFlags.font());
    flagsLayout->addWidget(flagsLabel);
    flagsLayout->addWidget(&m_shadowFlags);
    flagsLayout->addStretch(10);
    widgetLayout->addLayout(flagsLayout);
    setLayout(widgetLayout);

    QFont widgetFont = m_afShadow.font();
    widgetFont.setPointSizeF(widgetFont.pointSizeF() * 0.85);

    for (auto * widget : {&m_afShadow, &m_bcShadow, &m_deShadow, &m_hlShadow,}) {
        widget->setFont(widgetFont);
    }

    m_shadowFlags.setFont(widgetFont);

    auto registerChangedEmitter = [this](Register16 reg) {
        return [this, reg](UnsignedWord value) {
            Q_EMIT registerChanged(reg, value);
        };
    };

    connect(&m_afShadow, &RegisterPairWidget::valueChanged, registerChangedEmitter(Register16::AFShadow));
    connect(&m_bcShadow, &RegisterPairWidget::valueChanged, registerChangedEmitter(Register16::BCShadow));
    connect(&m_deShadow, &RegisterPairWidget::valueChanged, registerChangedEmitter(Register16::DEShadow));
    connect(&m_hlShadow, &RegisterPairWidget::valueChanged, registerChangedEmitter(Register16::HLShadow));
}

ShadowRegistersWidget::~ShadowRegistersWidget() = default;

void ShadowRegistersWidget::setRegister(Register16 reg, UnsignedWord value)
{
    switch (reg) {
        case Register16::AFShadow:
            m_afShadow.setValue(value);
            m_shadowFlags.setAllFlags(value & 0xff);
            break;

        case Register16::BCShadow:
            m_bcShadow.setValue(value);
            break;

        case Register16::DEShadow:
            m_deShadow.setValue(value);
            break;

        case Register16::HLShadow:
            m_hlShadow.setValue(value);
            break;

        default:
            Util::debug << "Only registers AF', BC', DE', HL' are present in this widget.\n";
            break;
    }
}

UnsignedWord ShadowRegistersWidget::registerValue(Register16 reg)
{
    switch (reg) {
        case Register16::AFShadow:
            return m_afShadow.value();

        case Register16::BCShadow:
            return m_bcShadow.value();

        case Register16::DEShadow:
            return m_deShadow.value();

        case Register16::HLShadow:
            return m_hlShadow.value();

        default:
            Util::debug << "Only registers AF', BC', DE', HL' are present in this widget.\n";
            return 0;
    }
}

void ShadowRegistersWidget::setRegisters(const Z80::Registers & registers)
{
    m_afShadow.setValue(registers.afShadow);
    m_bcShadow.setValue(registers.bcShadow);
    m_deShadow.setValue(registers.deShadow);
    m_hlShadow.setValue(registers.hlShadow);
    m_shadowFlags.setAllFlags(registers.fShadow);
}
