#include <cstring>
#include <filesystem>
#include <iostream>
#include <fstream>

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
#include "threadpauser.h"

using namespace Spectrum::Qt;

/**
 * TODO refactor the rough snapshot read methods into helper classes, and add buffer overflow checking
 */

MainWindow::MainWindow(QWidget * parent)
: QMainWindow(parent),
  m_spectrum(),
  m_spectrumThread(m_spectrum),
  m_display(),
  m_displayWidget(),
  m_load(QIcon::fromTheme(QStringLiteral("document-open")), tr("Load Snapshot")),
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
    m_spectrum.setKeyboard(&m_keyboard);
    installEventFilter(&m_keyboard);
    m_debugWindow.installEventFilter(this);
    m_load.setShortcut(::Qt::Key::Key_Control + ::Qt::Key::Key_O);
    m_pauseResume.setShortcuts({::Qt::Key::Key_Pause, ::Qt::Key::Key_Control + ::Qt::Key_P,});
    m_debug.setShortcut(::Qt::Key::Key_Control + ::Qt::Key::Key_D);
    m_debug.setCheckable(true);
    m_debugStep.setShortcut(::Qt::Key::Key_Space);
    m_saveScreenshot.setShortcut(::Qt::Key::Key_Print);
    m_refreshScreen.setShortcut(::Qt::Key::Key_F5);

    m_emulationSpeedSlider.setMinimum(0);
    m_emulationSpeedSlider.setMaximum(10);
    m_emulationSpeedSlider.setValue(1);

    m_emulationSpeedSpin.setMinimum(0);
    m_emulationSpeedSpin.setMaximum(10);
    m_emulationSpeedSpin.setValue(1);
    m_emulationSpeedSpin.setSuffix(QStringLiteral("x"));
    m_emulationSpeedSpin.setSpecialValueText(tr("Unlimited"));

    // update screen at 100 FPS
    m_displayRefreshTimer.setTimerType(::Qt::TimerType::PreciseTimer);
    m_displayRefreshTimer.setInterval(10);
    connect(&m_displayRefreshTimer, &QTimer::timeout, this, &MainWindow::refreshSpectrumDisplay);

    createToolbars();
	m_spectrum.addDisplayDevice(&m_display);
	setCentralWidget(&m_displayWidget);

	connect(&m_spectrumThread, &Thread::paused, this, &MainWindow::threadPaused);
	connect(&m_spectrumThread, &Thread::resumed, this, &MainWindow::threadResumed);

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
    tempToolBar->addAction(&m_saveScreenshot);
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
	connect(&m_reset, &QAction::triggered, &m_spectrumThread, &Thread::reset);
	connect(&m_debug, &QAction::triggered, this, &MainWindow::debugTriggered);
	connect(&m_debugStep, &QAction::triggered, this, &MainWindow::stepTriggered);
	connect(&m_saveScreenshot, &QAction::triggered, this, &MainWindow::saveScreenshotTriggered);
	connect(&m_refreshScreen, &QAction::triggered, this, &MainWindow::refreshSpectrumDisplay);
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
        speed = 1;
    }

    m_emulationSpeedSlider.setValue(speed);
    settings.endGroup();
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

void MainWindow::loadSnaSnapshot(const QString & fileName)
{
    ThreadPauser pauser(m_spectrumThread);

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

    for (auto * reg : {&registers.i, &registers.lShadow, &registers.hShadow, &registers.eShadow, &registers.dShadow, &registers.cShadow, &registers.bShadow, &registers.fShadow, &registers.aShadow, &registers.l, &registers.h, &registers.e, &registers.d, &registers.c, &registers.b, &registers.iyl, &registers.iyh, &registers.ixl, &registers.ixh}) {
        *reg = *(byte++);
    }

    bool iff = *(byte++) & 0x04;
    cpu.setIff1(iff);
    cpu.setIff2(iff);

    for (auto * reg : {&registers.r, &registers.f, &registers.a, &registers.spL, &registers.spH}) {
        *reg = *(byte++);
    }

    cpu.setInterruptMode(static_cast<Z80::Z80::InterruptMode>(*(byte++) & 0x03));
    m_display.setBorder(static_cast<DisplayDevice::Colour>(*(byte++) & 0x07));
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

void MainWindow::loadZ80Snapshot(const QString & fileName)
{
    ThreadPauser pauser(m_spectrumThread);
    std::error_code err;
    auto fileSize = std::filesystem::file_size(fileName.toStdString(), err);

    if (err) {
        std::cerr << "failed to determine file size for .z80 file '" << fileName.toStdString() << "'\n";
        return;
    }

    char buffer[49186];
    {
        std::ifstream in(fileName.toStdString());

        if (!in.is_open()) {
            // TODO display "not found" message
            std::cout << "Could not open file '" << fileName.toStdString() << "'\n";
            return;
        }

        while (!in.fail() && !in.eof() && in.tellg() < fileSize) {
            in.read(buffer + in.tellg(), fileSize - in.tellg());
        }

        if (in.fail() && !in.eof()) {
            // TODO display "read error" message
            std::cout << "Error reading file '" << fileName.toStdString() << "'\n";
            return;
        }
    }

    auto * ram = m_spectrum.memory() + 16384;
    auto & cpu = *(m_spectrum.z80());
    auto & registers = cpu.registers();
    auto * byte = buffer;

    // see https://worldofspectrum.org/faq/reference/z80format.htm

    // NOTE the enumerated values are the lengths, in bytes, of the additional header
    enum class Format : uint8_t
    {
        Version1 = 0,
        Version2 = 23,
        Version3 = 54,
        Version3WithPort1ffdOut = 55,
    };

    enum class V1MachineType : uint8_t
    {
        Spectrum48k = 0,
    };

    enum class V2MachineType : uint8_t
    {
        Spectrum48k = 0,
        Spectrum48kInterface1,
        SamRam,
        Spectrum128k,
        Spectrum128kInterface1,
        SpectrumPlus3 = 7,
        SpectrumPlus3Mistaken,
        Pentagon128k,
        Scorpion256k,
        DidaktikKompakt,
        SpectrumPlus2,
        SpectrumPlus2A,
        TC2048,
        TC2068,
        TS2068,
    };

    enum class V3MachineType : uint8_t
    {
        Spectrum48k = 0,
        Spectrum48kInterface1,
        SamRam,
        Spectrum48kMgt,
        Spectrum128k,
        Spectrum128kInterface1,
        Spectrum128kMgt,
        SpectrumPlus3,
        SpectrumPlus3Mistaken,
        Pentagon128k,
        Scorpion256k,
        DidaktikKompakt,
        SpectrumPlus2,
        SpectrumPlus2A,
        TC2048,
        TC2068,
        TS2068 = 128,
    };

    // determine file format at machine type
    // bytes 6 and 7 will both be 0 if it's v2 or v3 file (with extended header); anything else is v1 file (no extended
    // header)
    Format format;
    std::uint8_t machine = 0;

    if (0 == (buffer[6] | buffer[7])) {
        format = static_cast<Format>(buffer[30] | (static_cast<std::uint16_t>(buffer[31]) << 8));
        machine = buffer[34];

        switch (format) {
            case Format::Version2:
                if (machine != static_cast<std::uint8_t>(V2MachineType::Spectrum48k) &&
                    machine != static_cast<std::uint8_t>(V2MachineType::Spectrum48kInterface1)) {
                    std::cerr << "The .z80 file '" << fileName.toStdString()
                              << "' is for a Spectrum model not supported by this emulator.\n";
                    return;
                }
                break;

            case Format::Version3:
            case Format::Version3WithPort1ffdOut:
                if (machine != static_cast<std::uint8_t>(V3MachineType::Spectrum48k) &&
                    machine != static_cast<std::uint8_t>(V3MachineType::Spectrum48kInterface1) &&
                    machine != static_cast<std::uint8_t>(V3MachineType::Spectrum48kMgt)) {
                    std::cerr << "The .z80 file '" << fileName.toStdString()
                              << "' is for a Spectrum model not supported by this emulator.\n";
                    return;
                }
                break;

            default:
                std::cerr << "The version (" << static_cast<uint32_t>(format) << ") of the .z80 file '"
                          << fileName.toStdString() << "' is not recognised.\n";
                return;
        }
    } else {
        format = Format::Version1;
    }

    for (auto * reg : {&registers.a, &registers.f, &registers.c, &registers.b, &registers.l, &registers.h,
                       &registers.pcL, &registers.pcH, &registers.spL, &registers.spH, &registers.i, &registers.r,}) {
        *reg = *(byte++);
    }

    std::uint8_t fileFormatFlags = *(byte++);

    // flag byte contains bit 7 of R register in bit 0
    registers.r |= (fileFormatFlags & 0x01) << 7;
    m_display.setBorder(static_cast<DisplayDevice::Colour>((fileFormatFlags & 0b00001110) >> 1));

    for (auto & reg : {&registers.e, &registers.d, &registers.cShadow, &registers.bShadow, &registers.eShadow,
                       &registers.dShadow, &registers.lShadow, &registers.hShadow, &registers.aShadow,
                       &registers.fShadow, &registers.iyl, &registers.iyh, &registers.ixl, &registers.ixh,}) {
        *reg = *(byte++);
    }

    cpu.setIff1(*(byte++));
    cpu.setIff2(*(byte++));

    const auto featureFlags = *(byte++);

    // bits 0 and 1 contain the interrupt mode
    //
    // for the meaning of other bits, see https://worldofspectrum.org/faq/reference/z80format.htm
    cpu.setInterruptMode(static_cast<Z80::Z80::InterruptMode>(featureFlags & 0x03));

    if (Format::Version1 != format) {
        std::cerr << "skipping extended header length word, we already know it\n";
        byte += 2;
        std::cerr << "reading PC for version2/3 format file\n";
        std::cerr << "PC from V1 header was " << registers.pc;
        registers.pcL = *(byte++);
        registers.pcH = *(byte++);
        std::cerr << "; now " << registers.pc << "\n";

        // none of the remainder of the extra header is relevant to us at present. for the meaning of the fields see
        // https://worldofspectrum.org/faq/reference/z80format.htm
        std::cerr << "skipping remaining " << (static_cast<std::uint16_t>(format) - 2) << " bytes of extended header\n";
        byte += static_cast<std::uint16_t>(format) - 2;
    }

    // now we have the RAM image

    // bit 5 of the format flag indicates whether or not the RAM image is compressed
    if (Format::Version1 == format) {
        if (fileFormatFlags & 0b00100000) {
            // locate the end marker
            auto * end = byte;
            unsigned char endMarker[4] = {0x00, 0xed, 0xed, 0x00};
            auto * bufferMax = buffer + fileSize - 4;

            while (end <= bufferMax &&
                   *reinterpret_cast<std::uint32_t *>(end) != *reinterpret_cast<std::uint32_t *>(endMarker)) {
                ++end;
            }

            if (end > bufferMax) {
                std::cerr << "failed to find end of memory image marker in file '" << fileName.toStdString() << "'\n";
                return;
            }

            auto * ramByte = ram;
            const std::uint16_t rleMarker = 0xeded;

            while (byte < end) {
                if (rleMarker == *(reinterpret_cast<std::uint16_t *>(byte))) {
                    byte += 2;
                    auto count = *reinterpret_cast<std::uint8_t *>(byte++);
                    auto byteValue = *(byte++);
                    std::memset(ramByte, byteValue, count);
                    ramByte += count;
                } else {
                    *(ramByte++) = *(byte++);
                }
            }
        } else {
            std::memcpy(ram, byte, buffer + fileSize - byte);
        }
    } else {
        while (byte < (buffer + fileSize)) {
            auto dataSize = Z80::Z80::z80ToHostByteOrder(*reinterpret_cast<std::uint16_t *>(byte));
            byte += 2;
            auto page = static_cast<std::uint8_t>(*(byte++));
            bool readPage = true;
            decltype(ram) ramByte;

            // NOTE: ram points to the first byte of addressable RAM (0x0000 - 0x3fff is ROM), whereas the .z80 defines
            // the pages as offsets from 0x0000, so we need to adjust accordingly
            switch (page) {
                case 4:
                    ramByte = ram + 0x8000 - 0x4000;
                    break;

                case 5:
                    ramByte = ram + 0xc000 - 0x4000;
                    break;

                case 8:
                    ramByte = ram;
                    break;

                default:
                    std::cerr << "Ignored RAM page " << static_cast<std::uint16_t>(page) << " - not relevant to Spectrum48K\n";
                    byte += dataSize;
                    readPage = false;
                    break;
            }

            if (readPage) {
                std::cout << "reading page #" << static_cast<std::uint16_t >(page) << " of " << dataSize << " bytes\n";
                auto * end = byte + dataSize;
                const std::uint16_t rleMarker = 0xeded;

                while (byte < end) {
                    if (rleMarker == *(reinterpret_cast<std::uint16_t *>(byte))) {
                        byte += 2;
                        auto count = *reinterpret_cast<std::uint8_t *>(byte++);
                        auto byteValue = *(byte++);
                        std::memset(ramByte, byteValue, count);
                        ramByte += count;
                    } else {
                        *(ramByte++) = *(byte++);
                    }
                }
            }
        }
    }

    // update the display
    m_display.redrawDisplay(ram);
    m_displayWidget.setImage(m_display.image());
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
        s_filters << tr("JPEG files (*.jpeg|*.jpg)");
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
        filters << tr("Snapshot (SNA) files (*.sna)");
        filters << tr("Z80 Snapshot (Z80) files (*.z80)");
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Load snapshot"), m_lastSnapshotLoadDir, filters.join(";;"), &lastFilter);

    if(fileName.isEmpty()) {
        return;
    }

    m_lastSnapshotLoadDir = QFileInfo(fileName).path();
    auto format = lastFilter;

    if (!format.isEmpty()) {
        if (auto matches = QRegularExpression("^.*\\(\\*\\.([a-zA-Z0-9_-]+)\\)$").match(format); matches.hasMatch()) {
            format = matches.captured(1).toLower();
        } else {
            format.clear();
        }
    }

    if (format.isEmpty()) {
        if (auto matches = QRegularExpression("^.*\\.([a-zA-Z0-9_-]+)$").match(fileName); matches.hasMatch()) {
            format = matches.captured(1).toLower();
        }
    }

    if ("sna" == format) {
        loadSnaSnapshot(fileName);
    } else if ("z80" == format) {
        loadZ80Snapshot(fileName);
    } else {
        std::cerr << "unrecognised format '" << format.toStdString() << "' from filename '" << fileName.toStdString() << "'\n";
    }
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
    connect(&m_spectrumThread, &Thread::stepped, this, &MainWindow::threadStepped, ::Qt::UniqueConnection);
}

void MainWindow::threadResumed()
{
    m_displayRefreshTimer.start();
    m_pauseResume.setIcon(QIcon::fromTheme(QStringLiteral("media-playback-pause")));
    m_pauseResume.setText(tr("Pause"));

    // TODO disable widgets
    m_debugStep.setEnabled(false);
    disconnect(&m_spectrumThread, &Thread::stepped, this, &MainWindow::threadStepped);
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
