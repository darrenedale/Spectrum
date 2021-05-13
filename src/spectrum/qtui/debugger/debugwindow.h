#ifndef SPECTRUM_QTUI_DEBUGGER_SPECTRUMDEBUGWINDOW_H
#define SPECTRUM_QTUI_DEBUGGER_SPECTRUMDEBUGWINDOW_H

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <QMainWindow>
#include <QAction>
#include <QTreeView>
#include <QTableView>
#include "../../../util/debug.h"
#include "../hexspinbox.h"
#include "../thread.h"
#include "registerswidget.h"
#include "shadowregisterswidget.h"
#include "interruptwidget.h"
#include "programpointerswidget.h"
#include "../registerpairwidget.h"
#include "disassemblywidget.h"
#include "memorywidget.h"
#include "keyboardmonitorwidget.h"
#include "../pokewidget.h"
#include "watchesmodel.h"
#include "watchesview.h"
#include "breakpointsmodel.h"
#include "breakpointsview.h"
#include "memorycontextmenu.h"
#include "../../debugger/breakpoint.h"
#include "../../debugger/memorychangedbreakpoint.h"
#include "../../debugger/integermemorywatch.h"

class QLineEdit;

namespace Spectrum::QtUi::Debugger
{
    using Spectrum::Debugger::Breakpoint;
    using Spectrum::Debugger::MemoryChangedBreakpoint;
    using Spectrum::Debugger::IntegerMemoryWatch;

    /**
     * A debug window for a running Spectrum.
     *
     * The debug window enables the user to monitor and manipulate the state of the running Spectrum. While the Spectrum is paused, it is possible to manipulate
     * various aspects of its internal state, such as register values, memory values and interrupt status. It also provides the user with the ability to set
     * various types of breakpoint, which will pause the running Spectrum when the breakpoint condition is met, and to set watches of various types on arbitrary
     * memory addresses.
     *
     * While the Spectrum is actually executing instructions the debug window is disabled and its display is not updated with the state of the machine. As soon
     * as the Spectrum is paused, all the widgets in the window are enabled, after they have been updated with the current state of the Spectrum. The main
     * registers of the Spectrum's Z80 and the disassembly of its memory are always visible; all other widgets are placed in dock areas and can be added or
     * removed as required.
     *
     * The following widgets are currently available:
     * - main registers (including flags)
     * - disassembly of full Z80 addressable memory
     * - byte view of full Z80 addressable memory (including byte/word/string search)
     * - shadow registers
     * - interrupt status
     * - PC and SP (program pointers)
     * - breakpoints
     * - memory watches
     * - keyboard monitor
     *
     * Future plans:
     * - memory pager (for 128K models)
     * - joystick monitor
     */
	class DebugWindow
	: public QMainWindow
	{
		Q_OBJECT

    public:
	    /**
	     * (Default) initialise a new debug window.
	     */
        explicit DebugWindow(QWidget * = nullptr);

        /**
         * Initialise a new debug window to monitor the Spectrum in a given thread.
         *
         * The thread is borrowed, not owned. It is the responsibility of the caller to ensure that the thread remains valid for the duration for which it is
         * borrowed by the debug window.
         */
        explicit DebugWindow(Thread *, QWidget * = nullptr);

        /**
         * Destructor.
         */
        ~DebugWindow() override;

        /**
         * Show a message in the status area in the toolbar.
         *
         * @param status The message to show.
         * @param timeout How long to display the message for.
         */
        void showStatusMessage(const QString & status, int timeout = 0);

        /**
         * Clear the current message from the status area in the toolbar.
         */
        void clearStatusMessage();

        /**
         * Scroll the memory view to show the current PC.
         */
        void locateProgramCounterInMemory();

        /**
         * Scroll the memory view to show the current SP.
         */
        void locateStackPointerInMemory();

        /**
         * Scroll the disassembly view to show the instruction at the current PC.
         */
        void locateProgramCounterInDisassembly();

        /**
         * Scroll the disassembly view to show the instruction at the current SP.
         */
        void locateStackPointerInDisassembly();

        /**
         * Update the display in all widgets with the current state of the Spectrum in the monitored thread.
         */
        void updateStateDisplay();

        /**
         * Set a breakpoint for when the program counter hits a given address.
         *
         * @param address
         */
        void breakAtProgramCounter(::Z80::UnsignedWord address);

        /**
         * Set a breakpoint that triggers if the SP register's value falls below a given address.
         *
         * @param address
         */
        void breakIfStackPointerBelow(::Z80::UnsignedWord address);

        /**
         * Set a breakpoint that triggers when the value at a given memory address changes.
         *
         * @tparam ValueType The int type of the value to watch. Can be 8- or 16-bit to watch the 8- or 16-bit value at the given address.
         * @param address The address to monitor.
         */
        template<class ValueType>
        void breakOnMemoryChange(::Z80::UnsignedWord address)
        {
            auto breakpoint = std::make_unique<MemoryChangedBreakpoint<ValueType>>(address);

            if (hasBreakpoint(*breakpoint)) {
                Util::debug << "breakpoint monitoring 0x" << std::hex << std::setfill('0') << std::setw(4) << address << std::dec << std::setfill(' ') << " for " << (sizeof(ValueType) * 8) << "-bit changes already set\n";
                return;
            }

            breakpoint->addObserver(&m_memoryBreakpointObserver);
            m_breakpointsModel.addBreakpoint(std::move(breakpoint));
            showStatusMessage(tr("Breakpoint set monitoring 0x%1 for %2-bit changes.").arg(address, 4, 16, QLatin1Char('0')).arg(sizeof(ValueType) * 8));
        }

        /**
         * Add a watch for an integer value at a specified memory address.
         *
         * @param address The address to watch.
         */
        template<Spectrum::Debugger::IntegerMemoryWatchType ValueType>
        void watchIntegerMemoryAddress(::Z80::UnsignedWord address)
        {
            assert(m_thread);
            assert(m_thread->spectrum().memory());
            m_watchesModel.addWatch(std::make_unique<IntegerMemoryWatch<ValueType>>(m_thread->spectrum().memory(), address));
        }

        /**
         * Add a watch for a fixed-length string at a specified memory address.
         *
         * @param address The address to watch.
         * @param length The length of the string.
         */
        void watchStringMemoryAddress(::Z80::UnsignedWord address, std::optional<int> length = {});

        /**
         * Check whether a breakpoint has been added to the debug window.
         *
         * The check is performed using the == operator for the breakpoint.
         *
         * @return
         */
        [[nodiscard]] bool hasBreakpoint(const Breakpoint &) const;

        /**
         * Add a breakpoint to the debug window.
         *
         * The breakpoint won't be added if there is already an identical breakpoint in the debug window. Identical
         * breakpoints are determined by the == operator for the breakpoint class. Usually this means the two
         * breakpoints are of identical in type and have the same properties (e.g. if two ProgramCounterBreakpoint
         * objects are monitoring for the same address, they are considered identical).
         *
         * If successful, the debug window takes over ownership of the breakpoint. Successful means the breakpoint was
         * added because it was not found to be a duplicate of an existing breakpoint. If it is not added, the caller is
         * responsible for its disposal.
         *
         * You are encouraged to use the built-in methods to add specific breakpoints where possible (e.g.
         * breakAtProgramCounter()).
         *
         * @return true if the breakpoint was added, false if not.
         */
        bool addBreakpoint(std::unique_ptr<Breakpoint>);

	protected:
	    /**
	     * Handle the show event on the debug window.
	     */
	    void showEvent(QShowEvent *) override;

        /**
         * Handle the close event on the debug window.
         */
	    void closeEvent(QCloseEvent *) override;

	    /**
	     * Show the context menu for the memory view widget.
	     */
        void memoryContextMenuRequested(const QPoint &);

        /**
         * Show the context menu for the watches widget.
         */
        void watchesContextMenuRequested(const QPoint &);

        /**
         * Show the context menu for the breakpoints widget.
         */
        void breakpointsContextMenuRequested(const QPoint &);

    private:
	    /**
	     * Helper class to observe instruction execution on the monitored thread's Spectrum's CPU.
	     *
	     * An instance of the observer is added to the CPU in order to check breakpoints after each executed instruction.
	     *
	     * TODO consider using an anonymous class instead?
	     */
	    class InstructionObserver
        : public ::Spectrum::Z80::Observer
        {
        public:
	        explicit InstructionObserver(DebugWindow & owner)
	        : window(owner) {}

	        void notify(::Spectrum::Z80 * cpu) override;
	        const DebugWindow & window;
        };

	    /**
	     * Helper base class to observe breakpoints and pause the monitored thread's Spectrum when they are triggered.
	     */
	    class BreakpointObserver
        : public Breakpoint::Observer
        {
        public:
            explicit BreakpointObserver(DebugWindow & owner)
            : window(owner) {}
            void notify(Breakpoint *) override;
            DebugWindow & window;
        };

	    /**
	     * Helper class to observe memory breakpoints.
	     *
	     * Extends the BreakpointObserver base class to trigger the debug window to scroll the memory view to the address from the triggered breakpoint.
	     */
	    class MemoryBreakpointObserver
        : public BreakpointObserver
        {
        public:
            using BreakpointObserver::BreakpointObserver;
            void notify(Breakpoint *) override;
        };

        /**
         * Helper class to observe program counter breakpoints.
         *
         * Extends the BreakpointObserver base class to trigger the debug window to scroll the memory and disassembly views to the address from the triggered
         * breakpoint.
         */
	    class ProgramCounterBreakpointObserver
        : public BreakpointObserver
        {
        public:
            using BreakpointObserver::BreakpointObserver;
            void notify(Breakpoint *) override;
        };

        /**
         * Helper class to observe stack pointer breakpoints.
         *
         * Extends the BreakpointObserver base class to trigger the debug window to scroll the memory and disassembly views to the address from the triggered
         * breakpoint.
         */
	    class StackPointerBelowBreakpointObserver
        : public BreakpointObserver
        {
        public:
            using BreakpointObserver::BreakpointObserver;
            void notify(Breakpoint *) override;
        };

	    /**
	     * Helper to create the toolbars for the window.
	     *
	     * Extracted primarily for ease of maintenance.
	     */
        void createToolbars();

        /**
         * Helper to create the dock widgets for the window.
         *
         * Extracted primarily for ease of maintenance.
         */
        void createDockWidgets();

        /**
         * Helper to layout the component widgets for the debug window.
         *
         * Extracted primarily for ease of maintenance.
         */
        void layoutWidget();

        /**
         * Helper to connect to the signals on the component widgets for the debug window.
         *
         * Extracted primarily for ease of maintenance.
         */
        void connectWidgets();

        /**
         * Called when the pause/resume action has been triggered.
         */
	    void pauseResumeTriggered();

        /**
         * Called when the debug step action has been triggered.
         */
	    void stepTriggered();

        /**
         * Called when the thread has been paused.
         *
         * NOTE The thread can be paused from outside this class.
         */
        void threadPaused();

        /**
         * Called when the thread has been resumed.
         *
         * NOTE The thread can be resumed from outside this class.
         */
        void threadResumed();

        /**
         * Called when the thread has stepped a single instruction.
         *
         * NOTE The thread can be stepped from outside this class.
         */
        void threadStepped();

        /**
         * Called when the Spectrum being managed by the monitored thread has changed.
         *
         * This signal is trapped to ensure that the component widgets monitor the state of the new Spectrum.
         */
        void threadSpectrumChanged();

        /**
         * Called when the user has triggered the action to set a PC breakpoint.
         *
         * @param address The address that is the subject of the breakpoint.
         */
        void setProgramCounterBreakpointTriggered(::Z80::UnsignedWord address);

        /**
         * Called when a program counter breakpoint has been triggered.
         *
         * @param address The address of the breakpoint.
         */
        void programCounterBreakpointTriggered(::Z80::UnsignedWord address);

        /**
         * Called when a "stack pointer below" breakpoint has been triggered.
         *
         * @param address The address of the breakpoint.
         */
        void stackPointerBelowBreakpointTriggered(::Z80::UnsignedWord address);

        /**
         * Called when a memory monitoring breakpoint has been triggered.
         *
         * @param address The address the breakpoint is monitoring.
         */
        void memoryChangeBreakpointTriggered(::Z80::UnsignedWord address);

        /**
         * The thread managing the Spectrum being monitored.
         */
        Thread * m_thread;

        /**
         * The widget representing the main Z80 registers.
         */
        RegistersWidget m_registers;

        /**
         * The Z80 memory disassembly.
         */
        DisassemblyWidget m_disassembly;

        /**
         * The widget representing the Z80 shadow registers.
         *
         * This is placed in a dock widget to the left of the debug window by default.
         */
        ShadowRegistersWidget m_shadowRegisters;

        /**
         * The widget representing the Z80 interrupt state.
         */
        InterruptWidget m_interrupts;

        /**
         * The widget representing the Z80's program pointers.
         *
         * The PC and SP are the Z80's program pointers.
         */
        ProgramPointersWidget m_pointers;

        /**
         * The widget representing a view of the Z80's addressable memory.
         *
         * For 16K/48K models this will be the full memory of the computer; for 128K models this will be the currently paged memory banks.
         */
        MemoryWidget m_memoryWidget;

        /**
         * Action enabling the user to pause/resume the Spectrum.
         */
        QAction m_pauseResume;

        /**
         * Action enabling the user to trigger execution of a single instruction when the Spectrum is paused.
         */
        QAction m_step;

        /**
         * Action enabling the user to refresh the content of all widgets with the current state of the Spectrum.
         */
        QAction m_refresh;

        /**
         * Displays a (transient) status message in the main toolbar when certain events occur.
         */
        QLabel m_status;

        /**
         * Action to navigate the memory view and disassembly to the current program counter address.
         *
         * This is used in multiple places.
         */
        QAction m_navigateToPc;

        /**
         * Action to set a program counter breakpoint at a targeted address.
         *
         * This is used in context menus, etc., where there is an address in the context (e.g. the memory view).
         */
        QAction m_breakpointAtPc;

        /**
         * Action to navigate the memory view and disassembly to the current stack pointer address.
         *
         * This is used in multiple places.
         */
        QAction m_navigateToSp;

        /**
         * Action to set a breakpoint when the PC reaches the address currently on the top of the stack.
         *
         * This is used in context menus, etc., where there is an address in the context (e.g. the memory view).
         */
        QAction m_breakpointAtStackTop;

        /**
         * The widget representing the current state of the Spectrum keyboard.
         *
         * This is placed in a dock at the bottom of the window by default.
         */
        KeyboardMonitorWidget m_keyboardMonitor;

        /**
         * Widget enabling the user to poke a custom memory location with an 8--bit value.
         */
        PokeWidget m_poke;

        /**
         * The data model for the memory watches the user has added.
         */
        WatchesModel m_watchesModel;

        /**
         * The view of the watches.
         */
        WatchesView m_watches;

        /**
         * The data model for the breakpoints the user has added.
         */
        BreakpointsModel m_breakpointsModel;

        /**
         * The view of the breakpoints.
         */
        BreakpointsView m_breakpoints;

        /**
         * The custom context menu for the memory view.
         */
        MemoryContextMenu m_memoryMenu;

        /**
         * Observes each instruction executed by the Spectrum's Z80.
         *
         * The observer checks all the breakpoints after each instruction executed.
         */
        InstructionObserver m_cpuObserver;

        /**
         * Observes all the program counter breakpoints the user has added.
         *
         * All notifications are forwarded to programCounterBreakpointTriggered() on this DebugWindow.
         */
        ProgramCounterBreakpointObserver m_pcObserver;

        /**
         * Observes all the memory monitoring breakpoints the user has added.
         *
         * All notifications are forwarded to memoryChangeBreakpointTriggered() on this DebugWindow.
         */
        MemoryBreakpointObserver m_memoryBreakpointObserver;

        /**
         * Observes all the memory monitoring breakpoints the user has added.
         *
         * All notifications are forwarded to stackPointerBelowBreakpointTriggered() on this DebugWindow.
         */
        StackPointerBelowBreakpointObserver m_spBelowObserver;

        /**
         * Timer to clear the status message after the requested timeout.
         *
         * @see showStatusMessage()
         */
        QTimer m_statusClearTimer;
    };
}

#endif // SPECTRUM_QTUI_DEBUGGER_SPECTRUMDEBUGWINDOW_H
