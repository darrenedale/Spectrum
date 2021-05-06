//
// Created by darren on 24/03/2021.
//

#include <QVBoxLayout>
#include <QHBoxLayout>
#include "../application.h"
#include "memorywidget.h"

using namespace Spectrum::QtUi::Debugger;

MemoryWidget::MemoryWidget(Spectrum::BaseSpectrum::MemoryType * memory, QWidget * parent)
: QWidget(parent),
  m_memory(memory),
  m_memoryLocation(),
  m_search()
{
    auto * widgetLayout = new QVBoxLayout();
    widgetLayout->addWidget(&m_memory);
    auto * controlsLayout = new QHBoxLayout();
    controlsLayout->addWidget(new QLabel(tr("Go to")));
    controlsLayout->addWidget(&m_memoryLocation);
    controlsLayout->addStretch(10);
    controlsLayout->addWidget(&m_search);
    widgetLayout->addLayout(controlsLayout);
    setLayout(widgetLayout);

    connect(&m_memoryLocation, qOverload<int>(&HexSpinBox::valueChanged), this, &MemoryWidget::locationValueChanged);
    connect(&m_memoryLocation, qOverload<int>(&HexSpinBox::valueChanged), &m_memory, &MemoryView::scrollToAddress);
    connect(&m_search, &MemorySearchWidget::unsignedByteSearchRequested, &m_memory, &MemoryView::find<::Z80::UnsignedByte>);
    connect(&m_search, &MemorySearchWidget::unsignedWordSearchRequested, &m_memory, &MemoryView::find<::Z80::UnsignedWord>);
    connect(&m_search, &MemorySearchWidget::signedByteSearchRequested, &m_memory, &MemoryView::find<::Z80::SignedByte>);
    connect(&m_search, &MemorySearchWidget::signedWordSearchRequested, &m_memory, &MemoryView::find<::Z80::SignedWord>);
    connect(&m_search, &MemorySearchWidget::stringSearchRequested, &m_memory, &MemoryView::find<const QByteArray &>);
}

MemoryWidget::~MemoryWidget() = default;

std::optional<Z80::UnsignedWord> MemoryWidget::addressAt(const QPoint & pos) const
{
    return m_memory.addressAt(m_memory.mapFromParent(pos));
}

void MemoryWidget::scrollToAddress(::Z80::UnsignedWord addr)
{
    m_memoryLocation.setValue(addr);
    m_memory.scrollToAddress(addr);
}

void MemoryWidget::setHighlight(::Z80::UnsignedWord address, const QColor & foreground, const QColor & background)
{
    m_memory.setHighlight(address, foreground, background);
}

void MemoryWidget::removeHighlight(::Z80::UnsignedWord address)
{
    m_memory.removeHighlight(address);
}

void MemoryWidget::clearHighlights()
{
    m_memory.clearHighlights();
}

template<class search_t>
void MemoryWidget::find(search_t value, std::optional<::Z80::UnsignedWord> fromAddress)
{
    // simply forward calls to the MemoryView method with the appropriate type
    static_assert(std::is_same_v<QByteArray, std::remove_cvref_t<search_t>> || (std::is_integral_v<search_t> && 2 >= sizeof(search_t)), "only QByhteArrays and 8-/16-bit integral types can be used with find() template");
    m_memory.find(value, fromAddress);
}
