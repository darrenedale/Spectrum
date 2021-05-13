//
// Created by darren on 14/03/2021.
//

#ifndef SPECTRUM_QTUI_POKEWIDGET_H
#define SPECTRUM_QTUI_POKEWIDGET_H

#include <QToolButton>
#include "hexspinbox.h"
#include "../../z80/types.h"

namespace Spectrum::QtUi
{
    /**
     * Widget to enable the user to enter the address and byte value for a poke into the Spectrum's memory.
     *
     * The widget lays out an address widget (HexSpinBox), value widget (HexSpinBox) and trigger button (QToolButton) in a simple horizontal layout. It has one
     * signal which is emitted when the user clicks the trigger button, and which carries the current address and value the user wishes to poke.
     */
    class PokeWidget
    : public QWidget
    {
        Q_OBJECT

    public:
        /**
         * Initialise a new PokeWidget with a target address.
         *
         * @param address The address to poke into.
         */
        PokeWidget(::Z80::UnsignedWord address, QWidget *);

        /**
         * Initialise a new PokeWidget with a value to poke.
         *
         * @param value The value to poke.
         */
        explicit PokeWidget(::Z80::UnsignedByte value, QWidget * = nullptr);

        /**
         * (Default) initialise a new PokeWidget with an optional address and value to poke.
         *
         * @param address The address to poke into. Defaults to 0x0000.
         * @param value The value to poke. Defaults to 0x00.
         */
        explicit PokeWidget(::Z80::UnsignedWord address = 0x0000, ::Z80::UnsignedByte value = 0x00, QWidget * = nullptr);

        /**
         * Poke widgets cannot be copy constructed.
         */
        PokeWidget(const PokeWidget &) = delete;

        /**
         * Poke widgets cannot be move constructed.
         */
        PokeWidget(PokeWidget &&) = delete;

        /**
         * Poke widgets cannot be copy assigned.
         */
        void operator=(const PokeWidget &) = delete;

        /**
         * Poke widgets cannot be move assigned.
         */
        void operator=(PokeWidget &&) = delete;

        /**
         * Destructor.
         */
        ~PokeWidget() override;

        /**
         * The address to poke into.
         *
         * @return The address.
         */
        [[nodiscard]] ::Z80::UnsignedWord address() const
        {
            return m_address.value();
        }

        /**
         * The value to poke.
         *
         * @return The value.
         */
        [[nodiscard]] ::Z80::UnsignedByte value() const
        {
            return m_value.value();
        }

        /**
         * Set the address to poke into.
         *
         * @param address The address.
         */
        void setAddress(::Z80::UnsignedWord address)
        {
            m_address.setValue(address);
        }

        /**
         * Set the value to poke.
         *
         * @param value The value.
         */
        void setValue(::Z80::UnsignedByte value)
        {
            m_value.setValue(value);
        }

        /**
         * Place the focus on the address widget.
         */
        void focusAddress()
        {
            m_address.setFocus();
        }

        /**
         * Place the focus on the value widget.
         */
        void focusValue()
        {
            m_value.setFocus();
        }

    Q_SIGNALS:
        /**
         * Emitted when the button to actually action the poke is clicked.
         *
         * @param address The address to poke into.
         * @param value The value to poke.
         */
        void pokeClicked(::Z80::UnsignedWord address, ::Z80::UnsignedByte value);

    private:
        /**
         * The address widget.
         */
        HexSpinBox m_address;

        /**
         * The value widget.
         */
        HexSpinBox m_value;

        /**
         * The "do the poke" button.
         */
        QToolButton m_poke;
    };
}

#endif //SPECTRUM_QTUI_POKEWIDGET_H
