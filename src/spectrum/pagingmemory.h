//
// Created by darren on 17/04/2021.
//

#ifndef SPECTRUM_PAGINGMEMORY_H
#define SPECTRUM_PAGINGMEMORY_H

#include <array>
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstring>
#include "pagedmemoryinterface.h"
#include "../memory.h"
#include "../z80/types.h"

namespace Spectrum
{
    /**
     * Abstract Base class for the paging memory model used in later Spectrums.
     *
     * Each ROM and page of RAM is 16kb in size. In the default setup, the bottom 16kb of the 64kb address space contains one of the ROMs, while the top 16kb
     * contains one of the RAM pages. The address space between 0x4000 and 0x7fff inclusive always contains page 5; the space between 0x8000 and 0xbfff
     * inclusive always contains page 2. For other paging models, reimplement mapAddress() - see the special paging mode of MemoryPlus2a for an example.
     *
     * Implements PagedMemoryInterface.
     *
     * @tparam NumRoms The number of ROMs available to be paged in. Must be > 0.
     * @tparam NumPages The number of pages of RAM available. Must be >= 6; defaults to 8.
     */
    template<int NumRoms, int NumPages = 8>
    class PagingMemory
    : public Memory<::Z80::UnsignedByte>,
      public PagedMemoryInterface
    {
        // must have at least one ROM and six pages of RAM - why 6? because page #5 is always mapped to 0x4000 - 0x7fff so that page must be present
        static_assert(0 < NumRoms);
        static_assert(6 < NumPages);

    public:
        /**
         * Convenience constant for the size of a ROM.
         */
        static constexpr const int RomSize = 0x4000;

        /**
         * Convenience constant for the number of ROMs available in the memory model.
         */
        static constexpr const int RomCount = NumRoms;

        /**
         * Convenience constant for the number of pages of RAM available in the memory model.
         */
        static constexpr const int PageCount = NumPages;

        /**
         * Convenience alias for the internal storage type.
         */
        using PageStorage = std::array<Byte, PageSize>;

        /**
         * Convenience alias for a ROM number.
         *
         * TODO consider making a strong type for this.
         */
        using RomNumber = int;

        /**
         * Initialise a new PagingMemory object.
         *
         * @param addressableSize Size of the address space, defaults to 64kb (16-bit Z80 address space).
         */
        explicit PagingMemory(Size addressableSize = 0x10000)
        : Memory(addressableSize, {}),
          m_romNumber(0),
          m_pagedRam(0),
          m_roms(),
          m_ramPages()
        {}

        /**
         * Copy construction is disabled.
         */
        PagingMemory(const PagingMemory &) = delete;

        /**
         * Move construction is disabled.
         */
        PagingMemory(PagingMemory &&) = delete;

        /**
         * Copy assignment is disabled.
         */
        void operator=(const PagingMemory &) = delete;

        /**
         * Move assignment is disabled.
         */
        void operator=(PagingMemory &&) = delete;

        /**
         * Destructor.
         */
        ~PagingMemory() override = default;

        [[nodiscard]] std::unique_ptr<Memory> clone() const override = 0;

        /**
         * Fetch the number of ROMs the memory has available.
         *
         * @return
         */
        constexpr RomNumber romCount() const
        {
            return RomCount;
        }

        /**
         * Fetch the number of RAM pages the memory has available.
         *
         * @return
         */
        constexpr PageNumber pageCount() const override
        {
            return PageCount;
        }

        /**
         * Clear the memory.
         *
         * All ROMs and all pages are filled with 0 bytes.
         */
        void clear() override
        {
            for (auto & rom : m_roms) {
                rom.fill(0);
            }

            for (auto & page : m_ramPages) {
                page.fill(0);
            }
        }

        /**
         * Determine the currently paged-in ROM.
         *
         * This is only relevant if the paging mode is Normal. In Special paging mode, the ROM is not used.
         *
         * @return
         */
        [[nodiscard]] inline RomNumber currentRom() const
        {
            return m_romNumber;
        }

        /**
         * Determine the currently paged-in RAM page.
         *
         * This is only relevant if the paging mode is Normal. In Special paging mode, the paged-in pages are determined
         * by the special paging mode configuration.
         *
         * @return
         */
        [[nodiscard]] inline PageNumber currentRamPage() const
        {
            return m_pagedRam;
        }

        /**
         * Page in the specified ROM.
         *
         * The specified ROM will be paged in when the paging mode is Normal. While the paging mode is Special the
         * configured special paging setup will be in effect.
         *
         * @param rom
         */
        inline void pageRom(RomNumber rom)
        {
            assert(0 <= rom && rom < RomCount);
            m_romNumber = rom;
        }

        /**
         * Page in the specified RAM page.
         *
         * The specified page will be paged in when the paging mode is Normal. While the paging mode is Special the
         * configured special paging setup will be in effect.
         *
         * @param page
         */
        inline void pageRam(PageNumber page)
        {
            assert(0 <= page && page < PageCount);
            m_pagedRam = page;
        }

        /**
         * Fetch a const pointer to the fist byte in the specified page of RAM.
         *
         * @return
         */
        [[nodiscard]] const Byte * pagePointer(PageNumber page) const override
        {
            assert(0 <= page && page < PageCount);
            return m_ramPages[page].data();
        }

        /**
         * Fetch a pointer to the fist byte in the specified page of RAM.
         *
         * @return
         */
        Byte * pagePointer(PageNumber page) override
        {
            assert(0 <= page && page < PageCount);
            return m_ramPages[page].data();
        }

        /**
         * Fetch a const pointer to the fist byte in the currently paged-in page of RAM.
         *
         * @return
         */
        [[nodiscard]] inline const Byte * currentRamPagePointer() const
        {
            return pagePointer(currentRamPage());
        }

        /**
         * Fetch a pointer to the fist byte in the currently paged-in page of RAM.
         *
         * @return
         */
        inline Byte * currentRamPagePointer()
        {
            return pagePointer(currentRamPage());
        }

        /**
         * Fetch a const pointer to the fist byte in the specified ROM.
         *
         * @return
         */
        [[nodiscard]] const Byte * romPointer(RomNumber rom) const
        {
            return m_roms[rom].data();
        }

        /**
         * Fetch a pointer to the fist byte in the specified ROM.
         *
         * @return
         */
        Byte * romPointer(RomNumber rom)
        {
            return m_roms[rom].data();
        }

        /**
        * Fetch a const pointer to the fist byte in the currently paged-in ROM.
        *
        * @return
        */
        [[nodiscard]] inline const Byte * currentRomPointer() const
        {
            return romPointer(currentRom());
        }

        /**
         * Fetch a pointer to the fist byte in the currently paged-in ROM.
         *
         * @return
         */
        inline Byte * currentRomPointer()
        {
            return romPointer(currentRom());
        }
        
        /**
         * Load the content of a file into the specified ROM.
         *
         * @param fileName The file containing the ROM image.
         * @param rom The ROM number to load into. Must be 0 <= rom < RomCount.
         * @return
         */
        bool loadRom(const std::string & fileName, RomNumber rom = 0)
        {
            assert(0 <= rom && rom < RomCount);
            std::ifstream in(fileName, std::ios::binary | std::ios::in);

            if (!in) {
                std::cerr << "failed to open ROM image file \"" << fileName << "\"\n";
                return false;
            }

            in.read(reinterpret_cast<std::ifstream::char_type *>(m_roms[rom].data()), RomSize);

            if (in.fail() && !in.eof()) {
                std::cerr << "failed to read ROM image file \"" << fileName << "\"\n";
                return false;
            }

            return true;
        }

        /**
         * Read directly from a specified memory page.
         *
         * Useful when writing snapshots, for example.
         *
         * @param page The page to read from. Must be 0 <= page < PageCount.
         * @param data The buffer into which to read the data. Must be big enough to hold size bytes.
         * @param size The number of bytes to read. Defaults to a full page. Must not extend beyond the size of a page.
         * @param offset The offset into the page at which to start reading. Defaults to the first byte in the page.
         */
        void readFromPage(PageNumber page, Byte * buffer, ::Z80::UnsignedWord size = PageSize, ::Z80::UnsignedWord offset = 0) const override
        {
            assert(0 <= page && page < PageCount);
            assert(offset + size <= PageSize);
            std::memcpy(buffer, m_ramPages[page].data() + offset, size);
        }

        /**
         * Write directly into a specified memory page.
         *
         * Useful when reading snapshots, for example.
         *
         * @param page The page to write to. Must be 0 <= page < PageCount.
         * @param data The data to write
         * @param size The number of bytes to write. Defaults to a full page. Must not extend beyond the size of a page.
         * @param offset The offset into the page at which to start writing. Defaults to the first byte in the page.
         */
        void writeToPage(PageNumber page, const Byte * data, ::Z80::UnsignedWord size = PageSize, ::Z80::UnsignedWord offset = 0) override
        {
            assert(0 <= page && page < PageCount);
            assert(offset + size <= PageSize);
            std::memcpy(m_ramPages[page].data() + offset, data, size);
        }

    protected:
        /**
         * The (emulated) address of the first byte of ROM.
         */
        static constexpr const ::Z80::UnsignedWord RomBase = 0x0000;

        /**
         * The (emulated) address of the last byte of ROM.
         */
        static constexpr const ::Z80::UnsignedWord RomTop = RomBase + RomSize - 1;

        /**
         * The (emulated) address of the first byte of pageable RAM.
         */
        static constexpr const ::Z80::UnsignedWord PagedRamBase = 0xc000;

        /**
         * The (emulated) address of the last byte of pageable RAM.
         */
        static constexpr const ::Z80::UnsignedWord PagedRamTop = PagedRamBase + PageSize - 1;

        /**
         * The (emulated) address of the first byte of the fixed location of RAM page 5.
         */
        static constexpr const ::Z80::UnsignedWord Page5Base = 0x4000;

        /**
         * The (emulated) address of the last byte of the fixed location of RAM page 5.
         */
        static constexpr const ::Z80::UnsignedWord Page5Top = Page5Base + PageSize - 1;

        /**
         * The (emulated) address of the first byte of the fixed location of RAM page 2.
         */
        static constexpr const ::Z80::UnsignedWord Page2Base = 0x8000;

        /**
         * The (emulated) address of the last byte of the fixed location of RAM page 2.
         */
        static constexpr const ::Z80::UnsignedWord Page2Top = Page2Base + PageSize - 1;

        /**
         * Map an emulated memory address to a physical address inside one of the ROMs/RAM banks.
         *
         * The default implementation follows the paging scheme outlined in the class docs above.
         *
         * @param address The emulated address to map. Must not exceed the address space.
         *
         * @return
         */
        [[nodiscard]] Byte * mapAddress(Address address) const override
        {
            assert(address <= 0xffff);
            
            if (address <= RomTop) {
                return const_cast<Byte *>(currentRomPointer() + address);
            }

            if (address >= PagedRamBase) {
                return const_cast<Byte *>(currentRamPagePointer() + address - PagedRamBase);
            }

            if (address >= Page2Base) {
                return const_cast<Byte *>(m_ramPages[2].data() + address - Page2Base);
            }

            if (address >= Page5Base) {
                return const_cast<Byte *>(m_ramPages[5].data() + address - Page5Base);
            }

            // unreachable code
            assert(false);
        }

    protected:
        /**
         * The currently paged-in ROM.
         */
        RomNumber m_romNumber;

        /**
         * Storage for the ROMs.
         */
        std::array<PageStorage, RomCount> m_roms;

        /**
         * The currently paged-in RAM page.
         */
        PageNumber m_pagedRam;

        /**
         * Storage for the RAM pages.
         */
        std::array<PageStorage, PageCount> m_ramPages;
    };
}

#endif //SPECTRUM_PAGINGMEMORY_H
