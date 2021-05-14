//
// Created by darren on 18/03/2021.
//

#ifndef SPECTRUM_QTUI_DEBUGGER_DISASSEMBLYWIDGET_H
#define SPECTRUM_QTUI_DEBUGGER_DISASSEMBLYWIDGET_H

#include <QScrollArea>
#include "../../basespectrum.h"
#include "../../../z80/assembly/disassembler.h"

namespace Spectrum::QtUi::Debugger
{
    /**
     * A widget to show the disassembly of a bunch of Z80 machine code.
     *
     * The widget inherits from QScrollArea and provides its own internal widget that the QScrollArea renders. The takeWidget() and setWidget() methods are
     * explicitly deleted since the widget is a private implementation detail for instances of this class and client code must not change it. Client code may
     * safely call widget() to fetch a pointer to the internal widget, but its type is opaque and therefore it cannot be manipulated or queried in much of a
     * meaningful way. Under no circumstances should the internal widget be re-parented.
     */
    class DisassemblyWidget
    : public QScrollArea
    {
        Q_OBJECT

    public:
        /**
         * Initialise a new disassembly widget with a memory object.
         *
         * The memory object is borrowed. It is the responsibility of the calling code to ensure that the widget does not retain a reference to it after it has
         * been destroyed.
         */
        explicit DisassemblyWidget(BaseSpectrum::MemoryType * = nullptr, QWidget * = nullptr);

        /**
         * Initialise a new disassembly widget with the memory from a Spectrum.
         *
         * The memory from the Spectrum is borrowed. It is the responsibility of the calling code to ensure that the widget does not retain a reference to it
         * after it has been destroyed.
         */
        explicit DisassemblyWidget(const BaseSpectrum & spectrum, QWidget * parent = nullptr)
        : DisassemblyWidget(spectrum.memory(), parent)
        {}

        /**
         * DisassemblyWidgets cannot be copy-constructed.
         */
        DisassemblyWidget(const DisassemblyWidget &) = delete;

        /**
         * DisassemblyWidgets cannot be move-constructed.
         */
        DisassemblyWidget(DisassemblyWidget &&) = delete;

        /**
         * DisassemblyWidgets cannot be copy assigned.
         */
        void operator=(const DisassemblyWidget &) = delete;

        /**
         * DisassemblyWidgets cannot be move assigned.
         */
        void operator=(DisassemblyWidget &&) = delete;

        /**
         * Destructor.
         */
        ~DisassemblyWidget() override;

        /**
         * Set the memory to disassemble.
         *
         * The memory is borrowed. It is the responsibility of the calling code to ensure that the widget does not retain a reference to it after it has been
         * destroyed.
         */
        void setMemory(BaseSpectrum::MemoryType *);

        /**
         * Update the disassembly from a given address.
         *
         * This is a relatively expensive operation so should only be called when necessary. The internal disassembly
         * is updated, but the view is not. Call update() if you also want the view updated.
         *
         * @param fromAddress
         */
        void updateMnemonics(::Z80::UnsignedWord fromAddress);

        /**
         * Scroll the view if necessary so that a given address is visible.
         *
         * @param address
         */
        void scrollToAddress(::Z80::UnsignedWord address);

        /**
         * Scroll the view if necessary so that the current PC is visible.
         */
        void scrollToPc()
        {
            scrollToAddress(pc());
        }

        /**
         * Set the current PC in the disassembly.
         *
         * The current PC can be visibly indicated to aid debugging.
         *
         * @param pc
         */
        void setPc(::Z80::UnsignedWord pc);

        /**
         * Fetch the current PC in the disassembly.
         *
         * @param pc
         */
        [[nodiscard]] ::Z80::UnsignedWord pc() const;

        /**
         * Determine whether the current PC is being indicated in the disassembly.
         *
         * @return
         */
        [[nodiscard]] bool pcIndicatorEnabled() const;

        /**
         * Show or hide the indicator for the current PC.
         *
         * This does not automatically track the PC of the Spectrum whose memory is borrowed. It simply switches on or off the visualisation. In order to ensure
         * the correct address is visualised for the PC, client code must call setPc() with the correct address to update the view.
         *
         * @param enabled Whether or not the PC indicator should be enabled.
         */
        void enablePcIndicator(bool enabled = true);

        /**
         * Switch off the indicator for the current PC in the view.
         */
        void disablePcIndicator()
        {
            enablePcIndicator(false);
        }

        /**
         * Explicitly delete the takeWidget() method inherited from QScrollArea. Client code must not alter the contained widget.
         */
        QWidget * takeWidget() = delete;

        /**
         * Explicitly delete the setWidget() method inherited from QScrollArea. Client code must not alter the contained widget.
         */
        void  setWidget(QWidget *) = delete;
    };
}

#endif // SPECTRUM_QTUI_DEBUGGER_DISASSEMBLYWIDGET_H
