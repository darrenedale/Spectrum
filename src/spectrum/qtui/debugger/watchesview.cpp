//
// Created by darren on 08/05/2021.
//

#include "watchesview.h"
#include "watchesmodel.h"
#include "../hexspinboxdelegate.h"
#include "../application.h"

using namespace Spectrum::QtUi::Debugger;
using WatchesViewBase = Spectrum::QtUi::ActionableItemView<QTreeView>;

namespace
{
    /**
     * The index in the array of the "remove" action.
     */
    constexpr const int RemoveAction = 0;
}

WatchesView::WatchesView(QWidget * parent)
: WatchesViewBase(parent),
  m_itemActions({
          std::make_unique<QAction>(Application::icon(QStringLiteral("list-remove"), QStringLiteral("remove")), tr("Remove")),
      })
{
    setItemDelegateForColumn(0, new HexSpinBoxDelegate(this));
    setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    setItemsExpandable(false);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setItemActionAlignment(ItemActionVerticalAlignment::Centre);

    m_itemActions[RemoveAction]->setToolTip(tr("Remove this watch."));
    addItemAction(m_itemActions[RemoveAction].get());

    connect(m_itemActions[RemoveAction].get(), &QAction::triggered, [this]() {
        dynamic_cast<WatchesModel *>(model())->removeWatch(actionItemIndex());
    });
}

void WatchesView::setModel(WatchesModel * model)
{
    QTreeView::setModel(model);
}
