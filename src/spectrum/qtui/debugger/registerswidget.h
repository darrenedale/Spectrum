//
// Created by darren on 23/03/2021.
//

#ifndef SPECTRUM_QTUI_DEBUGGER_REGISTERSWIDGET_H
#define SPECTRUM_QTUI_DEBUGGER_REGISTERSWIDGET_H

#include <QWidget>

#include "../registerpairwidget.h"
#include "flagswidget.h"
#include "../../../z80/types.h"

namespace Spectrum::QtUi::Debugger
{
    /**
     * Widget to manipulate the registers of a Z80 CPU.
     */
    class RegistersWidget
    : public QWidget
    {
        Q_OBJECT

    public:
        explicit RegistersWidget(QWidget * parent = nullptr);
        RegistersWidget(const RegistersWidget &) = delete;
        RegistersWidget(RegistersWidget &&) = delete;
        void operator=(const RegistersWidget &) = delete;
        void operator=(RegistersWidget &&) = delete;
        ~RegistersWidget() override;

        void setRegisters(const ::Z80::Registers &);
        void setRegister(::Z80::Register16, ::Z80::UnsignedWord);
        ::Z80::UnsignedWord registerValue(::Z80::Register16);

        Q_SIGNALS:
        void registerChanged(::Z80::Register16, ::Z80::UnsignedWord);

    private:
        RegisterPairWidget m_af;
        RegisterPairWidget m_bc;
        RegisterPairWidget m_de;
        RegisterPairWidget m_hl;
        RegisterPairWidget m_ix;
        RegisterPairWidget m_iy;
        FlagsWidget m_flags;
    };
}

#endif //SPECTRUM_QTUI_DEBUGGER_REGISTERSWIDGET_H
