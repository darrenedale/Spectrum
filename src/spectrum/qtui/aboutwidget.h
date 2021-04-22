//
// Created by darren on 22/04/2021.
//

#ifndef SPECTRUM_QTUI_ABOUTWIDGET_H
#define SPECTRUM_QTUI_ABOUTWIDGET_H

#include <QWidget>

namespace Spectrum::QtUi
{
    class AboutWidget
    : public QWidget
    {
        Q_OBJECT

    public:
        explicit AboutWidget(QWidget * parent = nullptr);
        AboutWidget(const AboutWidget &) = delete;
        AboutWidget(AboutWidget &&) = delete;
        void operator=(const AboutWidget &) = delete;
        void operator=(AboutWidget &&) = delete;
        ~AboutWidget() override;

    protected:
        void showEvent(QShowEvent *) override;
        void closeEvent(QCloseEvent *) override;
    };
}

#endif //SPECTRUM_QTUI_ABOUTWIDGET_H
