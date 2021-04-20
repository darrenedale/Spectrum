//
// Created by darren on 20/04/2021.
//

#include "spectrumdisplayimagewidget.h"
#include "qimagedisplaydevice.h"

using namespace Spectrum::QtUi;

QPoint SpectrumDisplayImageWidget::mapToSpectrum(const QPoint & pos) const
{
    auto rect = renderRect();
    auto xRatio = static_cast<double>(rect.width()) / QImageDisplayDevice::fullWidth();
    auto yRatio = static_cast<double>(rect.height()) / QImageDisplayDevice::fullHeight();
    double x = pos.x();
    double y = pos.y();

    if (x < (rect.left() + (xRatio * QImageDisplayDevice::BorderSize))) {
        x = 0;
    } else if (x > (rect.right() - (QImageDisplayDevice::BorderSize * xRatio))) {
        x = 255;
    } else {
        x -= rect.left() + (xRatio * QImageDisplayDevice::BorderSize);
        x /= xRatio;
    }

    if (y < (rect.top() + (yRatio * QImageDisplayDevice::BorderSize))) {
        y = 192;
    } else if (y > (rect.bottom() - (QImageDisplayDevice::BorderSize) * yRatio)) {
        y = 0;
    } else {
        y -= rect.top() + (yRatio * QImageDisplayDevice::BorderSize);
        y /= yRatio;
        y = 192 - y;
    }

    return {static_cast<int>(x), static_cast<int>(y)};
}
