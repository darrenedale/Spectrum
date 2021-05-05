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
        /**
         * Default-initialise a new RegistersWidget.
         *
         * @param parent The widget's parent.
         */
        explicit RegistersWidget(QWidget * parent = nullptr);
        RegistersWidget(const RegistersWidget &) = delete;
        RegistersWidget(RegistersWidget &&) = delete;
        void operator=(const RegistersWidget &) = delete;
        void operator=(RegistersWidget &&) = delete;

        /**
         * Destructor.
         */
        ~RegistersWidget() override;

        /**
         * Set the values of all registers to those in the given register set.
         */
        void setRegisters(const ::Z80::Registers &);

        /**
         * Set the value of a register to a specified value.
         *
         * @param reg16 The register to set.
         * @param value The new value for the register.
         */
        void setRegister(::Z80::Register16 reg16, ::Z80::UnsignedWord value);

        /**
         * Fetch the value of a register from the widget.
         *
         * @param reg16 The register whose value is sought.
         *
         * @return The register value.
         */
        ::Z80::UnsignedWord registerValue(::Z80::Register16 reg16);

    Q_SIGNALS:
        /**
         * Emitted when any register changes value.
         */
        void registerChanged(::Z80::Register16, ::Z80::UnsignedWord);

    private:
        /**
         * The widget for the AF register pair.
         */
        RegisterPairWidget m_af;

        /**
         * The widget for the BC register pair.
         */
        RegisterPairWidget m_bc;

        /**
         * The widget for the DE register pair.
         */
        RegisterPairWidget m_de;

        /**
         * The widget for the HL register pair.
         */
        RegisterPairWidget m_hl;

        /**
         * The widget for the IX register pair.
         */
        RegisterPairWidget m_ix;

        /**
         * The widget for the IY register pair.
         */
        RegisterPairWidget m_iy;

        /**
         * The widget for the CPU flags.
         *
         * The CPU flags are stored in the F (low byte) part of the AF register.
         */
        FlagsWidget m_flags;
    };
}

#endif //SPECTRUM_QTUI_DEBUGGER_REGISTERSWIDGET_H
