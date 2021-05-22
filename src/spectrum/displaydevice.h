#ifndef SPECTRUM_DISPLAYDEVICE_H
#define SPECTRUM_DISPLAYDEVICE_H

#include <span>
#include <cstdint>
#include "types.h"
#include "../z80/types.h"
#include "../z80/iodevice.h"

namespace Spectrum
{
    /**
     * Interface for display devices for Spectrums.
     *
     * Display devices only use Z80 IO to set the border colour. All other output is handled by direct memory access to
     * the Spectrum display file. In the context of this emulator, the Spectrum object emulates the ULA interrupt that
     * times display updates and calls redrawDisplay() with the appropriate memory pointer.
     */
	class DisplayDevice
    : public ::Z80::IODevice
	{
    public:
        /**
         * Display devices are output-only devices.
         *
         * @param port
         * @return
         */
        bool checkReadPort(::Z80::UnsignedWord port) const override
        {
            return false;
        }

        /**
         * As per all ULA functions, display devices accept data on all even ports.
         *
         * @param port
         * @return
         */
        bool checkWritePort(::Z80::UnsignedWord port) const override
        {
            return !(port & 0x01);
        }

        /**
         * Display devices are write-only.
         *
         * @param port
         * @return
         */
        Z80::UnsignedByte readByte(::Z80::UnsignedWord port) override
        {
            return 0;
        }

        /**
         * Bits 0, 1 and 2 set the display border colour.
         *
         * @param port
         * @param value
         */
        void writeByte(::Z80::UnsignedWord port, ::Z80::UnsignedByte value) override
        {
            setBorder(static_cast<Colour>(value & 0b00000111));
        }

        /**
         * Fetch the current border colour.
         *
         * @return The border colour.
         */
        virtual Colour border() const = 0;

        /**
         * Set the border colour.
         *
         * @param colour The border colour.
         * @param bright Whether the bright (true) or normal (false) version of the colour should be used.
         */
        virtual void setBorder(Colour colour, bool bright = false) = 0;

        /**
         * Render the Spectrum's display file.
         *
         * @param displayMemory 6912 bytes representing the Spectrum's display file.
         */
        virtual void redrawDisplay(const DisplayFile & displayMemory) = 0;

	protected:
	    /**
	     * Alias for an attribute byte.
	     */
	    using Attribute = std::uint8_t;

	    // some useful constants for the default Spectrum DisplayFile
	    static constexpr const int Width = 256;
	    static constexpr const int Height = 192;
	    static constexpr const int AttributesOffset = 0x1800;
	    static constexpr const int AttributeInkMask = 0b00000111;
	    static constexpr const int AttributePaperMask = 0b00111000;
	    static constexpr const int AttributeBrightMask = 0b01000000;
	    static constexpr const int AttributeFlashMask = 0b10000000;

	    /**
	     * Helper to determine whether an attribute byte from the Spectrum display file includes the Bright flag.
	     *
	     * @param attr The attribute to check.
	     *
	     * @return True if the attribute has the bright bit set, false otherwise.
	     */
        constexpr bool isBright(Attribute attr)
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
        constexpr bool isFlashing(Attribute attr)
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
        constexpr Colour inkColour(Attribute attr)
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
        constexpr Colour paperColour(Attribute attr)
        {
            return static_cast<Colour>((attr & AttributePaperMask) >> 3);
        }
	};
}

#endif // SPECTRUM_DISPLAYDEVICE_H
