#ifndef QSPECTRUMDEBUGWINDOW_H
#define QSPECTRUMDEBUGWINDOW_H

#include <cstdint>

#include <QMainWindow>
#include <QAction>

#include "hexspinbox.h"
#include "registerswidget.h"
#include "shadowregisterswidget.h"
#include "interruptwidget.h"
#include "programpointerswidget.h"
#include "registerpairwidget.h"
#include "flagswidget.h"
#include "disassemblywidget.h"
#include "memorydebugwidget.h"
#include "keyboardmonitorwidget.h"
#include "custompokewidget.h"

class QLineEdit;

namespace Spectrum::QtUi
{
	class Thread;
	class Breakpoint;

	class DebugWindow
	:	public QMainWindow
	{
		Q_OBJECT

    public:
        explicit DebugWindow(QWidget * = nullptr);
        explicit DebugWindow(Thread *, QWidget * = nullptr);
        ~DebugWindow() override;

        void setStatus(const QString & status);
        void clearStatus();

        void locateProgramCounterInMemory();
        void locateStackPointerInMemory();
        void locateProgramCounterInDisassembly();
        void locateStackPointerInDisassembly();

        void updateStateDisplay();

	protected:
	    void showEvent(QShowEvent *) override;
	    void closeEvent(QCloseEvent *) override;

        void memoryContextMenuRequested(const QPoint &);

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
        void setProgramCounterBreakpointTriggered(::Z80::UnsignedWord address);

        Thread * m_thread;

        RegistersWidget m_registers;
        DisassemblyWidget m_disassembly;
        ShadowRegistersWidget m_shadowRegisters;
        InterruptWidget m_interrupts;
        ProgramPointersWidget m_pointers;
        MemoryDebugWidget m_memoryWidget;
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
        CustomPokeWidget m_poke;

        InstructionObserver m_cpuObserver;
        Breakpoints m_breakpoints;

        /**
         * Set a breakpoint when the program counter hits a given address.
         *
         * @param addr
         */
        void breakAtProgramCounter(::Z80::UnsignedWord addr);
    };
}

#endif // QSPECTRUMDEBUGWINDOW_H
