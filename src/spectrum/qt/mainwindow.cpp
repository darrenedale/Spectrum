#include <cstring>
#include <filesystem>
#include <iostream>
#include <fstream>

#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QFileDialog>
#include <QEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QDebug>
#include <QSettings>
#include <QDateTime>
#include <QPainter>
#include <memory>

#include "mainwindow.h"
#include "threadpauser.h"
#include "../snapshot.h"
#include "../snasnapshotreader.h"
#include "../z80snapshotreader.h"
#include "../zxsnapshotreader.h"
#include "../z80snapshotwriter.h"
#include "../snasnapshotwriter.h"

using namespace Spectrum::Qt;

using InterruptMode = ::Z80::InterruptMode;

namespace
{
    constexpr const int DefaultStatusBarMessageTimeout = 5000;
    auto z80ToHostByteOrder = ::Z80::Z80::z80ToHostByteOrder;
    auto hostToZ80ByteOrder = ::Z80::Z80::hostToZ80ByteOrder;
}

MainWindow::MainWindow(QWidget * parent)
: QMainWindow(parent),
  m_spectrum("spectrum48.rom"),
  m_spectrumThread(m_spectrum),
  m_display(),
  m_displayWidget(),
  m_load(QIcon::fromTheme(QStringLiteral("document-open")), tr("Load Snapshot")),
  m_save(QIcon::fromTheme(QStringLiteral("document-save")), tr("save Snapshot")),
  m_pauseResume(QIcon::fromTheme(QStringLiteral("media-playback-start")), tr("Start/Pause")),
  m_saveScreenshot(QIcon::fromTheme(QStringLiteral("image")), tr("Screenshot")),
  m_refreshScreen(QIcon::fromTheme("view-refresh"), tr("Refresh")),
  m_reset(QIcon::fromTheme(QStringLiteral("start-over")), tr("Reset")),
  m_debug(tr("Debug")),
  m_debugStep(QIcon::fromTheme(QStringLiteral("debug-step-instruction")), tr("Step")),
  m_emulationSpeedSlider(::Qt::Horizontal),
  m_emulationSpeedSpin(nullptr),
  m_debugWindow(&m_spectrumThread),
  m_displayRefreshTimer(nullptr)
{
    m_spectrum.setExecutionSpeedConstrained(true);
    m_displayWidget.keepAspectRatio();
    m_displayWidget.setFocusPolicy(::Qt::FocusPolicy::ClickFocus);
    m_displayWidget.setFocus();
    m_displayWidget.installEventFilter(&m_joystick);
    m_displayWidget.installEventFilter(&m_keyboard);
    m_displayWidget.installEventFilter(this);
    m_debugWindow.installEventFilter(this);
    m_load.setShortcuts({::Qt::Key::Key_Control + ::Qt::Key::Key_O, ::Qt::Key::Key_F1});
    m_save.setShortcuts({::Qt::Key::Key_Control + ::Qt::Key::Key_S, ::Qt::Key::Key_F2});
    m_pauseResume.setShortcuts({::Qt::Key::Key_Pause, ::Qt::Key::Key_Escape, ::Qt::Key::Key_Control + ::Qt::Key_P,});
    m_debug.setShortcuts({::Qt::Key::Key_Control + ::Qt::Key::Key_D, ::Qt::Key::Key_F12,});
    m_debug.setCheckable(true);
    m_debugStep.setShortcut(::Qt::Key::Key_Space);
    m_saveScreenshot.setShortcut(::Qt::Key::Key_Print);
    m_refreshScreen.setShortcut(::Qt::Key::Key_F5);

    m_emulationSpeedSlider.setMinimum(0);
    m_emulationSpeedSlider.setMaximum(1000);
    m_emulationSpeedSlider.setValue(100);
    m_emulationSpeedSlider.setSingleStep(10);

    m_emulationSpeedSpin.setMinimum(0);
    m_emulationSpeedSpin.setMaximum(1000);
    m_emulationSpeedSpin.setValue(100);
    m_emulationSpeedSpin.setSingleStep(10);
    m_emulationSpeedSpin.setSuffix(QStringLiteral("%"));
    m_emulationSpeedSpin.setSpecialValueText(tr("Unlimited"));

    m_statusBarEmulationSpeed.setToolTip(tr("Current emulation speed."));
    m_statusBarMHz.setToolTip(tr("Current speed of the emulated Z80."));

    // update screen at 100 FPS
    m_displayRefreshTimer.setTimerType(::Qt::TimerType::PreciseTimer);
    m_displayRefreshTimer.setInterval(10);
    connect(&m_displayRefreshTimer, &QTimer::timeout, this, &MainWindow::refreshSpectrumDisplay);

    createToolbars();
    createStatusBar();

    m_spectrum.setJoystickInterface(&m_joystick);
    m_spectrum.setKeyboard(&m_keyboard);
	m_spectrum.addDisplayDevice(&m_display);

    setCentralWidget(&m_displayWidget);

	connect(&m_spectrumThread, &Thread::paused, this, &MainWindow::threadPaused);
	connect(&m_spectrumThread, &Thread::resumed, this, &MainWindow::threadResumed);

    setAcceptDrops(true);
    connectSignals();
	m_spectrumThread.start();
	threadResumed();
    updateStatusBarSpeedWidget();
}

MainWindow::~MainWindow()
{
    m_spectrum.setKeyboard(nullptr);
    m_spectrum.removeDisplayDevice(&m_display);
	m_spectrumThread.stop();

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
    tempToolBar->addAction(&m_save);
    tempToolBar->addSeparator();
    tempToolBar->addAction(&m_pauseResume);
    tempToolBar->addAction(&m_reset);
    tempToolBar->addAction(&m_debug);
    tempToolBar->addAction(&m_debugStep);
    tempToolBar->addAction(&m_saveScreenshot);
    tempToolBar->addAction(&m_refreshScreen);

    tempToolBar = addToolBar(QStringLiteral("Speed"));
    tempToolBar->addWidget(new QLabel(tr("Speed")));
    tempToolBar->addWidget(&m_emulationSpeedSlider);
    tempToolBar->addWidget(&m_emulationSpeedSpin);
}

void MainWindow::createStatusBar()
{
    auto * statusBar = new QStatusBar(this);
    statusBar->setSizeGripEnabled(true);
    statusBar->addPermanentWidget(&m_statusBarPause);
    statusBar->addPermanentWidget(&m_statusBarEmulationSpeed);
    statusBar->addPermanentWidget(&m_statusBarMHz);
    setStatusBar(statusBar);
}

void MainWindow::connectSignals()
{
	connect(&m_emulationSpeedSlider, &QSlider::valueChanged, &m_emulationSpeedSpin, &QSpinBox::setValue);
	connect(&m_emulationSpeedSpin, qOverload<int>(&QSpinBox::valueChanged), &m_emulationSpeedSlider, &QSlider::setValue);

	connect(&m_emulationSpeedSlider, &QSlider::valueChanged, this, &MainWindow::emulationSpeedChanged);

	connect(&m_load, &QAction::triggered, this, &MainWindow::loadSnapshotTriggered);
	connect(&m_save, &QAction::triggered, this, &MainWindow::saveSnapshotTriggered);
	connect(&m_pauseResume, &QAction::triggered, this, &MainWindow::pauseResumeTriggered);
	connect(&m_reset, &QAction::triggered, &m_spectrumThread, &Thread::reset);
	connect(&m_debug, &QAction::triggered, this, &MainWindow::debugTriggered);
	connect(&m_debugStep, &QAction::triggered, this, &MainWindow::stepTriggered);
	connect(&m_saveScreenshot, &QAction::triggered, this, &MainWindow::saveScreenshotTriggered);
	connect(&m_refreshScreen, &QAction::triggered, this, &MainWindow::refreshSpectrumDisplay);
}

void MainWindow::refreshSpectrumDisplay()
{
    if (m_debug.isChecked()) {
        auto image = m_display.image();
        image = image.scaledToWidth(image.width() * 2);
        auto pen = QColor(*reinterpret_cast<QRgb *>(image.bits()));

        if (pen.lightness() > 128) {
            pen = ::Qt::GlobalColor::black;
        } else {
            pen = ::Qt::GlobalColor::white;
        }

        QPainter painter(&image);
        QFont font = painter.font();
        font.setPixelSize(10);
        font.setFixedPitch(true);
        painter.setFont(font);
        painter.setPen(pen);
        int y = 2;
        auto & registers = m_spectrum.z80()->registers();
        QLatin1Char fill('0');
        painter.drawText(2, y += 10, QStringLiteral("PC: $%1").arg(registers.pc, 4, 16, fill));
        painter.drawText(2, y += 10, QStringLiteral("SP: $%1").arg(registers.sp, 4, 16, fill));
        painter.drawText(2, y += 10, QStringLiteral("AF: $%1").arg(registers.af, 4, 16, fill));
        painter.drawText(2, y += 10, QStringLiteral("BC: $%1").arg(registers.bc, 4, 16, fill));
        painter.drawText(2, y += 10, QStringLiteral("DE: $%1").arg(registers.de, 4, 16, fill));
        painter.drawText(2, y + 10, QStringLiteral("HL: $%1").arg(registers.hl, 4, 16, fill));
        painter.end();

        m_displayWidget.setImage(image);
    } else {
        m_displayWidget.setImage(m_display.image());
    }
}

void MainWindow::saveScreenshot(const QString & fileName)
{
    ThreadPauser pauser(m_spectrumThread);

    if (fileName.endsWith(QStringLiteral(".scr"), ::Qt::CaseSensitivity::CaseInsensitive)) {
        std::ofstream outFile(fileName.toStdString());

        if (!outFile.is_open()) {
            std::cout << "Could not open file '" << fileName.toStdString() << "' for writing\n";
            return;
        }

        outFile.write(reinterpret_cast<const char *>(m_spectrum.displayMemory()), m_spectrum.displayMemorySize());
        outFile.close();
    } else {
        m_display.image().save(fileName);
    }
}

QString MainWindow::guessSnapshotFormat(const QString & fileName)
{
    if (auto matches = QRegularExpression("^.*\\.([a-zA-Z0-9_-]+)$").match(fileName); matches.hasMatch()) {
        return matches.captured(1).toLower();
    }

    return {};
}

void MainWindow::loadSnapshot(const QString & fileName, QString format)
{
    ThreadPauser pauser(m_spectrumThread);

    if (format.isEmpty()) {
        format = guessSnapshotFormat(fileName);

        if (format.isEmpty()) {
            statusBar()->showMessage(tr("The snapshot format for %1 could not be determined.").arg(fileName), DefaultStatusBarMessageTimeout);
        }
    }

    // TODO use a factory to get a reader for a format
    std::unique_ptr<SnapshotReader> reader;

    if ("sna" == format) {
        reader = std::make_unique<SnaSnapshotReader>(fileName.toStdString());
    } else if ("z80" == format) {
        reader = std::make_unique<Z80SnapshotReader>(fileName.toStdString());
    } else if ("sp" == format) {
        // TODO SP snapshot reader class
        loadSpSnapshot(fileName);
        return;
    } else if ("zx" == format) {
        reader = std::make_unique<ZXSnapshotReader>(fileName.toStdString());
    }

    if (!reader) {
        std::cerr << "unrecognised format '" << format.toStdString() << "' from filename '" << fileName.toStdString() << "'\n";
        statusBar()->showMessage(tr("The snapshot format for %1 could not be determined.").arg(fileName), DefaultStatusBarMessageTimeout);
    }

    if (!reader->isOpen()) {
        std::cerr << "Snapshot file '" << fileName.toStdString() << "' could not be opened.\n";
        statusBar()->showMessage(tr("The snapshot file %1 could not be opened.").arg(fileName), DefaultStatusBarMessageTimeout);
        return;
    }

    bool successful;
    const auto & snapshot = reader->read(successful);

    if (!successful) {
        statusBar()->showMessage(tr("The snapshot file %1 is not valid.").arg(fileName), DefaultStatusBarMessageTimeout);
        return;
    }

    snapshot.applyTo(m_spectrum);
    m_display.redrawDisplay(m_spectrum.displayMemory());
    m_displayWidget.setImage(m_display.image());

    if ("sna" == format) {
        // RETN instruction is required to resume execution of the .SNA
        m_spectrum.z80()->execute(reinterpret_cast<const Z80::UnsignedByte *>("\xed\x45"));
    }

    statusBar()->showMessage(tr("The snapshot file %1 was successfully loaded.").arg(fileName), DefaultStatusBarMessageTimeout);
}

// TODO extract this to a SnapshotReader class
void MainWindow::loadSpSnapshot(const QString & fileName)
{
    // NOTE all data is in Z80 byte order
    struct Header
    {
        char signature[2];
        std::uint16_t length;
        std::uint16_t baseAddress;

        struct
        {
            std::uint16_t bc;
            std::uint16_t de;
            std::uint16_t hl;
            std::uint16_t af;
            std::uint16_t ix;
            std::uint16_t iy;
        } registers;

        struct
        {
            std::uint16_t bc;
            std::uint16_t de;
            std::uint16_t hl;
            std::uint16_t af;
        } shadowRegisters;

        struct {
            std::uint8_t r;
            std::uint8_t i;
        } interruptRegisters;

        std::uint16_t sp;
        std::uint16_t pc;
        std::uint16_t reserved1;
        std::uint8_t border;
        std::uint8_t reserved2;
        std::uint16_t status;
    };

    ThreadPauser pauser(m_spectrumThread);

    // read file
    Header header = {};
    std::ifstream::char_type * programBuffer;

    {
        std::ifstream in(fileName.toStdString());

        if (!in) {
            std::cerr << "Could not open file '" << fileName.toStdString() << "'\n";
            statusBar()->showMessage(tr("The snapshot file %1 could not be opened.").arg(fileName), DefaultStatusBarMessageTimeout);
            return;
        }

        // read header
        in.read(reinterpret_cast<std::ifstream::char_type *>(&header), sizeof(header));

        if (in.fail()) {
            std::cerr << "Error reading header from '" << fileName.toStdString() << "'\n";
            statusBar()->showMessage(tr("The snapshot file %1 could not be read.").arg(fileName), DefaultStatusBarMessageTimeout);
            return;
        }

        // validate header
        if (*reinterpret_cast<const std::uint16_t *>("SP") != *reinterpret_cast<const std::uint16_t *>(header.signature)) {
            std::cerr << "Not an SP file.";
            statusBar()->showMessage(tr("The snapshot file %1 is not valid (not an SP file).").arg(fileName), DefaultStatusBarMessageTimeout);
            return;
        }

        // NOTE from here on, the length and baseAddress members of the header are in host byte order
        header.length = z80ToHostByteOrder(header.length);
        header.baseAddress = z80ToHostByteOrder(header.baseAddress);

        if (0x0000ffff < static_cast<int>(header.baseAddress) + header.length - 1) {
            std::cerr << std::hex << std::setfill('0');
            std::cerr << "Program extends beyond upper bounds of RAM (0x" << std::setw(4) << header.baseAddress << " + " << std::dec << header.length << ") > 0xffff\n";
            std::cerr << "Base address: 0x" << std::hex << std::setw(4) << header.baseAddress << "\n";
            std::cerr << "Length      : " << std::dec <<  header.length << " bytes\n";
            std::cerr << "End address : 0x" << std::hex <<  std::setw(5) << (static_cast<std::uint32_t>(header.baseAddress) + header.length) << "\n";
            std::cerr << "Program extends beyond upper bounds of RAM (0x" << std::setw(4) << header.baseAddress << " + " << std::dec << header.length << ") > 0xffff\n";
            std::cerr << std::setfill(' ');
            statusBar()->showMessage(tr("The snapshot file %1 is not valid (program machine code image too large).").arg(fileName), DefaultStatusBarMessageTimeout);
            return;
        }

        header.border = header.border & 0x07;

        programBuffer = new std::ifstream::char_type[header.length];
        in.read(programBuffer, header.length);

        if (in.fail()) {
            std::cerr << "Error reading program from '" << fileName.toStdString() << "'\n";
            delete[] programBuffer;
            statusBar()->showMessage(tr("The snapshot file %1 could not be read.").arg(fileName), DefaultStatusBarMessageTimeout);
            return;
        }

        if (!in.eof()) {
            std::cerr << "Warning: ignored extraneous content at end of file (from byte " << in.tellg() << " onward).\n";
        }
    }

    // set state
    auto * memory = m_spectrum.memory();
    auto & cpu = *(m_spectrum.z80());
    auto & registers = cpu.registers();

    registers.bc = z80ToHostByteOrder(header.registers.bc);
    registers.de = z80ToHostByteOrder(header.registers.de);
    registers.hl = z80ToHostByteOrder(header.registers.hl);
    registers.af = z80ToHostByteOrder(header.registers.af);
    registers.ix = z80ToHostByteOrder(header.registers.ix);
    registers.iy = z80ToHostByteOrder(header.registers.iy);

    registers.bcShadow = z80ToHostByteOrder(header.shadowRegisters.bc);
    registers.deShadow = z80ToHostByteOrder(header.shadowRegisters.de);
    registers.hlShadow = z80ToHostByteOrder(header.shadowRegisters.hl);
    registers.afShadow = z80ToHostByteOrder(header.shadowRegisters.af);

    registers.r = header.interruptRegisters.r;
    registers.i = header.interruptRegisters.i;

    registers.sp = z80ToHostByteOrder(header.sp);
    registers.pc = z80ToHostByteOrder(header.pc);

    m_display.setBorder(static_cast<Colour>(header.border));

    // NOTE from here on, the status member of the header is in host byte order
    header.status = z80ToHostByteOrder(header.status);
    // bit 0 = IFF1
    cpu.setIff1(header.status & 0x0001);
    // bit 2 = IFF2
    cpu.setIff2(header.status & 0x0004);

    // bit 1 = IM
    cpu.setInterruptMode(header.status & 0x0002 ? ::InterruptMode::IM2 : ::InterruptMode::IM1);

    // bit 4 = interrupt pending
    if (header.status & 0x0010) {
        cpu.interrupt();
    }

    // NOTE bit 5 of status indicates flash state but we don't use this

    std::memcpy(memory + header.baseAddress, programBuffer, header.length);

    delete[] programBuffer;
    statusBar()->showMessage(tr("The snapshot file %1 was successfully loaded.").arg(fileName), DefaultStatusBarMessageTimeout);
}

void MainWindow::saveSnapshot(const QString & fileName, QString format)
{
    ThreadPauser pauser(m_spectrumThread);

    if (format.isEmpty()) {
        format = guessSnapshotFormat(fileName);

        if (format.isEmpty()) {
            statusBar()->showMessage(tr("The snapshot format to use could not be determined from the filename %1.").arg(fileName), DefaultStatusBarMessageTimeout);
            return;
        }
    }

    if ("sna" == format) {
        auto * z80 = m_spectrum.z80();
        z80->execute(reinterpret_cast<const Z80::UnsignedByte *>("\xcd\x00\x00"), false);
        SnaSnapshotWriter writer(Snapshot{m_spectrum});
        z80->execute(reinterpret_cast<const Z80::UnsignedByte *>("\xed\x45"), false);

        if (!writer.writeTo(fileName.toStdString())) {
            std::cerr << "failed to write snapshot to '" << fileName.toStdString() << "'\n";
            statusBar()->showMessage(tr("Failed to save snapshot to %1.").arg(fileName), DefaultStatusBarMessageTimeout);
        } else {
            statusBar()->showMessage(tr("Snapshot successfully saved to %1.").arg(fileName), DefaultStatusBarMessageTimeout);
        }
    } else if ("z80" == format) {
        Z80SnapshotWriter writer(Snapshot{m_spectrum});

        if (!writer.writeTo(fileName.toStdString())) {
            std::cerr << "failed to write snapshot to '" << fileName.toStdString() << "'\n";
            statusBar()->showMessage(tr("Failed to save snapshot to %1.").arg(fileName), DefaultStatusBarMessageTimeout);
        } else {
            statusBar()->showMessage(tr("Snapshot successfully saved to %1.").arg(fileName), DefaultStatusBarMessageTimeout);
        }
    } else if ("sp" == format) {
        std::cerr << "Not yet implemented.\n";
        statusBar()->showMessage(tr("Saving .sp snapshots is not yet implemented.").arg(fileName), DefaultStatusBarMessageTimeout);
    } else {
        std::cerr << "unrecognised format '" << format.toStdString() << "' from filename '" << fileName.toStdString() << "'\n";
        statusBar()->showMessage(tr("Unrecognised snapshot format %1.").arg(format), DefaultStatusBarMessageTimeout);
    }
}

bool MainWindow::eventFilter(QObject * target, QEvent * event)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
    // while user holds tab on display widget, temporarily run as fast as possible
    if (&m_displayWidget == target) {
        switch (event->type()) {
            case QEvent::Type::KeyPress:
                if (::Qt::Key::Key_Tab == dynamic_cast<QKeyEvent *>(event)->key()) {
                    m_spectrum.setExecutionSpeedConstrained(false);
                    updateStatusBarSpeedWidget();
                    return true;
                }
                break;


            case QEvent::Type::KeyRelease:
                if (::Qt::Key::Key_Tab == dynamic_cast<QKeyEvent *>(event)->key()) {
                    m_spectrum.setExecutionSpeedConstrained(0 < m_emulationSpeedSlider.value());
                    updateStatusBarSpeedWidget();
                    return true;
                }
                break;

        }
    }

    if (target == &m_debugWindow) {
        switch (event->type()) {
            case QEvent::Type::Show:
                m_debug.setChecked(true);
                break;

            case QEvent::Type::Hide:
                m_debug.setChecked(false);
                m_spectrumThread.setDebugMode(false);
                break;
        }
    }
#pragma clang diagnostic pop

    return false;
}

void MainWindow::closeEvent(QCloseEvent * ev)
{
    QSettings settings;
    settings.beginGroup("mainwindow");
    settings.setValue("position", pos());
    settings.setValue("size", size());
    settings.setValue("lastSnapshotLoadDir", m_lastSnapshotLoadDir);
    settings.setValue("lastScreenshotDir", m_lastScreenshotDir);
    settings.setValue("emulationSpeed", m_emulationSpeedSlider.value());
    settings.endGroup();
    m_debugWindow.close();
    QWidget::closeEvent(ev);
}

void MainWindow::showEvent(QShowEvent * ev)
{
    QSettings settings;
    bool ok;
    settings.beginGroup("mainwindow");
    setGeometry({settings.value(QStringLiteral("position")).toPoint(), settings.value(QStringLiteral("size")).toSize()});
    m_lastSnapshotLoadDir = settings.value(QStringLiteral("lastSnapshotLoadDir")).toString();
    m_lastScreenshotDir = settings.value(QStringLiteral("lastScreenshotDir")).toString();
    auto speed = settings.value(QStringLiteral("emulationSpeed"), 1).toInt(&ok);

    if (!ok) {
        speed = 100;
    }

    m_emulationSpeedSlider.setValue(speed);
    settings.endGroup();
}

void MainWindow::keyPressEvent(QKeyEvent * event)
{
    if (!event->isAutoRepeat() && ::Qt::Key::Key_Tab == event->key()) {
        m_spectrum.setExecutionSpeedConstrained(false);
        updateStatusBarSpeedWidget();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent * event)
{
    if (::Qt::Key::Key_Tab == event->key()) {
        m_spectrum.setExecutionSpeedConstrained(0 < m_emulationSpeedSlider.value());
        updateStatusBarSpeedWidget();
        return;
    }

    QMainWindow::keyReleaseEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent * event)
{
    if (event->mimeData()->hasUrls() && !event->mimeData()->urls().isEmpty() && QStringLiteral("file") == event->mimeData()->urls().first().scheme()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent * event)
{
    if (event->mimeData()->hasUrls()) {
        auto url = event->mimeData()->urls().first();

        if (QStringLiteral("file") != url.scheme()) {
            std::cerr << "remote URLs cannot yet be loaded.\n";
            return;
        }

        loadSnapshot(url.path());
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
    ThreadPauser pauser(m_spectrumThread);

    if(s_filters.isEmpty()) {
        s_filters << tr("Spectrum Screenshot (SCR) files (*.scr)");
        s_filters << tr("Portable Network Graphics (PNG) files (*.png)");
        s_filters << tr("JPEG files (*.jpeg *.jpg)");
        s_filters << tr("Windows Bitmap (BMP) files (*.bmp)");
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save screenshot"), m_lastScreenshotDir, s_filters.join(";;"));

    if(fileName.isEmpty()) {
        return;
    }

    m_lastScreenshotDir = QFileInfo(fileName).path();
    saveScreenshot(fileName);
}

void MainWindow::loadSnapshotTriggered()
{
    static QStringList filters;
    static QString lastFilter;

    ThreadPauser pauser(m_spectrumThread);

    if(filters.isEmpty()) {
        filters << tr("All supported snapshot files (*.sna *.z80 *.sp *.zx)");
        filters << tr("SNA Snapshots (*.sna)");
        filters << tr("Z80 Snapshots (*.z80)");
        filters << tr("SP Snapshots (*.sp)");
        filters << tr("ZX Snapshots (*.zx)");
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Load snapshot"), m_lastSnapshotLoadDir, filters.join(";;"), &lastFilter);

    if(fileName.isEmpty()) {
        return;
    }

    m_lastSnapshotLoadDir = QFileInfo(fileName).path();
    auto format = lastFilter;

    if (!format.isEmpty()) {
        if (auto matches = QRegularExpression(R"(^.*\(\*\.([a-zA-Z0-9_-]+)\)$)").match(format); matches.hasMatch()) {
            format = matches.captured(1).toLower();
        } else {
            format.clear();
        }
    }

    loadSnapshot(fileName, format);
}

void MainWindow::saveSnapshotTriggered()
{
    static QStringList filters;
    static QString lastFilter;

    ThreadPauser pauser(m_spectrumThread);

    if(filters.isEmpty()) {
        filters << tr("SNA Snapshots (*.sna)");
        filters << tr("Z80 Snapshots (*.z80)");
        filters << tr("SP Snapshots (*.sp)");
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save snapshot"), m_lastSnapshotLoadDir, filters.join(";;"), &lastFilter);

    if(fileName.isEmpty()) {
        return;
    }

    m_lastSnapshotLoadDir = QFileInfo(fileName).path();
    auto format = lastFilter;

    if (!format.isEmpty()) {
        if (auto matches = QRegularExpression(R"(^.*\(\*\.([a-zA-Z0-9_-]+)\)$)").match(format); matches.hasMatch()) {
            format = matches.captured(1).toLower();
        } else {
            format.clear();
        }
    }

    // we appear to be generating non-functional .z80 snapshots - is the Z80 still running while the snapshot is being
    // taken?
    saveSnapshot(fileName, format);
}

void MainWindow::emulationSpeedChanged(int speed)
{
    if (0 == speed) {
        m_spectrum.setExecutionSpeedConstrained(false);
    } else {
        m_spectrum.setExecutionSpeedConstrained(true);
        m_spectrum.setExecutionSpeed(speed);
//        m_spectrum.z80()->setClockSpeed(static_cast<int>(Spectrum::DefaultClockSpeed * (speed / 100.0)));
    }

    updateStatusBarSpeedWidget();
}

void MainWindow::updateStatusBarSpeedWidget()
{
    if (m_spectrum.executionSpeedConstrained()) {
        m_statusBarEmulationSpeed.setText(tr("%1%").arg(m_spectrum.executionSpeedPercent()));
    } else {
        m_statusBarEmulationSpeed.setText(tr("%1%").arg("∞"));    // infinity
    }

    auto mhz = m_spectrum.z80()->clockSpeedMHz();
    int precision = 2;
    auto tmpMhz = mhz;

    while (tmpMhz > 10) {
        ++precision;
        tmpMhz /= 10;
    }

    m_statusBarMHz.setText(tr("%1 MHz").arg(mhz, 0, 'g', precision));
}

void MainWindow::debugTriggered()
{
    if (m_debug.isChecked()) {
        m_spectrumThread.setDebugMode(true);
        m_debugWindow.show();
        m_debugWindow.activateWindow();
        m_debugWindow.raise();
    }
    else {
        m_spectrumThread.setDebugMode(false);
        m_debugWindow.close();
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
    m_statusBarPause.setText(tr("Paused"));

    m_debugStep.setEnabled(true);
    connect(&m_spectrumThread, &Thread::stepped, this, &MainWindow::threadStepped, ::Qt::UniqueConnection);
}

void MainWindow::threadResumed()
{
    m_displayRefreshTimer.start();
    m_pauseResume.setIcon(QIcon::fromTheme(QStringLiteral("media-playback-pause")));
    m_pauseResume.setText(tr("Pause"));
    m_statusBarPause.setText(tr("Running"));

    m_debugStep.setEnabled(false);
    disconnect(&m_spectrumThread, &Thread::stepped, this, &MainWindow::threadStepped);
}

void MainWindow::threadStepped()
{
    refreshSpectrumDisplay();
}
