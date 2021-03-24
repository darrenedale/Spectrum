#ifndef QSPECTRUMDISPLAY_H
#define QSPECTRUMDISPLAY_H

#include <cstdint>

#include <QRgb>

#include "../displaydevice.h"

namespace Spectrum::Qt
{
	class QImageDisplayDevice
	:   public DisplayDevice
	{
    public:
        QImageDisplayDevice();

        QImage & image()
        {
            return m_image;
        }

        [[nodiscard]] const QImage & image() const
        {
            return m_image;
        }

        [[nodiscard]] Colour border() const override;
        void setBorder(Colour, bool = false) override;
        void redrawDisplay(const uint8_t *) override;

        void setBlackAndWhite(const QColor & foreground = DefaultBlackAndWhiteForeground, const QColor & background = DefaultBlackAndWhiteBackground)
        {
            m_colourMode = ColourMode::BlackAndWhite;
            m_bwForeground = foreground.rgb();
            m_bwBackground = background.rgb();
        }

        void setMonochrome()
        {
            m_colourMode = ColourMode::Monochrome;
        }

        void setColour()
        {
            m_colourMode = ColourMode::Colour;
        }

    protected:
        static constexpr const QRgb DefaultBlackAndWhiteForeground = 0xff000000;
        static constexpr const QRgb DefaultBlackAndWhiteBackground = 0xffcdcdcd;

        static constexpr int fullWidth();
        static constexpr int fullHeight();

	private:
	    enum class ColourMode : std::uint8_t
        {
	        Colour = 0,
	        Monochrome = 1,
	        BlackAndWhite = 2,
        };

	    QImage m_image;
	    Colour m_border;
	    std::uint8_t m_frameCounter;
	    ColourMode m_colourMode;
	    QRgb m_bwForeground;
        QRgb m_bwBackground;
    };
}

#endif // QSPECTRUMDISPLAY_H
