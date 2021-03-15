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

        const QImage & image() const
        {
            return m_image;
        }

        [[nodiscard]] Colour border() const override;
        void setBorder(Colour, bool = false) override;
        void redrawDisplay(const uint8_t *) override;

	protected:
        static constexpr int fullWidth();
        static constexpr int fullHeight();

	private:
	    QImage m_image;
	    Colour m_border;
	    std::uint8_t m_frameCounter;
    };
}

#endif // QSPECTRUMDISPLAY_H
