//
// Created by darren on 08/05/2021.
//

#ifndef SPECTRUM_QTUI_DEBUGGER_BREAKPOINTSVIEW_H
#define SPECTRUM_QTUI_DEBUGGER_BREAKPOINTSVIEW_H

#include <array>
#include <QTreeView>
#include <QAction>
#include "../actionableitemview.h"

namespace Spectrum::QtUi::Debugger
{
    class BreakpointsModel;

    /**
     * View of breakpoints for the debug window.
     *
     * The QTreeView class is extended via ActionableItemView to add an overlay with two item actions that can be clicked by the user to affect the state of the
     * breakpoint under the mouse pointer:
     * - "enable/disable" toggles whether the breakpoint is currently enabled or disabled
     * - "remove" deletes the breakpoint
     */
    class BreakpointsView
    : public ActionableItemView<QTreeView>
    {
        Q_OBJECT

    public:
        /**
         * Initialise a new breakpoints view.
         *
         * @param parent The owner of the view widget.
         */
        explicit BreakpointsView(QWidget * parent = nullptr);

        /**
         * Set the model that provides data for the view.
         */
        void setModel(BreakpointsModel * model);

    private:
        /**
         * Handler for when the mouse enters a new breakpoint in the view.
         *
         * The item action states are udpated to reflect the state of the identified breakpoint.
         *
         * @param index The model index of the entered breakpoint.
         */
        void onItemEntered(const QModelIndex & index) override;

        /**
         * Handler for when the data in the model has changed.
         *
         * If the currently hovered item has changed, the action states are updated to reflect the new data.
         *
         * @param topLeft
         * @param bottomRight
         */
        void onModelDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight);

        /**
         * The item actions that are are available in the view.
         */
        std::array<std::unique_ptr<QAction>, 2> m_itemActions;
    };
}

#endif //SPECTRUM_QTUI_DEBUGGER_BREAKPOINTSVIEW_H
