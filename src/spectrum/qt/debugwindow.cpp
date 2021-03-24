#include "debugwindow.h"

#include <iostream>
#include <iomanip>
#include <QLineEdit>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QDockWidget>
#include <QMenu>
#include <QSettings>

#include "thread.h"
#include "registerpairwidget.h"
#include "programcounterbreakpoint.h"

using namespace Spectrum::Qt;

using ::Z80::InterruptMode;
using ::Z80::UnsignedWord;
using ::Z80::Register16;

DebugWindow::DebugWindow(QWidget * parent )
: DebugWindow(nullptr, parent)
{
}

DebugWindow::DebugWindow(Thread * thread, QWidget * parent )
: QMainWindow(parent),
  m_thread(thread),
  m_registers(),
  m_disassembly(m_thread->spectrum()),
  m_shadowRegisters(),
  m_pointers(),
  m_interrupts(),
  m_memoryWidget(thread->spectrum()),
  m_step(QIcon::fromTheme(QStringLiteral("debug-step-instruction")), tr("Step")),
  m_pauseResume(QIcon::fromTheme(QStringLiteral("media-playback-start")), tr("Resume")),
  m_refresh(QIcon::fromTheme(QStringLiteral("view-refresh")), tr("Refresh")),
  m_status(),
  m_navigateToPc(tr("Navigate to PC")),
  m_breakpointAtPc(tr("Set breakpoint here")),
  m_navigateToSp(tr("Navigate to SP")),
  m_breakpointAtStackTop(tr("Set breakpoint at address on top of stack")),
  m_keyboardMonitor(&m_thread->spectrum()),
  m_poke(),
  m_cpuObserver((*this)),
  m_breakpoints()
{
    m_disassembly.enablePcIndicator(true);

    m_memoryWidget.setContextMenuPolicy(::Qt::ContextMenuPolicy::CustomContextMenu);

    m_pointers.addProgramCounterAction(&m_navigateToPc);
    m_pointers.addProgramCounterAction(&m_breakpointAtPc);
    m_pointers.addStackPointerAction(&m_navigateToSp);
    m_pointers.addStackPointerAction(&m_breakpointAtStackTop);

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
    auto * toolBar = addToolBar(tr("Debug"));
    toolBar->setObjectName(QStringLiteral("debug-toolBar"));
    toolBar->addAction(&m_pauseResume);
    toolBar->addAction(&m_step);
    toolBar->addSeparator();
    toolBar->addAction(&m_refresh);
    toolBar->addWidget(&m_status);
}

void DebugWindow::createDockWidgets()
{
    auto * dock = new QDockWidget(tr("Keyboard"), this);
    dock->setObjectName(QStringLiteral("keyboard-monitor-dock"));
    dock->setWidget(&m_keyboardMonitor);
    addDockWidget(::Qt::DockWidgetArea::BottomDockWidgetArea, dock);

    dock = new QDockWidget(tr("Memory"), this);
    dock->setObjectName(QStringLiteral("memory-dock"));
    dock->setWidget(&m_memoryWidget);
    addDockWidget(::Qt::DockWidgetArea::RightDockWidgetArea, dock);

    dock = new QDockWidget(tr("Poke"), this);
    dock->setObjectName(QStringLiteral("poke-dock"));
    dock->setWidget(&m_poke);
    addDockWidget(::Qt::DockWidgetArea::RightDockWidgetArea, dock);

    dock = new QDockWidget(tr("Shadow registers"));
    dock->setObjectName(QStringLiteral("shadow-registers-dock"));
    auto * shadowRegisters = new QWidget();
    auto * shadowRegistersLayout = new QVBoxLayout();

    shadowRegistersLayout->setSpacing(2);
    shadowRegistersLayout->addWidget(&m_shadowRegisters);
    shadowRegistersLayout->addStretch(10);
    shadowRegisters->setLayout(shadowRegistersLayout);
    dock->setWidget(shadowRegisters);
    addDockWidget(::Qt::DockWidgetArea::LeftDockWidgetArea, dock);
}

void DebugWindow::layoutWidget()
{
	QLabel * tmpLabel;

	// registers
	auto * plainRegisters = new QGroupBox(tr("Registers"));
	auto * plainRegistersLayout = new QVBoxLayout();

    plainRegistersLayout->addWidget(&m_registers);
	plainRegistersLayout->setSpacing(2);
	plainRegistersLayout->addStretch(10);
	plainRegisters->setLayout(plainRegistersLayout);

	// interrupts and instruction counter
	auto * interrupts = new QGroupBox(tr("Interrupts/Refresh"));
	auto * interruptsLayout =  new QVBoxLayout();
	interruptsLayout->addWidget(&m_interrupts);
	interrupts->setLayout(interruptsLayout);

	// program counter and stack pointer
    auto * pointers = new QGroupBox(tr("Pointers"));
    auto * pointersLayout = new QVBoxLayout();
    pointersLayout->addWidget(&m_pointers);
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

    connect(&m_navigateToPc, &QAction::triggered, this, &DebugWindow::locateProgramCounterInMemory);
    connect(&m_navigateToPc, &QAction::triggered, this, &DebugWindow::locateProgramCounterInDisassembly);
    connect(&m_breakpointAtPc, &QAction::triggered, [this]() {
        setProgramCounterBreakpointTriggered(m_pointers.registerValue(Register16::PC));
    });

    connect(&m_navigateToSp, &QAction::triggered, this, &DebugWindow::locateStackPointerInDisassembly);
    connect(&m_navigateToSp, &QAction::triggered, this, &DebugWindow::locateStackPointerInMemory);
    connect(&m_breakpointAtStackTop, &QAction::triggered, [this]() {
        auto addr = m_pointers.registerValue(Register16::SP);

        if (addr > 0xffff - 2) {
            std::cerr << "Can't set a breakpoint at address on top of stack - stack is currently < 2 bytes in size\n";
            m_status.setText("Can't set breakpoint - the top of the stack does not contain an address.");
            return;
        }

        breakAtProgramCounter(m_thread->spectrum().z80()->peekUnsignedWord(addr));
    });

    connect(&m_memoryWidget, &QWidget::customContextMenuRequested, this, &DebugWindow::memoryContextMenuRequested);
    connect(&m_memoryWidget, &MemoryDebugWidget::programCounterBreakpointRequested, this, &DebugWindow::setProgramCounterBreakpointTriggered);

    connect(&m_poke, &PokeWidget::pokeClicked, [this](Z80::UnsignedWord address, Z80::UnsignedByte value) -> void {
        m_thread->spectrum().memory()[address] = value;
        updateStateDisplay();
    });

    connect(&m_registers, &RegistersWidget::registerChanged, [this](::Z80::Register16 reg, ::Z80::UnsignedWord value) {
        assert(m_thread);
        auto * cpu = m_thread->spectrum().z80();
        assert(cpu);
        cpu->setRegisterValue(reg, value);
    });

    connect(&m_shadowRegisters, &ShadowRegistersWidget::registerChanged, [this](::Z80::Register16 reg, ::Z80::UnsignedWord value) {
        assert(m_thread);
        auto * cpu = m_thread->spectrum().z80();
        assert(cpu);
        cpu->setRegisterValue(reg, value);
    });

    connect(&m_pointers, &ProgramPointersWidget::registerChanged, [this](::Z80::Register16 reg, ::Z80::UnsignedWord value) {
        assert(m_thread);
        auto * cpu = m_thread->spectrum().z80();
        assert(cpu);
        cpu->setRegisterValue(reg, value);
    });

    connect(&m_interrupts, &InterruptWidget::registerChanged, [this](::Z80::Register8 reg, ::Z80::UnsignedByte value) {
        assert(m_thread);
        auto * cpu = m_thread->spectrum().z80();
        assert(cpu);
        cpu->setRegisterValue(reg, value);
    });

    connect(&m_interrupts, &InterruptWidget::interruptModeChanged, [this](::Z80::InterruptMode mode) {
        assert(m_thread);
        auto * cpu = m_thread->spectrum().z80();
        assert(cpu);
        cpu->setInterruptMode(mode);
    });
}

void DebugWindow::closeEvent(QCloseEvent * ev)
{
    m_thread->spectrum().z80()->removeInstructionObserver(&m_cpuObserver);
    QSettings settings;
    settings.beginGroup(QStringLiteral("debugwindow"));
    settings.setValue(QStringLiteral("position"), pos());
    settings.setValue(QStringLiteral("size"), size());
    settings.setValue(QStringLiteral("windowState"), saveState());
    settings.endGroup();
    QMainWindow::closeEvent(ev);
}

void DebugWindow::showEvent(QShowEvent * ev)
{
    m_thread->spectrum().z80()->addInstructionObserver(&m_cpuObserver);
    QSettings settings;
    settings.beginGroup(QStringLiteral("debugwindow"));
    setGeometry({settings.value(QStringLiteral("position")).toPoint(), settings.value(QStringLiteral("size")).toSize()});
    restoreState(settings.value(QStringLiteral("windowState")).toByteArray());
    settings.endGroup();
    QMainWindow::showEvent(ev);
}

void DebugWindow::updateStateDisplay()
{
    assert(m_thread);
	auto * cpu = m_thread->spectrum().z80();
	assert(cpu);
	auto & registers = cpu->registers();
	m_registers.setRegisters(registers);
	m_shadowRegisters.setRegisters(registers);
	m_pointers.setRegisters(registers);
	m_interrupts.setRegisters(registers);
	m_interrupts.setInterruptMode((cpu->interruptMode()));
	m_memoryWidget.clearHighlights();
	m_memoryWidget.setHighlight(m_pointers.registerValue(::Z80::Register16::PC), qRgb(0x80, 0xe0, 0x80), qRgba(0, 0, 0, 0.0));
	m_memoryWidget.setHighlight(m_pointers.registerValue(::Z80::Register16::SP), qRgb(0x80, 0x80, 0xe0), qRgba(0, 0, 0, 0.0));
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
    m_registers.setEnabled(true);
    m_pointers.setEnabled(true);
    m_interrupts.setEnabled(true);
    m_disassembly.setEnabled(true);
    m_shadowRegisters.setEnabled(true);
    m_poke.setEnabled(true);
    m_memoryWidget.setEnabled(true);
    m_keyboardMonitor.setEnabled(true);
    m_step.setEnabled(true);
    updateStateDisplay();
    connect(m_thread, &Thread::stepped, this, &DebugWindow::threadStepped, ::Qt::UniqueConnection);
}

void DebugWindow::threadResumed()
{
    m_pauseResume.setIcon(QIcon::fromTheme(QStringLiteral("media-playback-pause")));
    m_pauseResume.setText(tr("Pause"));
    m_registers.setEnabled(false);
    m_pointers.setEnabled(false);
    m_interrupts.setEnabled(false);
    m_disassembly.setEnabled(false);
    m_shadowRegisters.setEnabled(false);
    m_poke.setEnabled(false);
    m_memoryWidget.setEnabled(false);
    m_keyboardMonitor.setEnabled(false);
    m_step.setEnabled(false);
    disconnect(m_thread, &Thread::stepped, this, &DebugWindow::threadStepped);
}

void DebugWindow::threadStepped()
{
    updateStateDisplay();
}

void DebugWindow::setProgramCounterBreakpointTriggered(UnsignedWord addr)
{
    if (0 > addr || 0xffff < addr) {
        std::cerr << "invalid breakpoint address: " << std::hex << std::setfill('0') << std::setw(4) << addr << std::dec << std::setfill(' ') << "\n";
        return;
    }

    breakAtProgramCounter(addr);
}

void DebugWindow::breakAtProgramCounter(UnsignedWord addr)
{
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
        m_disassembly.scrollToAddress(addr);
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

void DebugWindow::locateProgramCounterInMemory()
{
    m_memoryWidget.scrollToAddress(m_pointers.registerValue(Register16::PC));
}

void DebugWindow::locateStackPointerInMemory()
{
    m_memoryWidget.scrollToAddress(m_pointers.registerValue(Register16::SP));
}

void DebugWindow::locateProgramCounterInDisassembly()
{
    m_disassembly.scrollToAddress(m_pointers.registerValue(Register16::PC));
}

void DebugWindow::locateStackPointerInDisassembly()
{
    m_disassembly.scrollToAddress(m_pointers.registerValue(Register16::SP));
}

void DebugWindow::InstructionObserver::notify(::Spectrum::Z80 * cpu)
{
    const auto & spectrum = window.m_thread->spectrum();

    for (auto * breakpoint : window.m_breakpoints) {
        breakpoint->check(spectrum);
    }
}
