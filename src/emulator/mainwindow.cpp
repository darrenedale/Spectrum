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
#include <QEvent>
#include <QDebug>
#include <QSettings>
#include <QDateTime>
#include <QPainter>

#include "mainwindow.h"
#include "../qt/ui/imagewidget.h"

using namespace Spectrum;

MainWindow::MainWindow(QWidget * parent)
: QMainWindow(parent),
  m_spectrum(),
  m_spectrumThread(m_spectrum),
  m_display(),
  m_displayWidget(),
  m_load(QIcon::fromTheme(QStringLiteral("document-open")), tr("Load Snapshot")),
  m_pauseResume(QIcon::fromTheme(QStringLiteral("media-playback-start")), tr("Start/Pause")),
  m_screenshot(QIcon::fromTheme(QStringLiteral("image")), tr("Screenshot")),
  m_refreshScreen(QIcon::fromTheme("view-refresh"), tr("Refresh")),
  m_reset(QIcon::fromTheme(QStringLiteral("start-over")), tr("Reset")),
  m_debug(tr("Debug")),
  m_debugStep(QIcon::fromTheme(QStringLiteral("debug-step-instruction")), tr("Step")),
  m_emulationSpeedSlider(Qt::Horizontal),
  m_emulationSpeedSpin(nullptr),
  m_debugWindow(&m_spectrumThread),
  m_displayRefreshTimer(nullptr)
{
    m_spectrum.setKeyboard(&m_keyboard);
    installEventFilter(&m_keyboard);
    m_debugWindow.installEventFilter(this);
    m_pauseResume.setShortcut(Qt::Key::Key_Control + Qt::Key::Key_P);
    m_debug.setShortcut(Qt::Key::Key_Control + Qt::Key::Key_D);
    m_debug.setCheckable(true);
    m_debugStep.setShortcut(Qt::Key::Key_Space);
    m_screenshot.setShortcut(Qt::Key::Key_Print);
    m_refreshScreen.setShortcut(Qt::Key::Key_F5);

    m_emulationSpeedSlider.setMinimum(0);
    m_emulationSpeedSlider.setMaximum(10);
    m_emulationSpeedSlider.setValue(1);

    m_emulationSpeedSpin.setMinimum(0);
    m_emulationSpeedSpin.setMaximum(10);
    m_emulationSpeedSpin.setValue(1);
    m_emulationSpeedSpin.setSuffix(QStringLiteral("x"));
    m_emulationSpeedSpin.setSpecialValueText(tr("Unlimited"));

    // update screen at 100 FPS
    m_displayRefreshTimer.setTimerType(Qt::TimerType::PreciseTimer);
    m_displayRefreshTimer.setInterval(10);
    connect(&m_displayRefreshTimer, &QTimer::timeout, this, &MainWindow::refreshSpectrumDisplay);

    createToolbars();
	m_spectrum.addDisplayDevice(&m_display);
	setCentralWidget(&m_displayWidget);

	connect(&m_spectrumThread, &SpectrumThread::paused, this, &MainWindow::threadPaused);
	connect(&m_spectrumThread, &SpectrumThread::resumed, this, &MainWindow::threadResumed);

    connectSignals();
	m_spectrumThread.start();
	threadResumed();
}

MainWindow::~MainWindow()
{
	m_spectrumThread.quit();

	if (!m_spectrumThread.wait(250)) {
        std::cout << "Waiting for Spectrum thread to finish ";
        const int waitThreshold = 3000;
        int waited = 0;

        while (waited < waitThreshold && !m_spectrumThread.wait(100)) {
            std::cout << '.';
            waited += 100;
        }

        std::cout << '\n';

        if (!m_spectrumThread.isFinished()) {
            std::cerr << "forcibly terminating SpectrumThread @" << std::hex
                      << static_cast<void *>(&m_spectrumThread) << "\n";
            m_spectrumThread.terminate();
        }
    }
}

void MainWindow::createToolbars()
{
	auto * tempToolBar = addToolBar(QStringLiteral("Main"));
    tempToolBar->addAction(&m_load);
    tempToolBar->addSeparator();
    tempToolBar->addAction(&m_pauseResume);
    tempToolBar->addAction(&m_reset);
    tempToolBar->addAction(&m_debug);
    tempToolBar->addAction(&m_debugStep);
    tempToolBar->addAction(&m_screenshot);
    tempToolBar->addAction(&m_refreshScreen);

    tempToolBar = addToolBar(QStringLiteral("Speed"));
    tempToolBar->addWidget(new QLabel(tr("Speed")));
    tempToolBar->addWidget(&m_emulationSpeedSlider);
    tempToolBar->addWidget(&m_emulationSpeedSpin);
}

void MainWindow::connectSignals()
{
	connect(&m_emulationSpeedSlider, &QSlider::valueChanged, &m_emulationSpeedSpin, &QSpinBox::setValue);
	connect(&m_emulationSpeedSpin, qOverload<int>(&QSpinBox::valueChanged), &m_emulationSpeedSlider, &QSlider::setValue);

	connect(&m_emulationSpeedSlider, &QSlider::valueChanged, this, &MainWindow::emulationSpeedChanged);

	connect(&m_load, &QAction::triggered, this, &MainWindow::loadSnapshotTriggered);
	connect(&m_pauseResume, &QAction::triggered, this, &MainWindow::pauseResumeTriggered);
	connect(&m_reset, &QAction::triggered, &m_spectrumThread, &SpectrumThread::reset);
	connect(&m_debug, &QAction::triggered, this, &MainWindow::debugTriggered);
	connect(&m_debugStep, &QAction::triggered, this, &MainWindow::stepTriggered);
	connect(&m_screenshot, &QAction::triggered, this, &MainWindow::saveScreenshotTriggered);
	connect(&m_refreshScreen, &QAction::triggered, this, &MainWindow::refreshSpectrumDisplay);
}

void MainWindow::closeEvent(QCloseEvent * ev)
{
    QSettings settings;
    settings.beginGroup("mainwindow");
    settings.setValue("position", pos());
    settings.setValue("size", size());
    settings.endGroup();
    m_debugWindow.close();
    QWidget::closeEvent(ev);
}

void MainWindow::showEvent(QShowEvent * ev)
{
    QSettings settings;
    settings.beginGroup("mainwindow");
    setGeometry({settings.value(QStringLiteral("position")).toPoint(), settings.value(QStringLiteral("size")).toSize()});
    settings.endGroup();
}

void MainWindow::saveScreenshot(const QString & fileName) const
{
	m_display.image().save(fileName);
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

    auto paused = m_spectrumThread.isPaused();
    m_spectrumThread.pause();
    
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

    cpu.setInterruptMode(static_cast<Z80::Z80::InterruptMode>(*(byte++) & 0x03));
    m_display.setBorder(static_cast<SpectrumDisplayDevice::Colour>(*(byte++) & 0x07));
    std::memcpy(ram, byte, 0xc000);

    // update the display
    Z80::UnsignedByte retn[2] = {0xed, 0x45};
    m_display.redrawDisplay(ram);
    m_displayWidget.setImage(m_display.image());

    // RETN instruction is required to resume execution of the .SNA
    cpu.execute(retn);

    if (!paused) {
        m_spectrumThread.resume();
    }
}

void MainWindow::pauseResumeTriggered()
{
    if (m_spectrumThread.isPaused()) {
        m_spectrumThread.resume();
    } else {
        m_spectrumThread.pause();
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

void MainWindow::refreshSpectrumDisplay()
{
    auto image = m_display.image().scaledToWidth(512);

    QPainter painter(&image);
    QFont font = painter.font();
    font.setPixelSize(10);
    font.setFixedPitch(true);
    painter.setFont(font);
    int y = 2;
    auto & registers = m_spectrum.z80()->registers();
    QLatin1Char fill('0');
    painter.drawText(2, y+= 10, QStringLiteral("PC: $%1").arg(registers.pc, 4, 16, fill));
    painter.drawText(2, y+= 10, QStringLiteral("SP: $%1").arg(registers.sp, 4, 16, fill));
    painter.drawText(2, y+= 10, QStringLiteral("AF: $%1").arg(registers.af, 4, 16, fill));
    painter.drawText(2, y+= 10, QStringLiteral("BC: $%1").arg(registers.bc, 4, 16, fill));
    painter.drawText(2, y+= 10, QStringLiteral("DE: $%1").arg(registers.de, 4, 16, fill));
    painter.drawText(2, y+= 10, QStringLiteral("HL: $%1").arg(registers.hl, 4, 16, fill));
    painter.end();

    m_displayWidget.setImage(image);
}

void MainWindow::debugTriggered()
{
    if (m_debug.isChecked()) {
        m_debugWindow.show();
        m_debugWindow.activateWindow();
        // TODO bring to front
    }
    else {
        m_debugWindow.hide();
    }
}

void MainWindow::stepTriggered()
{
    m_spectrumThread.step();
}

void MainWindow::threadPaused()
{
    m_displayRefreshTimer.stop();
    refreshSpectrumDisplay();
    m_pauseResume.setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
    m_pauseResume.setText(tr("Resume"));

    // TODO enable widgets
    m_debugStep.setEnabled(true);
    connect(&m_spectrumThread, &SpectrumThread::stepped, this, &MainWindow::threadStepped, Qt::UniqueConnection);
}

void MainWindow::threadResumed()
{
    m_displayRefreshTimer.start();
    m_pauseResume.setIcon(QIcon::fromTheme(QStringLiteral("media-playback-pause")));
    m_pauseResume.setText(tr("Pause"));

    // TODO disable widgets
    m_debugStep.setEnabled(false);
    disconnect(&m_spectrumThread, &SpectrumThread::stepped, this, &MainWindow::threadStepped);
}

void MainWindow::threadStepped()
{
    refreshSpectrumDisplay();
}

bool MainWindow::eventFilter(QObject * target, QEvent * event)
{
    if (target == &m_debugWindow) {
        switch (event->type()) {
            case QEvent::Type::Show:
                m_debug.setChecked(true);
                break;

            case QEvent::Type::Hide:
                m_debug.setChecked(false);
                break;
        }
    }

    return false;
}
