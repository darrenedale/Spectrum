#include "debugwindow.h"

#include <iostream>
#include <iomanip>
#include <QSpinBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QDockWidget>
#include <QMenu>
#include <QSettings>

#include "../spectrum.h"
#include "../../z80/z80.h"
#include "thread.h"
#include "registerpairwidget.h"
#include "keyboardmonitorwidget.h"
#include "pokewidget.h"
#include "programcounterbreakpoint.h"

using namespace Spectrum::Qt;

using InterruptMode = ::Z80::InterruptMode;

DebugWindow::DebugWindow(QWidget * parent )
: DebugWindow(nullptr, parent)
{
}

DebugWindow::DebugWindow(Thread * thread, QWidget * parent )
: QMainWindow(parent),
  m_thread(thread),
  m_af(QStringLiteral("AF")),
  m_bc(QStringLiteral("BC")),
  m_de(QStringLiteral("DE")),
  m_hl(QStringLiteral("HL")),
  m_ix(QStringLiteral("IX")),
  m_iy(QStringLiteral("IY")),
  m_disassembly(m_thread->spectrum()),
  m_afshadow(QStringLiteral("AF'")),
  m_bcshadow(QStringLiteral("BC'")),
  m_deshadow(QStringLiteral("DE'")),
  m_hlshadow(QStringLiteral("HL'")),
  m_pc(4),
  m_sp(4),
  m_i(2),
  m_r(2),
  m_im(),
  m_flags(),
  m_shadowFlags(),
  m_memoryWidget(thread->spectrum()),
  m_memoryLocation(),
  m_setBreakpoint(),
  m_memoryPc(),
  m_memorySp(),
  m_step(QIcon::fromTheme(QStringLiteral("debug-step-instruction")), tr("Step")),
  m_pauseResume(QIcon::fromTheme(QStringLiteral("media-playback-start")), tr("Resume")),
  m_refresh(QIcon::fromTheme(QStringLiteral("view-refresh")), tr("Refresh")),
  m_status(),
  m_keyboardMonitor(&m_thread->spectrum()),
  m_poke(),
  m_shadowRegistersDock(nullptr),
  m_memoryDock(nullptr),
  m_keyboardDock(nullptr),
  m_pokeDock(nullptr),
  m_cpuObserver((*this)),
  m_breakpoints()
{
    m_disassembly.enablePcIndicator(true);

    m_pc.setMinimum(0);
    m_pc.setMaximum(0xffff);
    m_sp.setMinimum(0);
    m_sp.setMaximum(0xffff);

    m_i.setMinimum(0);
    m_i.setMaximum(0xff);
    m_r.setMinimum(0);
    m_r.setMaximum(0xff);

    m_im.setMinimum(0);
    m_im.setMaximum(2);

    m_memoryWidget.setContextMenuPolicy(::Qt::ContextMenuPolicy::CustomContextMenu);
    m_setBreakpoint.setIcon(QIcon::fromTheme(QStringLiteral("process-stop")));
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
    createDockWidgets();
    layoutWidget();

	connect(m_thread, &Thread::paused, this, &DebugWindow::threadPaused, ::Qt::UniqueConnection);
	connect(m_thread, &Thread::resumed, this, &DebugWindow::threadResumed, ::Qt::UniqueConnection);

	connectWidgets();

	if (m_thread->isPaused()) {
	    threadPaused();
	} else {
	    threadResumed();
	}
}

DebugWindow::~DebugWindow()
{
    m_thread->spectrum().z80()->removeInstructionObserver(&m_cpuObserver);
}

void DebugWindow::createToolbars()
{
    auto * toolbar = addToolBar(tr("Debug"));
    toolbar->addAction(&m_pauseResume);
    toolbar->addAction(&m_step);
    toolbar->addSeparator();
    toolbar->addAction(&m_refresh);
    toolbar->addWidget(&m_status);
}

void DebugWindow::createDockWidgets()
{
    m_keyboardDock = new QDockWidget(tr("Keyboard"), this);
    m_keyboardDock->setWidget(&m_keyboardMonitor);
    addDockWidget(::Qt::DockWidgetArea::BottomDockWidgetArea, m_keyboardDock);

    m_memoryDock = new QDockWidget(tr("Memory"), this);
    auto * memory = new QWidget();
    auto * memoryLayout = new QVBoxLayout();
    memoryLayout->addWidget(&m_memoryWidget);
    auto * tmpLayout = new QHBoxLayout();
    tmpLayout->addWidget(&m_memoryLocation);
    tmpLayout->addWidget(&m_setBreakpoint);
    tmpLayout->addStretch(10);
    tmpLayout->addWidget(&m_memoryPc);
    tmpLayout->addWidget(&m_memorySp);
    memoryLayout->addLayout(tmpLayout);
    memory->setLayout(memoryLayout);
    m_memoryDock->setWidget(memory);
    addDockWidget(::Qt::DockWidgetArea::RightDockWidgetArea, m_memoryDock);

    m_pokeDock = new QDockWidget(tr("Poke"), this);
    m_pokeDock->setWidget(&m_poke);
    addDockWidget(::Qt::DockWidgetArea::RightDockWidgetArea, m_pokeDock);

    // shadow registers
    m_shadowRegistersDock = new QDockWidget(tr("Shadow registers"));
    auto * shadowRegisters = new QWidget();
    auto * shadowRegistersLayout = new QVBoxLayout();

    shadowRegistersLayout->setSpacing(2);
    shadowRegistersLayout->addWidget(&m_afshadow);
    shadowRegistersLayout->addWidget(&m_bcshadow);
    shadowRegistersLayout->addWidget(&m_deshadow);
    shadowRegistersLayout->addWidget(&m_hlshadow);

    auto * flagsLayout = new QHBoxLayout();
    auto * tmpLabel = new QLabel("Flags");
    tmpLabel->setBuddy(&m_shadowFlags);
    tmpLabel->setFont(m_shadowFlags.font());
    flagsLayout->addWidget(tmpLabel);
    flagsLayout->addWidget(&m_shadowFlags);
    flagsLayout->addStretch(10);
    shadowRegistersLayout->addLayout(flagsLayout);
    shadowRegistersLayout->addStretch(10);

    shadowRegisters->setLayout(shadowRegistersLayout);
    m_shadowRegistersDock->setWidget(shadowRegisters);
    addDockWidget(::Qt::DockWidgetArea::LeftDockWidgetArea, m_shadowRegistersDock);
}

void DebugWindow::layoutWidget()
{
	// registers
	auto * plainRegisters = new QGroupBox(tr("Registers"));
	auto * plainRegistersLayout = new QVBoxLayout();
	QLabel * tmpLabel;

	plainRegistersLayout->setSpacing(2);
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

	// interrupts and instruction counter
	auto * interrupts = new QGroupBox(tr("Interrupts/Refresh"));
	auto * interruptsLayout =  new QHBoxLayout();
	tmpLabel = new QLabel(tr("Mode"));
	tmpLabel->setBuddy(&m_im);
    interruptsLayout->addWidget(tmpLabel, 0);
    interruptsLayout->addWidget(&m_im, 10);
	tmpLabel = new QLabel(tr("I"));
	tmpLabel->setBuddy(&m_i);
    interruptsLayout->addWidget(tmpLabel, 0);
    interruptsLayout->addWidget(&m_i, 10);
	tmpLabel = new QLabel(tr("R"));
	tmpLabel->setBuddy(&m_r);
    interruptsLayout->addWidget(tmpLabel, 0);
    interruptsLayout->addWidget(&m_r, 10);

	interrupts->setLayout(interruptsLayout);

	// program counter and stack pointer
    auto * pointers = new QGroupBox(tr("Pointers"));
    auto * pointersLayout = new QVBoxLayout();

    auto * regLayout = new QHBoxLayout();
    tmpLabel = new QLabel("SP");
    tmpLabel->setBuddy(&m_sp);
    tmpLabel->setFont(m_sp.font());
    tmpLabel->setMinimumHeight(m_sp.minimumHeight());
    regLayout->addWidget(tmpLabel, 1);
    regLayout->addWidget(&m_sp, 10);
    pointersLayout->addLayout(regLayout);

    regLayout = new QHBoxLayout();
    tmpLabel = new QLabel("PC");
    tmpLabel->setBuddy(&m_pc);
    tmpLabel->setFont(m_pc.font());
    tmpLabel->setMinimumHeight(m_pc.minimumHeight());
    regLayout->addWidget(tmpLabel, 1);
    regLayout->addWidget(&m_pc, 10);
    pointersLayout->addLayout(regLayout);
    pointers->setLayout(pointersLayout);

    auto * disassembly = new QGroupBox(tr("Disassembly"));
    auto * disassemblyLayout = new QVBoxLayout();
    disassemblyLayout->addWidget(&m_disassembly);
    disassembly->setLayout(disassemblyLayout);

	auto * leftLayout = new QVBoxLayout();
    leftLayout->addWidget(plainRegisters);
    leftLayout->addWidget(interrupts);
    leftLayout->addWidget(pointers);

	auto * mainLayout = new QHBoxLayout();
	mainLayout->addLayout(leftLayout);
	mainLayout->addWidget(disassembly);

    auto * centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

void DebugWindow::connectWidgets()
{
    connect(&m_pauseResume, &QAction::triggered, this, &DebugWindow::pauseResumeTriggered);
    connect(&m_refresh, &QAction::triggered, this, &DebugWindow::updateStateDisplay);
    connect(&m_step, &QAction::triggered, this, &DebugWindow::stepTriggered);

    connect(&m_memoryWidget, &QWidget::customContextMenuRequested, this, &DebugWindow::memoryContextMenuRequested);
    connect(&m_memoryLocation, qOverload<int>(&HexSpinBox::valueChanged), this, &DebugWindow::memoryLocationChanged);
    connect(&m_setBreakpoint, &QToolButton::clicked, this, &DebugWindow::setBreakpointTriggered);
    connect(&m_memoryPc, &QToolButton::clicked, this, &DebugWindow::scrollMemoryToPcTriggered);
    connect(&m_memorySp, &QToolButton::clicked, this, &DebugWindow::scrollMemoryToSpTriggered);
    connect(&m_poke, &PokeWidget::pokeClicked, [this](Z80::UnsignedWord address, Z80::UnsignedByte value) -> void {
        m_thread->spectrum().memory()[address] = value;
        updateStateDisplay();
    });

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

    for (auto * widget : {&m_sp, &m_pc, &m_i, &m_r, }) {
        connect(widget, &QSpinBox::editingFinished, [this, widget]() {
            assert(m_thread);
            auto * cpu = m_thread->spectrum().z80();
            assert(cpu);
            auto & registers = cpu->registers();
            if(widget == &m_pc) registers.pc = widget->value();
            else if(widget == &m_sp) registers.sp = widget->value();
            else if(widget == &m_i) registers.i = widget->value();
            else if(widget == &m_r) registers.r = widget->value();
        });
    }

    connect(&m_im, qOverload<int>(&QSpinBox::valueChanged), [this](int value) {
        assert(m_thread);
        auto * cpu = m_thread->spectrum().z80();
        assert(cpu);
        cpu->setInterruptMode(static_cast<InterruptMode>(value));
    });
}

void DebugWindow::closeEvent(QCloseEvent * ev)
{
    m_thread->spectrum().z80()->removeInstructionObserver(&m_cpuObserver);
    QSettings settings;
    settings.beginGroup(QStringLiteral("debugwindow"));
    settings.setValue(QStringLiteral("position"), pos());
    settings.setValue(QStringLiteral("size"), size());
    settings.endGroup();
    QMainWindow::closeEvent(ev);
}

void DebugWindow::showEvent(QShowEvent * ev)
{
    m_thread->spectrum().z80()->addInstructionObserver(&m_cpuObserver);
    QSettings settings;
    settings.beginGroup(QStringLiteral("debugwindow"));
    setGeometry({settings.value(QStringLiteral("position")).toPoint(), settings.value(QStringLiteral("size")).toSize()});
    settings.endGroup();
    QMainWindow::showEvent(ev);
}

void DebugWindow::updateStateDisplay()
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
	m_im.setValue(static_cast<int>(cpu->interruptMode()));
	m_i.setValue(registers.i);
	m_r.setValue(registers.r);
	m_flags.setAllFlags(registers.f);
	m_shadowFlags.setAllFlags(registers.fShadow);
	m_memoryWidget.clearHighlights();
	m_memoryWidget.setHighlight(m_pc.value(), qRgb(0x80, 0xe0, 0x80), qRgba(0, 0, 0, 0.0));
	m_memoryWidget.setHighlight(m_sp.value(), qRgb(0x80, 0x80, 0xe0), qRgba(0, 0, 0, 0.0));
	m_memoryWidget.update();
	m_keyboardMonitor.updateStateDisplay();
	m_disassembly.updateMnemonics(cpu->pc());
	m_disassembly.setPc(cpu->pc());
	m_disassembly.scrollToPc();
}

void DebugWindow::pauseResumeTriggered()
{
    assert(m_thread);

    if (m_thread->isPaused()) {
        m_thread->resume();
    } else {
        m_thread->pause();
    }
}

void DebugWindow::stepTriggered()
{
    assert(m_thread);
    m_thread->step();
}

void DebugWindow::threadPaused()
{
    m_pauseResume.setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
    m_pauseResume.setText(tr("Resume"));
    centralWidget()->setEnabled(true);
    m_poke.setEnabled(true);
    m_shadowRegistersDock->setEnabled(true);
    m_memoryDock->setEnabled(true);
    m_keyboardDock->setEnabled(true);
    m_step.setEnabled(true);
    updateStateDisplay();
    connect(m_thread, &Thread::stepped, this, &DebugWindow::threadStepped, ::Qt::UniqueConnection);
}

void DebugWindow::threadResumed()
{
    m_pauseResume.setIcon(QIcon::fromTheme(QStringLiteral("media-playback-pause")));
    m_pauseResume.setText(tr("Pause"));
    centralWidget()->setEnabled(false);
    m_poke.setEnabled(false);
    m_shadowRegistersDock->setEnabled(false);
    m_memoryDock->setEnabled(false);
    m_keyboardDock->setEnabled(false);
    m_step.setEnabled(false);
    disconnect(m_thread, &Thread::stepped, this, &DebugWindow::threadStepped);
}

void DebugWindow::threadStepped()
{
    updateStateDisplay();
}

void DebugWindow::scrollMemoryToPcTriggered()
{
    auto addr = m_pc.value();
    m_memoryLocation.setValue(addr);
    m_memoryWidget.scrollToAddress(addr);
}

void DebugWindow::scrollMemoryToSpTriggered()
{
    auto addr = m_sp.value();
    m_memoryLocation.setValue(addr);
    m_memoryWidget.scrollToAddress(addr);
}

void DebugWindow::memoryLocationChanged()
{
    m_memoryWidget.scrollToAddress(m_memoryLocation.value());
}

void DebugWindow::setBreakpointTriggered()
{
    auto addr = m_memoryLocation.value();

    if (0 > addr || 0xffff < addr) {
        std::cerr << "invalid breakpoint address: " << std::hex << std::setfill('0') << std::setw(4) << addr << std::dec << std::setfill(' ') << "\n";
        return;
    }

    auto existingBreakpoint = std::find_if(m_breakpoints.cbegin(), m_breakpoints.cend(), [addr](const auto * breakpoint) -> bool {
        auto * pcBreakpoint = dynamic_cast<const ProgramCounterBreakpoint *>(breakpoint);
        return pcBreakpoint && pcBreakpoint->address() == addr;
    });

    if (existingBreakpoint != m_breakpoints.cend()) {
        std::cerr << "breakpoint already set: 0x" << std::hex << std::setfill('0') << std::setw(4) << addr << std::dec << std::setfill(' ') << "\n";
        return;
    }

    std::cout << "setting breakpoint at 0x" << std::hex << std::setfill('0') << std::setw(4) << addr << std::dec << std::setfill(' ') << "\n";
    setStatus(tr("Breakpoint set at PC = 0x%1.").arg(addr, 4, 16, QLatin1Char('0')));

    auto * breakpoint = new ProgramCounterBreakpoint(*m_thread, addr);
    connect(breakpoint, &ProgramCounterBreakpoint::triggered, [this, addr]() {
        std::cout << "PC breakpoint hit, navigating to memory location\n";
        setStatus(tr("Breakpoint hit: PC = 0x%1.").arg(addr, 4, 16, QLatin1Char('0')));
        show();
        activateWindow();
        raise();
        m_memoryWidget.scrollToAddress(addr);
        // TODO navigate disassembly to breakpoint address
    });

    m_breakpoints.push_back(breakpoint);
}

void DebugWindow::setStatus(const QString & status)
{
    m_status.setText(status);
}

void DebugWindow::clearStatus()
{
    m_status.clear();
}

void DebugWindow::memoryContextMenuRequested(const QPoint & pos)
{
    const auto address = m_memoryWidget.addressAt(pos);

    if (!address) {
        return;
    }

    auto value = m_thread->spectrum().memory()[*address];
    QMenu menu(this);
    menu.addSection(QStringLiteral("0x%1 : 0x%2")
        .arg(*address, 4, 16, QLatin1Char('0'))
        .arg(value, 2, 16, QLatin1Char('0')));

    menu.addAction(tr("Poke..."), [this, address = *address, value = m_thread->spectrum().memory()[*address]]() {
        m_poke.setValue(value);
        m_poke.setAddress(address);
    });

    menu.exec(m_memoryWidget.mapToGlobal(pos));
}

void DebugWindow::InstructionObserver::notify(::Spectrum::Z80 * cpu)
{
    const auto & spectrum = window.m_thread->spectrum();

    for (auto * breakpoint : window.m_breakpoints) {
        breakpoint->check(spectrum);
    }
}
