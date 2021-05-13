//
// Created by darren on 08/05/2021.
//

#include "breakpointsview.h"
#include "breakpointsmodel.h"
#include "../application.h"

using namespace Spectrum::QtUi::Debugger;
using BreakpointsViewBase = Spectrum::QtUi::ActionableItemView<QTreeView>;

namespace
{
    /**
     * The index in the array of the "enable/disable" action.
     */
    constexpr const int EnabledAction = 0;

    /**
     * The index in the array of the "remove" action.
     */
    constexpr const int RemoveAction = 1;
}

BreakpointsView::BreakpointsView(QWidget * parent)
: BreakpointsViewBase(parent),
    m_itemActions({
        std::make_unique<QAction>(Application::icon(QStringLiteral("dialog-ok-apply"), QStringLiteral("ok")), tr("Enabled")),
        std::make_unique<QAction>(Application::icon(QStringLiteral("list-remove"), QStringLiteral("remove")), tr("Remove")),
    })
{
    setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    setItemsExpandable(false);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setItemActionAlignment(ItemActionVerticalAlignment::Centre);

    m_itemActions[EnabledAction]->setCheckable(true);
    m_itemActions[EnabledAction]->setToolTip(tr("Enable or disable this breakpoint."));
    m_itemActions[RemoveAction]->setToolTip(tr("Remove this breakpoint."));

    addItemAction(m_itemActions[EnabledAction].get());
    addItemAction(m_itemActions[RemoveAction].get());

    connect(m_itemActions[EnabledAction].get(), &QAction::toggled, [this](bool enabled) {
        const auto & idx = actionItemIndex();

        if (enabled) {
            dynamic_cast<BreakpointsModel *>(model())->enableBreakpoint(idx);
        } else {
            dynamic_cast<BreakpointsModel *>(model())->disableBreakpoint(idx);
        }
    });

    connect(m_itemActions[RemoveAction].get(), &QAction::triggered, [this]() {
        dynamic_cast<BreakpointsModel *>(model())->removeBreakpoint(actionItemIndex());
    });
}

void BreakpointsView::setModel(BreakpointsModel * model)
{
    disconnect(this->model(), &QAbstractItemModel::dataChanged, this, &BreakpointsView::onModelDataChanged);
    QTreeView::setModel(model);
    connect(model, &QAbstractItemModel::dataChanged, this, &BreakpointsView::onModelDataChanged);
}

void BreakpointsView::onItemEntered(const QModelIndex & index)
{
    // stop the action firing when we (un)check it - it's not the user triggering it, we're just ensuring its state reflects the hovered item
    QSignalBlocker blocker(m_itemActions[EnabledAction].get());

    // this has to be done first so that the current action item index is set before we check the action
    BreakpointsViewBase::onItemEntered(index);
    m_itemActions[EnabledAction]->setChecked(index.data(BreakpointsModel::EnabledRole).toBool());
}

void BreakpointsView::onModelDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)
{
    const auto & index = actionItemIndex();

    if (!index.isValid()) {
        return;
    }

    if (topLeft.row() <= index.row() && bottomRight.row() >= index.row()) {
        // stop the action firing when we check it - it's not the user triggering it, we're just ensuring its state reflects the hovered item
        QSignalBlocker blocker(m_itemActions[EnabledAction].get());
        m_itemActions[EnabledAction]->setChecked(index.data(BreakpointsModel::EnabledRole).toBool());
    }
}
