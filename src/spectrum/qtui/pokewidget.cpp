//
// Created by darren on 14/03/2021.
//

#include <QHBoxLayout>
#include <QLabel>
#include "application.h"
#include "pokewidget.h"

using namespace Spectrum::QtUi;

PokeWidget::PokeWidget(
    const ::Z80::UnsignedWord address,
    const Z80::UnsignedByte value,
    const ValueBase valueBase,
    QWidget * parent
) : QWidget(parent),
  m_address(4),
  m_value(ValueBase::Decimal == valueBase ? std::make_unique<QSpinBox>() : std::make_unique<HexSpinBox>(2)),
  m_poke(),
  m_valueBase(valueBase)
{
    m_address.setValue(address);
    m_value->setValue(value);
    m_poke.setIcon(QIcon::fromTheme("dialog-ok-apply", Application::icon("ok")));

    auto * layout = new QHBoxLayout();
    layout->addWidget(new QLabel(tr("Address")), 1);
    layout->addWidget(&m_address, 10);
    layout->addWidget(new QLabel(tr("Value")), 1);
    layout->addWidget(m_value.get(), 10);
    layout->addWidget(&m_poke, 1);
    setLayout(layout);

    connect(&m_poke, &QToolButton::clicked, [this]() {
        Q_EMIT pokeClicked(m_address.value(), m_value->value());
    });
}

PokeWidget::PokeWidget(const ::Z80::UnsignedWord address, const ::Z80::UnsignedByte value, QWidget * parent)
: PokeWidget(address, value, ValueBase::Decimal, parent)
{}

PokeWidget::PokeWidget(const ::Z80::UnsignedWord address, QWidget * parent)
: PokeWidget(address, 0x00, ValueBase::Decimal, parent)
{}

PokeWidget::PokeWidget(const ::Z80::UnsignedWord address, const ValueBase valueBase, QWidget * parent)
: PokeWidget(address, 0x00, valueBase, parent)
{}

PokeWidget::PokeWidget(const ::Z80::UnsignedByte value, QWidget * parent)
: PokeWidget(0x0000, value, ValueBase::Decimal, parent)
{}

PokeWidget::PokeWidget(const ::Z80::UnsignedByte value, const ValueBase valueBase, QWidget * parent)
: PokeWidget(0x0000, value, valueBase, parent)
{}

PokeWidget::~PokeWidget() = default;

void PokeWidget::setValueBase(const ValueBase valueBase) noexcept
{
    if (valueBase == m_valueBase) {
        return;
    }

    m_valueBase = valueBase;
    auto * newWidget = (ValueBase::Decimal == valueBase ? new QSpinBox() : new HexSpinBox(2));
    layout()->replaceWidget(m_value.get(), newWidget);
    m_value.reset(newWidget);
}
