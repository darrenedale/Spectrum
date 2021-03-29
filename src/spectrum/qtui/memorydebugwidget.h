//
// Created by darren on 24/03/2021.
//

#ifndef SPECTRUM_MEMORYDEBUGWIDGET_H
#define SPECTRUM_MEMORYDEBUGWIDGET_H

#include <optional>
#include <QWidget>
#include <QToolButton>

#include "../basespectrum.h"
#include "memoryview.h"
#include "hexspinbox.h"

namespace Spectrum::QtUi
{
    class MemoryDebugWidget
    : public QWidget
    {
        Q_OBJECT

    public:
        explicit MemoryDebugWidget(BaseSpectrum::MemoryType * = nullptr, QWidget * = nullptr);

        explicit MemoryDebugWidget(const BaseSpectrum & spectrum, QWidget * parent = nullptr)
        : MemoryDebugWidget(spectrum.memory(), parent)
        {}

        MemoryDebugWidget(const MemoryDebugWidget &) = delete;
        MemoryDebugWidget(MemoryDebugWidget &) = delete;
        void operator=(const MemoryDebugWidget &) = delete;
        void operator=(MemoryDebugWidget &&) = delete;
        ~MemoryDebugWidget() override;

        [[nodiscard]] std::optional<::Z80::UnsignedWord> addressAt(const QPoint &) const;
        void scrollToAddress(::Z80::UnsignedWord addr);

        void clearHighlights();
        void setHighlight(::Z80::UnsignedWord address, const QColor & foreground, const QColor & background);

    Q_SIGNALS:
        // TODO other types of memory-related breakpoints (e.g. break on mod, break on value)
        void programCounterBreakpointRequested(::Z80::UnsignedWord address) const;
        void locationValueChanged(::Z80::UnsignedWord address) const;

    protected:
        virtual void programCounterBreakpointClicked();

    private:
        MemoryView m_memory;
        HexSpinBox m_memoryLocation;
        QToolButton m_setBreakpoint;
    };
}

#endif //SPECTRUM_MEMORYDEBUGWIDGET_H
