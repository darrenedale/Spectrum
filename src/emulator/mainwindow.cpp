#include <iostream>

#include <QToolBar>
#include <QAction>
#include <QSlider>
#include <QSpinBox>
#include <QFileDialog>
#include <QDebug>
#include <QSettings>
#include <QDateTime>

#include "mainwindow.h"
#include "../qt/spectrumthread.h"
#include "../qt/ui/imagewidget.h"
#include "../qt/ui/qspectrumdebugwindow.h"

using namespace Spectrum;

MainWindow::MainWindow(QWidget * parent)
: QMainWindow(parent),
  m_spectrum(),
  m_spectrumThread(std::make_unique<SpectrumThread>(m_spectrum)),
  m_display(),
  m_displayWidget(std::make_unique<ImageWidget>()),
  m_startPause(nullptr),
  m_snapshot(nullptr),
  m_reset(nullptr),
  m_debug(nullptr),
  m_debugStep(nullptr),
  m_emulationSpeedSlider(nullptr),
  m_emulationSpeedSpin(nullptr),
  m_debugWindow(nullptr),
  m_displayRefreshTimer(nullptr)
{
    // update screen at 100 FPS
    m_displayRefreshTimer.setTimerType(Qt::TimerType::PreciseTimer);
    m_displayRefreshTimer.setInterval(10);
    m_displayRefreshTimer.callOnTimeout([this]() {
        m_displayWidget->setImage(m_display.image());
    });

	createWidgets();
	m_spectrum.addDisplayDevice(&m_display);
	setCentralWidget(m_displayWidget.get());
	connectWidgets();
	m_spectrumThread->start();
	m_displayRefreshTimer.start();
}

MainWindow::~MainWindow()
{
	m_spectrumThread->quit();
	const int waitThreshold = 3000;
	int waited = 0;

	while (waited < waitThreshold && !m_spectrumThread->wait(100)) {
	    waited += 100;
	}

	if (!m_spectrumThread->isFinished()) {
	    std::cerr << "forcibly terminating SpectrumThread @" << std::hex << static_cast<void *>(m_spectrumThread.get()) << "\n";
	    m_spectrumThread->terminate();
	}

	delete m_debugWindow;
}

void MainWindow::createWidgets()
{
	QToolBar * myToolBar = addToolBar(QStringLiteral("Main"));
	m_startPause = myToolBar->addAction(QIcon::fromTheme("media-playback-start"), tr("Start/Pause"));
	m_reset = myToolBar->addAction(tr("Reset"));
	m_debug = myToolBar->addAction(tr("Debug"));
	m_debug->setCheckable(true);
	m_debugStep = myToolBar->addAction(QIcon::fromTheme("debug-step-instruction"), tr("Step"));
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

	m_debugWindow = new QSpectrumDebugWindow(&m_spectrum);
}

void MainWindow::connectWidgets()
{
	connect(m_emulationSpeedSlider, &QSlider::valueChanged, m_emulationSpeedSpin, &QSpinBox::setValue);
	connect(m_emulationSpeedSpin, qOverload<int>(&QSpinBox::valueChanged), m_emulationSpeedSlider, &QSlider::setValue);

	connect(m_startPause, &QAction::triggered, this, &MainWindow::startPauseClicked);
	connect(m_reset, &QAction::triggered, m_spectrumThread.get(), &SpectrumThread::reset, Qt::QueuedConnection);
	connect(m_debug, &QAction::triggered, m_spectrumThread.get(), &SpectrumThread::setDebugMode, Qt::QueuedConnection);
	connect(m_debug, &QAction::triggered, m_debugWindow, &QSpectrumDebugWindow::setVisible);
	connect(m_debug, &QAction::triggered, m_debugWindow, &QSpectrumDebugWindow::updateStateDisplay);
	connect(m_debugStep, &QAction::triggered, m_spectrumThread.get(), &SpectrumThread::step, Qt::QueuedConnection);
	connect(m_reset, &QAction::triggered, m_spectrumThread.get(), &SpectrumThread::reset, Qt::QueuedConnection);
	connect(m_snapshot, &QAction::triggered, this, &MainWindow::snapshotTriggered);

	connect(m_spectrumThread.get(), &SpectrumThread::started, [this]() {
	    m_startPause->setIcon(QIcon::fromTheme("media-playback-pause"));
	});

	connect(m_spectrumThread.get(), &SpectrumThread::finished, [this]() {
	    m_startPause->setIcon(QIcon::fromTheme("media-playback-start"));
	});
}

void MainWindow::updateSpectrumDisplay(const QImage & image)
{
    m_displayWidget->setImage(image);
}

void MainWindow::saveSnapshot(const QString & fileName) const
{
	m_display.image().save(fileName);
}

void MainWindow::startPauseClicked()
{
	if (m_spectrumThread->paused()) {
        m_spectrumThread->resume();
        m_displayRefreshTimer.start();
        m_startPause->setIcon(QIcon::fromTheme("media-playback-pause"));
    } else {
        m_spectrumThread->pause();
        m_displayRefreshTimer.stop();
        m_startPause->setIcon(QIcon::fromTheme("media-playback-start"));
    }
}

void MainWindow::snapshotTriggered()
{
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

void MainWindow::closeEvent(QCloseEvent * ev)
{
    QSettings settings;
    settings.beginGroup("mainwindow");
    settings.setValue("position", pos());
    settings.setValue("size", size());
    settings.endGroup();
    m_debugWindow->close();
    QWidget::closeEvent(ev);
}

void MainWindow::showEvent(QShowEvent * ev)
{
    QSettings settings;
    settings.beginGroup("mainwindow");
    setGeometry({settings.value(QStringLiteral("position")).toPoint(), settings.value(QStringLiteral("size")).toSize()});
    settings.endGroup();
}
