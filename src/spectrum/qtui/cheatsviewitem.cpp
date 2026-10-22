//
// Created by darren on 29/03/2021.
//

#include <utility>
#include <QHBoxLayout>
#include <QLabel>

#include "application.h"
#include "cheatsviewitem.h"
#include "../../util/debug.h"

using namespace Spectrum::QtUi;

CheatsViewItem::CheatsViewItem(const QString & name, QString  uuid, QWidget * parent)
: QWidget(parent),
  m_uuid(std::move(uuid)),
  m_reapply(this),
  m_onOff(this),
  m_remove(this)
{
    auto * layout = new QHBoxLayout();
    layout->addWidget(new QLabel(name), 10);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    m_reapply.setAutoRaise(true);
    m_reapply.setIcon(QIcon::fromTheme(QStringLiteral("view-refresh"), Application::icon(QStringLiteral("refresh"))));
    m_reapply.setText(tr("Re-apply"));
    m_reapply.setToolTip(tr("Re-apply the cheat '%1'.").arg(name));
    m_reapply.setEnabled(false);
    layout->addWidget(&m_reapply);

    m_onOff.setAutoRaise(true);
    m_onOff.setCheckable(true);
    m_onOff.setChecked(false);
    m_onOff.setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok-apply"), Application::icon(QStringLiteral("ok"))));
    m_onOff.setText(tr("Off"));
    m_onOff.setToolTip(tr("Switch the cheat '%1' on/off.").arg(name));
    layout->addWidget(&m_onOff);

    m_remove.setAutoRaise(true);
    m_remove.setIcon(QIcon::fromTheme(QStringLiteral("list-remove"), Application::icon(QStringLiteral("remove"))));
    m_remove.setText(tr("Remove"));
    m_remove.setToolTip(tr("Remove the cheat '%1' from this list.").arg(name));
    layout->addWidget(&m_remove);

    setLayout(layout);

    connect(&m_onOff, &QToolButton::toggled, [this](const bool on) {
        m_reapply.setEnabled(on);

        if (on) {
            Q_EMIT activationRequested(m_uuid);
        } else {
            Q_EMIT deactivationRequested(m_uuid);
        }
    });

    connect(&m_reapply, &QToolButton::clicked, [this]() {
        Q_EMIT activationRequested(m_uuid);
    });

    connect(&m_remove, &QToolButton::clicked, [this]() {
        Q_EMIT removeClicked(m_uuid);
    });
}

CheatsViewItem::~CheatsViewItem() = default;

void CheatsViewItem::setActivated(const bool activated)
{
    m_onOff.setChecked(activated);

    if (activated) {
        m_onOff.setText(tr("On"));
    } else {
        m_onOff.setText(tr("Off"));
    }
}

void CheatsViewItem::reset()
{
    QSignalBlocker blocker(m_onOff);
    m_onOff.setChecked(false);
    m_onOff.setText(tr("Off"));
}
