#include "imagewidget.h"

#include <QImage>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QPainter>


ImageWidget::ImageWidget( QWidget * parent )
:	QWidget(parent),
	m_image(0) {
}


ImageWidget::ImageWidget( QImage * i, QWidget * parent )
:	QWidget(parent),
	m_image(i) {
}


ImageWidget::~ImageWidget() {
	if(m_image) delete m_image;
	m_image = 0;
}


void ImageWidget::setImage( QImage * i ) {
	m_image = i;
	update();
}


void ImageWidget::resizeEvent( QResizeEvent * ev ) {
	ev->accept();
	update();
}


void ImageWidget::paintEvent( QPaintEvent * ev ) {
	Q_UNUSED(ev);

	if(m_image) {
		QPainter p(this);
		p.drawImage(QRect(QPoint(0, 0), size()), *m_image);
		p.end();
	}
}
