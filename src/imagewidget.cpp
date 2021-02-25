#include "imagewidget.h"

#include <QImage>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QPainter>

ImageWidget::ImageWidget( QWidget * parent )
:	ImageWidget(QImage(), parent)
{
}

ImageWidget::ImageWidget(QImage image, QWidget * parent )
:	QWidget(parent),
	m_image(image)
{
}

void ImageWidget::setImage(const QImage & image)
{
	m_image = image;
	update();
}

void ImageWidget::resizeEvent(QResizeEvent * ev)
{
	ev->accept();
	update();
}

void ImageWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawImage(QRect(QPoint(0, 0), size()), m_image);
    p.end();
}

ImageWidget::~ImageWidget() = default;
