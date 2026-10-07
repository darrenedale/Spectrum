#ifndef SPECTRUM_QTUI_POKEFINDER_POKEFINDERWINDOW_H
#define SPECTRUM_QTUI_POKEFINDER_POKEFINDERWINDOW_H

#include <QMainWindow>

#include "../../pokefinder.h"

namespace Spectrum::QtUi
{
    class Thread;
}

class QAction;
class QTreeView;
class QPushButton;
class QSpinBox;
class QToolButton;

namespace Spectrum::QtUi::PokeFinder
{
    using Spectrum::PokeFinder;

    /**
     * A poke finder window for a running Spectrum.
     */
	class PokeFinderWindow
	: public QMainWindow
	{
		Q_OBJECT

        public:
	        /** (Default) initialise a new poke finder window. */
            explicit PokeFinderWindow(QWidget * = nullptr);

            /**
             * Initialise a new poke finder window to monitor the Spectrum in a given thread.
             *
             * The thread is borrowed, not owned. It is the responsibility of the caller to ensure that the thread remains valid for the duration for which it is
             * borrowed by the poke finder window.
             */
            explicit PokeFinderWindow(Thread *, QWidget * = nullptr);

	        PokeFinderWindow(const PokeFinderWindow &) = delete;
	        PokeFinderWindow(PokeFinderWindow &&) = delete;
	        PokeFinderWindow operator=(const PokeFinderWindow &) = delete;
	        PokeFinderWindow operator=(PokeFinderWindow &&) = delete;

            ~PokeFinderWindow() override;

	    protected:
	        /**
	         * Handle the show event on the poke finder window.
	         */
	        void showEvent(QShowEvent *) override;

            /**
             * Handle the close event on the poke finder window.
             */
	        void closeEvent(QCloseEvent *) override;

        private:

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
            // void createDockWidgets();

            /**
             * Helper to layout the component widgets for the poke finder window.
             *
             * Extracted primarily for ease of maintenance.
             */
            void layoutWidget();

            /**
             * Helper to connect to the signals on the component widgets for the poke finder window.
             *
             * Extracted primarily for ease of maintenance.
             */
            void connectWidgets();

            /**
             * Called when the pause/resume action has been triggered.
             */
	        void pauseResumeTriggered();

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
             * Called when the Spectrum being managed by the monitored thread has changed.
             *
             * This signal is trapped to ensure that the component widgets monitor the state of the new Spectrum.
             */
            void threadSpectrumChanged();

            /** The thread managing the Spectrum being monitored. */
            Thread * m_thread;

	        /** Action enabling the user to pause/resume the Spectrum. */
            QAction m_pauseResume;

	        QSpinBox * m_before;
	        QPushButton * m_scanBefore;

	        QSpinBox * m_after;
	        QPushButton * m_scanAfter;

	        QTreeView * m_addressList;

	        QPushButton * m_clear;

	        PokeFinder m_pokeFinder;
    };
}

#endif // SPECTRUM_QTUI_POKEFINDER_POKEFINDERWINDOW_H
