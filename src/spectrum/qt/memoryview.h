//
// Created by darren on 06/03/2021.
//

#ifndef SPECTRUM_MEMORYVIEW_H
#define SPECTRUM_MEMORYVIEW_H

#include <optional>
#include <QScrollArea>

#include "../spectrum48k.h"
#include "../../z80/z80.h"

namespace Spectrum::Qt
{
    class MemoryView
    : public QScrollArea
    {
        Q_OBJECT

    public:
        explicit MemoryView(Z80::UnsignedByte * = nullptr, QWidget * = nullptr);

        explicit MemoryView(const Spectrum48k & spectrum, QWidget * parent = nullptr)
        : MemoryView(spectrum.memory(), parent)
        {}

        MemoryView(const MemoryView &) = delete;
        MemoryView(MemoryView &&) = delete;
        void operator=(const MemoryView &) = delete;
        void operator=(MemoryView &&) = delete;
        ~MemoryView() override;

        void setHighlight(Z80::UnsignedWord address, const QColor & foregroupd, const QColor & background);
        void removeHighlight(Z80::UnsignedWord address);
        void clearHighlights();

        std::optional<Z80::UnsignedWord> addressAt(const QPoint &) const;
        void scrollToAddress(Z80::UnsignedWord addr);

        QWidget * takeWidget() = delete;
        void  setWidget(QWidget *) = delete;

    private:
        Z80::UnsignedByte * m_memory;
    };
}

#endif //SPECTRUM_MEMORYVIEW_H
