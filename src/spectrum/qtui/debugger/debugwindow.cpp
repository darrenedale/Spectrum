
#include <iomanip>
#include <iostream>
#include <print>

#include <QLineEdit>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QDockWidget>
#include <QHeaderView>
#include <QMenu>
#include <QClipboard>
#include <QSettings>

#include "breakpointscontextmenu.h"
#include "debugwindow.h"
#include "watchescontextmenu.h"
#include "../application.h"
#include "../hexspinboxdelegate.h"
#include "../registerpairwidget.h"
#include "../thread.h"
#include "../../spectrum48k.h"
#include "../../debugger/programcounterbreakpoint.h"
#include "../../debugger/stackpointerbelowbreakpoint.h"
#include "../../debugger/memorychangedbreakpoint.h"
#include "../../debugger/integermemorywatch.h"
#include "../../debugger/stringmemorywatch.h"
#include "../../../util/assert.h"
#include "../../../util/debug.h"

using namespace Spectrum::QtUi::Debugger;
using ::Z80::InterruptMode;
using ::Z80::UnsignedWord;
using ::Z80::UnsignedByte;
using ::Z80::Register16;
using Spectrum::Debugger::ProgramCounterBreakpoint;
using Spectrum::Debugger::StackPointerBelowBreakpoint;
using Spectrum::Debugger::MemoryBreakpoint;
using Spectrum::Debugger::MemoryWatch;
using Spectrum::Debugger::StringMemoryWatch;

namespace
{
    // when a status message is not permanent, how long should it be displayed for (in ms)?
    constexpr const int DefaultTransientMessageTimeout = 5000;
}

DebugWindow::DebugWindow(QWidget * parent )
: DebugWindow(nullptr, parent)
{}

DebugWindow::DebugWindow(Thread * thread, QWidget * parent )
: QMainWindow(parent),
  m_thread(thread),
  m_registers(),
  m_disassembly(m_thread->spectrum()),
  m_shadowRegisters(),
  m_pointers(),
  m_interrupts(),
  m_memoryWidget(thread->spectrum()),
  m_step(QIcon::fromTheme(QStringLiteral("debug-step-instruction"), Application::icon(QStringLiteral("step"))), tr("Step")),
  m_pauseResume(QIcon::fromTheme(QStringLiteral("media-playback-start"), Application::icon(QStringLiteral("resume"))), tr("Resume")),
  m_refresh(QIcon::fromTheme(QStringLiteral("view-refresh"), Application::icon(QStringLiteral("refresh"))), tr("Refresh")),
  m_status(),
  m_navigateToPc(tr("Navigate to PC")),
  m_breakpointAtPc(tr("Set breakpoint here")),
  m_navigateToSp(tr("Navigate to SP")),
  m_breakpointAtStackTop(tr("Set breakpoint at address on top of stack")),
  m_keyboardMonitor(&m_thread->spectrum()),
  m_watchesModel(),
  m_breakpointsModel(),
  m_watches(),
  m_breakpoints(),
  m_memoryMenu(0),
  m_poke(),
  m_cpuObserver((*this)),
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

    auto font = m_watches.font();
    font.setPointSizeF(font.pointSizeF() * 0.85);
    m_watches.setFont(font);
    m_watches.setModel(&m_watchesModel);

    m_breakpoints.setFont(font);
    m_breakpoints.setModel(&m_breakpointsModel);

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

    dock = new QDockWidget(tr("Watches"), this);
    dock->setObjectName(QStringLiteral("watches-dock"));
    dock->setWidget(&m_watches);
    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, dock);

    dock = new QDockWidget(tr("Breakpoints"), this);
    dock->setObjectName(QStringLiteral("breakpoints-dock"));
    dock->setWidget(&m_breakpoints);
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
        const auto addr = m_pointers.registerValue(Register16::SP);

        if (addr > 0xffff - 2) {
            Util::debugln("Can't set a breakpoint at address on top of stack - stack is currently < 2 bytes in size");
            showStatusMessage("Can't set breakpoint - the top of the stack does not contain an address.", DefaultTransientMessageTimeout);
            return;
        }

        breakAtProgramCounter(m_thread->spectrum().z80()->peekUnsignedHostWord(addr));
    });

    connect(&m_memoryWidget, &QWidget::customContextMenuRequested, this, &DebugWindow::memoryContextMenuRequested);
    connect(&m_watches, &QWidget::customContextMenuRequested, this, &DebugWindow::watchesContextMenuRequested);
    connect(&m_breakpoints, &QWidget::customContextMenuRequested, this, &DebugWindow::breakpointsContextMenuRequested);

    connect(&m_poke, &PokeWidget::pokeClicked, [this](const ::Z80::UnsignedWord address, const ::Z80::UnsignedByte value) -> void {
        m_thread->spectrum().memory()->writeByte(address, value);
        updateStateDisplay();
    });

    connect(&m_registers, &RegistersWidget::registerChanged, [this](const ::Z80::Register16 reg, const ::Z80::UnsignedWord value) {
        sp_assert(m_thread, "detected null thread in lambda attached to RegistersWidget::registerChanged signal in DebuWindow");
        auto * cpu = m_thread->spectrum().z80();
        sp_assert(cpu, "null CPU found in lambda attached to RegistersWidget::interruptModeChanged signal in DebuWindow");
        cpu->setRegisterValue(reg, value);
    });

    connect(&m_shadowRegisters, &ShadowRegistersWidget::registerChanged, [this](const ::Z80::Register16 reg, const ::Z80::UnsignedWord value) {
        sp_assert(m_thread, "detected null thread in lambda attached to ShadowRegistersWidget::registerChanged signal in DebuWindow");
        auto * cpu = m_thread->spectrum().z80();
        sp_assert(cpu, "null CPU found in lambda attached to ShadowRegistersWidget::interruptModeChanged signal in DebuWindow");
        cpu->setRegisterValue(reg, value);
    });

    connect(&m_pointers, &ProgramPointersWidget::registerChanged, [this](const ::Z80::Register16 reg, const ::Z80::UnsignedWord value) {
        sp_assert(m_thread, "detected null thread in lambda attached to ProgramPointersWidget::registerChanged signal in DebuWindow");
        auto * cpu = m_thread->spectrum().z80();
        sp_assert(cpu, "null CPU found in lambda attached to ProgramPointersWidget::interruptModeChanged signal in DebuWindow");
        cpu->setRegisterValue(reg, value);
    });

    connect(&m_interrupts, &InterruptWidget::registerChanged, [this](const ::Z80::Register8 reg, const ::Z80::UnsignedByte value) {
        sp_assert(m_thread, "detected null thread in lambda attached to InterruptWidget::registerChanged signal in DebuWindow");
        auto * cpu = m_thread->spectrum().z80();
        sp_assert(cpu, "null CPU found in lambda attached to InterruptWidget::registerChanged signal in DebuWindow");
        cpu->setRegisterValue(reg, value);
    });

    connect(&m_interrupts, &InterruptWidget::interruptModeChanged, [this](const ::Z80::InterruptMode mode) {
        sp_assert(m_thread, "detected null thread in lambda attached to InterruptWidget::interruptModeChanged signal in DebuWindow");
        auto * cpu = m_thread->spectrum().z80();
        sp_assert(cpu, "null CPU found in lambda attached to InterruptWidget::interruptModeChanged signal in DebuWindow");
        cpu->setInterruptMode(mode);
    });

    // memory widget context menu
    connect(&m_memoryMenu, &MemoryContextMenu::poke, [this](const ::Z80::UnsignedWord address) {
        m_poke.setAddress(address);
        m_poke.setValue(m_thread->spectrum().memory()->readByte(address));
        m_poke.focusValue();
    });

    connect(&m_memoryMenu, &MemoryContextMenu::breakAtProgramCounter, this, &DebugWindow::breakAtProgramCounter);
    connect(&m_memoryMenu, &MemoryContextMenu::breakOnWordChange, this, &DebugWindow::breakOnMemoryChange<::Z80::UnsignedWord>);
    connect(&m_memoryMenu, &MemoryContextMenu::breakOnByteChange, this, &DebugWindow::breakOnMemoryChange<::Z80::UnsignedByte>);
    connect(&m_memoryMenu, &MemoryContextMenu::watchWord, this, &DebugWindow::watchIntegerMemoryAddress<::Z80::UnsignedWord>);
    connect(&m_memoryMenu, &MemoryContextMenu::watchByte, this, &DebugWindow::watchIntegerMemoryAddress<::Z80::UnsignedByte>);
    connect(&m_memoryMenu, &MemoryContextMenu::watchString, [this](const ::Z80::UnsignedWord address) {
        watchStringMemoryAddress(address);
    });
}

void DebugWindow::closeEvent(QCloseEvent * ev)
{
    m_thread->spectrum().z80()->removeInstructionObserver(&m_cpuObserver);
    QSettings settings;

#if defined(USE_QT_5)
    settings.beginGroup(QStringLiteral("qt5-debugwindow"));
#else
    settings.beginGroup(QStringLiteral("qt6-debugwindow"));
#endif

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

// We prefix the group name rather than nesting groups because we save as .ini files, and .ini files don't support group
// nesting, so Qt prefixes each setting with the full sub-group path using \ as a separator. It doesn't matter much here
// because only Qt will read these main window settings, but with core emulator settings and potential other UIs we want
// the config file to be parseable without having to handle the Qt-isms
#if defined(USE_QT_5)
    settings.beginGroup(QStringLiteral("qt5-debugwindow"));
#else
    settings.beginGroup(QStringLiteral("qt6-debugwindow"));
#endif

    setGeometry({settings.value(QStringLiteral("position")).toPoint(), settings.value(QStringLiteral("size")).toSize()});
    restoreState(settings.value(QStringLiteral("windowState")).toByteArray());
    settings.endGroup();

    QMainWindow::showEvent(event);
}

void DebugWindow::updateStateDisplay()
{
    sp_assert(m_thread, "detected null thread in DebugWindow::updateStateDisplay");
	auto * cpu = m_thread->spectrum().z80();
    sp_assert(cpu, "null CPU found in DebugWindow::updateStateDisplay");
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

	// trigger a refresh of the values of all watches
    m_watchesModel.update();
}

void DebugWindow::pauseResumeTriggered()
{
    sp_assert(m_thread, "detected null thread in DebugWindow::pauseResumeTriggered()");

    if (m_thread->isPaused()) {
        m_thread->resume();
    } else {
        m_thread->pause();
    }
}

void DebugWindow::stepTriggered()
{
    sp_assert(m_thread, "detected null thread in DebugWindow::stepTriggered");
    m_thread->step();
}

void DebugWindow::threadPaused()
{
    m_pauseResume.setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start"), Application::icon(QStringLiteral("resume"))));
    m_pauseResume.setText(tr("Resume"));
    m_registers.setEnabled(true);
    m_pointers.setEnabled(true);
    m_interrupts.setEnabled(true);
    m_disassembly.setEnabled(true);
    m_shadowRegisters.setEnabled(true);
    m_poke.setEnabled(true);
    m_memoryWidget.setEnabled(true);
    m_watches.setEnabled(true);
    m_keyboardMonitor.setEnabled(true);
    m_step.setEnabled(true);
    updateStateDisplay();
    connect(m_thread, &Thread::stepped, this, &DebugWindow::threadStepped, Qt::UniqueConnection);
}

void DebugWindow::threadResumed()
{
    m_pauseResume.setIcon(QIcon::fromTheme(QStringLiteral("media-playback-pause"), Application::icon(QStringLiteral("pause"))));
    m_pauseResume.setText(tr("Pause"));
    m_registers.setEnabled(false);
    m_pointers.setEnabled(false);
    m_interrupts.setEnabled(false);
    m_disassembly.setEnabled(false);
    m_shadowRegisters.setEnabled(false);
    m_poke.setEnabled(false);
    m_memoryWidget.setEnabled(false);
    m_watches.setEnabled(false);
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

    // make sure all watchers are observing the new memory
    for (int idx = m_watchesModel.rowCount() - 1; idx >= 0; --idx) {
        // NOTE all addresses should remain valid because all Spectrum memory has 64k addressable
        m_watchesModel.watch(idx)->setMemory(spectrum.memory());
    }
}

void DebugWindow::setProgramCounterBreakpointTriggered(const UnsignedWord address)
{
    if (0 > address || 0xffff < address) {
        Util::debugln("invalid breakpoint address: {:#04x}", address);
        return;
    }

    breakAtProgramCounter(address);
}

void DebugWindow::breakAtProgramCounter(UnsignedWord address)
{
    auto breakpoint = std::make_unique<ProgramCounterBreakpoint>(address);

    if (hasBreakpoint(*breakpoint)) {
        Util::debugln("breakpoint already set: {:#04x}", address);
        return;
    }

    breakpoint->addObserver(&m_pcObserver);
    m_breakpointsModel.addBreakpoint(std::move(breakpoint));
    showStatusMessage(tr("Breakpoint set at PC = 0x%1.").arg(address, 4, 16, QLatin1Char('0')), DefaultTransientMessageTimeout);
}

void DebugWindow::breakIfStackPointerBelow(UnsignedWord address)
{
    auto breakpoint = std::make_unique<StackPointerBelowBreakpoint>(address);

    if (hasBreakpoint(*breakpoint)) {
        Util::debugln("stack pointer breakpoint already set at {:#04x}", address);
        return;
    }

    breakpoint->addObserver(&m_spBelowObserver);
    m_breakpointsModel.addBreakpoint(std::move(breakpoint));
    showStatusMessage(tr("Breakpoint set at SP < 0x%1.").arg(address, 4, 16, QLatin1Char('0')), DefaultTransientMessageTimeout);
}

bool DebugWindow::addBreakpoint(std::unique_ptr<Breakpoint> breakpoint)
{
    if (hasBreakpoint(*breakpoint)) {
        return false;
    }

    m_breakpointsModel.addBreakpoint(std::move(breakpoint));
    return true;
}

bool DebugWindow::hasBreakpoint(const Breakpoint & breakpoint) const
{
    return m_breakpointsModel.hasBreakpoint(breakpoint);
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

    m_memoryMenu.setAddress(*address);
    m_memoryMenu.exec(m_memoryWidget.mapToGlobal(pos));
}

void DebugWindow::watchesContextMenuRequested(const QPoint & pos)
{
    WatchesContextMenu menu(&m_watchesModel, m_watches.indexAt(pos));
    connect(&menu, &WatchesContextMenu::locateInMemoryView, &m_memoryWidget, &MemoryWidget::scrollToAddress);
    menu.exec(m_watches.mapToGlobal(pos));
}

void DebugWindow::breakpointsContextMenuRequested(const QPoint & pos)
{
    BreakpointsContextMenu menu(&m_breakpointsModel, m_breakpoints.indexAt(pos));
    menu.exec(m_breakpoints.mapToGlobal(pos));
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
    showStatusMessage(tr("Breakpoint hit: PC = 0x%1.").arg(address, 4, 16, QLatin1Char('0')));
    show();
    activateWindow();
    raise();
    m_memoryWidget.scrollToAddress(address);
    m_disassembly.scrollToAddress(address);

}

void DebugWindow::memoryChangeBreakpointTriggered(const UnsignedWord address)
{
    showStatusMessage(tr("Monitored memory location modified: 0x%1.").arg(address, 4, 16, QLatin1Char('0')));
    show();
    activateWindow();
    raise();
    m_memoryWidget.scrollToAddress(address);
}

void DebugWindow::stackPointerBelowBreakpointTriggered(const ::Z80::UnsignedWord address)
{
    showStatusMessage(tr("Stack pointer is below 0x%1.").arg(address, 4, 16, QLatin1Char('0')));
    show();
    activateWindow();
    raise();
    m_memoryWidget.scrollToAddress(address);
    m_disassembly.scrollToPc();
}

void DebugWindow::watchStringMemoryAddress(::Z80::UnsignedWord address, std::optional<int> length)
{
    auto * memory = m_thread->spectrum().memory();

    if (!length) {
        // make sure the default length fits inside the addressable memory
        length = std::min(10, static_cast<int>(memory->addressableSize() - address));
    }

    sp_assert(0 < *length && address + *length < memory->addressableSize(), "string watch with address {:#04x} and length {} would exceed extent of addressable memory", address, *length);
    m_watchesModel.addWatch(std::make_unique<StringMemoryWatch>(memory, address, static_cast<MemoryWatch::WatchSize>(*length)));
}

void DebugWindow::InstructionObserver::notify(::Spectrum::Z80 * cpu)
{
    const auto & spectrum = window.m_thread->spectrum();
    const auto breakpointCount = window.m_breakpointsModel.rowCount();

    for (auto idx = 0; idx < breakpointCount; ++idx) {
        if (window.m_breakpointsModel.breakpointIsEnabled(idx)) {
            window.m_breakpointsModel.breakpoint(idx)->check(spectrum);
        }
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
