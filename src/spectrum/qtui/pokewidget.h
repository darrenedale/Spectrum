//
// Created by darren on 14/03/2021.
//

#ifndef SPECTRUM_POKEWIDGET_H
#define SPECTRUM_POKEWIDGET_H

#include <QToolButton>
#include "hexspinbox.h"
#include "../../z80/types.h"

namespace Spectrum::QtUi
{
    class PokeWidget
    : public QWidget
    {
        Q_OBJECT

    public:
        PokeWidget(::Z80::UnsignedWord address, QWidget *);
        explicit PokeWidget(::Z80::UnsignedByte, QWidget * = nullptr);
        explicit PokeWidget(::Z80::UnsignedWord address = 0x0000, ::Z80::UnsignedByte = 0x00, QWidget * = nullptr);
        PokeWidget(const PokeWidget &) = delete;
        PokeWidget(PokeWidget &&) = delete;
        void operator=(const PokeWidget &) = delete;
        void operator=(PokeWidget &&) = delete;
        ~PokeWidget() override;

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

        void focusAddress()
        {
            m_address.setFocus();
        }

        void focusValue()
        {
            m_value.setFocus();
        }

    Q_SIGNALS:
        void pokeClicked(::Z80::UnsignedWord address, ::Z80::UnsignedByte value);

    private:
        HexSpinBox m_address;
        HexSpinBox m_value;
        QToolButton m_poke;
    };
}

#endif //SPECTRUM_POKEWIDGET_H
