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
#include "memorywidget.h"
#include "keyboardmonitorwidget.h"
#include "pokewidget.h"

class QLineEdit;

namespace Spectrum::Qt
{
	class Thread;
	class Breakpoint;

	class DebugWindow
	:	public QMainWindow
	{
		Q_OBJECT

    public:
        explicit DebugWindow(QWidget * = nullptr);
        explicit DebugWindow(Thread * spectrum, QWidget * = nullptr);
        ~DebugWindow() override;

        void setStatus(const QString & status);
        void clearStatus();

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
        void memoryLocationChanged();
        void setBreakpointTriggered();
        void scrollMemoryToPcTriggered();
        void scrollMemoryToSpTriggered();

        Thread * m_thread;

        RegistersWidget m_registers;
        DisassemblyWidget m_disassembly;
        ShadowRegistersWidget m_shadowRegisters;
        InterruptWidget m_interrupts;
        ProgramPointersWidget m_pointers;
//        HexSpinBox m_sp;
//        HexSpinBox m_pc;
        MemoryWidget m_memoryWidget;
        HexSpinBox m_memoryLocation;
        QToolButton m_setBreakpoint;
        QToolButton m_memoryPc;
        QToolButton m_memorySp;
        QAction m_pauseResume;
        QAction m_step;
        QAction m_refresh;
        QLabel m_status;

        KeyboardMonitorWidget m_keyboardMonitor;
        PokeWidget m_poke;

        QDockWidget * m_memoryDock;
        QDockWidget * m_pokeDock;

        InstructionObserver m_cpuObserver;
        Breakpoints m_breakpoints;
	};
}

#endif // QSPECTRUMDEBUGWINDOW_H
