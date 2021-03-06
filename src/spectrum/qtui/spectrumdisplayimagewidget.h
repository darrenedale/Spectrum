//
// Created by darren on 20/04/2021.
//

#ifndef SPECTRUM_SPECTRUMDISPLAYIMAGEWIDGET_H
#define SPECTRUM_SPECTRUMDISPLAYIMAGEWIDGET_H

#include "imagewidget.h"

namespace Spectrum::QtUi
{
    /**
     * A customisation of the generic ImageWidget specifically for rendering images generated by QImageDisplayDevice.
     */
    class SpectrumDisplayImageWidget
    : public ImageWidget
    {
        Q_OBJECT

    public:
        using ImageWidget::ImageWidget;

        [[nodiscard]] QPoint mapToSpectrum(const QPoint & pos) const;

    private:

    };
}

#endif //SPECTRUM_SPECTRUMDISPLAYIMAGEWIDGET_H
