//
// Created by darren on 10/03/2021.
//

#ifndef SPECTRUM_KEYBOARDMONITORWIDGET_H
#define SPECTRUM_KEYBOARDMONITORWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QTimer>

namespace Spectrum
{
    class BaseSpectrum;
}

namespace Spectrum::QtUi
{
    class KeyboardMonitorWidget
    : public QWidget
    {
        Q_OBJECT

    public:
        explicit KeyboardMonitorWidget(BaseSpectrum * spectrum = nullptr, QWidget * parent = nullptr);
        explicit KeyboardMonitorWidget(QWidget * parent)
        : KeyboardMonitorWidget(nullptr, parent)
        {}

        [[nodiscard]] BaseSpectrum * spectrum() const
        {
            return m_spectrum;
        }

        void setSpectrum(BaseSpectrum *);

        void updateStateDisplay();

    private:
        BaseSpectrum * m_spectrum;
        QToolButton m_1;
        QToolButton m_2;
        QToolButton m_3;
        QToolButton m_4;
        QToolButton m_5;

        QToolButton m_6;
        QToolButton m_7;
        QToolButton m_8;
        QToolButton m_9;
        QToolButton m_0;

        QToolButton m_q;
        QToolButton m_w;
        QToolButton m_e;
        QToolButton m_r;
        QToolButton m_t;

        QToolButton m_y;
        QToolButton m_u;
        QToolButton m_i;
        QToolButton m_o;
        QToolButton m_p;

        QToolButton m_a;
        QToolButton m_s;
        QToolButton m_d;
        QToolButton m_f;
        QToolButton m_g;

        QToolButton m_h;
        QToolButton m_j;
        QToolButton m_k;
        QToolButton m_l;
        QToolButton m_enter;

        QToolButton m_capsShift;
        QToolButton m_z;
        QToolButton m_x;
        QToolButton m_c;
        QToolButton m_v;

        QToolButton m_b;
        QToolButton m_n;
        QToolButton m_m;
        QToolButton m_symbolShift;
        QToolButton m_space;

        QTimer m_updateTimer;
    };
}

#endif //SPECTRUM_KEYBOARDMONITORWIDGET_H
