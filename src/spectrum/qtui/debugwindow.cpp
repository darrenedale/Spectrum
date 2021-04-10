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

#include "../spectrum48k.h"
#include "thread.h"
#include "registerpairwidget.h"
#include "programcounterbreakpoint.h"
#include "stackpointerbelowbreakpoint.h"
#include "memorychangedbreakpoint.h"

using namespace Spectrum::QtUi;

using ::Z80::InterruptMode;
using ::Z80::UnsignedWord;
using ::Z80::UnsignedByte;
using ::Z80::Register16;

namespace
{
    // when a status message is not permanent, how long should it be displayed for (in ms)?
    constexpr const int DefaultTransientMessageTimeout = 5000;
}

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
  m_breakpoints(),
  m_memoryBreakpointObserver(*this),
  m_pcObserver(*this),
  m_spBelowObserver(*this),
  m_statusClearTimer()
{
    m_disassembly.enablePcIndicator(true);

    m_memoryWidget.setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

    m_pointers.addProgramCounterAction(&m_navigateToPc);
    m_pointers.addProgramCounterAction(&m_breakpointAtPc);
    m_pointers.addStackPointerAction(&m_navigateToSp);
    m_pointers.addStackPointerAction(&m_breakpointAtStackTop);

    m_statusClearTimer.setSingleShot(true);

    setWindowTitle(tr("Spectrum Debugger"));
    createToolbars();
    createDockWidgets();
    layoutWidget();

	connect(m_thread, &Thread::paused, this, &DebugWindow::threadPaused, Qt::UniqueConnection);
	connect(m_thread, &Thread::resumed, this, &DebugWindow::threadResumed, Qt::UniqueConnection);
	connect(m_thread, &Thread::spectrumChanged, this, &DebugWindow::threadSpectrumChanged, Qt::UniqueConnection);

	connect(&m_statusClearTimer, &QTimer::timeout, this, &DebugWindow::clearStatusMessage);

	connectWidgets();

	if (m_thread->isPaused()) {
	    threadPaused();
	} else {
	    threadResumed();
	}

#if (!defined(NDEBUG))
	// break if stack pointer points into ROM
    breakIfStackPointerBelow(0x4000);
#endif
}

DebugWindow::~DebugWindow()
{
    m_thread->spectrum().z80()->removeInstructionObserver(&m_cpuObserver);

    for (auto * breakpoint : m_breakpoints) {
        delete breakpoint;
    }

    m_breakpoints.clear();
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
    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, dock);

    dock = new QDockWidget(tr("Memory"), this);
    dock->setObjectName(QStringLiteral("memory-dock"));
    dock->setWidget(&m_memoryWidget);
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dock);

    dock = new QDockWidget(tr("Poke"), this);
    dock->setObjectName(QStringLiteral("poke-dock"));
    dock->setWidget(&m_poke);
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dock);

    dock = new QDockWidget(tr("Shadow registers"));
    dock->setObjectName(QStringLiteral("shadow-registers-dock"));
    auto * shadowRegisters = new QWidget();
    auto * shadowRegistersLayout = new QVBoxLayout();

    shadowRegistersLayout->setSpacing(2);
    shadowRegistersLayout->addWidget(&m_shadowRegisters);
    shadowRegistersLayout->addStretch(10);
    shadowRegisters->setLayout(shadowRegistersLayout);
    dock->setWidget(shadowRegisters);
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, dock);
}

void DebugWindow::layoutWidget()
{
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
            showStatusMessage("Can't set breakpoint - the top of the stack does not contain an address.", DefaultTransientMessageTimeout);
            return;
        }

        breakAtProgramCounter(m_thread->spectrum().z80()->peekUnsignedHostWord(addr));
    });

    connect(&m_memoryWidget, &QWidget::customContextMenuRequested, this, &DebugWindow::memoryContextMenuRequested);
    connect(&m_memoryWidget, &MemoryDebugWidget::programCounterBreakpointRequested, this, &DebugWindow::setProgramCounterBreakpointTriggered);

    connect(&m_poke, &CustomPokeWidget::pokeClicked, [this](::Z80::UnsignedWord address, ::Z80::UnsignedByte value) -> void {
        m_thread->spectrum().memory()->writeByte(address, value);
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
    settings.beginGroup(QStringLiteral("debugWindow"));
    settings.setValue(QStringLiteral("position"), pos());
    settings.setValue(QStringLiteral("size"), size());
    settings.setValue(QStringLiteral("windowState"), saveState());
    settings.endGroup();
    QMainWindow::closeEvent(ev);
}

void DebugWindow::showEvent(QShowEvent * event)
{
    m_thread->spectrum().z80()->addInstructionObserver(&m_cpuObserver);

    QSettings settings;
    settings.beginGroup(QStringLiteral("debugWindow"));
    setGeometry({settings.value(QStringLiteral("position")).toPoint(), settings.value(QStringLiteral("size")).toSize()});
    restoreState(settings.value(QStringLiteral("windowState")).toByteArray());
    settings.endGroup();

    QMainWindow::showEvent(event);
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
	m_interrupts.setInterruptMode(cpu->interruptMode());
	m_interrupts.setIff1(cpu->iff1());
	m_interrupts.setIff2(cpu->iff2());
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
    connect(m_thread, &Thread::stepped, this, &DebugWindow::threadStepped, Qt::UniqueConnection);
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

void DebugWindow::threadSpectrumChanged()
{
    auto & spectrum = m_thread->spectrum();
    m_memoryWidget.setMemory(m_thread->spectrum().memory());
    m_disassembly.setMemory(m_thread->spectrum().memory());
    m_keyboardMonitor.setSpectrum(&spectrum);
}

void DebugWindow::setProgramCounterBreakpointTriggered(UnsignedWord addr)
{
    if (0 > addr || 0xffff < addr) {
        std::cerr << "invalid breakpoint address: " << std::hex << std::setfill('0') << std::setw(4) << addr << std::dec << std::setfill(' ') << "\n";
        return;
    }

    breakAtProgramCounter(addr);
}

void DebugWindow::breakAtProgramCounter(UnsignedWord address)
{
    auto * breakpoint = new ProgramCounterBreakpoint(address);

    if (!addBreakpoint(breakpoint)) {
        std::cerr << "breakpoint already set: 0x" << std::hex << std::setfill('0') << std::setw(4) << address << std::dec << std::setfill(' ') << "\n";
        delete breakpoint;
        return;
    }

    breakpoint->addObserver(&m_pcObserver);
    std::cout << "setting breakpoint at 0x" << std::hex << std::setfill('0') << std::setw(4) << address << std::dec << std::setfill(' ') << "\n";
    showStatusMessage(tr("Breakpoint set at PC = 0x%1.").arg(address, 4, 16, QLatin1Char('0')), DefaultTransientMessageTimeout);
}

void DebugWindow::breakIfStackPointerBelow(UnsignedWord address)
{
    auto * breakpoint = new StackPointerBelowBreakpoint(address);

    if (!addBreakpoint(breakpoint)) {
        std::cerr << "stack pointer breakpoint already set at 0x" << std::hex << std::setfill('0') << std::setw(4) << address << std::dec << std::setfill(' ') << "\n";
        delete breakpoint;
        return;
    }

    breakpoint->addObserver(&m_spBelowObserver);
    showStatusMessage(tr("Breakpoint set at SP < 0x%1.").arg(address, 4, 16, QLatin1Char('0')), DefaultTransientMessageTimeout);
}

bool DebugWindow::addBreakpoint(Breakpoint * breakpoint)
{
    if (hasBreakpoint(*breakpoint)) {
        return false;
    }

    m_breakpoints.push_back(breakpoint);
    return true;
}

bool DebugWindow::hasBreakpoint(const Breakpoint & breakpoint) const
{
    auto existingBreakpoint = std::find_if(m_breakpoints.cbegin(), m_breakpoints.cend(), [&breakpoint](const auto * existingBreakpoint) -> bool {
        return (*existingBreakpoint) == breakpoint;
    });

    return existingBreakpoint != m_breakpoints.cend();
}

void DebugWindow::showStatusMessage(const QString & status, int timeout)
{
    m_status.setText(status);
    m_statusClearTimer.stop();

    if (0 < timeout) {
        m_statusClearTimer.start(timeout);
    }
}

void DebugWindow::clearStatusMessage()
{
    m_status.clear();
}

void DebugWindow::memoryContextMenuRequested(const QPoint & pos)
{
    const auto address = m_memoryWidget.addressAt(pos);

    if (!address) {
        return;
    }

    auto value = m_thread->spectrum().memory()->readByte(*address);
    QMenu menu(this);
    menu.addSection(QStringLiteral("0x%1 : 0x%2")
        .arg(*address, 4, 16, QLatin1Char('0'))
        .arg(value, 2, 16, QLatin1Char('0')));

    menu.addAction(tr("Poke..."), [this, address = *address, value = m_thread->spectrum().memory()->readByte(*address)]() {
        m_poke.setValue(value);
        m_poke.setAddress(address);
        m_poke.focusValue();
    });

    auto * action = menu.addAction(tr("Break on PC"), [this, address = *address]() {
        breakAtProgramCounter(address);
    });

    action->setToolTip(tr("Break when the PC reaches 0x%1.").arg(*address, 4, 16, QLatin1Char('0')));

    action = menu.addAction(tr("Break on change (word)"), [this, address = *address]() {
        breakOnMemoryChange<UnsignedWord>(address);
    });

    action->setToolTip(tr("Break when the 16-bit value at 0x%1 changes.").arg(*address, 4, 16, QLatin1Char('0')));

    action = menu.addAction(tr("Break on change (byte)"), [this, address = *address]() {
        breakOnMemoryChange<UnsignedByte>(address);
    });

    action->setToolTip(tr("Break when the 16-bit value at 0x%1 changes.").arg(*address, 4, 16, QLatin1Char('0')));

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

void DebugWindow::programCounterBreakpointTriggered(UnsignedWord address)
{
    std::cout << "PC breakpoint hit, navigating to memory location\n";
    showStatusMessage(tr("Breakpoint hit: PC = 0x%1.").arg(address, 4, 16, QLatin1Char('0')));
    show();
    activateWindow();
    raise();
    m_memoryWidget.scrollToAddress(address);
    m_disassembly.scrollToAddress(address);

}

void DebugWindow::memoryChangeBreakpointTriggered(UnsignedWord address)
{
    std::cout << "Memory monitor breakpoint hit, navigating to memory location\n";
    showStatusMessage(tr("Monitored memory location modified: 0x%1.").arg(address, 4, 16, QLatin1Char('0')));
    show();
    activateWindow();
    raise();
    m_memoryWidget.scrollToAddress(address);
}

void DebugWindow::stackPointerBelowBreakpointTriggered(::Z80::UnsignedWord address)
{
    std::cout << "Stack pointer breakpoint hit, navigating to memory location\n";
    showStatusMessage(tr("Stack pointer is below 0x%1.").arg(address, 4, 16, QLatin1Char('0')));
    show();
    activateWindow();
    raise();
    m_memoryWidget.scrollToAddress(address);
    m_disassembly.scrollToPc();
}

void DebugWindow::InstructionObserver::notify(::Spectrum::Z80 * cpu)
{
    const auto & spectrum = window.m_thread->spectrum();

    for (auto * breakpoint : window.m_breakpoints) {
        breakpoint->check(spectrum);
    }
}

void DebugWindow::BreakpointObserver::notify(Breakpoint *)
{
    window.m_thread->pause();
}

void DebugWindow::MemoryBreakpointObserver::notify(Breakpoint * breakpoint)
{
    BreakpointObserver::notify(breakpoint);
    window.memoryChangeBreakpointTriggered(dynamic_cast<MemoryBreakpoint *>(breakpoint)->address());
}

void DebugWindow::ProgramCounterBreakpointObserver::notify(Breakpoint * breakpoint)
{
    BreakpointObserver::notify(breakpoint);
    window.programCounterBreakpointTriggered(dynamic_cast<ProgramCounterBreakpoint *>(breakpoint)->address());
}

void DebugWindow::StackPointerBelowBreakpointObserver::notify(Breakpoint * breakpoint)
{
    BreakpointObserver::notify(breakpoint);
    window.stackPointerBelowBreakpointTriggered(dynamic_cast<StackPointerBelowBreakpoint *>(breakpoint)->address());
}
