#ifndef QSPECTRUMDEBUGWINDOW_H
#define QSPECTRUMDEBUGWINDOW_H

#include <cstdint>

#include <QMainWindow>
#include <QAction>

#include "../../qt/hexspinbox.h"
#include "registerpairwidget.h"
#include "flagswidget.h"
#include "memorywidget.h"
#include "keyboardmonitorwidget.h"

class QLineEdit;

namespace Spectrum::Qt
{
	class Thread;

	class DebugWindow
	:	public QMainWindow
	{
		Q_OBJECT

    public:
        explicit DebugWindow(QWidget * = nullptr);
        explicit DebugWindow(Thread * spectrum, QWidget * = nullptr);

        void updateStateDisplay();

	protected:
	    void showEvent(QShowEvent *) override;
	    void closeEvent(QCloseEvent *) override;

    private:
        void createToolbars();
        void createDockWidgets();
        void layoutWidget();
        void connectWidgets();

	    void pauseResumeTriggered();
	    void stepTriggered();
        void threadPaused();
        void threadResumed();
        void threadStepped();
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
        QToolButton m_memoryPc;
        QToolButton m_memorySp;
        QAction m_pauseResume;
        QAction m_step;
        QAction m_refresh;

        KeyboardMonitorWidget m_keyboardMonitor;
	};
}

#endif // QSPECTRUMDEBUGWINDOW_H
