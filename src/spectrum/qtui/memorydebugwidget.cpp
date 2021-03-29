//
// Created by darren on 24/03/2021.
//

#include <QVBoxLayout>
#include <QHBoxLayout>

#include "memorydebugwidget.h"

using namespace Spectrum::QtUi;

MemoryDebugWidget::MemoryDebugWidget(Spectrum::BaseSpectrum::MemoryType * memory, QWidget * parent)
: QWidget(parent),
  m_memory(memory),
  m_memoryLocation(),
  m_setBreakpoint()
{
    m_setBreakpoint.setIcon(QIcon::fromTheme(QStringLiteral("process-stop")));

    auto * widgetLayout = new QVBoxLayout();
    widgetLayout->addWidget(&m_memory);
    auto * controlsLayout = new QHBoxLayout();
    controlsLayout->addWidget(&m_memoryLocation);
    controlsLayout->addWidget(&m_setBreakpoint);
    controlsLayout->addStretch(10);
    widgetLayout->addLayout(controlsLayout);
    setLayout(widgetLayout);

    connect(&m_memoryLocation, qOverload<int>(&HexSpinBox::valueChanged), this, &MemoryDebugWidget::locationValueChanged);
    connect(&m_memoryLocation, qOverload<int>(&HexSpinBox::valueChanged), &m_memory, &MemoryView::scrollToAddress);
    connect(&m_setBreakpoint, &QToolButton::clicked, this, &MemoryDebugWidget::programCounterBreakpointClicked);
}

MemoryDebugWidget::~MemoryDebugWidget() = default;

void MemoryDebugWidget::programCounterBreakpointClicked()
{
    Q_EMIT programCounterBreakpointRequested(m_memoryLocation.value());
}

std::optional<Z80::UnsignedWord> MemoryDebugWidget::addressAt(const QPoint & pos) const
{
    return m_memory.addressAt(m_memory.mapFromParent(pos));
}

void MemoryDebugWidget::scrollToAddress(::Z80::UnsignedWord addr)
{
    m_memoryLocation.setValue(addr);
    m_memory.scrollToAddress(addr);
}

void MemoryDebugWidget::clearHighlights()
{
    m_memory.clearHighlights();
}

void MemoryDebugWidget::setHighlight(::Z80::UnsignedWord address, const QColor & foreground, const QColor & background)
{
    m_memory.setHighlight(address, foreground, background);
}
