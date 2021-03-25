//
// Created by darren on 18/03/2021.
//

#ifndef SPECTRUM_QT_DISASSEMBLYWIDGET_H
#define SPECTRUM_QT_DISASSEMBLYWIDGET_H

#include <QScrollArea>

#include "../basespectrum.h"
#include "../../z80/assembly/disassembler.h"

namespace Spectrum::Qt
{
    class DisassemblyWidget
    : public QScrollArea
    {
        Q_OBJECT

    public:
        explicit DisassemblyWidget(BaseSpectrum::MemoryType * = nullptr, QWidget * = nullptr);

        explicit DisassemblyWidget(const BaseSpectrum & spectrum, QWidget * parent = nullptr)
        : DisassemblyWidget(spectrum.memory(), parent)
        {}

        DisassemblyWidget(const DisassemblyWidget &) = delete;
        DisassemblyWidget(DisassemblyWidget &&) = delete;
        void operator=(const DisassemblyWidget &) = delete;
        void operator=(DisassemblyWidget &&) = delete;
        ~DisassemblyWidget() override;

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
         * @param enabled
         */
        void enablePcIndicator(bool enabled = true);

        void disablePcIndicator()
        {
            enablePcIndicator(false);
        }

        QWidget * takeWidget() = delete;
        void  setWidget(QWidget *) = delete;
    };
}

#endif // SPECTRUM_QT_DISASSEMBLYWIDGET_H
