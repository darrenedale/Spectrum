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
#include <QKeyEvent>
#include <QMimeData>
#include <QDebug>
#include <QSettings>
#include <QDateTime>
#include <QPainter>

#include "mainwindow.h"
#include "threadpauser.h"
#include "../snapshot.h"
#include "../z80snapshotwriter.h"
#include "../snasnapshotwriter.h"

using namespace Spectrum::Qt;

/**
 * TODO refactor the rough snapshot read methods into helper classes, and add buffer overflow checking
 */

namespace
{
    auto z80ToHostByteOrder = ::Z80::Z80::z80ToHostByteOrder;
    auto hostToZ80ByteOrder = ::Z80::Z80::hostToZ80ByteOrder;
};

MainWindow::MainWindow(QWidget * parent)
: QMainWindow(parent),
  m_spectrum(),
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

    // update screen at 100 FPS
    m_displayRefreshTimer.setTimerType(::Qt::TimerType::PreciseTimer);
    m_displayRefreshTimer.setInterval(10);
    connect(&m_displayRefreshTimer, &QTimer::timeout, this, &MainWindow::refreshSpectrumDisplay);

    createToolbars();
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
}

MainWindow::~MainWindow()
{
    m_spectrum.setKeyboard(nullptr);
    m_spectrum.removeDisplayDevice(&m_display);
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
        painter.drawText(2, y += 10, QStringLiteral("HL: $%1").arg(registers.hl, 4, 16, fill));
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
    if (format.isEmpty()) {
        format = guessSnapshotFormat(fileName);
    }

    if ("sna" == format) {
        loadSnaSnapshot(fileName);
    } else if ("z80" == format) {
        loadZ80Snapshot(fileName);
    } else if ("sp" == format) {
        loadSpSnapshot(fileName);
    } else {
        std::cerr << "unrecognised format '" << format.toStdString() << "' from filename '" << fileName.toStdString() << "'\n";
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
    m_display.setBorder(static_cast<Colour>(*(byte++) & 0x07));
    std::memcpy(ram, byte, 0xc000);

    // update the display
    Z80::UnsignedByte retn[2] = {0xed, 0x45};
    m_display.redrawDisplay(ram);
    m_displayWidget.setImage(m_display.image());

    // RETN instruction is required to resume execution of the .SNA
    cpu.execute(retn);
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

    auto * ram = m_spectrum.memory() + 0x4000;
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
    m_display.setBorder(static_cast<Colour>((fileFormatFlags & 0b00001110) >> 1));

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
    std::ifstream::char_type * programBuffer = nullptr;

    {
        std::ifstream in(fileName.toStdString());

        if (!in) {
            // TODO display "not found" message
            std::cerr << "Could not open file '" << fileName.toStdString() << "'\n";
            return;
        }

        // read header
        in.read(reinterpret_cast<std::ifstream::char_type *>(&header), sizeof(header));

        if (in.fail()) {
            std::cerr << "Error reading header from '" << fileName.toStdString() << "'\n";
            return;
        }

        // validate header
        if (*reinterpret_cast<const std::uint16_t *>("SP") != *reinterpret_cast<const std::uint16_t *>(header.signature)) {
            std::cerr << "Not an SP file.";
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
            return;
        }

        header.border = header.border & 0x07;

        programBuffer = new std::ifstream::char_type[header.length];
        in.read(programBuffer, header.length);

        if (in.fail()) {
            std::cerr << "Error reading program from '" << fileName.toStdString() << "'\n";
            delete[] programBuffer;
            return;
        }

        if (!in.eof()) {
            std::cerr << "Warning: ignored extraneous content at end of file (from byte " << in.tellg() << " onward.\n";
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
    cpu.setInterruptMode(header.status & 0x0002 ? ::Z80::Z80::InterruptMode::IM2 : ::Z80::Z80::InterruptMode::IM1);

    // bit 4 = interrupt pending
    if (header.status & 0x0010) {
        cpu.interrupt();
    }

    // NOTE bit 5 of status indicates flash state but we don't use this

    std::memcpy(memory + header.baseAddress, programBuffer, header.length);

    delete[] programBuffer;
}

void MainWindow::saveSnapshot(const QString & fileName, QString format)
{
    ThreadPauser pauser(m_spectrumThread);

    if (format.isEmpty()) {
        format = guessSnapshotFormat(fileName);
    }

    if ("sna" == format) {
        auto * z80 = m_spectrum.z80();
        z80->execute(reinterpret_cast<const Z80::UnsignedByte *>("\xcd\x00\x00"), false);
        SnaSnapshotWriter writer(Snapshot{m_spectrum});
        z80->execute(reinterpret_cast<const Z80::UnsignedByte *>("\xed\x45"), false);

        if (!writer.writeTo(fileName.toStdString())) {
            std::cerr << "failed to write snapshot to '" << fileName.toStdString() << "'\n";
        }
    } else if ("z80" == format) {
        Z80SnapshotWriter writer(Snapshot{m_spectrum});

        if (!writer.writeTo(fileName.toStdString())) {
            std::cerr << "failed to write snapshot to '" << fileName.toStdString() << "'\n";
        }
    } else if ("sp" == format) {
        std::cerr << "Not yet implemented.\n";
    } else {
        std::cerr << "unrecognised format '" << format.toStdString() << "' from filename '" << fileName.toStdString() << "'\n";
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
                    return true;
                }
                break;


            case QEvent::Type::KeyRelease:
                if (::Qt::Key::Key_Tab == dynamic_cast<QKeyEvent *>(event)->key()) {
                    m_spectrum.setExecutionSpeedConstrained(0 < m_emulationSpeedSlider.value());
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
        m_spectrumThread.spectrum().setExecutionSpeedConstrained(false);
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent * event)
{
    if (::Qt::Key::Key_Tab == event->key()) {
        m_spectrumThread.spectrum().setExecutionSpeedConstrained(0 < m_emulationSpeedSlider.value());
        return;
    }

    QMainWindow::keyReleaseEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent * event)
{
    if (event->mimeData()->hasUrls() && QStringLiteral("file") == event->mimeData()->urls().first().scheme()) {
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
        filters << tr("All supported snapshot files (*.sna *.z80 *.sp)");
        filters << tr("SNA Snapshots (*.sna)");
        filters << tr("Z80 Snapshots (*.z80)");
        filters << tr("SP Snapshots (*.sp)");
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
        if (auto matches = QRegularExpression("^.*\\(\\*\\.([a-zA-Z0-9_-]+)\\)$").match(format); matches.hasMatch()) {
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
        m_spectrum.z80()->setClockSpeed(static_cast<int>(Spectrum::DefaultClockSpeed * (speed / 100.0)));
    }
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
