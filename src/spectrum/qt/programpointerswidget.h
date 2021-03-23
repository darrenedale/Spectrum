//
// Created by darren on 23/03/2021.
//

#ifndef SPECTRUM_PROGRAMPOINTERSWIDGET_H
#define SPECTRUM_PROGRAMPOINTERSWIDGET_H

#include "hexspinbox.h"
#include "../../z80/types.h"
#include "../../z80/registers.h"

namespace Spectrum::Qt
{
    class ProgramPointersWidget
    : public QWidget
    {
        Q_OBJECT

    public:
        explicit ProgramPointersWidget(QWidget * = nullptr);
        ProgramPointersWidget(const ProgramPointersWidget &) = delete;
        ProgramPointersWidget(ProgramPointersWidget &) = delete;
        void operator=(const ProgramPointersWidget &) = delete;
        void operator=(ProgramPointersWidget &&) = delete;
        ~ProgramPointersWidget() override;

        void setRegisters(const ::Z80::Registers &);
        void setRegister(::Z80::Register16, ::Z80::UnsignedWord);
        [[nodiscard]] ::Z80::UnsignedWord registerValue(::Z80::Register16) const;

    Q_SIGNALS:
        void registerChanged(::Z80::Register16 reg, ::Z80::UnsignedWord value);

    private:
        HexSpinBox m_sp;
        HexSpinBox m_pc;

    };
}

#endif //SPECTRUM_PROGRAMPOINTERSWIDGET_H
