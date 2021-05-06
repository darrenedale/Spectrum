//
// Created by darren on 06/03/2021.
//

#ifndef SPECTRUM_QTUI_MEMORYVIEW_H
#define SPECTRUM_QTUI_MEMORYVIEW_H

#include <optional>
#include <QScrollArea>
#include "../basespectrum.h"
#include "../../z80/z80.h"
#include "../../util/debug.h"

namespace Spectrum::QtUi
{
    /**
     * Widget providing a scrollable view of the contents of a memory object.
     *
     * Memory is presented as a matrix of hexadecimal unsigned bytes. Each row represents 16 bytes, and is prefixed with the address of the first byte in the
     * row. The view shows all addressable memory. For all Spectrum memory models, this results in 4096 rows of 16 bytes. Note that for 128k models, only the
     * paged-in memory banks are visualised.
     */
    class MemoryView
    : public QScrollArea
    {
        Q_OBJECT

    public:
        /**
         * Initialise a view with a given memory object.
         *
         * @param memory The memory to visualise.
         * @param parent The widget that owns the view.
         */
        explicit MemoryView(BaseSpectrum::MemoryType * memory = nullptr, QWidget * parent = nullptr);

        /**
         * Initialise a memory view with the memory from a given Spectrum model object.
         *
         * @param spectrum The spectrum whose memory should be visualised.
         * @param parent The widget that owns the view.
         */
        explicit MemoryView(const BaseSpectrum & spectrum, QWidget * parent = nullptr)
        : MemoryView(spectrum.memory(), parent)
        {}

        MemoryView(const MemoryView &) = delete;
        MemoryView(MemoryView &&) = delete;
        void operator=(const MemoryView &) = delete;
        void operator=(MemoryView &&) = delete;

        /**
         * Destructor.
         */
        ~MemoryView() override;

        /**
         * Set the memory object tha that the view should visualise.
         */
        void setMemory(BaseSpectrum::MemoryType *);

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
         * Determine the address at a given point in the view.
         *
         * @return The address, or an empty optional if there is no address at the given point (i.e. it's in empty space or a label).
         */
        [[nodiscard]] std::optional<::Z80::UnsignedWord> addressAt(const QPoint &) const;

        /**
         * Scroll the view to a given address.
         *
         * @param address The address to scroll to.
         */
        void scrollToAddress(::Z80::UnsignedWord address);

        /**
         * Search for an 8-bit unsigned value in the memory.
         *
         * If no from address is provided, the search will proceed from the address at which the last search term was found. If the last search term was not
         * found, or no searches have yet been performed, the search will proceed from the address 0x0000.
         *
         * @param value The value to find.
         * @param fromAddress The optional address at which to start searching.
         */
        void find(::Z80::UnsignedByte value, std::optional<::Z80::UnsignedWord> fromAddress)
        {
            find(QByteArray(reinterpret_cast<const char *>(&value), sizeof(::Z80::UnsignedByte)), fromAddress);
        }

        /**
         * Search for an 8-bit signed value in the memory.
         *
         * If no from address is provided, the search will proceed from the address at which the last search term was found. If the last search term was not
         * found, or no searches have yet been performed, the search will proceed from the address 0x0000.
         *
         * @param value The value to find.
         * @param fromAddress The optional address at which to start searching.
         */
        void find(::Z80::SignedByte value, std::optional<::Z80::UnsignedWord> fromAddress)
        {
            find(QByteArray(reinterpret_cast<const char *>(&value), sizeof(::Z80::SignedByte)), fromAddress);
        }

        /**
         * Search for a 16-bit unsigned value in the memory.
         *
         * The value is expected in the host byte order (since it is expected that ultimately it will be supplied by the user). It will be converted internally
         * to Z80 byte order if required, so that if the user enters "12345" as the search term, what is located will be the value 12345 as the Z80 sees it.
         *
         * If no from address is provided, the search will proceed from the address at which the last search term was found. If the last search term was not
         * found, or no searches have yet been performed, the search will proceed from the address 0x0000.
         *
         * @param value The value to find.
         * @param fromAddress The optional address at which to start searching.
         */
        void find(::Z80::UnsignedWord value, std::optional<::Z80::UnsignedWord> fromAddress);

        /**
         * Search for a 16-bit signed value in the memory.
         *
         * The value is expected in the host byte order (since it is expected that ultimately it will be supplied by the user). It will be converted internally
         * to Z80 byte order if required, so that if the user enters "-9876" as the search term, what is located will be the value -9876 as the Z80 sees it.
         *
         * If no from address is provided, the search will proceed from the address at which the last search term was found. If the last search term was not
         * found, or no searches have yet been performed, the search will proceed from the address 0x0000.
         *
         * @param value The value to find.
         * @param fromAddress The optional address at which to start searching.
         */
        void find(::Z80::SignedWord value, std::optional<::Z80::UnsignedWord> fromAddress);

        /**
         * Search for a string of bytes in the memory.
         *
         * If no from address is provided, the search will proceed from the address at which the last search term was found. If the last search term was not
         * found, or no searches have yet been performed, the search will proceed from the address 0x0000.
         *
         * @param value The string of bytes to search form.
         * @param fromAddress The optional address at which to start searching.
         */
        void find(const QByteArray & value, std::optional<::Z80::UnsignedWord> fromAddress);

        /**
         * Template to service all supported find() requests without the second fromAddress parameter.
         *
         * We use a template to overload rather than default arguments for the second parameter in the above find() functions so that we can connect as a slot
         * in Qt connections (Qt connections don't like default args).
         *
         * @tparam search_t The type of the search value. Must be either QByteArray (or a const/ref to such) or an int type of 8- or 16-bits.
         * @param value The search value.
         */
        template<class search_t>
        void find(search_t value)
        {
            static_assert((std::is_integral_v<search_t> && sizeof(search_t) <= 2) || std::is_same_v<QByteArray, std::remove_cvref_t<search_t>>, "find() method must be instantiated with an integral type of 16 bits or fewer or QByteArray");
            find(value, {});
        }

    private:
        /**
         * Internal data structure for storing details of the last search performed.
         */
        struct SearchDetails
        {
            QByteArray query;                           // the search term
            std::optional<::Z80::UnsignedWord> foundAt; // where it was found, if at all
        };

        /**
         * The memory being visualised.
         */
        BaseSpectrum::MemoryType * m_memory;

        /**
         * The last search term, and the address (if any) at which it was found.
         */
        std::optional<SearchDetails> m_lastSearch;
    };
}

#endif //SPECTRUM_QTUI_MEMORYVIEW_H
