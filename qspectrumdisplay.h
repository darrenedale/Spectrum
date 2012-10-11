#ifndef QSPECTRUMDISPLAY_H
#define QSPECTRUMDISPLAY_H

#include "imagewidget.h"
#include "spectrumdisplaydevice.h"

#include <QRgb>

namespace Spectrum {
	class QSpectrumDisplay
	:	public ImageWidget,
		public SpectrumDisplayDevice {

		Q_OBJECT

		public:
			explicit QSpectrumDisplay( QWidget * parent = 0 );
			~QSpectrumDisplay( void );
			virtual void redrawDisplay( const unsigned char * displayMemory );

			virtual int heightForWidth( int w ) const {
				return (w * 192) / 256;
			}

		signals:

		public slots:

		private:
			static QRgb s_colourMap[16];
	};
}

#endif // QSPECTRUMDISPLAY_H
