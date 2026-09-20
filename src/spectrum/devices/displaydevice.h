#ifndef SPECTRUM_DISPLAYDEVICE_H
#define SPECTRUM_DISPLAYDEVICE_H

#include <cstdint>

#include "../types.h"
#include "../../z80/types.h"
#include "../../z80/iodevice.h"

namespace Spectrum::Devices
{
    namespace BaseZ80 = Z80;
    using BaseZ80::UnsignedByte;
    using BaseZ80::UnsignedWord;

    /**
     * Interface for display devices for Spectrums.
     *
     * Display devices only use Z80 IO to set the border colour. All other output is handled by direct memory access to
     * the Spectrum display file. In the context of this emulator, the Spectrum object emulates the ULA interrupt that
     * times display updates and calls redrawDisplay() with the appropriate memory pointer.
     */
	class DisplayDevice
    : public BaseZ80::IODevice
	{
    public:
        /**
         * Display devices are output-only devices.
         *
         * @param port
         * @return
         */
        [[nodiscard]]
	    bool checkReadPort(UnsignedWord port) const override
        {
            return false;
        }

        /**
         * As per all ULA functions, display devices accept data on all even ports.
         *
         * @param port
         * @return
         */
	    [[nodiscard]]
        bool checkWritePort(const UnsignedWord port) const override
        {
            return !(port & 0x01);
        }

        /**
         * Display devices are write-only.
         *
         * @param port
         * @return
         */
        Z80::UnsignedByte readByte(UnsignedWord port) override
        {
            return 0;
        }

        /**
         * Bits 0, 1 and 2 set the display border colour.
         *
         * @param port
         * @param value
         */
        void writeByte(UnsignedWord port, const UnsignedByte value) override
        {
            setBorder(static_cast<Colour>(value & 0b00000111), false);
        }

        /**
         * Fetch the current border colour.
         *
         * @return The border colour.
         */
	    [[nodiscard]]
        virtual Colour border() const noexcept = 0;

        /**
         * Set the border colour.
         *
         * @param colour The border colour.
         * @param bright Whether the bright (true) or normal (false) version of the colour should be used.
         */
        virtual void setBorder(Colour colour, bool bright) noexcept = 0;

        /**
         * Render the Spectrum's display file.
         *
         * @param displayMemory 6912 bytes representing the Spectrum's display file.
         */
        virtual void redrawDisplay(const DisplayFile & displayMemory) = 0;

	protected:
	    /** Alias for an attribute byte. */
	    using Attribute = std::uint8_t;

	    // some useful constants for the default Spectrum DisplayFile
	    static constexpr int Width = 256;
	    static constexpr int Height = 192;
	    static constexpr int AttributesOffset = 0x1800;
	    static constexpr int AttributeInkMask = 0b00000111;
	    static constexpr int AttributePaperMask = 0b00111000;
	    static constexpr int AttributeBrightMask = 0b01000000;
	    static constexpr int AttributeFlashMask = 0b10000000;

	    /**
	     * Helper to determine whether an attribute byte from the Spectrum display file includes the Bright flag.
	     *
	     * @param attr The attribute to check.
	     *
	     * @return True if the attribute has the bright bit set, false otherwise.
	     */
        static constexpr bool isBright(const Attribute attr) noexcept
        {
            return attr & AttributeBrightMask;
        }

        /**
         * Helper to determine whether an attribute byte from the Spectrum display file includes the Flash flag.
         *
         * @param attr The attribute to check.
         *
         * @return True if the attribute has the flash bit set, false otherwise.
         */
        static constexpr bool isFlashing(const Attribute attr) noexcept
        {
            return attr & AttributeFlashMask;
        }

        /**
         * Helper to extract the ink colour from an attribute byte.
         *
         * @param attr The attribute byte.
         *
         * @return The ink colour.
         */
        static constexpr Colour inkColour(const Attribute attr) noexcept
        {
            return static_cast<Colour>(attr & AttributeInkMask);
        }

        /**
         * Helper to extract the paper colour from an attribute byte.
         *
         * @param attr The attribute byte.
         *
         * @return The paper colour.
         */
        static constexpr Colour paperColour(const Attribute attr) noexcept
        {
            return static_cast<Colour>((attr & AttributePaperMask) >> 3);
        }
	};
}

#endif // SPECTRUM_DISPLAYDEVICE_H
