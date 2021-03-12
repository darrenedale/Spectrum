#include "imagewidget.h"

#include <QImage>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QPainter>

using namespace Spectrum;

ImageWidget::ImageWidget(QWidget * parent)
:	ImageWidget({}, parent)
{
}

ImageWidget::ImageWidget(QImage image, QWidget * parent )
:	QWidget(parent),
	m_image(std::move(image))
{
}

void ImageWidget::setImage(QImage image)
{
	m_image = std::move(image);
	update();
}

void ImageWidget::resizeEvent(QResizeEvent * ev)
{
	ev->accept();
	update();
}

void ImageWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawImage(QRect(QPoint(0, 0), size()), m_image);
    painter.end();
}

ImageWidget::~ImageWidget() = default;
