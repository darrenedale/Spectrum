//
// Created by darren on 03/05/2021.
//

#ifndef SPECTRUM_QTUI_DEBUGGER_BREAKPOINTSCONTEXTMENU_H
#define SPECTRUM_QTUI_DEBUGGER_BREAKPOINTSCONTEXTMENU_H

#include <QMenu>
#include "breakpointsmodel.h"
#include "../../../z80/types.h"

namespace Spectrum::QtUi::Debugger
{
    /**
     * Context menu for the breakpoints view in the debug window.
     *
     * Extracted to a separate class primarily for maintainability.
     */
    class BreakpointsContextMenu
    : public QMenu
    {
    Q_OBJECT

    public:
        /**
         * Initialise a new menu.
         *
         * The model must outlive the context menu. For safety reasons, the menu will close if the model's data changes (the index may no longer be valid) or
         * if the model is destroyed.
         *
         * @param model The model containing the breakpoints in the view.
         * @param idx The model index of the item under the cursor when the context menu was requested (i.e. the subject of the menu).
         * @param parent The parent widget for the menu.
         */
        BreakpointsContextMenu(BreakpointsModel * model, const QModelIndex & idx, QWidget * parent = nullptr);

    Q_SIGNALS:
        /**
         * Emitted when the "remove breakpoint" item is selected by the user.
         */
        void removeBreakpointTriggered(Spectrum::Debugger::Breakpoint *);

        /**
         * Emitted when the "enable breakpoint" item is selected by the user.
         */
        void enableBreakpointTriggered(Spectrum::Debugger::Breakpoint *);

        /**
         * Emitted when the "disable breakpoint" item is selected by the user.
         */
        void disableBreakpointTriggered(Spectrum::Debugger::Breakpoint *);

    protected:
        /**
         * Called when "remove breakpoint" is selected.
         */
        void onRemoveBreakpointTriggered();

        /**
         * Called when "enable breakpoint" is selected.
         */
        void onEnableBreakpointTriggered();

        /**
         * Called when "disable breakpoint" is selected.
         */
        void onDisableBreakpointTriggered();

        /**
         * Called when "clear all breakpoints" is selected.
         */
        void onClearTriggered();

        /**
         * Helper to add the menu items for a specific breakpoint.
         *
         * This is used when the context menu is initiated for a specific item in the view.
         */
        void addItemsForBreakpoint(Spectrum::Debugger::Breakpoint &);

    private:
        /**
         * The model containing the items in the view whose context menu this object represents.
         */
        BreakpointsModel * m_model;

        /**
         * The index in the model for the item that is the subject of the menu.
         *
         * This will be an invalid index if the menu was not invoked on a specific item.
         */
        QModelIndex m_index;
    };
}

#endif //SPECTRUM_QTUI_DEBUGGER_BREAKPOINTSCONTEXTMENU_H
