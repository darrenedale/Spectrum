//
// Created by darren on 23/03/2021.
//

#ifndef SPECTRUM_QTUI_INTERRUPTWIDGET_H
#define SPECTRUM_QTUI_INTERRUPTWIDGET_H

#include <QWidget>
#include <QSpinBox>
#include <QToolButton>

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

        void setIff1(bool enabled);
        bool iff1() const;

        void setIff2(bool enabled);
        bool iff2() const;

    Q_SIGNALS:
        void registerChanged(::Z80::Register8, ::Z80::UnsignedByte);
        void interruptModeChanged(::Z80::InterruptMode);
        void iff1Changed(bool);
        void iff2Changed(bool);

    private:
        QSpinBox m_im;
        HexSpinBox m_i;
        HexSpinBox m_r;
        QToolButton m_iff1;
        QToolButton m_iff2;
    };
}

#endif //SPECTRUM_QTUI_INTERRUPTWIDGET_H
