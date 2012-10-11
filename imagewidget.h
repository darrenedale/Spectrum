#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>

class QImage;
class QResizeEvent;
class QPaintEvent;

class ImageWidget
:	public QWidget {

	Q_OBJECT

	public:
		explicit ImageWidget( QWidget * parent = 0 );
		ImageWidget( QImage * i, QWidget * parent = 0 );
		~ImageWidget();

		inline QImage * image( void ) {
			return m_image;
		}

		void setImage( QImage * i );

	protected:
		void resizeEvent( QResizeEvent * );
		void paintEvent( QPaintEvent * );

	private:
		QImage * m_image;
};

#endif // IMAGEWIDGET_H
