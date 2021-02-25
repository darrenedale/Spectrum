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
			void redrawDisplay(const uint8_t *) override;

			int heightForWidth(int w) const override;
	};
}

#endif // QSPECTRUMDISPLAY_H
