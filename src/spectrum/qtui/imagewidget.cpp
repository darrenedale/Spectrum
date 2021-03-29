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
    int x = 0;
    int y = 0;
    int w = width();
    int h = height();

    if (m_keepAspectRatio) {
        auto imageRatio = static_cast<float>(m_image.width()) / static_cast<float>(m_image.height());
        auto widgetRatio = static_cast<float>(width()) / static_cast<float>(height());

        if (imageRatio > widgetRatio) {
            // image ratio is wider than widget ratio, adjust top
            auto scale = static_cast<float>(width()) / static_cast<float>(m_image.width());
            h = static_cast<int>(static_cast<float>(m_image.height()) * scale);
            y += (height() - h) / 2;
        } else {
            // image ratio is taller than widget ratio, adjust left
            auto scale = static_cast<float>(height()) / static_cast<float>(m_image.height());
            w = static_cast<int>(static_cast<float>(m_image.width()) * scale);
            x += (width() - w) / 2;
        }
    }

    painter.drawImage(QRect(x, y, w, h), m_image);
    painter.end();
}

void ImageWidget::setKeepAspectRatio(bool keep)
{
    m_keepAspectRatio = keep;
    update();
}

ImageWidget::~ImageWidget() = default;
