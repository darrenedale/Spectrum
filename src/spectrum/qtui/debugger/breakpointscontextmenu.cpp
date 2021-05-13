//
// Created by darren on 07/05/2021.
//

#include "breakpointscontextmenu.h"
#include "../application.h"

using namespace Spectrum::QtUi::Debugger;
using Spectrum::Debugger::Breakpoint;

BreakpointsContextMenu::BreakpointsContextMenu(BreakpointsModel * model, const QModelIndex & idx, QWidget * parent)
: QMenu(parent),
  m_model(model),
  m_index(idx)
{
    assert(model);
    // for safety, the menu is closed if the model is destroyed or changes - in the case, the index may become invalid but will still report validity, and any
    // attempt to fetch the watch from the model could dereference an invalid pointer
    connect(model, &QAbstractItemModel::destroyed, this, &QMenu::close);
    connect(model, &QAbstractItemModel::dataChanged, this, &QMenu::close);

    // if the menu subject is a valid watch, add menu items specific to that watch
    if (idx.isValid()) {
        addItemsForBreakpoint(*model->breakpoint(idx));
    }

    addSeparator();
    auto * action = addAction(Application::icon(QStringLiteral("edit-clear-list"), QStringLiteral("clear")), tr("Clear all breakpoints"), this, &BreakpointsContextMenu::onClearTriggered);
    action->setToolTip(tr("Remove all breakpoints from the list."));

    // if the model has no watches disable the "clear" action - it will do nothing
    if (0 == model->rowCount()) {
        action->setEnabled(false);
    }
}


void BreakpointsContextMenu::addItemsForBreakpoint(Breakpoint & breakpoint)
{
    const auto title = QString::fromStdString(breakpoint.conditionDescription());
    addSection(title);

    QAction * action;

    if (m_index.data(BreakpointsModel::EnabledRole).toBool()) {
        action = addAction(Application::icon(QStringLiteral("media-playback-pause"), QStringLiteral("pause")), tr("Disable breakpoint"), this, &BreakpointsContextMenu::onDisableBreakpointTriggered);
        action->setToolTip(tr("Disable the breakpoint %1.").arg(title));
    } else {
        action = addAction(Application::icon(QStringLiteral("dialog-ok-apply"), QStringLiteral("ok")), tr("Enable breakpoint"), this, &BreakpointsContextMenu::onEnableBreakpointTriggered);
        action->setToolTip(tr("Enable the breakpoint %1.").arg(title));
    }

    action = addAction(Application::icon(QStringLiteral("list-clear"), QStringLiteral("remove")), tr("Remove breakpoint"), this, &BreakpointsContextMenu::onRemoveBreakpointTriggered);
    action->setToolTip(tr("Remove the breakpoint %1.").arg(title));
}

void BreakpointsContextMenu::onRemoveBreakpointTriggered()
{
    m_model->removeBreakpoint(m_index);
    Q_EMIT removeBreakpointTriggered(m_model->breakpoint(m_index));
}

void BreakpointsContextMenu::onEnableBreakpointTriggered()
{
    m_model->enableBreakpoint(m_index);
    Q_EMIT enableBreakpointTriggered(m_model->breakpoint(m_index));
}

void BreakpointsContextMenu::onDisableBreakpointTriggered()
{
    m_model->disableBreakpoint(m_index);
    Q_EMIT disableBreakpointTriggered(m_model->breakpoint(m_index));
}

void BreakpointsContextMenu::onClearTriggered()
{
    m_model->clear();
}