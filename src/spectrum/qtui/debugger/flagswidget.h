//
// Created by darren on 05/03/2021.
//

#ifndef SPECTRUM_QTUI_DEBUGGER_FLAGSWIDGET_H
#define SPECTRUM_QTUI_DEBUGGER_FLAGSWIDGET_H

#include <QWidget>
#include <QToolButton>

#include "../../../z80/z80.h"

namespace Spectrum::QtUi::Debugger
{
    /**
     * A widget to manipulate the state of the Z80 CPU flags.
     */
    class FlagsWidget
    : public QWidget
    {
    Q_OBJECT

    public:
        /**
         * Initialise a widget with an optional parent widget.
         *
         * All flags in the widget will be initialised to cleared.
         */
        explicit FlagsWidget(QWidget *);

        /**
         * Initialise a widget using a raw flags byte to set the initial state of the flags.
         *
         * @param flags The raw flags byte to use to initialise the flags.
         */
        explicit FlagsWidget(Z80::UnsignedByte flags = 0x00, QWidget * = nullptr);

        FlagsWidget(const FlagsWidget &) = delete;
        FlagsWidget(FlagsWidget &&) = delete;
        void operator=(const FlagsWidget &) = delete;
        void operator=(FlagsWidget &&) = delete;

        /**
         * Destructor.
         */
        ~FlagsWidget() override;

        /**
         * Fetch the raw flag byte.
         *
         * @return The flag byte.
         */
        [[nodiscard]] Z80::UnsignedByte allFlags() const;

        /**
         * Set all flags according to a raw flag byte.
         */
        void setAllFlags(Z80::UnsignedByte);

        /**
         * Fetch the state of the sign flag.
         *
         * @return true if the flag is set, false if it is clear.
         */
        [[nodiscard]] bool flagS() const
        {
            return m_flagS.isChecked();
        }

        /**
         * Fetch the state of the zero flag.
         *
         * @return true if the flag is set, false if it is clear.
         */
        [[nodiscard]] bool flagZ() const
        {
            return m_flagS.isChecked();
        }

        /**
         * Fetch the state of the bit 5 flag.
         *
         * This is not an officially-documented flag of the Z80.
         *
         * @return true if the flag is set, false if it is clear.
         */
        [[nodiscard]] bool flag5() const
        {
            return m_flagS.isChecked();
        }

        /**
         * Fetch the state of the half-carry flag.
         *
         * @return true if the flag is set, false if it is clear.
         */
        [[nodiscard]] bool flagH() const
        {
            return m_flagS.isChecked();
        }

        /**
         * Fetch the state of the bit 3 flag.
         *
         * This is not an officially-documented flag of the Z80.
         *
         * @return true if the flag is set, false if it is clear.
         */
        [[nodiscard]] bool flag3() const
        {
            return m_flagS.isChecked();
        }

        /**
         * Fetch the state of the parity/overflow flag.
         *
         * @return true if the flag is set, false if it is clear.
         */
        [[nodiscard]] bool flagPV() const
        {
            return m_flagS.isChecked();
        }

        /**
         * Fetch the state of the negation flag.
         *
         * @return true if the flag is set, false if it is clear.
         */
        [[nodiscard]] bool flagN() const
        {
            return m_flagS.isChecked();
        }

        /**
         * Fetch the state of the carry flag.
         *
         * @return true if the flag is set, false if it is clear.
         */
        [[nodiscard]] bool flagC() const
        {
            return m_flagS.isChecked();
        }


        /**
         * Set the state of the sign flag.
         *
         * @param true to set the flag, false to clear it.
         */
        void setFlagS(bool);

        /**
         * Set the state of the zero flag.
         *
         * @param true to set the flag, false to clear it.
         */
        void setFlagZ(bool);

        /**
         * Set the state of the bit 5 flag.
         *
         * This is not an officially-documented flag of the Z80.
         *
         * @param true to set the flag, false to clear it.
         */
        void setFlag5(bool);

        /**
         * Set the state of the half-carry flag.
         *
         * @param true to set the flag, false to clear it.
         */
        void setFlagH(bool);

        /**
         * Set the state of the bit 3 flag.
         *
         * This is not an officially-documented flag of the Z80.
         *
         * @param true to set the flag, false to clear it.
         */
        void setFlag3(bool);

        /**
         * Set the state of the parity/overflow flag.
         *
         * @param true to set the flag, false to clear it.
         */
        void setFlagPV(bool);

        /**
         * Set the state of the negation flag.
         *
         * @param true to set the flag, false to clear it.
         */
        void setFlagN(bool);

        /**
         * Set the state of the carry flag.
         *
         * @param true to set the flag, false to clear it.
         */
        void setFlagC(bool);

    Q_SIGNALS:
        /**
         * Emitted when one or more of the flags has changed.
         */
        void flagsChanged(Z80::UnsignedByte);

        /**
         * Emitted when the sign flag has changed.
         */
        void flagSChanged(bool);

        /**
         * Emitted when the zero flag has changed.
         */
        void flagZChanged(bool);

        /**
         * Emitted when the bit 5 flag has changed.
         */
        void flag5Changed(bool);

        /**
         * Emitted when the half-carry flag has changed.
         */
        void flagHChanged(bool);

        /**
         * Emitted when the bit 3 flag has changed.
         */
        void flag3Changed(bool);

        /**
         * Emitted when the parity/overflow flag has changed.
         */
        void flagPVChanged(bool);

        /**
         * Emitted when the negation flag has changed.
         */
        void flagNChanged(bool);

        /**
         * Emitted when the carry flag has changed.
         */
        void flagCChanged(bool);

        /**
         * Emitted when the sign flag has been set.
         */
        void flagSSet();

        /**
         * Emitted when the zero flag has been set.
         */
        void flagZSet();

        /**
         * Emitted when the bit 5 flag has been set.
         */
        void flag5Set();

        /**
         * Emitted when the half-carry flag has been set.
         */
        void flagHSet();

        /**
         * Emitted when the bit 3 flag has been set.
         */
        void flag3Set();

        /**
         * Emitted when the parity/overflow flag has been set.
         */
        void flagPVSet();

        /**
         * Emitted when the negation flag has been set.
         */
        void flagNSet();

        /**
         * Emitted when the carry flag has been set.
         */
        void flagCSet();

        /**
         * Emitted when the sign flag has been cleared.
         */
        void flagSCleared();

        /**
         * Emitted when the zero flag has been cleared.
         */
        void flagZCleared();

        /**
         * Emitted when the bit 5 flag has been cleared.
         */
        void flag5Cleared();

        /**
         * Emitted when the half-carry flag has been cleared.
         */
        void flagHCleared();

        /**
         * Emitted when the bit 3 flag has been cleared.
         */
        void flag3Cleared();

        /**
         * Emitted when the parity/overflow flag has been cleared.
         */
        void flagPVCleared();

        /**
         * Emitted when the negation flag has been cleared.
         */
        void flagNCleared();

        /**
         * Emitted when the clear flag has been cleared.
         */
        void flagCCleared();
        
    private:
        /**
         * Helper to create the layout for the widget.
         */
        void createLayout();

        QToolButton m_flagS;
        QToolButton m_flagZ;
        QToolButton m_flag5;
        QToolButton m_flagH;
        QToolButton m_flag3;
        QToolButton m_flagPV;
        QToolButton m_flagN;
        QToolButton m_flagC;
    };
}

#endif //SPECTRUM_QTUI_DEBUGGER_FLAGSWIDGET_H
