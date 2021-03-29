//
// Created by darren on 23/03/2021.
//

#ifndef SPECTRUM_QT_INTERRUPTWIDGET_H
#define SPECTRUM_QT_INTERRUPTWIDGET_H

#include <QWidget>
#include <QSpinBox>

#include "hexspinbox.h"
#include "../../z80/types.h"
#include "../../z80/registers.h"

namespace Spectrum::QtUi
{
    class InterruptWidget
    : public QWidget
    {
        Q_OBJECT

    public:
        explicit InterruptWidget(QWidget * = nullptr);
        InterruptWidget(const InterruptWidget &) = delete;
        InterruptWidget(InterruptWidget &) = delete;
        void operator=(const InterruptWidget &) = delete;
        void operator=(InterruptWidget &&) = delete;
        ~InterruptWidget() override;

        void setInterruptMode(::Z80::InterruptMode);
        [[nodiscard]] ::Z80::InterruptMode interruptMode() const;

        void setRegisters(const ::Z80::Registers &);
        void setRegister(::Z80::Register8, ::Z80::UnsignedByte);
        ::Z80::UnsignedByte registerValue(::Z80::Register8);

    Q_SIGNALS:
        void registerChanged(::Z80::Register8, ::Z80::UnsignedByte);
        void interruptModeChanged(::Z80::InterruptMode);

    private:
        QSpinBox m_im;
        HexSpinBox m_i;
        HexSpinBox m_r;

    };
}

#endif //SPECTRUM_QT_INTERRUPTWIDGET_H
