//
// Created by darren on 24/03/2021.
//

#ifndef SPECTRUM_QTUI_DEBUGGER_MEMORYWIDGET_H
#define SPECTRUM_QTUI_DEBUGGER_MEMORYWIDGET_H

#include <optional>
#include <QWidget>
#include <QToolButton>
#include "../../basespectrum.h"
#include "../memoryview.h"
#include "../hexspinbox.h"
#include "memorysearchwidget.h"

namespace Spectrum::QtUi::Debugger
{
    /**
     * A memory widget for the debugger.
     *
     * Presents a searchable view of a memory object. The memory can be searched for (un)signed 8- and 16-bit values, and for strings of characters. It is also
     * possible to navigate directly to a specified address.
     */
    class MemoryWidget
    : public QWidget
    {
        Q_OBJECT

    public:
        /**
         * Initialise a memory widget with a memory object to visualise.
         *
         * The memory to visualise is borrowed not owned. It is the responsibility of the caller to ensure that the memory object outlives the widget, or is
         * exchanged for another (setMemory()) before it is destroyed.
         *
         * @param memory The memory to visualise.
         */
        explicit MemoryWidget(BaseSpectrum::MemoryType * memory = nullptr, QWidget * = nullptr);

        /**
         * Initialise a memory widget using the memory from a Spectrum instance.
         *
         * The memory to visualise is borrowed not owned. It is the responsibility of the caller to ensure that the memory object outlives the widget, or is
         * exchanged for another (setMemory()) before it is destroyed.
         *
         * @param memory The memory to visualise.
         */
        explicit MemoryWidget(const BaseSpectrum & spectrum, QWidget * parent = nullptr)
        : MemoryWidget(spectrum.memory(), parent)
        {}

        MemoryWidget(const MemoryWidget &) = delete;
        MemoryWidget(MemoryWidget &) = delete;
        void operator=(const MemoryWidget &) = delete;
        void operator=(MemoryWidget &&) = delete;

        /**
         * Destructor.
         */
        ~MemoryWidget() override;

        /**
         * Set the memory that is being visualised.
         *
         * @param memory The memory.
         */
        void setMemory(BaseSpectrum::MemoryType * memory)
        {
            m_memory.setMemory(memory);
        }

        /**
         * Identify the address in the memory object at given widget coordinates.
         *
         * @return The address, or an empty optional if there is no address at the given coordinates.
         */
        [[nodiscard]] std::optional<::Z80::UnsignedWord> addressAt(const QPoint &) const;

        /**
         * Scroll the widget's viewport such that the given address is visible.
         *
         * The widget will scroll the smallest distance possible to make the address visible.
         *
         * @param addr The address to scroll to.
         */
        void scrollToAddress(::Z80::UnsignedWord addr);

        /**
         * Set a highlight on a given address.
         *
         * Highlighting will render the value at the given address in a custom foreground and background.
         *
         * @param address The address to highlight.
         * @param foreground The foreground colour for the highlighted address.
         * @param background The background colour for the highlighted address.
         */
        void setHighlight(::Z80::UnsignedWord address, const QColor & foreground, const QColor & background);

        /**
         * Remove the highlight from a given address.
         *
         * It is safe to call this with an address that does not have a highlight. In such cases, the call is a no-op.
         *
         * @param address The address whose highlight should be removed.
         */
        void removeHighlight(::Z80::UnsignedWord address);

        /**
         * Clear all highlights.
         */
        void clearHighlights();

        /**
         * Perform searches in the memory for 8- or 16-bit (un)signed integers or QByteArrays.
         *
         * If no from address is provided, the search will proceed from the address at which the last search term was found. If the last search term was not
         * found, or no searches have yet been performed, the search will proceed from the address 0x0000.
         *
         * The calls are forwarded directly to the contained MemoryView widget.
         *
         * @tparam search_t The type to search for. Must be an 8- or 16-bit integer type, or QByteArray (optionally const qualified and/or a reference).
         * @param value The search term.
         * @param fromAddress The optional address from which to start the search.
         */
        template<class search_t>
        void find(search_t value, std::optional<::Z80::UnsignedWord> fromAddress = {});

    Q_SIGNALS:
        /**
         * Handler for when the "go to address" value is changed by the user.
         *
         * @param address The address the user has entered.
         */
        void locationValueChanged(::Z80::UnsignedWord address) const;

    private:
        /**
         * The view of the memory.
         */
        MemoryView m_memory;

        /**
         * The widget for the user to enter an address to scroll to.
         */
        HexSpinBox m_memoryLocation;

        /**
         * The search box widget.
         */
        MemorySearchWidget m_search;
    };
}

#endif //SPECTRUM_QTUI_DEBUGGER_MEMORYWIDGET_H
