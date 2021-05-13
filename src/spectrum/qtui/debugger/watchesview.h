//
// Created by darren on 08/05/2021.
//

#ifndef SPECTRUM_QTUI_DEBUGGER_WATCHESSVIEW_H
#define SPECTRUM_QTUI_DEBUGGER_WATCHESSVIEW_H

#include <array>
#include <QTreeView>
#include "../actionableitemview.h"

namespace Spectrum::QtUi::Debugger
{
    class WatchesModel;

    /**
     * View of memory watches for the debug window.
     *
     * The QTreeView class is extended via ActionableItemView to add an overlay with a single item action that can be clicked by the user to affect the state of
     * the memory watch under the mouse pointer:
     * - "remove" deletes the breakpoint
     */
    class WatchesView
    : public ActionableItemView<QTreeView>
    {
    Q_OBJECT

    public:
        /**
         * Initialise a new watches view.
         *
         * @param parent The owner of the view widget.
         */
        explicit WatchesView(QWidget * parent = nullptr);

        /**
         * Set the model that provides data for the view.
         */
        void setModel(WatchesModel * model);

    private:
        /**
         * The item actions that are are available in the view.
         */
        std::array<std::unique_ptr<QAction>, 1> m_itemActions;
    };
}

#endif //SPECTRUM_QTUI_DEBUGGER_WATCHESSVIEW_H
