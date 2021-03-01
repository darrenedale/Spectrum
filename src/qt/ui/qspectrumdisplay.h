#ifndef QSPECTRUMDISPLAY_H
#define QSPECTRUMDISPLAY_H

#include <cstdint>

#include <QRgb>

#include "../../emulator/spectrumdisplaydevice.h"

namespace Spectrum
{
	class QSpectrumDisplay
	:	public QObject,
	    public SpectrumDisplayDevice
	{
    Q_OBJECT

    public:
        explicit QSpectrumDisplay(QObject * owner = nullptr);
        ~QSpectrumDisplay() override;

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

    Q_SIGNALS:
	    void displayUpdated(const QImage &);

	protected:
        static constexpr int fullWidth();
        static constexpr int fullHeight();

	private:
	    QImage m_image;
    };
}

#endif // QSPECTRUMDISPLAY_H
