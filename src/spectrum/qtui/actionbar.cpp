//
// Created by darren on 26/04/2021.
//

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAction>
#include <QToolButton>
#include <QStyle>
#include "actionbar.h"
#include "widgetupdatesuspender.h"
#include "../../util/debug.h"

using namespace Spectrum::QtUi;

namespace
{
    constexpr const int DefaultSpacing = 2;

    /**
     * Helper to create a QToolButton for a given action.
     *
     * Keeps the tool buttons consistent.
     *
     * @param action
     * @return
     */
    QToolButton * createToolButton(QAction * action)
    {
        auto * button = new QToolButton();
        button->setAutoRaise(true);
        button->setDefaultAction(action);
        return button;
    }
}

ActionBar::ActionBar(Qt::Orientation orientation, QWidget * parent)
: QWidget(parent),
  m_iconSize()
{
    int iconSize = style()->pixelMetric(QStyle::PM_ButtonIconSize, nullptr, this);
    m_iconSize.setWidth(iconSize);
    m_iconSize.setHeight(iconSize);

    if (Qt::Orientation::Vertical == orientation) {
        QWidget::setLayout(new QVBoxLayout(this));
    } else {
        QWidget::setLayout(new QHBoxLayout(this));
    }

    layout()->setSpacing(DefaultSpacing);
}

ActionBar::~ActionBar() = default;

Qt::Orientation ActionBar::orientation() const
{
    if (qobject_cast<QVBoxLayout *>(layout())) {
        return Qt::Vertical;
    }

    return Qt::Horizontal;
}

void ActionBar::setOrientation(Qt::Orientation orient)
{
    if (orientation() == orient) {
        return;
    }

    QBoxLayout * newLayout;

    if (Qt::Orientation::Vertical == orient) {
        newLayout = new QVBoxLayout(this);
    } else {
        newLayout = new QHBoxLayout(this);
    }

    newLayout->setSpacing(DefaultSpacing);

    if (layout()) {
        while (0 < layout()->count()) {
            newLayout->addItem(layout()->itemAt(0));
        }

        delete layout();
    }

    QWidget::setLayout(newLayout);
}

void ActionBar::setIconSize(const QSize & size)
{
    if (size == iconSize()) {
        return;
    }

    m_iconSize = size;

    for (int idx = 0; idx < layout()->count(); ++idx) {
        auto * button = qobject_cast<QToolButton *>(layout()->itemAt(idx)->widget());

        if (!button) {
            continue;
        }

        button->setIconSize(size);
    }
}

void ActionBar::addStretch(int size)
{
    auto * layout = qobject_cast<QBoxLayout *>(this->layout());
    assert(layout);
    layout->addStretch(size);
}

void ActionBar::insertStretch(int idx, int size)
{
    if (0 > idx || layout()->count() <= idx) {
        Util::debug << "index " << idx << " is outside the current bounds of the action bar - adding stretch to the end of the action bar layout\n";
        addStretch(size);
        return;
    }

    auto * layout = qobject_cast<QBoxLayout *>(this->layout());
    assert(layout);
    layout->insertStretch(idx, size);
}

void ActionBar::addAction(QAction * action)
{
    QWidget::addAction(action);
    auto * layout = qobject_cast<QBoxLayout *>(this->layout());
    assert(layout);
    layout->addWidget(createToolButton(action));
}

void ActionBar::addActions(const QList<QAction *> & actions)
{
    WidgetUpdateSuspender suspender(*this);

    for (auto * action : actions) {
        addAction(action);
    }
}

void ActionBar::insertAction(QAction * before, QAction * action)
{
    if (!before || !actions().contains(before)) {
        addAction(action);
        return;
    }

    auto * layout = qobject_cast<QBoxLayout *>(this->layout());
    assert(layout);

    QWidget::insertAction(before, action);

    for (int idx = layout->count() - 1; idx >= 0; --idx) {
        auto * button = qobject_cast<QToolButton *>(layout->itemAt(idx)->widget());

        if (button && button->defaultAction() == before) {
            layout->insertWidget(idx, createToolButton(action));
            return;
        }
    }

    Util::debug << "widget for action " << before->text().toStdString() << " not found, appending rather than inserting\n";
    layout->addWidget(createToolButton(action));
}

void ActionBar::insertActions(QAction * before, const QList<QAction *> & actionList)
{
    if (!before || !actions().contains(before)) {
        addActions(actionList);
        return;
    }

    auto * layout = qobject_cast<QBoxLayout *>(this->layout());
    assert(layout);

    QWidget::insertActions(before, actionList);
    WidgetUpdateSuspender suspender(*this);

    for (int idx = layout->count() - 1; idx >= 0; --idx) {
        auto * button = qobject_cast<QToolButton *>(layout->itemAt(idx)->widget());

        if (button && button->defaultAction() == before) {
            for (auto * action : actionList) {
                layout->insertWidget(idx, createToolButton(action));
                ++idx;
            }
            return;
        }
    }

    Util::debug << "widget for action " << before->text().toStdString() << " not found, appending rather than inserting\n";

    for (auto * action : actionList) {
        layout->addWidget(createToolButton(action));
    }
}

void ActionBar::removeAction(QAction * action)
{
    QWidget::removeAction(action);

    for (int idx = 0; idx < layout()->count(); ++idx) {
        auto * button = qobject_cast<QToolButton *>(layout()->itemAt(idx)->widget());

        if (!button || button->defaultAction() != action) {
            continue;
        }

        layout()->removeWidget(button);
        delete button;
        return;
    }

    Util::debug << "action " << action->text().toStdString() << " not found\n";
}

void ActionBar::removeActions(const QList<QAction *> & actions)
{
    WidgetUpdateSuspender suspender(*this);

    for (auto * action : actions) {
        removeAction(action);
    }
}

void ActionBar::clear()
{
    for (auto * action : QWidget::actions()) {
        QWidget::removeAction(action);
    }

    auto * layout = this->layout();
    WidgetUpdateSuspender suspender(*this);

    while (0 < layout->count()) {
        auto * item = layout->itemAt(0);
        layout->removeItem(item);
        delete item;
    }
}
