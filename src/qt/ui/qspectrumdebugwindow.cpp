#include "qspectrumdebugwindow.h"

#include <QSpinBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QSettings>

#include "../spectrumthread.h"
#include "../../emulator/spectrum.h"
#include "../../z80/z80.h"
#include "registerpairwidget.h"

using namespace Spectrum;

QSpectrumDebugWindow::QSpectrumDebugWindow( QWidget * parent )
:	QSpectrumDebugWindow(nullptr, parent)
{
}

QSpectrumDebugWindow::QSpectrumDebugWindow(SpectrumThread * thread, QWidget * parent )
: QMainWindow(parent),
  m_thread(thread),
  m_af(QStringLiteral("AF")),
  m_bc(QStringLiteral("BC")),
  m_de(QStringLiteral("DE")),
  m_hl(QStringLiteral("HL")),
  m_ix(QStringLiteral("IX")),
  m_iy(QStringLiteral("IY")),
  m_afshadow(QStringLiteral("AF'")),
  m_bcshadow(QStringLiteral("BC'")),
  m_deshadow(QStringLiteral("DE'")),
  m_hlshadow(QStringLiteral("HL'")),
  m_pc(4),
  m_sp(4),
  m_flags(),
  m_shadowFlags(),
  m_memoryWidget(thread->spectrum()),
  m_memoryPc(),
  m_memorySp(),
  m_step(QIcon::fromTheme(QStringLiteral("debug-step-instruction")), tr("Step")),
  m_pauseResume(QIcon::fromTheme(QStringLiteral("media-playback-play")), tr("Resume")),
  m_refresh(QIcon::fromTheme(QStringLiteral("view-refresh")), tr("Refresh"))
{
    m_pc.setMinimum(0);
    m_pc.setMaximum(0xffff);
    m_sp.setMinimum(0);
    m_sp.setMaximum(0xffff);

    m_memoryPc.setText(QStringLiteral("PC"));
    m_memorySp.setText(QStringLiteral("SP"));

    QFont widgetFont = m_af.font();
    widgetFont.setPointSizeF(widgetFont.pointSizeF() * 0.85);

    for (auto * widget : {&m_af, &m_bc, &m_de, &m_hl, &m_ix, &m_iy, &m_afshadow, &m_bcshadow, &m_deshadow, &m_hlshadow, }) {
        widget->setFont(widgetFont);
    }

    m_flags.setFont(widgetFont);
    m_shadowFlags.setFont(widgetFont);
    m_pc.setFont(widgetFont);
    m_sp.setFont(widgetFont);

    setWindowTitle(tr("Spectrum Debugger"));
    createToolbars();
    layoutWidget();

	connect(m_thread, &SpectrumThread::paused, this, &QSpectrumDebugWindow::threadPaused, Qt::UniqueConnection);
	connect(m_thread, &SpectrumThread::resumed, this, &QSpectrumDebugWindow::threadResumed, Qt::UniqueConnection);

	connectWidgets();
}

void QSpectrumDebugWindow::createToolbars()
{
    auto * toolbar = addToolBar(tr("Debug"));
    toolbar->addAction(&m_pauseResume);
    toolbar->addAction(&m_step);
    toolbar->addSeparator();
    toolbar->addAction(&m_refresh);
}

void QSpectrumDebugWindow::layoutWidget()
{
	// registers
	auto * plainRegisters = new QGroupBox(tr("Registers"));
	auto * plainRegistersLayout = new QVBoxLayout();
	QLabel * tmpLabel;

	plainRegistersLayout->addWidget(&m_af);
	plainRegistersLayout->addWidget(&m_bc);
	plainRegistersLayout->addWidget(&m_de);
	plainRegistersLayout->addWidget(&m_hl);
	plainRegistersLayout->addWidget(&m_ix);
	plainRegistersLayout->addWidget(&m_iy);

	auto * flagsLayout = new QHBoxLayout();
    tmpLabel = new QLabel("Flags");
    tmpLabel->setBuddy(&m_flags);
    tmpLabel->setFont(m_flags.font());
	flagsLayout->addWidget(tmpLabel);
	flagsLayout->addWidget(&m_flags);
	flagsLayout->addStretch(10);
	plainRegistersLayout->addLayout(flagsLayout);

	plainRegistersLayout->addStretch(10);
	plainRegisters->setLayout(plainRegistersLayout);

	// program counter and stack pointer
    auto * pointers = new QGroupBox(tr("Pointers"));
    auto * pointersLayout = new QVBoxLayout();

    auto * regLayout = new QHBoxLayout();
    tmpLabel = new QLabel("SP");
    tmpLabel->setBuddy(&m_sp);
    tmpLabel->setFont(m_sp.font());
    tmpLabel->setMinimumHeight(m_sp.minimumHeight());
    regLayout->addWidget(tmpLabel);
    regLayout->addWidget(&m_sp);
    pointersLayout->addLayout(regLayout);

    regLayout = new QHBoxLayout();
    tmpLabel = new QLabel("PC");
    tmpLabel->setBuddy(&m_pc);
    tmpLabel->setFont(m_pc.font());
    tmpLabel->setMinimumHeight(m_pc.minimumHeight());
    regLayout->addWidget(tmpLabel);
    regLayout->addWidget(&m_pc);
    pointersLayout->addLayout(regLayout);
    pointers->setLayout(pointersLayout);

	// shadow registers
    auto * shadowRegisters = new QGroupBox(tr("Shadow registers"));
    auto * shadowRegistersLayout = new QVBoxLayout();

	shadowRegistersLayout->addWidget(&m_afshadow);
	shadowRegistersLayout->addWidget(&m_bcshadow);
	shadowRegistersLayout->addWidget(&m_deshadow);
	shadowRegistersLayout->addWidget(&m_hlshadow);

    flagsLayout = new QHBoxLayout();
    tmpLabel = new QLabel("Flags");
    tmpLabel->setBuddy(&m_shadowFlags);
    tmpLabel->setFont(m_shadowFlags.font());
    flagsLayout->addWidget(tmpLabel);
    flagsLayout->addWidget(&m_shadowFlags);
    flagsLayout->addStretch(10);
    shadowRegistersLayout->addLayout(flagsLayout);

    shadowRegistersLayout->addStretch(10);
    shadowRegisters->setLayout(shadowRegistersLayout);

    auto * memory = new QGroupBox(tr("Memory"));
    auto * memoryLayout = new QVBoxLayout();
    memoryLayout->addWidget(&m_memoryWidget);
    memory->setLayout(memoryLayout);
    auto * tmpLayout = new QHBoxLayout();
    tmpLayout->addStretch(10);
    tmpLayout->addWidget(&m_memoryPc);
    tmpLayout->addWidget(&m_memorySp);
    memoryLayout->addLayout(tmpLayout);

	auto * mainLayout = new QHBoxLayout();
    mainLayout->addWidget(plainRegisters);
    auto * rightLayout = new QVBoxLayout();
    rightLayout->addWidget(shadowRegisters);
    rightLayout->addWidget(pointers);
    mainLayout->addLayout(rightLayout);
    mainLayout->addWidget(memory);

    auto * centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

void QSpectrumDebugWindow::connectWidgets()
{
    connect(&m_pauseResume, &QAction::triggered, this, &QSpectrumDebugWindow::pauseResumeTriggered);
    connect(&m_refresh, &QAction::triggered, this, &QSpectrumDebugWindow::updateStateDisplay);
    connect(&m_step, &QAction::triggered, this, &QSpectrumDebugWindow::stepTriggered);

    connect(&m_memoryPc, &QToolButton::clicked, this, &QSpectrumDebugWindow::scrollMemoryToPcTriggered);
    connect(&m_memorySp, &QToolButton::clicked, this, &QSpectrumDebugWindow::scrollMemoryToSpTriggered);

    for (auto * widget : {&m_af, &m_bc, &m_de, &m_hl, &m_ix, &m_iy, &m_afshadow, &m_bcshadow, &m_deshadow, &m_hlshadow, }) {
        connect(widget, &RegisterPairWidget::valueChanged, [this, widget]() {
            assert(m_thread);
            auto * cpu = m_thread->spectrum().z80();
            assert(cpu);
            auto & registers = cpu->registers();

            if(widget == &m_af) registers.af = widget->value();
            else if(widget == &m_bc) registers.bc = widget->value();
            else if(widget == &m_de) registers.de = widget->value();
            else if(widget == &m_hl) registers.hl = widget->value();
            else if(widget == &m_ix) registers.ix = widget->value();
            else if(widget == &m_iy) registers.iy = widget->value();
            else if(widget == &m_afshadow) registers.afShadow = widget->value();
            else if(widget == &m_bcshadow) registers.bcShadow = widget->value();
            else if(widget == &m_deshadow) registers.deShadow = widget->value();
            else if(widget == &m_hlshadow) registers.hlShadow = widget->value();
        });
    }

    for (auto * widget : {&m_sp, &m_pc, }) {
        connect(widget, &QSpinBox::editingFinished, [this, widget]() {
            assert(m_thread);
            auto * cpu = m_thread->spectrum().z80();
            assert(cpu);
            auto & registers = cpu->registers();
            if(widget == &m_pc) registers.pc = widget->value();
            else if(widget == &m_sp) registers.sp = widget->value();
        });
    }
}

void QSpectrumDebugWindow::closeEvent(QCloseEvent * ev)
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("debugwindow"));
    settings.setValue(QStringLiteral("position"), pos());
    settings.setValue(QStringLiteral("size"), size());
    settings.endGroup();
    QWidget::closeEvent(ev);
}

void QSpectrumDebugWindow::showEvent(QShowEvent * ev)
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("debugwindow"));
    setGeometry({settings.value(QStringLiteral("position")).toPoint(), settings.value(QStringLiteral("size")).toSize()});
    settings.endGroup();
}

#include <iostream>

void QSpectrumDebugWindow::updateStateDisplay()
{
    assert(m_thread);
	auto * cpu = m_thread->spectrum().z80();
	assert(cpu);
	auto & registers = cpu->registers();
	m_af.setValue(registers.af);
	m_bc.setValue(registers.bc);
	m_de.setValue(registers.de);
	m_hl.setValue(registers.hl);
	m_ix.setValue(registers.ix);
	m_iy.setValue(registers.iy);
	m_afshadow.setValue(registers.afShadow);
	m_bcshadow.setValue(registers.bcShadow);
	m_deshadow.setValue(registers.deShadow);
	m_hlshadow.setValue(registers.hlShadow);
	m_pc.setValue(registers.pc);
	m_sp.setValue(registers.sp);
	m_flags.setAllFlags(registers.f);
	m_shadowFlags.setAllFlags(registers.fShadow);
	m_memoryWidget.clearHighlights();
	m_memoryWidget.setHighlight(m_pc.value(), qRgb(0x80, 0xe0, 0x80), qRgba(0, 0, 0, 0.0));
	m_memoryWidget.setHighlight(m_sp.value(), qRgb(0x80, 0x80, 0xe0), qRgba(0, 0, 0, 0.0));
	m_memoryWidget.update();
}

void QSpectrumDebugWindow::pauseResumeTriggered()
{
    assert(m_thread);

    if (m_thread->isPaused()) {
        m_thread->resume();
    } else {
        m_thread->pause();
    }
}

void QSpectrumDebugWindow::stepTriggered()
{
    assert(m_thread);
    m_thread->step();
}

void QSpectrumDebugWindow::threadPaused()
{
    m_pauseResume.setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
    m_pauseResume.setText(tr("Resume"));

    // TODO enable widgets
    centralWidget()->setEnabled(true);
    m_step.setEnabled(true);
    updateStateDisplay();
    connect(m_thread, &SpectrumThread::stepped, this, &QSpectrumDebugWindow::threadStepped, Qt::UniqueConnection);
}

void QSpectrumDebugWindow::threadResumed()
{
    m_pauseResume.setIcon(QIcon::fromTheme(QStringLiteral("media-playback-pause")));
    m_pauseResume.setText(tr("Pause"));

    // TODO disable widgets
    centralWidget()->setEnabled(false);
    m_step.setEnabled(false);
    disconnect(m_thread, &SpectrumThread::stepped, this, &QSpectrumDebugWindow::threadStepped);
}

void QSpectrumDebugWindow::threadStepped()
{
    updateStateDisplay();
}

void QSpectrumDebugWindow::scrollMemoryToPcTriggered()
{
    m_memoryWidget.scrollToAddress(m_pc.value());
}

void QSpectrumDebugWindow::scrollMemoryToSpTriggered()
{
    m_memoryWidget.scrollToAddress(m_sp.value());
}
