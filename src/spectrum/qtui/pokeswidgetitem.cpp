//
// Created by darren on 29/03/2021.
//

#include <utility>
#include <QHBoxLayout>
#include <QLabel>
#include "pokeswidgetitem.h"

using namespace Spectrum::QtUi;

PokesWidgetItem::PokesWidgetItem(const QString & name, QString  uuid, QWidget * parent)
: QWidget(parent),
  m_uuid(std::move(uuid)),
  m_onOff(this),
  m_remove(this)
{
    auto * layout = new QHBoxLayout();
    layout->addWidget(new QLabel(name), 10);
    layout->setSpacing(0);
    layout->setMargin(0);

    m_onOff.setAutoRaise(true);
    m_onOff.setCheckable(true);
    m_onOff.setChecked(false);
    m_onOff.setIcon(QIcon::fromTheme("dialog-ok-apply"));
    m_onOff.setText(tr("Off"));
    m_onOff.setToolTip(tr("Switch the poke '%1' on/off.").arg(name));
    layout->addWidget(&m_onOff);

    m_remove.setAutoRaise(true);
    m_remove.setIcon(QIcon::fromTheme("list-remove"));
    m_remove.setText(tr("Remove"));
    m_remove.setToolTip(tr("Remove the poke '%1' from this list.").arg(name));
    layout->addWidget(&m_remove);

    setLayout(layout);

    connect(&m_onOff, &QToolButton::toggled, [this](bool on) {
        if (on) {
            Q_EMIT activationRequested(m_uuid);
        } else {
            Q_EMIT deactivationRequested(m_uuid);
        }
    });

    connect(&m_remove, &QToolButton::clicked, [this]() {
        Q_EMIT removeClicked(m_uuid);
    });
}

PokesWidgetItem::~PokesWidgetItem() = default;

void PokesWidgetItem::setActivated(bool activated)
{
    m_onOff.setChecked(activated);

    if (activated) {
        m_onOff.setText(tr("On"));
    } else {
        m_onOff.setText(tr("Off"));
    }
}
