//
// Created by darren on 18/03/2021.
//

#ifndef SPECTRUM_QT_DISASSEMBLYWIDGET_H
#define SPECTRUM_QT_DISASSEMBLYWIDGET_H

#include <QScrollArea>

#include "../spectrum.h"
#include "../../z80/assembly/disassembler.h"

namespace Spectrum::Qt
{
    class DisassemblyWidget
    : public QScrollArea
    {
        Q_OBJECT

    public:
        explicit DisassemblyWidget(Z80::UnsignedByte * = nullptr, QWidget * = nullptr);

        explicit DisassemblyWidget(const Spectrum & spectrum, QWidget * parent = nullptr)
        : DisassemblyWidget(spectrum.memory(), parent)
        {}

        DisassemblyWidget(const DisassemblyWidget &) = delete;
        DisassemblyWidget(DisassemblyWidget &&) = delete;
        void operator=(const DisassemblyWidget &) = delete;
        void operator=(DisassemblyWidget &&) = delete;
        ~DisassemblyWidget() override;

        void setFirstAddress(Z80::UnsignedWord address);

        QWidget * takeWidget() = delete;
        void  setWidget(QWidget *) = delete;

    private:
        using Disassembler = ::Z80::Assembly::Disassembler;

        Disassembler m_disassembler;
    };
}

#endif // SPECTRUM_QT_DISASSEMBLYWIDGET_H
