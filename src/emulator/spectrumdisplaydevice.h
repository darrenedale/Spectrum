#ifndef SPECTRUMDISPLAYDEVICE_H
#define SPECTRUMDISPLAYDEVICE_H

#include <cstdint>

namespace Spectrum
{
    /**
     * Interface for display devices for Spectrums.
     */
	class SpectrumDisplayDevice
	{
    public:
        enum class Colour: std::uint8_t
        {
            Black = 0,
            Blue,
            Red,
            Magenta,
            Green,
            Cyan,
            Yellow,
            White,
        };

        virtual void setBorder(Colour, bool bright = false) = 0;
        virtual void redrawDisplay(const uint8_t * displayMemory) = 0;

	protected:
	    typedef std::uint8_t Attribute;

	    // some useful constants for the default Spectrum DisplayFile
	    static constexpr const int Width = 256;
	    static constexpr const int Height = 192;
	    static constexpr const int AttributesOffset = 0x1800;
	    static constexpr const int AttributeInkMask = 0b00000111;
	    static constexpr const int AttributePaperMask = 0b00111000;
	    static constexpr const int AttributeBrightMask = 0b01000000;
	    static constexpr const int AttributeFlashMask = 0b10000000;

        constexpr bool isBright(Attribute attr)
        {
            return attr & AttributeBrightMask;
        }

        constexpr bool isFlashing(Attribute attr)
        {
            return attr & AttributeFlashMask;
        }

        constexpr Colour inkColour(Attribute attr)
        {
            return static_cast<Colour>(attr & AttributeInkMask);
        }

        constexpr Colour paperColour(Attribute attr)
        {
            return static_cast<Colour>((attr & AttributePaperMask) >> 3);
        }
	};
}

#endif // SPECTRUMDISPLAYDEVICE_H
