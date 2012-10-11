#include "mainwindow.h"

#include "spectrum.h"
#include "qspectrumdisplay.h"
#include "spectrumthread.h"
#include "qspectrumdebugwindow.h"

#include <QToolBar>
#include <QAction>
#include <QSlider>
#include <QSpinBox>
#include <QFileDialog>
#include <QDebug>


using namespace Spectrum;


MainWindow::MainWindow( QWidget * parent )
:	QMainWindow(parent),
	m_speccyThread(0),
	m_speccy(),
	m_display(0),
	m_startPause(0),
	m_snapshot(0),
	m_reset(0),
	m_debug(0),
	m_debugStep(0),
	m_emulationSpeedSlider(0),
	m_emulationSpeedSpin(0),
	m_debugWindow(0) {
	std::cout << "creating widgets.\n";
	createWidgets();
	std::cout << "adding display device to spectrum.\n";
	m_speccy.addDisplayDevice(m_display);
	std::cout << "connecting widgets.\n";
	setCentralWidget(m_display);

	std::cout << "creating spectrum runner thread.\n";
	m_speccyThread = new SpectrumThread(&m_speccy);
	std::cout << "starting spectrum runner thread.\n";
	connectWidgets();
	m_speccyThread->start();
}


MainWindow::~MainWindow( void ) {
	m_speccyThread->quit();
	std::cerr << "waiting for spectrum runner thread to finish ...";

	while(!m_speccyThread->wait(100)) std::cerr << ".";

	std::cerr << "\n";
	delete m_speccyThread;
	m_speccyThread = 0;

	delete m_debugWindow;
	m_debugWindow = 0;
}


void MainWindow::createWidgets( void ) {
	std::cout << "creating spectrum display\n";
	m_display = new QSpectrumDisplay();
	std::cout << "creating toolbar\n";
	QToolBar * myToolBar = addToolBar("Main");
	m_startPause = myToolBar->addAction(QIcon::fromTheme("media-playback-start"), tr("Start/Pause"));
	m_startPause->setCheckable(true);
	m_reset = myToolBar->addAction(tr("Reset"));
	m_debug = myToolBar->addAction(tr("Debug"));
	m_debug->setCheckable(true);
	m_debugStep = myToolBar->addAction(tr("Step"));
	m_snapshot = myToolBar->addAction(QIcon::fromTheme("image"), tr("Snapshot"));
	myToolBar->addSeparator();
	m_emulationSpeedSlider = new QSlider(Qt::Horizontal);
	m_emulationSpeedSlider->setMinimum(0);
	m_emulationSpeedSlider->setMaximum(1000);
	myToolBar->addWidget(m_emulationSpeedSlider);
	m_emulationSpeedSpin = new QSpinBox();
	m_emulationSpeedSpin->setMinimum(0);
	m_emulationSpeedSpin->setMaximum(1000);
	m_emulationSpeedSpin->setSpecialValueText(tr("Unlimited"));
	myToolBar->addWidget(m_emulationSpeedSpin);

	m_debugWindow = new QSpectrumDebugWindow(&m_speccy);
}


void MainWindow::connectWidgets( void ) {
	connect(m_emulationSpeedSlider, SIGNAL(valueChanged(int)), m_emulationSpeedSpin, SLOT(setValue(int)));
	connect(m_emulationSpeedSpin, SIGNAL(valueChanged(int)), m_emulationSpeedSlider, SLOT(setValue(int)));

	connect(m_startPause, SIGNAL(toggled(bool)), this, SLOT(slotStartPause(bool)));
	connect(m_reset, SIGNAL(triggered()), m_speccyThread, SLOT(reset()));
	connect(m_debug, SIGNAL(triggered(bool)), m_speccyThread, SLOT(setDebugMode(bool)));
	connect(m_debug, SIGNAL(triggered(bool)), m_debugWindow, SLOT(setVisible(bool)));
	connect(m_debug, SIGNAL(triggered(bool)), m_debugWindow, SLOT(updateStateDisplay()));
	connect(m_debugStep, SIGNAL(triggered()), m_speccyThread, SLOT(step()));
	connect(m_reset, SIGNAL(triggered()), m_speccyThread, SLOT(reset()));
	connect(m_snapshot, SIGNAL(triggered()), this, SLOT(slotSnapshot()));
	connect(m_speccyThread, SIGNAL(debugStepTaken()), m_debugWindow, SLOT(updateStateDisplay()));
}


void MainWindow::saveSnapshot( const QString & fileName ) const {
	m_display->image()->save(fileName);
}


void MainWindow::slotStartPause( bool state ) {
	qDebug() << (state ? "resuming" : "pausing");
	if(state) m_speccyThread->resume();
	else m_speccyThread->pause();
}


void MainWindow::slotSnapshot( void ) {
	static QStringList s_filters;

	if(s_filters.isEmpty()) {
		s_filters << tr("Portable Network Graphics (PNG) files (*.png)");
		s_filters << tr("JPEG files (*.jpeg|*.jpg)");
		s_filters << tr("Windows Bitmap (BMP) files (*.bmp)");
	}

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save snapshot"), "", s_filters.join(";;"));
	if(fileName.isEmpty()) return;
	saveSnapshot(fileName);
}
