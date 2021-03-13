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

        void setBorder(Colour, bool = false) override;
        void redrawDisplay(const uint8_t *) override;

	protected:
        static constexpr int fullWidth();
        static constexpr int fullHeight();

	private:
	    QImage m_image;
	    std::uint8_t m_frameCounter;
    };
}

#endif // QSPECTRUMDISPLAY_H
