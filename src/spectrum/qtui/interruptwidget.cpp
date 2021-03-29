//
// Created by darren on 23/03/2021.
//

#include <iostream>
#include <iomanip>

#include <QHBoxLayout>
#include <QLabel>

#include "interruptwidget.h"

using namespace Spectrum::QtUi;

using ::Z80::Register8;
using ::Z80::UnsignedByte;
using ::Z80::InterruptMode;

InterruptWidget::InterruptWidget(QWidget * parent)
: QWidget(parent),
  m_im(),
  m_i(2),
  m_r(2)
{
    m_i.setMinimum(0);
    m_i.setMaximum(0xff);
    m_r.setMinimum(0);
    m_r.setMaximum(0xff);

    m_im.setMinimum(0);
    m_im.setMaximum(2);

    QFont widgetFont = m_i.font();
    widgetFont.setPointSizeF(widgetFont.pointSizeF() * 0.85);
    m_i.setFont(widgetFont);
    m_r.setFont(widgetFont);
    m_im.setFont(widgetFont);

    auto * widgetLayout =  new QHBoxLayout();
    auto * tmpLabel = new QLabel(tr("Mode"));
    tmpLabel->setBuddy(&m_im);
    widgetLayout->addWidget(tmpLabel, 0);
    widgetLayout->addWidget(&m_im, 10);
    tmpLabel = new QLabel(tr("I"));
    tmpLabel->setBuddy(&m_i);
    widgetLayout->addWidget(tmpLabel, 0);
    widgetLayout->addWidget(&m_i, 10);
    tmpLabel = new QLabel(tr("R"));
    tmpLabel->setBuddy(&m_r);
    widgetLayout->addWidget(tmpLabel, 0);
    widgetLayout->addWidget(&m_r, 10);

    setLayout(widgetLayout);

    connect(&m_i, &HexSpinBox::editingFinished, [this]() {
        Q_EMIT registerChanged(Register8::I, m_i.value());
    });

    connect(&m_r, &HexSpinBox::editingFinished, [this]() {
        Q_EMIT registerChanged(Register8::R, m_r.value());
    });

    connect(&m_im, qOverload<int>(&QSpinBox::valueChanged), [this](int value) {
        Q_EMIT interruptModeChanged(static_cast<InterruptMode>(value));
    });
}

InterruptWidget::~InterruptWidget() = default;

void InterruptWidget::setRegister(Register8 reg, UnsignedByte value)
{
    switch (reg) {
        case Register8::I:
            m_i.setValue(value);
            break;

        case Register8::R:
            m_r.setValue(value);
            break;

        default:
            std::cerr << "Only registers I and R are present in this widget.\n";
            break;
    }
}

UnsignedByte InterruptWidget::registerValue(Register8 reg)
{
    switch (reg) {
        case Register8::I:
            return m_i.value();

        case Register8::R:
            return m_r.value();

        default:
            std::cerr << "Only registers I and Y are present in this widget.\n";
            return 0;
    }
}

void InterruptWidget::setRegisters(const Z80::Registers & registers)
{
    m_i.setValue(registers.i);
    m_r.setValue(registers.r);
}

::Z80::InterruptMode InterruptWidget::interruptMode() const
{
    return static_cast<InterruptMode>(m_im.value());
}

void InterruptWidget::setInterruptMode(::Z80::InterruptMode mode)
{
    m_im.setValue(static_cast<int>(mode));
}

