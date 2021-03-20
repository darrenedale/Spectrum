//
// Created by darren on 04/03/2021.
//

#include <iostream>

#include <QStringBuilder>
#include <QHBoxLayout>
#include <QHelpEvent>

#include "registerpairwidget.h"

using namespace Spectrum::Qt;

namespace
{
    enum class ByteType
    {
        Low = 0,
        High,
    };

    QString guessRegisterByteName(const QString & name, ByteType type)
    {
        if ("IX" == name || "IY" == name) {
            return name % (ByteType::Low == type ? "L" : "H");
        }

        bool isShadow = (3 == name.length() && '\'' == name[2]);

        switch (type) {
            case ByteType::Low:
                return name[0] % (isShadow ? "'" : "");

            case ByteType::High:
                return name[1] % (isShadow ? "'" : "");
        }

        return {};
    }
}

RegisterPairWidget::RegisterPairWidget(const QString & registerName,QWidget * parent)
: RegisterPairWidget(registerName, 0x0000, parent)
{}

RegisterPairWidget::RegisterPairWidget(UnsignedWord value, QWidget * parent)
: RegisterPairWidget(QLatin1String(), value, parent)
{}

RegisterPairWidget::RegisterPairWidget(QWidget * parent)
: RegisterPairWidget(QLatin1String(), 0x0000, parent)
{}

RegisterPairWidget::RegisterPairWidget(const QString & registerName, UnsignedWord value, QWidget * parent)
: QWidget(parent),
  m_registerName(registerName),
  m_highByteName(guessRegisterByteName(registerName, ByteType::High)),
  m_lowByteName(guessRegisterByteName(registerName, ByteType::Low)),
  m_label16(m_registerName),
  m_labelLow(m_lowByteName),
  m_labelHigh(m_highByteName),
  m_spin16(4),
  m_spinLow(2),
  m_spinHigh(2)
{
    m_spin16.setMinimum(0);
    m_spin16.setMaximum(0xffff);
    m_spinLow.setMinimum(0);
    m_spinLow.setMaximum(0xff);
    m_spinHigh.setMinimum(0);
    m_spinHigh.setMaximum(0xff);
    m_label16.setAlignment(::Qt::AlignVCenter | ::Qt::AlignRight);
    m_labelLow.setAlignment(::Qt::AlignVCenter | ::Qt::AlignRight);
    m_labelHigh.setAlignment(::Qt::AlignVCenter | ::Qt::AlignRight);

    setValue(value);

    auto * layout = new QHBoxLayout(this);
    layout->addWidget(&m_label16);
    layout->addWidget(&m_spin16);
    layout->addWidget(&m_labelLow);
    layout->addWidget(&m_spinLow);
    layout->addWidget(&m_labelHigh);
    layout->addWidget(&m_spinHigh);

    connect(&m_spin16, qOverload<int>(&QSpinBox::valueChanged), this, &RegisterPairWidget::setBytesForRegisterPair);
    connect(&m_spinLow, qOverload<int>(&QSpinBox::valueChanged), this, &RegisterPairWidget::setRegisterPairForBytes);
    connect(&m_spinHigh, qOverload<int>(&QSpinBox::valueChanged), this, &RegisterPairWidget::setRegisterPairForBytes);

    auto emitValueChanged = [this]() {
        Q_EMIT this->valueChanged(this->value());
        Q_EMIT this->valueChangedZ80(valueZ80());
    };

    connect(&m_spin16, &QSpinBox::editingFinished, emitValueChanged);
    connect(&m_spinLow, &QSpinBox::editingFinished, emitValueChanged);
    connect(&m_spinHigh, &QSpinBox::editingFinished, emitValueChanged);
}

RegisterPairWidget::~RegisterPairWidget() noexcept = default;

#include <iostream>

void RegisterPairWidget::setValue(RegisterPairWidget::UnsignedWord value)
{
    m_spin16.setValue(value);
    m_spinHigh.setValue(value & 0xff);
    m_spinLow.setValue((value & 0xff00) >> 8);
}

RegisterPairWidget::UnsignedWord RegisterPairWidget::value() const
{
    return static_cast<UnsignedWord>(m_spin16.value());
}

void RegisterPairWidget::setRegisterName(const QString & name)
{
    m_registerName = name;
    m_label16.setText(name);
}

void RegisterPairWidget::setLowByteName(const QString & name)
{
    m_highByteName = name;
    m_labelLow.setText(name);
}

void RegisterPairWidget::setHighByteName(const QString & name)
{
    m_highByteName = name;
    m_labelHigh.setText(name);
}

void RegisterPairWidget::setRegisterPairForBytes()
{
    QSignalBlocker blocker(&m_spin16);
    m_spin16.setValue(((static_cast<UnsignedWord>(m_spinHigh.value()) << 8) & 0xff00) | m_spinLow.value());
}

void RegisterPairWidget::setBytesForRegisterPair()
{
    auto value = this->value();
    {
        QSignalBlocker blocker(&m_spinHigh);
        m_spinHigh.setValue(value & 0xff);
    }

    {
        QSignalBlocker blocker(&m_spinLow);
        m_spinLow.setValue((value & 0xff00) >> 8);
    }
}
