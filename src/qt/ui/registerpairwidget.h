//
// Created by darren on 04/03/2021.
//

#ifndef SPECTRUM_REGISTERPAIRWIDGET_H
#define SPECTRUM_REGISTERPAIRWIDGET_H

#include <QWidget>
#include <QString>
#include <QLabel>

#include "../../z80/z80.h"
#include "hexspinbox.h"

class QEvent;

namespace Spectrum
{
    class RegisterPairWidget
    : public QWidget
    {
        Q_OBJECT

        using UnsignedWord = Z80::UnsignedWord;
        using UnsignedByte = Z80::UnsignedByte;

    public:
        explicit RegisterPairWidget(const QString & registerName = QLatin1String(), UnsignedWord value = 0x0000, QWidget * parent = nullptr);
        RegisterPairWidget(const QString & registerName, QWidget * parent);
        explicit RegisterPairWidget(UnsignedWord value, QWidget * parent = nullptr);
        explicit RegisterPairWidget(QWidget * parent);
        RegisterPairWidget(const RegisterPairWidget &) = delete;
        RegisterPairWidget(RegisterPairWidget &&) = delete;
        void operator=(const RegisterPairWidget &) = delete;
        void operator=(RegisterPairWidget &&) = delete;
        ~RegisterPairWidget() noexcept override;

        [[nodiscard]] const QString & registerName() const
        {
            return m_registerName;
        }

        [[nodiscard]] const QString & lowByteName() const
        {
            return m_lowByteName;
        }

        [[nodiscard]] const QString & highByteNameName() const
        {
            return m_highByteName;
        }

        void setRegisterName(const QString & name);
        void setLowByteName(const QString & name);
        void setHighByteName(const QString & name);

        /**
         * Set the value using host byte order.
         *
         * @param value
         */
        void setValue(UnsignedWord value);

        /**
         * Set the value using Z80 byte order.
         *
         * @param value
         */
        void setValueZ80(UnsignedWord value)
        {
            setValue(Z80::Z80::z80ToHostByteOrder(value));
        }

        void setLowByte(UnsignedByte value)
        {
            setValue((this->value() & 0xff00) | value);
        }

        void setHighByte(UnsignedByte value)
        {
            setValue((this->value() & 0xff) | ((static_cast<UnsignedWord>(value) << 8) & 0xff00));
        }

        /**
         * Fetch the value in host byte order.
         *
         * Use this when you want your program to see the same value as the Z80 sees for the register.
         *
         * If the host and Z80 use the same byte order, this will be the same as valueZ80().
         *
         * @return
         */
        [[nodiscard]] UnsignedWord value() const;

        /**
         * Fetch the value in Z80 byte order.
         *
         * Use this when you need the bits in the word in the exact order the Z80 stores them.
         *
         * If the host and Z80 use the same byte order, this will be the same as value().
         *
         * @return
         */
        [[nodiscard]] UnsignedWord valueZ80() const
        {
            return Z80::Z80::hostToZ80ByteOrder(value());
        }

        [[nodiscard]] UnsignedByte lowByte() const
        {
            return static_cast<UnsignedByte>(value() & 0xff);
        }

        [[nodiscard]] UnsignedByte highByte() const
        {
            return static_cast<UnsignedByte>(((value() & 0xff00) >> 8) & 0xff);
        }

    Q_SIGNALS:
        /**
         * Value is always in host byte order.
         *
         * @param value
         */
        void valueChanged(UnsignedWord value);

        /**
         * Value is always in Z80 byte order.
         *
         * @param value
         */
        void valueChangedZ80(UnsignedWord value);

    private:
        using SpinBox = HexSpinBox;

        QString m_registerName;
        QString m_highByteName;
        QString m_lowByteName;
        QLabel m_label16;
        QLabel m_labelLow;
        QLabel m_labelHigh;
        SpinBox m_spin16;
        SpinBox m_spinLow;
        SpinBox m_spinHigh;

        void setRegisterPairForBytes();
        void setBytesForRegisterPair();
    };
}

#endif //SPECTRUM_REGISTERPAIRWIDGET_H
