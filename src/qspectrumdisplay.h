#ifndef QSPECTRUMDISPLAY_H
#define QSPECTRUMDISPLAY_H

#include <cstdint>

#include <QRgb>

#include "imagewidget.h"
#include "spectrumdisplaydevice.h"


namespace Spectrum
{
	class QSpectrumDisplay
	:	public ImageWidget,
		public SpectrumDisplayDevice
	{
		Q_OBJECT

		public:
			explicit QSpectrumDisplay(QWidget * = nullptr);
			~QSpectrumDisplay() override;

			void setImage(const QImage &) = delete;
			void setBorder(Colour, bool = false) override;
			void redrawDisplay(const uint8_t *) override;

			int heightForWidth(int w) const override;

	protected:
        static constexpr int fullWidth();
        static constexpr int fullHeight();
    };
}

#endif // QSPECTRUMDISPLAY_H
