//
// Created by darren on 03/05/2021.
//

#ifndef SPECTRUM_QTUI_DEBUGGER_WATCHESCONTEXTMENU_H
#define SPECTRUM_QTUI_DEBUGGER_WATCHESCONTEXTMENU_H

#include <QMenu>
#include "watchesmodel.h"
#include "../../debugger/stringmemorywatch.h"
#include "../../debugger/integermemorywatchbase.h"
#include "../../../z80/types.h"

namespace Spectrum::QtUi::Debugger
{
    /**
     * Context menu for the watches view in the debug window.
     *
     * Extracted to a separate class primarily for maintainability.
     */
    class WatchesContextMenu
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
         * @param model The model containing the watches in the view.
         * @param idx The model index of the item under the cursor when the context menu was requested (i.e. the subject of the menu).
         * @param parent The parent widget for the menu.
         */
        WatchesContextMenu(WatchesModel * model, const QModelIndex & idx, QWidget * parent = nullptr);

    Q_SIGNALS:
        /**
         * Signal emitted when user selects the "locate in memory view" item.
         */
        void locateInMemoryView(::Z80::UnsignedWord);

    protected:
        /**
         * Called when "Locate in memory view" is selected.
         */
        void onLocateInMemoryViewTriggered();

        /**
         * Called when "remove watch" is selected.
         */
        void onRemoveWatchTriggered();

        /**
         * Called when "copy value" is selected.
         */
        void onCopyValueTriggered();

        /**
         * Called when "copy address" is selected.
         */
        void onCopyAddressTriggered();

        /**
         * Called when "clear all watches" is selected.
         */
        void onClearTriggered();

        /**
         * Called when a character set is selected.
         *
         * Only valid for String watches.
         *
         * @tparam charset
         */
        template<Spectrum::Debugger::StringMemoryWatch::CharacterEncoding charset>
        void onCharacterSetTriggered();

        /**
         * Called when a byte order is selected.
         *
         * Only valid for Integer watches.
         *
         * @tparam byteOrder The byte order set selected.
         */
        template<Spectrum::Debugger::IntegerMemoryWatchBase::ByteOrder byteOrder>
        void onByteOrderTriggered();

        /**
         * Called when a numeric base is selected.
         *
         * Only valid for Integer watches.
         *
         * @tparam base The base selected.
         */
        template<Spectrum::Debugger::IntegerMemoryWatchBase::Base base>
        void onNumericBaseTriggered();

        /**
         * Helper to add the menu items for a specific watch.
         *
         * This is used when the context menu is initiated for a specific item in the view.
         */
        void addItemsForWatch(Spectrum::Debugger::MemoryWatch &);

        /**
         * Helper to add the menu items for a String watch.
         *
         * This is used when the context menu is initiated for a specific item in the view, and the item is a String watch.
         */
        void addItemsForStringWatch(Spectrum::Debugger::StringMemoryWatch &);

        /**
         * Helper to add the menu items for an Integer watch.
         *
         * This is used when the context menu is initiated for a specific item in the view, and the item is an Integer watch.
         */
        void addItemsForIntegerWatch(Spectrum::Debugger::IntegerMemoryWatchBase &);

    private:
        /**
         * The model containing the items in the view whose context menu this object represents.
         */
        WatchesModel * m_model;

        /**
         * The index in the model for the item that is the subject of the menu.
         *
         * This will be an invalid index if the menu was not invoked on a specific item.
         */
        QModelIndex m_index;
    };
}

#endif //SPECTRUM_QTUI_DEBUGGER_WATCHESCONTEXTMENU_H
