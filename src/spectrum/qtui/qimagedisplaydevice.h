#ifndef QSPECTRUMDISPLAY_H
#define QSPECTRUMDISPLAY_H

#include <cstdint>

#include <QRgb>

#include "../displaydevice.h"

namespace Spectrum::QtUi
{
    /**
     * A Spectrum display device that renders the Spectrum display file to a QImage.
     *
     * This doesn't place anything on the screen, it simply plugs into an emulated spectrum as a DisplayDevice, and renders the Spectrum display memmory to a
     * QImage when the Spectrum asks it to.
     */
	class QImageDisplayDevice
	:   public DisplayDevice
	{
    public:
	    /**
	     * The number of pixels in the image to use for the border.
	     */
        static constexpr const int BorderSize = 32;

        /**
	     * Initialise a new display device.
	     */
        QImageDisplayDevice();

        /**
         * Fetch a read-write reference to the image that is having the Spectrum display rendered to it.
         *
         * @return The image.
         */
        QImage & image()
        {
            return m_image;
        }

        /**
         * Fetch a read-only reference to the image that is having the Spectrum display rendered to it.
         *
         * @return The image.
         */
        [[nodiscard]] const QImage & image() const
        {
            return m_image;
        }

        /**
         * Fetch the current border colour for the display.
         *
         * @return The border colour.
         */
        [[nodiscard]] Colour border() const override;

        /**
         * Set the border colour for the display.
         */
        void setBorder(Colour, bool = false) override;

        /**
         * Render the Spectrum display file to the image.
         *
         * @param displayMemory A pointer to the Spectrum display file.
         */
        void redrawDisplay(const uint8_t * displayMemory) override;

        /**
         * Set the device to render the display in black and white.
         *
         * The display will be rendered entirely in two colours - set pixels will be one colour, unset pixels another. The current frame will not be re-
         * rendered, the next redraw will be rendered in black-and-white.
         *
         * @param foreground The colour to use for set (foreground) pixels.
         * @param background The colour to use for unset (background) pixels.
         */
        void setBlackAndWhite(const QColor & foreground = DefaultBlackAndWhiteForeground, const QColor & background = DefaultBlackAndWhiteBackground)
        {
            m_colourMode = ColourMode::BlackAndWhite;
            m_bwForeground = foreground.rgb();
            m_bwBackground = background.rgb();
        }

        /**
         * Set the device to render the display in monochrome.
         *
         * The display will be rendered entirely in shades of grey. The current frame will not be re-rendered, the next redraw will be rendered in monochrome.
         */
        void setMonochrome()
        {
            m_colourMode = ColourMode::Monochrome;
        }

        /**
         * Set the device to render the display in full colour.
         *
         * The display will be rendered according to the standard Spectrum colour palette. The current frame will not be re-rendered, the next redraw will be
         * rendered in monochrome.
         */
        void setColour()
        {
            m_colourMode = ColourMode::Colour;
        }

        /**
         * The full width of the image used to render the Spectrum display.
         *
         * This is equivalent to a single image pixel for each Spectrum screen pixel, plus 32 pixels each for the left and right borders.
         *
         * @return The image width.
         */
        static constexpr int fullWidth()
        {
            return Width + 2 * BorderSize;
        }

        /**
         * The full height of the image used to render the Spectrum display.
         *
         * This is equivalent to a single image pixel for each Spectrum screen pixel, plus 32 pixels each for the top and bottom borders.
         *
         * @return The image width.
         */
        static constexpr int fullHeight()
        {
            return Height + 2 * BorderSize;
        }

    protected:
	    /**
	     * The default foreground to use for black-and-white rendering.
	     */
        static constexpr const QRgb DefaultBlackAndWhiteForeground = 0xff000000;

        /**
         * The default background to use for black-and-white rendering.
         */
        static constexpr const QRgb DefaultBlackAndWhiteBackground = 0xffcdcdcd;

	private:
	    /**
	     * Enumerates the available colour rendering modes.
	     */
	    enum class ColourMode : std::uint8_t
        {
	        Colour = 0,
	        Monochrome = 1,
	        BlackAndWhite = 2,
        };

	    /**
	     * The image used to render the screen.
	     */
	    QImage m_image;

	    /**
	     * The current border colour.
	     */
	    Colour m_border;

	    /**
	     * The number of frames rendered.
	     *
	     * This is used to handle the flash attribute for 8x8 blocks of the display. This depends on the device being asked to render every frame of the
	     * Spectrum display.
	     */
	    std::uint8_t m_frameCounter;

	    /**
	     * The current colour mode.
	     */
	    ColourMode m_colourMode;

	    /**
	     * The current foreground colour to use for black-and-white rendering.
	     */
	    QRgb m_bwForeground;

        /**
         * The current background colour to use for black-and-white rendering.
         */
        QRgb m_bwBackground;
    };
}

#endif // QSPECTRUMDISPLAY_H
