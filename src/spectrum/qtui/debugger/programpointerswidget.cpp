//
// Created by darren on 23/03/2021.
//

#include <iostream>
#include <iomanip>
#include <QHBoxLayout>
#include <QLabel>
#include "programpointerswidget.h"
#include "../../../util/debug.h"


using namespace Spectrum::QtUi::Debugger;
using ::Z80::Register16;
using ::Z80::Registers;

using ::Z80::UnsignedWord;


ProgramPointersWidget::ProgramPointersWidget(QWidget * parent)
: QWidget(parent),
  m_sp(4),
  m_pc(4)
{
    m_pc.setMinimum(0);
    m_pc.setMaximum(0xffff);
    m_sp.setMinimum(0);
    m_sp.setMaximum(0xffff);

    QFont widgetFont = m_pc.font();
    widgetFont.setPointSizeF(widgetFont.pointSizeF() * 0.85);
    m_pc.setFont(widgetFont);
    m_sp.setFont(widgetFont);

    m_pc.setContextMenuPolicy(Qt::ContextMenuPolicy::ActionsContextMenu);
    m_sp.setContextMenuPolicy(Qt::ContextMenuPolicy::ActionsContextMenu);

    auto * widgetLayout = new QVBoxLayout();
    auto * regLayout = new QHBoxLayout();
    auto * tmpLabel = new QLabel("SP");
    tmpLabel->setBuddy(&m_sp);
    tmpLabel->setFont(m_sp.font());
    tmpLabel->setMinimumHeight(m_sp.minimumHeight());
    regLayout->addWidget(tmpLabel, 1);
    regLayout->addWidget(&m_sp, 10);
    widgetLayout->addLayout(regLayout);

    regLayout = new QHBoxLayout();
    tmpLabel = new QLabel("PC");
    tmpLabel->setBuddy(&m_pc);
    tmpLabel->setFont(m_pc.font());
    tmpLabel->setMinimumHeight(m_pc.minimumHeight());
    regLayout->addWidget(tmpLabel, 1);
    regLayout->addWidget(&m_pc, 10);
    widgetLayout->addLayout(regLayout);
    setLayout(widgetLayout);

    connect(&m_sp, &HexSpinBox::editingFinished, [this]() {
        Q_EMIT registerChanged(Register16::SP, m_sp.value());
    });

    connect(&m_pc, &HexSpinBox::editingFinished, [this]() {
        Q_EMIT registerChanged(Register16::PC, m_pc.value());
    });
}

ProgramPointersWidget::~ProgramPointersWidget() = default;

void ProgramPointersWidget::setRegisters(const Registers & registers)
{
    m_sp.setValue(registers.sp);
    m_pc.setValue(registers.pc);
}

void ProgramPointersWidget::setRegister(Register16 reg, UnsignedWord value)
{
    switch (reg) {
        case Register16::SP:
            m_sp.setValue(value);
            break;

        case Register16::PC:
            m_pc.setValue(value);
            break;

        default:
            Util::debug << "Only registers PC and SP are present in this widget.\n";
            break;
    }
}

UnsignedWord ProgramPointersWidget::registerValue(Register16 reg) const
{
    switch (reg) {
        case Register16::SP:
            return m_sp.value();

        case Register16::PC:
            return m_pc.value();

        default:
            Util::debug << "Only registers PC and SP are present in this widget.\n";
            return 0;
    }
}
