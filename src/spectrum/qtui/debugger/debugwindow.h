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
#include "flagswidget.h"
#include "disassemblywidget.h"
#include "memorywidget.h"
#include "keyboardmonitorwidget.h"
#include "../pokewidget.h"
#include "memorywatchesmodel.h"
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

	class DebugWindow
	: public QMainWindow
	{
		Q_OBJECT

    public:
        explicit DebugWindow(QWidget * = nullptr);
        explicit DebugWindow(Thread *, QWidget * = nullptr);
        ~DebugWindow() override;

        void showStatusMessage(const QString & status, int timeout = 0);
        void clearStatusMessage();

        void locateProgramCounterInMemory();
        void locateStackPointerInMemory();
        void locateProgramCounterInDisassembly();
        void locateStackPointerInDisassembly();

        void updateStateDisplay();

        /**
         * Set a breakpoint for when the program counter hits a given address.
         *
         * @param address
         */
        void breakAtProgramCounter(::Z80::UnsignedWord address);

        /**
         * Set a breakpoint
         * @param address
         */
        void breakIfStackPointerBelow(::Z80::UnsignedWord address);

        template<class ValueType>
        void breakOnMemoryChange(::Z80::UnsignedWord address)
        {
            auto * breakpoint = new MemoryChangedBreakpoint<ValueType>(address);

            if (!addBreakpoint(breakpoint)) {
                Util::debug << "breakpoint monitoring 0x" << std::hex << std::setfill('0') << std::setw(4) << address << std::dec << std::setfill(' ') << " for " << (sizeof(ValueType) * 8) << "-bit changes already set\n";
                delete breakpoint;
                return;
            }

            breakpoint->addObserver(&m_memoryBreakpointObserver);
            Util::debug << "setting breakpoint monitoring 0x" << std::hex << std::setfill('0') << std::setw(4) << address << std::dec << std::setfill(' ') << " for " << (sizeof(ValueType) * 8) << "-bit changes\n";
            showStatusMessage(
                    tr("Breakpoint set monitoring 0x%1 for %2-bit changes.").arg(address, 4, 16, QLatin1Char('0')).arg(
                            sizeof(ValueType) * 8));
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
        bool addBreakpoint(Breakpoint *);

	protected:
	    void showEvent(QShowEvent *) override;
	    void closeEvent(QCloseEvent *) override;

        void memoryContextMenuRequested(const QPoint &);
        void watchesContextMenuRequested(const QPoint &);

    private:
	    class InstructionObserver
        : public ::Spectrum::Z80::Observer
        {
        public:
	        explicit InstructionObserver(DebugWindow & owner)
	        : window(owner) {}

	        void notify(::Spectrum::Z80 * cpu) override;
	        const DebugWindow & window;
        };

	    // internal observers for breakpoints
	    class BreakpointObserver
        : public Breakpoint::Observer
        {
        public:
            explicit BreakpointObserver(DebugWindow & owner)
            : window(owner) {}
            void notify(Breakpoint *) override;
            DebugWindow & window;
        };

	    class MemoryBreakpointObserver
        : public BreakpointObserver
        {
        public:
            using BreakpointObserver::BreakpointObserver;
            void notify(Breakpoint *) override;
        };

	    class ProgramCounterBreakpointObserver
        : public BreakpointObserver
        {
        public:
            using BreakpointObserver::BreakpointObserver;
            void notify(Breakpoint *) override;
        };

	    class StackPointerBelowBreakpointObserver
        : public BreakpointObserver
        {
        public:
            using BreakpointObserver::BreakpointObserver;
            void notify(Breakpoint *) override;
        };

	    friend class BreakpointObserver;
	    friend class MemoryBreakpointObserver;

	    using Breakpoints = std::vector<Breakpoint *>;
        void createToolbars();
        void createDockWidgets();
        void layoutWidget();
        void connectWidgets();

	    void pauseResumeTriggered();
	    void stepTriggered();
        void threadPaused();
        void threadResumed();
        void threadStepped();
        void threadSpectrumChanged();

        // the user has triggered the action to set a breakpoint
        void setProgramCounterBreakpointTriggered(::Z80::UnsignedWord address);

        // the breakpoint has been triggered
        void programCounterBreakpointTriggered(::Z80::UnsignedWord address);
        void stackPointerBelowBreakpointTriggered(::Z80::UnsignedWord address);
        void memoryChangeBreakpointTriggered(::Z80::UnsignedWord address);

        Thread * m_thread;

        RegistersWidget m_registers;
        DisassemblyWidget m_disassembly;
        ShadowRegistersWidget m_shadowRegisters;
        Debugger::InterruptWidget m_interrupts;
        ProgramPointersWidget m_pointers;
        MemoryWidget m_memoryWidget;
        QAction m_pauseResume;
        QAction m_step;
        QAction m_refresh;
        QLabel m_status;

        // context menu actions for PC/SP widgets
        QAction m_navigateToPc;
        QAction m_breakpointAtPc;
        QAction m_navigateToSp;
        QAction m_breakpointAtStackTop;

        KeyboardMonitorWidget m_keyboardMonitor;
        PokeWidget m_poke;
        MemoryWatchesModel m_watchesModel;
        QTreeView m_watches;
        MemoryContextMenu m_memoryMenu;

        InstructionObserver m_cpuObserver;
        Breakpoints m_breakpoints;
        ProgramCounterBreakpointObserver m_pcObserver;
        MemoryBreakpointObserver m_memoryBreakpointObserver;
        StackPointerBelowBreakpointObserver m_spBelowObserver;

        QTimer m_statusClearTimer;
    };
}

#endif // SPECTRUM_QTUI_DEBUGGER_SPECTRUMDEBUGWINDOW_H
