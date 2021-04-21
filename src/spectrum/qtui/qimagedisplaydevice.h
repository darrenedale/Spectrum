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
     * This doesn't place anything on the screen, it simply plugs into an emulated spectrum as a DisplayDevice, and renders the Spectrum display memory to a
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
         *
         * If frameSkip is provided, the device will render this many many frames before skipping one, unless it's 0 in which case every frame will be rendered.
         * In most cases there is little value in setting this since modern hardware can easily handle rendering every Spectrum frame. However, it can be used
         * to achieve a performance gain if emulation is slow.
         *
         * @param frameSkip The number of frames to render before skipping one. Must be >= 0. 0 (the default) means every frame will be rendered.
	     */
        explicit QImageDisplayDevice(int frameSkip = 0);

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
         * Request the display to be redrawn.
         *
         * The display will be redrawn if the frame rate is unlimited, or it's limited but we've reached the time point for the next frame.
         *
         * @param displayMemory A pointer to the Spectrum display file.
         */
        void redrawDisplay(const uint8_t * displayMemory) override;

        /**
         * Actually render a frame.
         *
         * The rendering occurs regardless of any frame rate limiting.
         *
         * This is public so that, for example, the UI can present the user with an option to refresh the display while paused or in debug mode after a step,
         * etc.
         *
         * @param displayMemory
         */
        void renderFrame(const uint8_t * displayMemory);

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
         * Set the number of frames that are rendered before one is skipped.
         *
         * In other words, one in every (n + 1) frames is skipped.
         *
         * This can improve performance if the emulation is slow, but is unlikely to be necessary on any common hardware these days. Set it to 0 to render
         * every frame the Spectrum generates.
         *
         * @param frameSkip The number of frames to skip. Must be >= 0.
         */
        void setFrameSkip(int frameSkip)
        {
            assert (0 <= frameSkip);
            m_frameSkip = frameSkip + 1;
        }

        /**
         * Fetch the number of frames that are rendered before one is skipped.
         *
         * In other words, one in every (n + 1) frames is skipped.
         *
         * If this is 0, then no frames are skipped.
         *
         * @return The number of frames.
         */
        [[nodiscard]] int frameSkip() const
        {
            return m_frameSkip - 1;
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
	    using clock = std::chrono::steady_clock;

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
	     * How many frames to render before skipping one.
	     *
	     * What is actually stored is frameSkip + 1 so that we don't need to continually add one to it when checking whether the frame should be rendered.
	     */
	    std::uint8_t m_frameSkip;

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
