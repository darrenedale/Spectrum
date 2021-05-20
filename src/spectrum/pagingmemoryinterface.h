//
// Created by darren on 19/04/2021.
//

#ifndef SPECTRUM_PAGINGMEMORYINTERFACE_H
#define SPECTRUM_PAGINGMEMORYINTERFACE_H

#include "types.h"

namespace Spectrum
{
    /**
     * An interface for RAM paging memory models.
     *
     * Provides for a number of RAM pages, and access to contiguous blocks of 16kb (PageSize) of storage for those pages.
     *
     * TODO consider templating this for page size to make it generic.
     */
    class PagingMemoryInterface
    {
    public:
        /**
         * Convenience constant for the size of a page of RAM.
         */
        static constexpr const int PageSize = 0x4000;

        /**
         * Convenience alias for a RAM page number.
         */
        using PageNumber = int;

        /**
         * How many pages of RAM does the memory have available to page in?
         *
         * @return The number of pages.
         */
        virtual inline PageNumber pageCount() const = 0;

        /**
         * Fetch a read-only pointer to the first byte of RAM for a page.
         *
         * The pointer returned must point to a contiguous block of 16kb (PageSize).
         */
        [[nodiscard]] virtual const Byte * pagePointer(PageNumber page) const = 0;

        /**
         * Fetch a read-write pointer to the first byte of RAM for a page.
         *
         * The pointer returned must point to a contiguous block of 16kb (PageSize).
         */
        virtual Byte * pagePointer(PageNumber page) = 0;

        /**
         * Copy the content of a RAM page.
         *
         * @param page The page to copy.
         * @param buffer The buffer into which to copy the page.
         * @param size The number of bytes to copy. Defaults to the whole page.
         * @param offset The offset into the page of the first byte to copy. offset + size must be <= PageSize. Defaults to 0.
         */
        virtual void readFromPage(PageNumber page, Byte * buffer, ::Z80::UnsignedWord size = PageSize, ::Z80::UnsignedWord offset = 0) const = 0;

        /**
         * Write directly into a RAM page.
         *
         * @param page The page to write into.
         * @param buffer The buffer containing the bytes to write. It must be at least size bytes in length.
         * @param size The number of bytes to write. Defaults to the whole page.
         * @param offset The offset into the page of the first byte to write. offset + size must be <= PageSize. Defaults to 0.
         */
        virtual void writeToPage(PageNumber page, const Byte * data, ::Z80::UnsignedWord size = PageSize, ::Z80::UnsignedWord offset = 0) = 0;
    };
}

#endif //SPECTRUM_PAGINGMEMORYINTERFACE_H
