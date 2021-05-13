//
// Created by darren on 04/05/2021.
//

#ifndef SPECTRUM_QTUI_DEBUGGER_MEMORYCONTEXTMENU_H
#define SPECTRUM_QTUI_DEBUGGER_MEMORYCONTEXTMENU_H

#include <QMenu>
#include "../../../z80/types.h"

namespace Spectrum::QtUi::Debugger
{
    /**
     * Context menu for the watches view in the debug window.
     *
     * Extracted to a separate class primarily for maintainability.
     */
    class MemoryContextMenu
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
         * @param address The address for which to provide context menu actions.
         * @param parent The parent widget for the menu.
         */
        explicit MemoryContextMenu(::Z80::UnsignedWord address, QWidget * parent = nullptr);

        /**
         * Set the address that is the subject of the menu.
         *
         * @param address The address.
         */
        void setAddress(::Z80::UnsignedWord address);

        /**
         * Fetch the address that is the subject of the menu.
         *
         * @return The address.
         */
        [[nodiscard]] ::Z80::UnsignedWord address() const
        {
            return m_address;
        }

    Q_SIGNALS:
        /**
         * Signal emitted when user selects the "poke" item.
         */
        void poke(::Z80::UnsignedWord address);

        /**
         * Signal emitted when user selects the "break at PC" item.
         */
        void breakAtProgramCounter(::Z80::UnsignedWord address);

        /**
         * Signal emitted when user selects the "break on change (word)" item.
         */
        void breakOnWordChange(::Z80::UnsignedWord address);

        /**
         * Signal emitted when user selects the "break on change (byte)" item.
         */
        void breakOnByteChange(::Z80::UnsignedWord address);

        /**
         * Signal emitted when user selects the "watch word" item.
         */
        void watchWord(::Z80::UnsignedWord address);

        /**
         * Signal emitted when user selects the "watch byte" item.
         */
        void watchByte(::Z80::UnsignedWord address);

        /**
         * Signal emitted when user selects the "watch string" item.
         */
        void watchString(::Z80::UnsignedWord address);

    protected:
        /**
         * Called when "Poke" is selected.
         */
        void onPokeTriggered();

        /**
         * Handler called when user selects the "break at PC" item.
         */
        void onBreakAtProgramCounterTriggered();

        /**
         * Handler called when user selects one of the "break on change" items.
         */
        template<class int_t>
        void onBreakOnChangeTriggered();

        /**
         * Handler called when user selects the "watch byte/word" item.
         */
        template<class int_t>
        void onWatchIntegerTriggered();

        /**
         * Handler called when user selects the "watch string" item.
         */
        void onWatchStringTriggered();

    private:
        /**
         * The address that is the subject of the context menu.
         */
        ::Z80::UnsignedWord m_address;

        /**
         * The action added as the section title.
         *
         * We only keep a pointer to this so that we can update the text when the address changes.
         */
        QAction * m_sectionTitle;
    };
}

#endif //SPECTRUM_QTUI_DEBUGGER_MEMORYCONTEXTMENU_H
