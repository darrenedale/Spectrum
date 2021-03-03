#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>

#include <QToolBar>
#include <QAction>
#include <QLabel>
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
  m_load(nullptr),
  m_startPause(nullptr),
  m_screenshot(nullptr),
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

	if (!m_spectrumThread->wait(250)) {
        std::cout << "Waiting for Spectrum thread to finish ";
        const int waitThreshold = 3000;
        int waited = 0;

        while (waited < waitThreshold && !m_spectrumThread->wait(100)) {
            std::cout << '.';
            waited += 100;
        }

        std::cout << '\n';

        if (!m_spectrumThread->isFinished()) {
            std::cerr << "forcibly terminating SpectrumThread @" << std::hex
                      << static_cast<void *>(m_spectrumThread.get()) << "\n";
            m_spectrumThread->terminate();
        }
    }

	delete m_debugWindow;
}

void MainWindow::createWidgets()
{
	QToolBar * myToolBar = addToolBar(QStringLiteral("Main"));
	m_load = myToolBar->addAction(QIcon::fromTheme(QStringLiteral("document-open")), tr("Load Snapshot"));
	myToolBar->addSeparator();
	m_startPause = myToolBar->addAction(QIcon::fromTheme(QStringLiteral("media-playback-start")), tr("Start/Pause"));
	m_reset = myToolBar->addAction(tr("Reset"));
	m_debug = myToolBar->addAction(tr("Debug"));
	m_debug->setCheckable(true);
	m_debugStep = myToolBar->addAction(QIcon::fromTheme(QStringLiteral("debug-step-instruction")), tr("Step"));
    m_screenshot = myToolBar->addAction(QIcon::fromTheme(QStringLiteral("image")), tr("Screenshot"));

    myToolBar = addToolBar(QStringLiteral("Speed"));
    myToolBar->addWidget(new QLabel(tr("Speed")));
	m_emulationSpeedSlider = new QSlider(Qt::Horizontal);
	m_emulationSpeedSlider->setMinimum(0);
	m_emulationSpeedSlider->setMaximum(10);
	m_emulationSpeedSlider->setValue(1);
	myToolBar->addWidget(m_emulationSpeedSlider);
	m_emulationSpeedSpin = new QSpinBox();
	m_emulationSpeedSpin->setMinimum(0);
	m_emulationSpeedSpin->setMaximum(10);
	m_emulationSpeedSpin->setValue(1);
	m_emulationSpeedSpin->setSuffix(QStringLiteral("x"));
	m_emulationSpeedSpin->setSpecialValueText(tr("Unlimited"));
	myToolBar->addWidget(m_emulationSpeedSpin);

	m_debugWindow = new QSpectrumDebugWindow(&m_spectrum);
}

void MainWindow::connectWidgets()
{
	connect(m_emulationSpeedSlider, &QSlider::valueChanged, m_emulationSpeedSpin, &QSpinBox::setValue);
	connect(m_emulationSpeedSpin, qOverload<int>(&QSpinBox::valueChanged), m_emulationSpeedSlider, &QSlider::setValue);

	connect(m_emulationSpeedSlider, &QSlider::valueChanged, this, &MainWindow::emulationSpeedChanged);

	connect(m_load, &QAction::triggered, this, &MainWindow::loadSnapshotTriggered);
	connect(m_startPause, &QAction::triggered, this, &MainWindow::startPauseClicked);
	connect(m_reset, &QAction::triggered, m_spectrumThread.get(), &SpectrumThread::reset, Qt::QueuedConnection);
	connect(m_debug, &QAction::triggered, m_spectrumThread.get(), &SpectrumThread::setDebugMode, Qt::QueuedConnection);
	connect(m_debug, &QAction::triggered, m_debugWindow, &QSpectrumDebugWindow::setVisible);
	connect(m_debug, &QAction::triggered, m_debugWindow, &QSpectrumDebugWindow::updateStateDisplay);
	connect(m_debugStep, &QAction::triggered, m_spectrumThread.get(), &SpectrumThread::step, Qt::QueuedConnection);
	connect(m_reset, &QAction::triggered, m_spectrumThread.get(), &SpectrumThread::reset, Qt::QueuedConnection);
	connect(m_screenshot, &QAction::triggered, this, &MainWindow::saveScreenshotTriggered);

	connect(m_spectrumThread.get(), &SpectrumThread::started, [this]() {
	    m_startPause->setIcon(QIcon::fromTheme("media-playback-pause"));
	});

	connect(m_spectrumThread.get(), &SpectrumThread::finished, [this]() {
	    m_startPause->setIcon(QIcon::fromTheme("media-playback-start"));
	});
}

void MainWindow::saveScreenshot(const QString & fileName) const
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

void MainWindow::saveScreenshotTriggered()
{
	static QStringList s_filters;

	if(s_filters.isEmpty()) {
		s_filters << tr("Portable Network Graphics (PNG) files (*.png)");
		s_filters << tr("JPEG files (*.jpeg|*.jpg)");
		s_filters << tr("Windows Bitmap (BMP) files (*.bmp)");
	}

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save screenshot"), "", s_filters.join(";;"));

	if(fileName.isEmpty()) {
	    return;
	}

    saveScreenshot(fileName);
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

void MainWindow::loadSnapshot(const QString & fileName)
{
    char buffer[49179];
    {
        std::ifstream in(fileName.toStdString());

        if (!in.is_open()) {
            // TODO display "not found" message
            std::cout << "Could not open file '" << fileName.toStdString() << "'\n";
            return;
        }

        int bytesToRead = sizeof(buffer);
        auto bytesIn = in.tellg();

        while (!in.eof() && 0 < bytesToRead - bytesIn) {
            in.read(buffer + bytesIn, bytesToRead);
            bytesIn = in.tellg();
        }

        if (-1 != bytesIn && sizeof(buffer) != bytesIn) {
            // TODO display "read error" message
            std::cout << "Error reading file '" << fileName.toStdString() << "'\n";
            return;
        }
    }

    auto paused = m_spectrumThread->paused();
    m_spectrumThread->pause();
    
    auto * ram = m_spectrum.memory() + 16384;
    auto & cpu = *(m_spectrum.z80());
    auto & registers = cpu.registers();
    auto * byte = buffer;

    for (auto & reg : {&registers.i, &registers.lShadow, &registers.hShadow, &registers.eShadow, &registers.dShadow, &registers.cShadow, &registers.bShadow, &registers.fShadow, &registers.aShadow, &registers.l, &registers.h, &registers.e, &registers.d, &registers.c, &registers.b, &registers.iyl, &registers.iyh, &registers.ixl, &registers.ixh}) {
        *reg = *(byte++);
    }

    bool iff = *(byte++) & 0x04;
    cpu.setIff1(iff);
    cpu.setIff2(iff);

    for (auto & reg : {&registers.r, &registers.f, &registers.a, &registers.spL, &registers.spH}) {
        *reg = *(byte++);
    }

    cpu.setInterruptMode(*(byte++) & 0x03);
    m_display.setBorder(static_cast<SpectrumDisplayDevice::Colour>(*(byte++) & 0x07));
    std::memcpy(ram, byte, 0xc000);

    // update the display
    Z80::UnsignedByte retn[2] = {0xed, 0x45};
    m_display.redrawDisplay(ram);
    m_displayWidget->setImage(m_display.image());

    // RETN instruction is required to resume execution of the .SNA
    cpu.execute(retn);

    if (!paused) {
        m_spectrumThread->resume();
    }
}

void MainWindow::loadSnapshotTriggered()
{
    static QStringList filters;

    if(filters.isEmpty()) {
        filters << tr("Snapshot (SNA) files (*.sna)");
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Load snapshot"), "", filters.join(";;"));

    if(fileName.isEmpty()) {
        return;
    }

    loadSnapshot(fileName);
}

void MainWindow::emulationSpeedChanged(int speed)
{
    if (0 == speed) {
        m_spectrum.setExecutionSpeedConstrained(false);
    } else {
        m_spectrum.setExecutionSpeedConstrained(true);
        m_spectrum.z80()->setClockSpeed(Spectrum::DefaultClockSpeed * speed);
    }
}
