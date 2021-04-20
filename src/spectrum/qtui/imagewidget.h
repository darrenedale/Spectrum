#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>

class QImage;
class QResizeEvent;
class QPaintEvent;

namespace Spectrum
{
    class ImageWidget
    : public QWidget
    {
    Q_OBJECT

    public:
        explicit ImageWidget(QWidget * parent = nullptr);
        explicit ImageWidget(QImage, QWidget * parent = nullptr);
        ~ImageWidget() override;

        [[nodiscard]] inline const QImage & image() const
        {
            return m_image;
        }

        inline QImage & image()
        {
            return m_image;
        }

        void setImage(QImage);

        void setKeepAspectRatio(bool keep);

        inline void keepAspectRatio()
        {
            setKeepAspectRatio(true);
        }

        inline void ignoreAspectRatio()
        {
            setKeepAspectRatio(false);
        }

        [[nodiscard]] inline bool aspectRatioKept() const
        {
            return m_keepAspectRatio;
        }

        [[nodiscard]] inline bool aspectRatioIgnored() const
        {
            return !aspectRatioKept();
        }

    protected:
        /**
         * Fetch the rectangle into which to render the image.
         *
         * This will be The same as the widget rect unless the aspect ratio is being preserved.
         *
         * @return The render rectangle.
         */
        QRect renderRect() const;

        void resizeEvent(QResizeEvent *) override;
        void paintEvent(QPaintEvent *) override;

    private:
        QImage m_image;
        bool m_keepAspectRatio;
    };
}

#endif // IMAGEWIDGET_H
