//
// Created by darren on 14/03/2021.
//

#include <QHBoxLayout>
#include <QLabel>
#include "application.h"
#include "pokewidget.h"

using namespace Spectrum::QtUi;

PokeWidget::PokeWidget(::Z80::UnsignedWord address, Z80::UnsignedByte value, QWidget * parent)
: QWidget(parent),
  m_address(4),
  m_value(2),
  m_poke()
{
    m_address.setValue(address);
    m_value.setValue(value);
    m_poke.setIcon(QIcon::fromTheme("dialog-ok-apply", Application::icon("ok")));

    auto * layout = new QHBoxLayout();
    layout->addWidget(new QLabel(tr("Address")), 1);
    layout->addWidget(&m_address, 10);
    layout->addWidget(new QLabel(tr("Value")), 1);
    layout->addWidget(&m_value, 10);
    layout->addWidget(&m_poke, 1);
    setLayout(layout);

    connect(&m_poke, &QToolButton::clicked, [this]() {
        Q_EMIT pokeClicked(m_address.value(), m_value.value());
    });
}

PokeWidget::PokeWidget(::Z80::UnsignedWord address, QWidget * parent)
: PokeWidget(address, 0x00, parent)
{
}

PokeWidget::PokeWidget(::Z80::UnsignedByte value, QWidget * parent)
: PokeWidget(0x0000, value, parent)
{
}

PokeWidget::~PokeWidget() = default;
