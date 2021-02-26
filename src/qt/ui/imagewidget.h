#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>

class QImage;
class QResizeEvent;
class QPaintEvent;

class ImageWidget
:	public QWidget
{
	Q_OBJECT

	public:
		explicit ImageWidget(QWidget * parent = nullptr);
		explicit ImageWidget(QImage, QWidget * parent = nullptr);
		~ImageWidget() override;

		inline const QImage & image() const
		{
			return m_image;
		}

		inline QImage & image()
		{
			return m_image;
		}

		void setImage(const QImage &);

	protected:
		void resizeEvent(QResizeEvent *) override;
		void paintEvent(QPaintEvent *) override;

	private:
		QImage m_image;
};

#endif // IMAGEWIDGET_H
