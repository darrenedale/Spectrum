//
// Created by darren on 14/03/2021.
//

#ifndef SPECTRUM_CUSTOMPOKEWIDGET_H
#define SPECTRUM_CUSTOMPOKEWIDGET_H

#include <QToolButton>
#include "hexspinbox.h"
#include "../../z80/types.h"

namespace Spectrum::QtUi
{
    class CustomPokeWidget
    : public QWidget
    {
        Q_OBJECT

    public:
        CustomPokeWidget(::Z80::UnsignedWord address, QWidget *);
        explicit CustomPokeWidget(::Z80::UnsignedByte, QWidget * = nullptr);
        explicit CustomPokeWidget(::Z80::UnsignedWord address = 0x0000, ::Z80::UnsignedByte = 0x00, QWidget * = nullptr);
        CustomPokeWidget(const CustomPokeWidget &) = delete;
        CustomPokeWidget(CustomPokeWidget &&) = delete;
        void operator=(const CustomPokeWidget &) = delete;
        void operator=(CustomPokeWidget &&) = delete;
        ~CustomPokeWidget() override;

        [[nodiscard]] ::Z80::UnsignedWord address() const
        {
            return m_address.value();
        }

        [[nodiscard]] ::Z80::UnsignedByte value() const
        {
            return m_value.value();
        }

        void setAddress(::Z80::UnsignedWord address)
        {
            m_address.setValue(address);
        }

        void setValue(::Z80::UnsignedByte value)
        {
            m_value.setValue(value);
        }

    Q_SIGNALS:
        void pokeClicked(::Z80::UnsignedWord address, ::Z80::UnsignedByte value);

    private:
        HexSpinBox m_address;
        HexSpinBox m_value;
        QToolButton m_poke;
    };
}

#endif //SPECTRUM_CUSTOMPOKEWIDGET_H
