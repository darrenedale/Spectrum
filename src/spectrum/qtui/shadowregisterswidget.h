//
// Created by darren on 23/03/2021.
//

#ifndef SPECTRUM_QT_SHADOWREGISTERSWIDGET_H
#define SPECTRUM_QT_SHADOWREGISTERSWIDGET_H

#include <QWidget>

#include "registerpairwidget.h"
#include "flagswidget.h"
#include "../../z80/types.h"

namespace Spectrum::QtUi
{
    class ShadowRegistersWidget
            : public QWidget
    {
    Q_OBJECT

    public:
        explicit ShadowRegistersWidget(QWidget * parent = nullptr);
        ShadowRegistersWidget(const ShadowRegistersWidget &) = delete;
        ShadowRegistersWidget(ShadowRegistersWidget &&) = delete;
        void operator=(const ShadowRegistersWidget &) = delete;
        void operator=(ShadowRegistersWidget &&) = delete;
        ~ShadowRegistersWidget() override;

        void setRegisters(const ::Z80::Registers &);
        void setRegister(::Z80::Register16, ::Z80::UnsignedWord);
        ::Z80::UnsignedWord registerValue(::Z80::Register16);

    Q_SIGNALS:
        void registerChanged(::Z80::Register16, ::Z80::UnsignedWord);

    private:
        RegisterPairWidget m_afShadow;
        RegisterPairWidget m_bcShadow;
        RegisterPairWidget m_deShadow;
        RegisterPairWidget m_hlShadow;
        FlagsWidget m_shadowFlags;
    };
}

#endif //SPECTRUM_QT_SHADOWREGISTERSWIDGET_H
