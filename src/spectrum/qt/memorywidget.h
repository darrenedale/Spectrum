//
// Created by darren on 06/03/2021.
//

#ifndef SPECTRUM_MEMORYWIDGET_H
#define SPECTRUM_MEMORYWIDGET_H

#include <optional>
#include <QScrollArea>

#include "../spectrum.h"
#include "../../z80/z80.h"

namespace Spectrum::Qt
{
    class MemoryWidget
    : public QScrollArea
    {
        Q_OBJECT

    public:
        explicit MemoryWidget(Z80::UnsignedByte * = nullptr, QWidget * = nullptr);

        explicit MemoryWidget(const Spectrum & spectrum, QWidget * parent = nullptr)
        : MemoryWidget(spectrum.memory(), parent)
        {}

        MemoryWidget(const MemoryWidget &) = delete;
        MemoryWidget(MemoryWidget &&) = delete;
        void operator=(const MemoryWidget &) = delete;
        void operator=(MemoryWidget &&) = delete;
        ~MemoryWidget() override;

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

#endif //SPECTRUM_MEMORYWIDGET_H
