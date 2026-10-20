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
     * The widget lays out an address widget (HexSpinBox), value widget (QSpinBox/HexSpinBox) and trigger button
     * (QToolButton) in a simple horizontal layout. It has one signal which is emitted when the user clicks the trigger
     * button, and which carries the current address and value the user wishes to poke.
     */
    class PokeWidget
    : public QWidget
    {
        Q_OBJECT

        public:
            /** The numeric bases in which the value can be entered. */
            enum class ValueBase: std::uint8_t
            {
                Decimal,
                Hexadecimal,
            };

            /** Initialise a new PokeWidget with a target address. */
            PokeWidget(::Z80::UnsignedWord, QWidget *);

            /** Initialise a new PokeWidget with a target address and the numeric base for the value. */
            PokeWidget(::Z80::UnsignedWord, ValueBase, QWidget *);

            /** Initialise a new PokeWidget with a value to poke. */
            explicit PokeWidget(::Z80::UnsignedByte, QWidget * = nullptr);

            /** Initialise a new PokeWidget with a value to poke and the numeric base to display it in. */
            PokeWidget(::Z80::UnsignedByte, ValueBase, QWidget * = nullptr);

            /** Initialise a new PokeWidget with a target address and a value to poke. */
            PokeWidget(::Z80::UnsignedWord, ::Z80::UnsignedByte, QWidget *);

            /** (Default) initialise a new PokeWidget with an optional address, value and numeric base for the value. */
            explicit PokeWidget(
                ::Z80::UnsignedWord = 0x0000,
                ::Z80::UnsignedByte = 0x00,
                ValueBase = ValueBase::Hexadecimal,
                QWidget * = nullptr
            );

            /** Poke widgets cannot be copy constructed. */
            PokeWidget(const PokeWidget &) = delete;

            /** Poke widgets cannot be move constructed. */
            PokeWidget(PokeWidget &&) = delete;

            /** Poke widgets cannot be copy assigned. */
            void operator=(const PokeWidget &) = delete;

            /** Poke widgets cannot be move assigned. */
            void operator=(PokeWidget &&) = delete;

            /** The destructor is defaulted in the implementation file. */
            ~PokeWidget() override;

            /**
             * The address to poke into.
             *
             * @return The address.
             */
            [[nodiscard]]
            ::Z80::UnsignedWord address() const
            {
                return m_address.value();
            }

            /**
             * The value to poke.
             *
             * @return The value.
             */
            [[nodiscard]]
            ::Z80::UnsignedByte value() const
            {
                return m_value->value();
            }

            /**
             * Set the address to poke into.
             *
             * @param address The address.
             */
            void setAddress(const ::Z80::UnsignedWord address)
            {
                m_address.setValue(address);
            }

            /**
             * Set the value to poke.
             *
             * @param value The value.
             */
            void setValue(const ::Z80::UnsignedByte value)
            {
                m_value->setValue(value);
            }

            /** The numeric base of the value widget. */
            [[nodiscard]]
            ValueBase valueBase() const noexcept
            {
                return m_valueBase;
            }

            void setValueBase(ValueBase) noexcept;

            /** Place the focus on the address widget. */
            void focusAddress()
            {
                m_address.setFocus();
            }

            /** Place the focus on the value widget. */
            void focusValue()
            {
                m_value->setFocus();
            }

            /** Select the whole value in the address widget. */
            void selectAddress()
            {
                m_address.selectAll();
            }

            /** Select the whole value in the value widget. */
            void selectValue()
            {
                m_value->selectAll();
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
            /** The address widget. */
            HexSpinBox m_address;

            /** The value widget. */
            std::unique_ptr<QSpinBox> m_value;

            /** The "do the poke" button. */
            QToolButton m_poke;

            /** The numeric base of the value widget. */
            ValueBase m_valueBase;
    };
}

#endif //SPECTRUM_QTUI_POKEWIDGET_H
