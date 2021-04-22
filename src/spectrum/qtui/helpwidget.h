//
// Created by darren on 22/04/2021.
//

#ifndef SPECTRUM_QTUI_HELPWIDGET_H
#define SPECTRUM_QTUI_HELPWIDGET_H

#include <QWidget>

namespace Spectrum::QtUi
{
    class HelpWidget
            : public QWidget
    {
    Q_OBJECT

    public:
        explicit HelpWidget(QWidget * parent = nullptr);
        HelpWidget(const HelpWidget &) = delete;
        HelpWidget(HelpWidget &&) = delete;
        void operator=(const HelpWidget &) = delete;
        void operator=(HelpWidget &&) = delete;
        ~HelpWidget() override;

    protected:
        void showEvent(QShowEvent *) override;
        void closeEvent(QCloseEvent *) override;
    };
}

#endif //SPECTRUM_QTUI_HELPWIDGET_H
