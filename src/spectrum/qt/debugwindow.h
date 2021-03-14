#ifndef QSPECTRUMDEBUGWINDOW_H
#define QSPECTRUMDEBUGWINDOW_H

#include <cstdint>

#include <QMainWindow>
#include <QAction>

#include "hexspinbox.h"
#include "registerpairwidget.h"
#include "flagswidget.h"
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

	    void showPokeWidget(Z80::UnsignedWord, Z80::UnsignedByte);
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

        RegisterPairWidget m_af;
        RegisterPairWidget m_bc;
        RegisterPairWidget m_de;
        RegisterPairWidget m_hl;
        RegisterPairWidget m_ix;
        RegisterPairWidget m_iy;
        RegisterPairWidget m_afshadow;
        RegisterPairWidget m_bcshadow;
        RegisterPairWidget m_deshadow;
        RegisterPairWidget m_hlshadow;
        HexSpinBox m_sp;
        HexSpinBox m_pc;
        QSpinBox m_im;
        HexSpinBox m_i;
        HexSpinBox m_r;
        FlagsWidget m_flags;
        FlagsWidget m_shadowFlags;
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

        InstructionObserver m_cpuObserver;
        Breakpoints m_breakpoints;
	};
}

#endif // QSPECTRUMDEBUGWINDOW_H
