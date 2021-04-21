#include <filesystem>
#include <iostream>
#include <fstream>

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QDebug>
#include <QSettings>
#include <QDateTime>
#include <QPainter>
#include <QStandardPaths>
#include <QStringBuilder>
#include <memory>

#include "application.h"
#include "mainwindow.h"
#include "threadpauser.h"
#include "../spectrum16k.h"
#include "../spectrum48k.h"
#include "../spectrum128k.h"
#include "../spectrumplus2.h"
#include "../spectrumplus2a.h"
#include "../spectrumplus3.h"
#include "../kempstonjoystick.h"
#include "../interfacetwojoystick.h"
#include "../cursorjoystick.h"
#include "../fullerjoystick.h"
#include "../kempstonmouse.h"
#include "../snapshot.h"
#include "../io/snasnapshotreader.h"
#include "../io/z80snapshotreader.h"
#include "../io/spsnapshotreader.h"
#include "../io/zx82snapshotreader.h"
#include "../io/zxsnapshotreader.h"
#include "../io/z80snapshotwriter.h"
#include "../io/snasnapshotwriter.h"
#include "../io/spsnapshotwriter.h"
#include "../io/pokfilereader.h"
#include "../../util/debug.h"

using namespace Spectrum::QtUi;
using namespace Spectrum::Io;
using ::Z80::InterruptMode;
using ::Spectrum::Keyboard;

namespace fs = std::filesystem;

namespace
{
    constexpr const int DisplayRefreshRate = 50;   // how many times a second to redraw the screen
    constexpr const int DefaultStatusBarMessageTimeout = 5000;

    enum class JoystickMapping : std::uint8_t
    {
        None,
        Up,
        Down,
        Left,
        Right,
        Button1,
        Button2,
        Button3,
    };

    constexpr const char * Default16kRom = "roms/spectrum48.rom";
    constexpr const char * Default48kRom = "roms/spectrum48.rom";
    constexpr const char * Default128kRom0 = "roms/spectrum128-0.rom";
    constexpr const char * Default128kRom1 = "roms/spectrum128-1.rom";
    constexpr const char * DefaultPlus2Rom0 = "roms/spectrumplus2-0.rom";
    constexpr const char * DefaultPlus2Rom1 = "roms/spectrumplus2-1.rom";
    constexpr const char * DefaultPlus2aRom0 = "roms/spectrumplus3-0.rom";
    constexpr const char * DefaultPlus2aRom1 = "roms/spectrumplus3-1.rom";
    constexpr const char * DefaultPlus2aRom2 = "roms/spectrumplus3-2.rom";
    constexpr const char * DefaultPlus2aRom3 = "roms/spectrumplus3-3.rom";
    constexpr const char * DefaultPlus3Rom0 = "roms/spectrumplus3-0.rom";
    constexpr const char * DefaultPlus3Rom1 = "roms/spectrumplus3-1.rom";
    constexpr const char * DefaultPlus3Rom2 = "roms/spectrumplus3-2.rom";
    constexpr const char * DefaultPlus3Rom3 = "roms/spectrumplus3-3.rom";
    constexpr const char * DefaultTc2048Rom = "roms/tc2048.rom";

    // ms to wait for the thread to stop before forcibly terminating it
    constexpr const int ThreadStopWaitThreshold = 3000;

    std::vector<::Spectrum::Keyboard::Key> mapToSpectrumKeys(Qt::Key key)
    {
        // TODO configurable mapping
        switch (key) {
            case Qt::Key::Key_Backspace:
                return {::Spectrum::Keyboard::Key::CapsShift, Keyboard::Key::Num0};

            case Qt::Key::Key_Colon:
                return {Keyboard::Key::SymbolShift, Keyboard::Key::Z};

            case Qt::Key::Key_Semicolon:
                return {Keyboard::Key::SymbolShift, Keyboard::Key::O};

            case Qt::Key::Key_Apostrophe:
                return {Keyboard::Key::SymbolShift, Keyboard::Key::Num7};

            case Qt::Key::Key_Period:
                return {Keyboard::Key::SymbolShift, Keyboard::Key::M};

            case Qt::Key::Key_Comma:
                return {Keyboard::Key::SymbolShift, Keyboard::Key::N};

            case Qt::Key::Key_Underscore:
                return {Keyboard::Key::SymbolShift, Keyboard::Key::Num0};

            case Qt::Key::Key_Minus:
                return {Keyboard::Key::SymbolShift, Keyboard::Key::J};

            case Qt::Key::Key_Plus:
                return {Keyboard::Key::SymbolShift, Keyboard::Key::K};

            case Qt::Key::Key_Equal:
                return {Keyboard::Key::SymbolShift, Keyboard::Key::L};

            case Qt::Key::Key_Greater:
                return {Keyboard::Key::SymbolShift, Keyboard::Key::T};

            case Qt::Key::Key_Less:
                return {Keyboard::Key::SymbolShift, Keyboard::Key::R};

            case Qt::Key::Key_Slash:
                return {Keyboard::Key::SymbolShift, Keyboard::Key::V};

            case Qt::Key::Key_Question:
                return {Keyboard::Key::SymbolShift, Keyboard::Key::C};

            case Qt::Key::Key_CapsLock:
                return {Keyboard::Key::CapsShift, Keyboard::Key::Num2};

            case Qt::Key::Key_Left:
                return {Keyboard::Key::CapsShift, Keyboard::Key::Num5};

            case Qt::Key::Key_Right:
                return {Keyboard::Key::CapsShift, Keyboard::Key::Num8};

            case Qt::Key::Key_Up:
                return {Keyboard::Key::CapsShift, Keyboard::Key::Num7};

            case Qt::Key::Key_Down:
                return {Keyboard::Key::CapsShift, Keyboard::Key::Num6};

            case Qt::Key::Key_Shift:
                return {Keyboard::Key::CapsShift};

            case Qt::Key::Key_Alt:
            case Qt::Key::Key_AltGr:
                return {Keyboard::Key::SymbolShift};

            case Qt::Key::Key_Enter:
            case Qt::Key::Key_Return:
                return {Keyboard::Key::Enter};

            case Qt::Key::Key_Space:
                return {Keyboard::Key::Space};

                // TODO the shifted number keys assume a UK or US layout keyboard
            case Qt::Key::Key_1:
            case Qt::Key::Key_Exclam:
                return {Keyboard::Key::Num1};

            case Qt::Key::Key_2:
            case Qt::Key::Key_At:
            case Qt::Key::Key_QuoteDbl:
                return {Keyboard::Key::Num2};

            case Qt::Key::Key_3:
            case Qt::Key::Key_sterling:
            case Qt::Key::Key_NumberSign:
                return {Keyboard::Key::Num3};

            case Qt::Key::Key_4:
            case Qt::Key::Key_Dollar:
                return {Keyboard::Key::Num4};

            case Qt::Key::Key_5:
            case Qt::Key::Key_Percent:
                return {Keyboard::Key::Num5};

            case Qt::Key::Key_6:
            case Qt::Key::Key_AsciiCircum:
                return {Keyboard::Key::Num6};

            case Qt::Key::Key_7:
            case Qt::Key::Key_Ampersand:
                return {Keyboard::Key::Num7};

            case Qt::Key::Key_8:
            case Qt::Key::Key_Asterisk:
                return {Keyboard::Key::Num8};

            case Qt::Key::Key_9:
            case Qt::Key::Key_ParenLeft:
                return {Keyboard::Key::Num9};

            case Qt::Key::Key_0:
            case Qt::Key::Key_ParenRight:
                return {Keyboard::Key::Num0};

            case Qt::Key::Key_Q:
                return {Keyboard::Key::Q};

            case Qt::Key::Key_W:
                return {Keyboard::Key::W};

            case Qt::Key::Key_E:
                return {Keyboard::Key::E};

            case Qt::Key::Key_R:
                return {Keyboard::Key::R};

            case Qt::Key::Key_T:
                return {Keyboard::Key::T};

            case Qt::Key::Key_Y:
                return {Keyboard::Key::Y};

            case Qt::Key::Key_U:
                return {Keyboard::Key::U};

            case Qt::Key::Key_I:
                return {Keyboard::Key::I};

            case Qt::Key::Key_O:
                return {Keyboard::Key::O};

            case Qt::Key::Key_P:
                return {Keyboard::Key::P};

            case Qt::Key::Key_A:
                return {Keyboard::Key::A};

            case Qt::Key::Key_S:
                return {Keyboard::Key::S};

            case Qt::Key::Key_D:
                return {Keyboard::Key::D};

            case Qt::Key::Key_F:
                return {Keyboard::Key::F};

            case Qt::Key::Key_G:
                return {Keyboard::Key::G};

            case Qt::Key::Key_H:
                return {Keyboard::Key::H};

            case Qt::Key::Key_J:
                return {Keyboard::Key::J};

            case Qt::Key::Key_K:
                return {Keyboard::Key::K};

            case Qt::Key::Key_L:
                return {Keyboard::Key::L};

            case Qt::Key::Key_Z:
                return {Keyboard::Key::Z};

            case Qt::Key::Key_X:
                return {Keyboard::Key::X};

            case Qt::Key::Key_C:
                return {Keyboard::Key::C};

            case Qt::Key::Key_V:
                return {Keyboard::Key::V};

            case Qt::Key::Key_B:
                return {Keyboard::Key::B};

            case Qt::Key::Key_N:
                return {Keyboard::Key::N};

            case Qt::Key::Key_M:
                return {Keyboard::Key::M};

            default:
                return {};
        }
    }

    JoystickMapping mapToSpectrumJoystick(Qt::Key key)
    {
        switch (key) {
            case Qt::Key::Key_Up:
                return JoystickMapping::Up;

            case Qt::Key::Key_Down:
                return JoystickMapping::Down;

            case Qt::Key::Key_Left:
                return JoystickMapping::Left;

            case Qt::Key::Key_Right:
                return JoystickMapping::Right;

            case Qt::Key::Key_Control:
                return JoystickMapping::Button1;
        }

        return JoystickMapping::None;
    }
}

/**
 * @param parent
 */
MainWindow::MainWindow(QWidget * parent)
: QMainWindow(parent),
  m_spectrum(std::make_unique<Spectrum128k>(Default128kRom0, Default128kRom1)),
  m_spectrumThread(*m_spectrum),
  m_display(),
  m_displayWidget(),
  m_pokesWidget(),
  m_load(QIcon::fromTheme(QStringLiteral("document-open")), tr("Load snapshot")),
  m_save(QIcon::fromTheme(QStringLiteral("document-save")), tr("Save snapshot")),
  m_pauseResume(QIcon::fromTheme(QStringLiteral("media-playback-start")), tr("Start/Pause")),
  m_model16(tr("Spectrum 16k")),
  m_model48(tr("Spectrum 48k")),
  m_model128(tr("Spectrum 128k")),
  m_modelPlus2(tr("Spectrum +2")),
  m_modelPlus2a(tr("Spectrum +2a")),
  m_modelPlus3(tr("Spectrum +3")),
  m_saveScreenshot(QIcon::fromTheme(QStringLiteral("image")), tr("Screenshot")),
  m_colourDisplay(tr("Colour")),
  m_monochromeDisplay(tr("Monochrome")),
  m_bwDisplay(tr("Black and White")),
  m_joystickKempston(tr("Kempston")),
  m_joystickInterface2(tr("ZX Interface Two")),
  m_joystickCursor(tr("Cursor")),
  m_joystickFuller(tr("Fuller")),
  m_joystickNone(tr("None")),
  m_gameControllersMenu(tr("Game Controller")),
  m_gameControllersGroup(nullptr),
  m_kempstonMouse(tr("Kempston mouse")),
  m_reset(QIcon::fromTheme(QStringLiteral("start-over")), tr("Reset")),
  m_debug(tr("Debug")),
  m_debugStep(QIcon::fromTheme(QStringLiteral("debug-step-instruction")), tr("Step")),
  m_refreshScreen(QIcon::fromTheme("view-refresh"), tr("Refresh screen")),
  m_emulationSpeedSlider(Qt::Horizontal),
  m_emulationSpeedSpin(nullptr),
  m_debugWindow(&m_spectrumThread),
  m_displayRefreshTimer(nullptr),
  m_joystick(std::make_unique<Spectrum::KempstonJoystick>()),
  m_gameControllerHandler(m_joystick.get()),
  m_mouse(nullptr)
{
    setWindowTitle(QStringLiteral("Spectrum"));
    setMouseTracking(true);

    m_spectrum->setExecutionSpeedConstrained(true);

    m_debugWindow.setWindowFlag(Qt::WindowType::Window);
    m_displayWidget.keepAspectRatio();
    m_displayWidget.setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    m_displayWidget.setFocus();
    m_displayWidget.installEventFilter(this);

    // the pokes widget is going in a dock as it's intended to be supplementary functionality so scale its content down
    // in size so that it's not intrusive
    auto font = m_pokesWidget.font();
    font.setPointSizeF(font.pointSizeF() * 0.75);
    m_pokesWidget.setFont(font);
    auto iconSize = m_pokesWidget.actionIconSize();
    iconSize *= 0.66;
    m_pokesWidget.setActionIconSize(iconSize);

    m_debugWindow.installEventFilter(this);
    m_load.setShortcut(tr("Ctrl+O"));
    m_save.setShortcut(tr("Ctrl+S"));
    m_pauseResume.setShortcuts({Qt::Key::Key_Pause, Qt::Key::Key_Escape, tr("Ctrl+P"),});

    auto * model = new QActionGroup(this);

    for (auto * action : {&m_model16, &m_model48, &m_model128, &m_modelPlus2, &m_modelPlus2a, &m_modelPlus3, }) {
        action->setCheckable(true);
        action->setChecked(false);
        model->addAction(action);
    }

    m_model128.setChecked(true);

    m_colourDisplay.setCheckable(true);
    m_monochromeDisplay.setCheckable(true);
    m_bwDisplay.setCheckable(true);
    m_colourDisplay.setChecked(true);

    auto * displayColourMode = new QActionGroup(this);
    displayColourMode->addAction(&m_colourDisplay);
    displayColourMode->addAction(&m_monochromeDisplay);
    displayColourMode->addAction(&m_bwDisplay);

    auto * joystickInterface = new QActionGroup(this);

    for (auto * action : {&m_joystickKempston, &m_joystickInterface2, &m_joystickCursor, &m_joystickFuller, &m_joystickNone, }) {
        action->setCheckable(true);
        action->setChecked(false);
        joystickInterface->addAction(action);
    }

    m_joystickKempston.setChecked(true);
    m_joystickKempston.setToolTip(tr("Emulate a Kempston joystick interface."));
    m_joystickInterface2.setToolTip(tr("Emulate a ZX Interface 2 joystick interface."));
    m_joystickNone.setToolTip(tr("Don't emulate any joystick interface."));
    m_gameControllersMenu.setToolTip(tr("Choose which game controller to attach to the emulated Spectrum joystick interface."));

    rescanGameControllers();
    connect(QGamepadManager::instance(), &QGamepadManager::connectedGamepadsChanged, this, &MainWindow::rescanGameControllers);

    m_kempstonMouse.setCheckable(true);
    m_kempstonMouse.setChecked(false);

    m_debug.setShortcuts({tr("Ctrl+D"), Qt::Key::Key_F12,});
    m_debug.setCheckable(true);
    m_debugStep.setShortcut(Qt::Key::Key_Space);
    m_saveScreenshot.setShortcut(Qt::Key::Key_Print);
    m_refreshScreen.setShortcut(Qt::Key::Key_F5);

    m_emulationSpeedSlider.setRange(0, 1000);
    m_emulationSpeedSlider.setValue(100);
    m_emulationSpeedSlider.setSingleStep(10);

    m_emulationSpeedSpin.setRange(0, 1000);
    m_emulationSpeedSpin.setValue(100);
    m_emulationSpeedSpin.setSingleStep(10);
    m_emulationSpeedSpin.setSuffix(QStringLiteral("%"));
    m_emulationSpeedSpin.setSpecialValueText(tr("Unlimited"));

    m_statusBarEmulationSpeed.setToolTip(tr("Current emulation speed."));
    m_statusBarMHz.setToolTip(tr("Current speed of the emulated Z80."));

    // update screen at 100 FPS
    m_displayRefreshTimer.setTimerType(Qt::TimerType::PreciseTimer);
    m_displayRefreshTimer.setInterval(1000 / DisplayRefreshRate);
    connect(&m_displayRefreshTimer, &QTimer::timeout, this, &MainWindow::refreshSpectrumDisplay);

    createMenuBar();
    createToolBars();
    createDockWidgets();
    createStatusBar();

    m_spectrum->setJoystickInterface(m_joystick.get());
    m_spectrum->setKeyboard(&m_keyboard);
	m_spectrum->addDisplayDevice(&m_display);

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
    m_spectrum->setJoystickInterface(nullptr);
    m_spectrum->setKeyboard(nullptr);
    m_spectrum->removeDisplayDevice(&m_display);
    stopThread();
}

void MainWindow::stopThread()
{
    m_spectrumThread.stop();

#if (!defined(NDEBUG))
    Util::debug << "Stopping SpectrumThread @ " << static_cast<void *>(&m_spectrumThread) << ' ';

    if (!m_spectrumThread.wait(250)) {
        int waitFor = ThreadStopWaitThreshold;

        while (0 < waitFor && !m_spectrumThread.wait(100)) {
            Util::debug << '.';
            waitFor -= 100;
        }
    }

    Util::debug << '\n';
#else
    m_spectrumThread.wait(ThreadStopWaitThreshold);
#endif

    if (!m_spectrumThread.isFinished()) {
        Util::debug << "forcibly terminating SpectrumThread @" << std::hex
              << static_cast<void *>(&m_spectrumThread) << "\n";
        m_spectrumThread.terminate();
    }
}

void MainWindow::createMenuBar()
{
    auto * tempMenuBar = menuBar();
    auto * menu = tempMenuBar->addMenu(tr("File"));
    menu->addAction(&m_reset);
    menu->addSeparator();
    menu->addAction(&m_load);
    menu->addAction(&m_save);

    // F1-5 loads from slot 1-5
    // Shift + F1-5 saves to slot 1-5

    auto * subMenu = menu->addMenu(tr("Load slot"));
//    subMenu->addAction(tr("Default slot"), [this]() {
//        // TODO configuration to set default slot
//        loadSnapshotFromSlot(1);
//    }, Qt::Key::Key_F1);
//    subMenu->addSeparator();

    for (int slotIndex = 1; slotIndex <= 5; ++slotIndex) {
        subMenu->addAction(tr("Slot %1").arg(slotIndex),  [this, slotIndex]() {
            loadSnapshotFromSlot(slotIndex);
        }, QKeySequence(tr("F%1").arg(slotIndex)));
    }

    subMenu = menu->addMenu(tr("Save slot"));
//    subMenu->addAction(tr("Default slot"), [this]() {
//        // TODO configuration to set default slot
//        saveSnapshotToSlot(1, QStringLiteral("z80"));
//    }, Qt::Key::Key_F2);
//    subMenu->addSeparator();

    for (int slotIndex = 1; slotIndex <= 5; ++slotIndex) {
        subMenu->addAction(tr("Slot %1").arg(slotIndex), [this, slotIndex]() {
            saveSnapshotToSlot(slotIndex, QStringLiteral("z80"));
        }, QKeySequence(tr("Shift+F%1").arg(slotIndex)));
    }

    menu->addSeparator();
    menu->addAction(QIcon::fromTheme("application-exit"), tr("Quit"), [this]() {
        close();
    });

    menu = tempMenuBar->addMenu(tr("Spectrum"));
    menu->addAction(&m_pauseResume);
    menu->addAction(&m_reset);
    menu->addSeparator();
    subMenu = menu->addMenu(tr("Model"));
    subMenu->setToolTip(tr("Choose which Spectrum model to emulate."));

    for (auto * action : {&m_model16, &m_model48, &m_model128, &m_modelPlus2, &m_modelPlus2a, &m_modelPlus3, }) {
        subMenu->addAction(action);
    }

    subMenu = menu->addMenu(tr("Joystick Interface"));
    subMenu->setToolTip(tr("Choose which emulated joystick interface to attach to the Spectrum."));
    subMenu->addAction(&m_joystickKempston);
    subMenu->addAction(&m_joystickInterface2);
    subMenu->addAction(&m_joystickCursor);
    subMenu->addAction(&m_joystickFuller);
    subMenu->addAction(&m_joystickNone);

    menu->addMenu(&m_gameControllersMenu);

    menu->addAction(&m_kempstonMouse);
    menu->addSeparator();
    menu->addAction(m_pokesWidget.loadPokesAction());
    menu->addAction(m_pokesWidget.clearPokesAction());

    menu = tempMenuBar->addMenu(tr("Display"));
    menu->addAction(&m_saveScreenshot);
    menu->addSeparator();

    menu->addAction(&m_colourDisplay);
    menu->addAction(&m_monochromeDisplay);
    menu->addAction(&m_bwDisplay);

    menu = tempMenuBar->addMenu(tr("Debugger"));
    menu->addAction(&m_debug);
    menu->addAction(&m_debugStep);
    menu->addSeparator();
    menu->addAction(&m_refreshScreen);
}

void MainWindow::createToolBars()
{
	auto * tempToolBar = addToolBar(tr("Main"));
	tempToolBar->setObjectName(QStringLiteral("main-toolbar"));
    tempToolBar->addAction(&m_load);
    tempToolBar->addAction(&m_save);
    tempToolBar->addSeparator();
    tempToolBar->addAction(&m_pauseResume);
    tempToolBar->addAction(&m_reset);

    tempToolBar = addToolBar(tr("Debug"));
    tempToolBar->setObjectName(QStringLiteral("debug-toolbar"));
    tempToolBar->addAction(&m_debug);
    tempToolBar->addAction(&m_debugStep);
    tempToolBar->addAction(&m_refreshScreen);

    tempToolBar = addToolBar(tr("Speed"));
    tempToolBar->setObjectName(QStringLiteral("speed-toolbar"));
    auto geom = tempToolBar->geometry();
    geom.moveLeft(width() / 3 * 2);
    tempToolBar->setGeometry(geom);
    tempToolBar->addWidget(new QLabel(tr("Speed")));
    tempToolBar->addWidget(&m_emulationSpeedSlider);
    tempToolBar->addWidget(&m_emulationSpeedSpin);
}

void MainWindow::createDockWidgets()
{
    auto * dock = new QDockWidget(tr("Pokes"), this);
    dock->setObjectName(QStringLiteral("pokes-dock"));
    dock->setWidget(&m_pokesWidget);
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dock);
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

	connect(&m_model16, &QAction::triggered, this, &MainWindow::model16Triggered);
	connect(&m_model48, &QAction::triggered, this, &MainWindow::model48Triggered);
	connect(&m_model128, &QAction::triggered, this, &MainWindow::model128Triggered);
	connect(&m_modelPlus2, &QAction::triggered, this, &MainWindow::modelPlus2Triggered);
	connect(&m_modelPlus2a, &QAction::triggered, this, &MainWindow::modelPlus2aTriggered);
	connect(&m_modelPlus3, &QAction::triggered, this, &MainWindow::modelPlus3Triggered);

	connect(&m_joystickKempston, &QAction::triggered, this, &MainWindow::useKempstonJoystickTriggered);
	connect(&m_joystickInterface2, &QAction::triggered, this, &MainWindow::useInterfaceTwoJoystickTriggered);
	connect(&m_joystickCursor, &QAction::triggered, this, &MainWindow::useCursorJoystickTriggered);
	connect(&m_joystickFuller, &QAction::triggered, this, &MainWindow::useFullerJoystickTriggered);
	connect(&m_joystickNone, &QAction::triggered, this, &MainWindow::noJoystickTriggered);

	connect(&m_kempstonMouse, &QAction::triggered, this, &MainWindow::kempstonMouseToggled);

	connect(&m_debug, &QAction::triggered, this, &MainWindow::debugTriggered);
	connect(&m_debugStep, &QAction::triggered, this, &MainWindow::stepTriggered);
	connect(&m_saveScreenshot, &QAction::triggered, this, &MainWindow::saveScreenshotTriggered);
	connect(&m_refreshScreen, &QAction::triggered, this, &MainWindow::refreshSpectrumDisplay);

	connect(&m_colourDisplay, &QAction::toggled, [this](bool colour) {
	    if (colour) {
            m_display.setColour();
	    }
	});

	connect(&m_monochromeDisplay, &QAction::toggled, [this](bool mono) {
	    if (mono) {
            m_display.setMonochrome();
	    }
	});

	connect(&m_bwDisplay, &QAction::toggled, [this](bool bw) {
	    if (bw) {
            m_display.setBlackAndWhite();
	    }
	});

	connect(&m_pokesWidget, &PokesWidget::applyPokeRequested, [this](const PokeDefinition & poke) {
	    // TODO check if poke has any user-provided values
	    poke.apply(*m_spectrum);
	    statusBar()->showMessage(tr("%1 poke activated.").arg(QString::fromStdString(poke.name())));
	});

	connect(&m_pokesWidget, &PokesWidget::undoPokeRequested, [this](const PokeDefinition & poke) {
	    poke.undo(*m_spectrum);
        statusBar()->showMessage(tr("%1 poke deactivated.").arg(QString::fromStdString(poke.name())));
	});
}

void MainWindow::refreshSpectrumDisplay()
{
    auto image = m_display.image();

    if (m_debug.isChecked()) {
        image = image.scaledToWidth(image.width() * 2);
        auto pen = QColor(*reinterpret_cast<QRgb *>(image.bits()));

        if (pen.lightness() > 128) {
            pen = Qt::GlobalColor::black;
        } else {
            pen = Qt::GlobalColor::white;
        }

        QPainter painter(&image);
        QFont font = painter.font();
        font.setPixelSize(10);
        font.setFixedPitch(true);
        painter.setFont(font);
        painter.setPen(pen);
        int y = 2;
        auto & registers = m_spectrum->z80()->registers();
        QLatin1Char fill('0');
        painter.drawText(2, y += 10, QStringLiteral("PC: $%1").arg(registers.pc, 4, 16, fill));
        painter.drawText(2, y += 10, QStringLiteral("SP: $%1").arg(registers.sp, 4, 16, fill));
        painter.drawText(2, y += 10, QStringLiteral("AF: $%1").arg(registers.af, 4, 16, fill));
        painter.drawText(2, y += 10, QStringLiteral("BC: $%1").arg(registers.bc, 4, 16, fill));
        painter.drawText(2, y += 10, QStringLiteral("DE: $%1").arg(registers.de, 4, 16, fill));
        painter.drawText(2, y + 10, QStringLiteral("HL: $%1").arg(registers.hl, 4, 16, fill));
        painter.end();
    }

    if (m_spectrumThread.isPaused()) {
        QPainter painter(&image);
        QColor fillColour(0x66, 0x66, 0x66, 0x66);
        auto width = image.width() / 5;
        auto height = image.height() / 2;
        auto x = image.width() / 4;
        auto y = image.height() / 4;
        painter.fillRect(x, y, width, height, fillColour);
        painter.fillRect(image.width() - x - width, y, width, height, fillColour);
        painter.end();
    }

    m_displayWidget.setImage(image);
}

Spectrum::Model MainWindow::model() const
{
    assert(m_spectrum);
    return m_spectrum->model();
}

void MainWindow::setModel(Spectrum::Model model)
{
    std::unique_ptr<BaseSpectrum> newSpectrum;
    QString error;

    switch (model) {
        case Model::Spectrum16k:
            if (!fs::exists(Default16kRom)) {
                error = tr("The ROM file for the %1 is missing.").arg(QString::fromStdString(std::to_string(model)));
            } else {
                newSpectrum = std::make_unique<Spectrum16k>(Default16kRom);
                m_model16.setChecked(true);
            }
            break;

        case Model::Spectrum48k:
            if (!fs::exists(Default48kRom)) {
                error = tr("The ROM file for the %1 is missing.").arg(QString::fromStdString(std::to_string(model)));
            } else {
                newSpectrum = std::make_unique<Spectrum48k>(Default48kRom);
                m_model48.setChecked(true);
            }
            break;

        case Model::Spectrum128k:
            if (!fs::exists(Default128kRom0)) {
                error = tr("The %1 ROM file for the %2 is missing.").arg(tr("first"), QString::fromStdString(std::to_string(model)));
            } else if (!fs::exists(Default128kRom0)) {
                error = tr("The %1 ROM file for the %2 is missing.").arg(tr("second"), QString::fromStdString(std::to_string(model)));
            } else {
                newSpectrum = std::make_unique<Spectrum128k>(Default128kRom0, Default128kRom1);
                m_model128.setChecked(true);
            }
            break;

        case Model::SpectrumPlus2:
            if (!fs::exists(DefaultPlus2Rom0)) {
                error = tr("The %1 ROM file for the %2 is missing.").arg(tr("first"), QString::fromStdString(std::to_string(model)));
            } else if (!fs::exists(DefaultPlus2Rom1)) {
                error = tr("The %1 ROM file for the %2 is missing.").arg(tr("second"), QString::fromStdString(std::to_string(model)));
            } else {
                newSpectrum = std::make_unique<SpectrumPlus2>(DefaultPlus2Rom0, DefaultPlus2Rom1);
                m_modelPlus2.setChecked(true);
            }
            break;

        case Model::SpectrumPlus2a:
            if (!fs::exists(DefaultPlus2aRom0)) {
                error = tr("The %1 ROM file for the %2 is missing.").arg(tr("first"), QString::fromStdString(std::to_string(model)));
            } else if (!fs::exists(DefaultPlus2aRom1)) {
                error = tr("The %1 ROM file for the %2 is missing.").arg(tr("second"), QString::fromStdString(std::to_string(model)));
            } else if (!fs::exists(DefaultPlus2aRom2)) {
                error = tr("The %1 ROM file for the %2 is missing.").arg(tr("third"), QString::fromStdString(std::to_string(model)));
            } else if (!fs::exists(DefaultPlus2aRom3)) {
                error = tr("The %1 ROM file for the %2 is missing.").arg(tr("fourth"), QString::fromStdString(std::to_string(model)));
            } else {
                newSpectrum = std::make_unique<SpectrumPlus2a>(DefaultPlus2aRom0, DefaultPlus2aRom1, DefaultPlus2aRom2, DefaultPlus2aRom3);
                m_modelPlus2a.setChecked(true);
            }
            break;

        case Model::SpectrumPlus3:
            if (!fs::exists(DefaultPlus2aRom0)) {
                error = tr("The %1 ROM file for the %2 is missing.").arg(tr("first"), QString::fromStdString(std::to_string(model)));
            } else if (!fs::exists(DefaultPlus2aRom1)) {
                error = tr("The %1 ROM file for the %2 is missing.").arg(tr("second"), QString::fromStdString(std::to_string(model)));
            } else if (!fs::exists(DefaultPlus2aRom2)) {
                error = tr("The %1 ROM file for the %2 is missing.").arg(tr("third"), QString::fromStdString(std::to_string(model)));
            } else if (!fs::exists(DefaultPlus2aRom3)) {
                error = tr("The %1 ROM file for the %2 is missing.").arg(tr("fourth"), QString::fromStdString(std::to_string(model)));
            } else {
                newSpectrum = std::make_unique<SpectrumPlus3>(DefaultPlus3Rom0, DefaultPlus3Rom1, DefaultPlus3Rom2, DefaultPlus3Rom3);
                m_modelPlus3.setChecked(true);
            }
            break;
    }

    if (!newSpectrum) {
        assert(!error.isEmpty());
        Application::instance()->showMessage(error, DefaultStatusBarMessageTimeout);
        return;
    }
    
    detachSpectrumDevices();
    bool paused = m_spectrumThread.isPaused();
    m_displayRefreshTimer.stop();
    stopThread();
    m_spectrum = std::move(newSpectrum);

#if (!defined(NDEBUG))
    m_spectrum->dumpState();
#endif

    attachSpectrumDevices();
    m_spectrumThread.setSpectrum(*m_spectrum);
    m_spectrumThread.start();

    if (paused) {
        m_spectrumThread.pause();
    } else {
        m_displayRefreshTimer.start();
    }

    refreshSpectrumDisplay();
    setWindowTitle(QString::fromStdString(std::to_string(m_spectrum->model())));
}

void MainWindow::saveScreenshot(const QString & fileName)
{
    ThreadPauser pauser(m_spectrumThread);

    if (fileName.endsWith(QStringLiteral(".scr"), Qt::CaseSensitivity::CaseInsensitive)) {
        std::ofstream outFile(fileName.toStdString());

        if (!outFile.is_open()) {
            Util::debug << "Could not open file '" << fileName.toStdString() << "' for writing\n";
            return;
        }

        outFile.write(reinterpret_cast<const char *>(m_spectrum->displayMemory()), m_spectrum->displayMemorySize());
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

    QFile in(fileName);

    if (in.open(QIODevice::OpenModeFlag::ReadOnly)) {
        auto signature = in.read(4);

        if ("ZX82" == signature) {
            return QStringLiteral("zx82");
        } else if (signature.startsWith("SP")) {
            return QStringLiteral("sp");
        }
    }

    return {};
}

bool MainWindow::loadSnapshot(const QString & fileName, QString format)
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
        reader = std::make_unique<SpSnapshotReader>(fileName.toStdString());
    } else if ("zx82" == format) {
        reader = std::make_unique<ZX82SnapshotReader>(fileName.toStdString());
    } else if ("zx" == format) {
        reader = std::make_unique<ZXSnapshotReader>(fileName.toStdString());
    }

    if (!reader) {
        Util::debug << "unrecognised format '" << format.toStdString() << "' from filename '" << fileName.toStdString() << "'\n";
        statusBar()->showMessage(tr("The snapshot format for %1 could not be determined.").arg(fileName), DefaultStatusBarMessageTimeout);
        return false;
    }

    if (!reader->isOpen()) {
        Util::debug << "Snapshot file '" << fileName.toStdString() << "' could not be opened.\n";
        statusBar()->showMessage(tr("The snapshot file %1 could not be opened.").arg(fileName), DefaultStatusBarMessageTimeout);
        return false;
    }

    const auto * snapshot = reader->read();

    if (!snapshot) {
        statusBar()->showMessage(tr("The snapshot file %1 is not valid.").arg(fileName), DefaultStatusBarMessageTimeout);
        return false;
    }

    if (bool canApply = m_spectrum->canApplySnapshot(*snapshot); !canApply) {
        if (snapshot->model() != m_spectrum->model()) {
            if (QMessageBox::StandardButton::Yes != QMessageBox::question(
                    this,
                    tr("Incompatible Spectrum Model"),
                    tr("The loaded snapshot requires a %1.\n\nDo you want to change the emulated Spectrum model and load the snapshot?")
                            .arg(QString::fromStdString(std::to_string(snapshot->model())))
            )) {
                statusBar()->showMessage(
                        tr("The snapshot file %1 is not compatible with the running Spectrum (it requires a %2).")
                                .arg(fileName, QString::fromStdString(std::to_string(snapshot->model()))),
                        DefaultStatusBarMessageTimeout
                );
                return false;
            }

            setModel(snapshot->model());
            canApply = m_spectrum->canApplySnapshot(*snapshot);
        }

        if (!canApply) {
            statusBar()->showMessage(
                    tr("The snapshot file %1 cannot be loaded.")
                            .arg(fileName, QString::fromStdString(std::to_string(snapshot->model()))),
                    DefaultStatusBarMessageTimeout
            );
            return false;
        }
    }

    m_spectrum->applySnapshot(*snapshot);

    setWindowTitle(QStringLiteral("%1 | %2").arg(QString::fromStdString(std::to_string(m_spectrum->model())), QFileInfo(fileName).fileName()));
    m_display.redrawDisplay(m_spectrum->displayMemory());
    m_displayWidget.setImage(m_display.image());

    if ("sna" == format) {
        // RETN instruction is required to resume execution of the .SNA
        m_spectrum->z80()->execute(reinterpret_cast<const Z80::UnsignedByte *>("\xed\x45"), true);
    }

    statusBar()->showMessage(tr("The snapshot file %1 was successfully loaded.").arg(fileName), DefaultStatusBarMessageTimeout);
    return true;
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

    // snapshot must remain valid while the writer is alive
    std::unique_ptr<Snapshot> snapshot;
    std::unique_ptr<SnapshotWriter> writer;

    if ("sna" == format) {
        // push the current PC onto the stack
        m_spectrum->z80()->execute(reinterpret_cast<const Z80::UnsignedByte *>("\xcd\x00\x00"), false);
        snapshot = m_spectrum->snapshot();
        writer = std::make_unique<SnaSnapshotWriter>(*snapshot);
    } else if ("z80" == format) {
        snapshot = m_spectrum->snapshot();
        writer = std::make_unique<Z80SnapshotWriter>(*snapshot);
    } else if ("sp" == format) {
        snapshot = m_spectrum->snapshot();
        writer = std::make_unique<SpSnapshotWriter>(*snapshot);
    }

    if (!writer) {
        Util::debug << "unrecognised format '" << format.toStdString() << "' from filename '" << fileName.toStdString() << "'\n";
        statusBar()->showMessage(tr("Unrecognised snapshot format %1.").arg(format), DefaultStatusBarMessageTimeout);
    }

    if (!writer->writeTo(fileName.toStdString())) {
        Util::debug << "failed to write snapshot to '" << fileName.toStdString() << "'\n";
        statusBar()->showMessage(tr("Failed to save snapshot to %1.").arg(fileName), DefaultStatusBarMessageTimeout);
    } else {
        statusBar()->showMessage(tr("Snapshot successfully saved to %1.").arg(fileName), DefaultStatusBarMessageTimeout);
    }

    if (QStringLiteral("sna") == format) {
        // pop the PC off the stack
        m_spectrum->z80()->execute(reinterpret_cast<const Z80::UnsignedByte *>("\xed\x45"), false);
    }
}

void MainWindow::rescanGameControllers()
{
    // keep this so that we can restore selection after the rescan
    auto * currentController = m_gameControllerHandler.gameController();

    // all member actions are owned by the menu so this triggers deletion of the actions
    m_gameControllersMenu.clear();

    // NOTE actions that are part of a group are removed from the group automatically when they are deleted so removing them from the menu also removes them
    // from the group

    for (const auto gamepadId : QGamepadManager::instance()->connectedGamepads()) {
        QGamepad gamepad(gamepadId);
        Util::debug << "found connected gameController \"" << gamepad.name().toStdString() << "\" [" << gamepadId << "]\n";

        auto * action = m_gameControllersMenu.addAction(gamepad.name(), [this, gamepadId]() {
            m_gameControllerHandler.setGameController(gamepadId);
        });

        m_gameControllersGroup.addAction(action);
        action->setData(gamepad.name());
        action->setCheckable(true);
        action->setChecked(currentController && gamepadId == currentController->deviceId());
    }

    // add the keyboard-mapped controller
    auto * action = m_gameControllersMenu.addAction(tr("Keyboard"), [this]() {
        m_gameControllerHandler.setGameController(nullptr);
    });

    m_gameControllersGroup.addAction(action);
    action->setCheckable(true);
    action->setChecked(!currentController);
}

bool MainWindow::eventFilter(QObject * target, QEvent * event)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
    if (&m_displayWidget == target) {
        switch (event->type()) {
            case QEvent::Type::MouseMove:
                if (m_mouse) {
                    auto pos = m_displayWidget.mapToSpectrum(dynamic_cast<QMouseEvent *>(event)->pos());
                    m_mouse->setX(pos.x());
                    m_mouse->setY(pos.y());
                }
                break;
                
            case QEvent::Type::MouseButtonPress:
            case QEvent::Type::MouseButtonRelease:
                if (m_mouse) {
                    auto buttons = dynamic_cast<QMouseEvent *>(event)->buttons();
                    m_mouse->setButton1Pressed(buttons & Qt::MouseButton::LeftButton);
                    m_mouse->setButton2Pressed(buttons & Qt::MouseButton::RightButton);
                    m_mouse->setButton3Pressed(buttons & Qt::MouseButton::MiddleButton);
                }
                break;
                
            case QEvent::Type::KeyPress: {
                auto qtKey = static_cast<Qt::Key>(dynamic_cast<QKeyEvent *>(event)->key());

                // while user holds tab on display widget, temporarily run as fast as possible
                if (Qt::Key::Key_Tab == qtKey) {
                    m_spectrum->setExecutionSpeedConstrained(false);
                    updateStatusBarSpeedWidget();
                    return true;
                } else if (auto joystickMapping = mapToSpectrumJoystick(qtKey); !m_gameControllerHandler.gameController() && m_joystick && JoystickMapping::None != joystickMapping) {
                    // if there's a joystick interface connected to the emulated spectrum and it's a keyboard-mapped joystick and the keypress matches a key
                    // mapped to the joystick, the joystick swallows the keypress event
                    switch (joystickMapping) {
                        case JoystickMapping::Up:
                            m_joystick->setJoystick1Up(true);
                            break;

                        case JoystickMapping::Down:
                            m_joystick->setJoystick1Down(true);
                            break;

                        case JoystickMapping::Left:
                            m_joystick->setJoystick1Left(true);
                            break;

                        case JoystickMapping::Right:
                            m_joystick->setJoystick1Right(true);
                            break;

                        case JoystickMapping::Button1:
                            m_joystick->setJoystick1Button1Pressed(true);
                            break;

                        case JoystickMapping::Button2:
                            m_joystick->setJoystick1Button2Pressed(true);
                            break;

                        case JoystickMapping::Button3:
                            m_joystick->setJoystick1Button3Pressed(true);
                            break;
                    }
                } else {
                    for (const auto & key : mapToSpectrumKeys(qtKey)) {
                        m_keyboard.setKeyDown(key);
                    }
                }
            }
            break;

            case QEvent::Type::KeyRelease: {
                auto qtKey = static_cast<Qt::Key>(dynamic_cast<QKeyEvent *>(event)->key());

                if (Qt::Key::Key_Tab == qtKey) {
                    m_spectrum->setExecutionSpeedConstrained(0 < m_emulationSpeedSlider.value());
                    updateStatusBarSpeedWidget();
                    return true;
                } else if (auto joystickMapping = mapToSpectrumJoystick(qtKey); m_joystick && JoystickMapping::None !=
                                                                                joystickMapping) {
                    switch (joystickMapping) {
                        case JoystickMapping::Up:
                            m_joystick->setJoystick1Up(false);
                            break;

                        case JoystickMapping::Down:
                            m_joystick->setJoystick1Down(false);
                            break;

                        case JoystickMapping::Left:
                            m_joystick->setJoystick1Left(false);
                            break;

                        case JoystickMapping::Right:
                            m_joystick->setJoystick1Right(false);
                            break;

                        case JoystickMapping::Button1:
                            m_joystick->setJoystick1Button1Pressed(false);
                            break;

                        case JoystickMapping::Button2:
                            m_joystick->setJoystick1Button2Pressed(false);
                            break;

                        case JoystickMapping::Button3:
                            m_joystick->setJoystick1Button3Pressed(false);
                            break;
                    }
                } else {
                    for (const auto & key : mapToSpectrumKeys(qtKey)) {
                        m_keyboard.setKeyUp(key);
                    }
                }
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
    settings.beginGroup(QStringLiteral("mainwindow"));
    settings.setValue(QStringLiteral("position"), pos());
    settings.setValue(QStringLiteral("size"), size());
    settings.setValue(QStringLiteral("lastSnapshotLoadDir"), m_lastSnapshotLoadDir);
    settings.setValue(QStringLiteral("lastScreenshotDir"), m_lastScreenshotDir);
    settings.setValue(QStringLiteral("lastPokeLoadDir"), m_lastPokeLoadDir);
    settings.setValue(QStringLiteral("emulationSpeed"), m_emulationSpeedSlider.value());
    settings.setValue(QStringLiteral("windowState"), saveState());

    {
        QString joystickType;

        if (m_joystickKempston.isChecked()) {
            joystickType = QStringLiteral("kempston");
        } else if (m_joystickInterface2.isChecked()) {
            joystickType = QStringLiteral("zxinterfacetwo");
        } else if (m_joystickCursor.isChecked()) {
            joystickType = QStringLiteral("cursor");
        } else if (m_joystickNone.isChecked()) {
            joystickType = QStringLiteral("none");
        }

        settings.setValue(QStringLiteral("joystick1Interface"), joystickType);
    }

    {
        QString controller;

        for (auto * const action : m_gameControllersGroup.actions()) {
            if (action->isChecked()) {
                controller = action->data().toString();
                break;
            }
        }

        if (controller.isEmpty()) {
            settings.remove(QStringLiteral("joystick1Controller"));
        } else {
            settings.setValue(QStringLiteral("joystick1Controller"), controller);
        }
    }

    {
        QString mouseType;

        if (m_kempstonMouse.isChecked()) {
            mouseType = QStringLiteral("kempston");
        }

        if (mouseType.isEmpty()) {
            settings.remove(QStringLiteral("mouseInterface"));
        } else {
            settings.setValue(QStringLiteral("mouseInterface"), mouseType);
        }
    }

    {
        QString model;

        if (!m_spectrum) {
            model = QStringLiteral("none");
        } else {
            switch (m_spectrum->model()) {
                case Model::Spectrum16k:
                    model = QStringLiteral("16k");
                    break;

                case Model::Spectrum48k:
                    model = QStringLiteral("48k");
                    break;

                case Model::Spectrum128k:
                    model = QStringLiteral("128k");
                    break;

                case Model::SpectrumPlus2:
                    model = QStringLiteral("+2");
                    break;

                case Model::SpectrumPlus2a:
                    model = QStringLiteral("+2a");
                    break;

                case Model::SpectrumPlus3:
                    model = QStringLiteral("+3");
                    break;
            }
        }

        settings.setValue(QStringLiteral("model"), model);
    }

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
    m_lastPokeLoadDir = settings.value(QStringLiteral("lastPokeLoadDir")).toString();
    auto speed = settings.value(QStringLiteral("emulationSpeed"), 100).toInt(&ok);

    if (!ok) {
        speed = 100;
    }

    m_emulationSpeedSlider.setValue(speed);

    {
        auto joystick = settings.value(QStringLiteral("joystick1Interface")).toString();

        if (QStringLiteral("kempston") == joystick) {
            m_joystickKempston.setChecked(true);
            useKempstonJoystickTriggered();
        } else if (QStringLiteral("zxinterfacetwo") == joystick) {
            m_joystickInterface2.setChecked(true);
            useInterfaceTwoJoystickTriggered();
        } else if (QStringLiteral("cursor") == joystick) {
            m_joystickCursor.setChecked(true);
            useCursorJoystickTriggered();
        } else {
            m_joystickNone.setChecked(true);
            noJoystickTriggered();
        }
    }

    {
        auto controller = settings.value(QStringLiteral("joystick1Controller")).toString();
        bool found = false;
        QAction * keyboardControllerAction = nullptr;

        for (auto * const action : m_gameControllersGroup.actions()) {
            auto actionController = action->data().toString();

            if (actionController == controller) {
                action->setChecked(true);
                action->trigger();
                found = true;
                break;
            }

            if (actionController.isEmpty()) {
                keyboardControllerAction = action;
            }
        }

        // default to the keyboard controller if the settings contain a controller that's not currently available
        if (!found && keyboardControllerAction) {
            keyboardControllerAction->setChecked(true);
        }
    }

    {
        auto mouseType = settings.value(QStringLiteral("mouseInterface")).toString();

        if (QStringLiteral("kempston") == mouseType) {
            m_kempstonMouse.setChecked(true);
            kempstonMouseToggled(true);
        } else {
            m_kempstonMouse.setChecked(false);
            kempstonMouseToggled(false);
        }
    }

    // TODO probably need to move this to the constructor otherwise a show event will reset any existing, running
    //  spectrum
    {
        auto model = settings.value(QStringLiteral("model")).toString();

        if (QStringLiteral("16k") == model) {
            setModel(Model::Spectrum16k);
        } else if(QStringLiteral("48k") == model) {
            setModel(Model::Spectrum48k);
        } else if(QStringLiteral("128k") == model) {
            setModel(Model::Spectrum128k);
        } else if(QStringLiteral("+2") == model) {
            setModel(Model::SpectrumPlus2);
        } else if(QStringLiteral("+2a") == model) {
            setModel(Model::SpectrumPlus2a);
        } else if(QStringLiteral("+3") == model) {
            setModel(Model::SpectrumPlus3);
        }
    }

    restoreState(settings.value(QStringLiteral("windowState")).toByteArray());
    settings.endGroup();
}

void MainWindow::keyPressEvent(QKeyEvent * event)
{
    if (!event->isAutoRepeat() && Qt::Key::Key_Tab == event->key()) {
        m_spectrum->setExecutionSpeedConstrained(false);
        updateStatusBarSpeedWidget();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent * event)
{
    if (Qt::Key::Key_Tab == event->key()) {
        m_spectrum->setExecutionSpeedConstrained(0 < m_emulationSpeedSlider.value());
        updateStatusBarSpeedWidget();
        return;
    }

    QMainWindow::keyReleaseEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent * event)
{
    // NOTE when all this logic is combined into a single if() we get apparently random segfaults accessing the URL
    // scheme, even though we've checked to ensure that the list is not empty and therefore we're entitled to assume
    // that QList::(const)first() is valid
    if (!event->mimeData()->hasUrls()) {
        return;
    }

    const auto urls = event->mimeData()->urls();

    if (urls.isEmpty()) {
        return;
    }

    if (urls.constFirst().isLocalFile()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent * event)
{
    // NOTE when all this logic is combined into a single if() we get apparently random segfaults accessing the URL
    // scheme, even though we've checked to ensure that the list is not empty and therefore we're entitled to assume
    // that QList::(const)first() is valid
    if (!event->mimeData()->hasUrls()) {
        return;
    }

    const auto urls = event->mimeData()->urls();

    if (urls.isEmpty()) {
        return;
    }

    const auto & url = urls.constFirst();

    if (url.isLocalFile()) {
        loadSnapshot(url.toLocalFile());
    } else {
        Util::debug << "remote URLs cannot yet be loaded.\n";
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
        filters << tr("ZX82 Snapshots (*.zx82)");
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

    saveSnapshot(fileName, format);
}

void MainWindow::model16Triggered()
{
    setModel(Model::Spectrum16k);
}

void MainWindow::model48Triggered()
{
    setModel(Model::Spectrum48k);
}

void MainWindow::model128Triggered()
{
    setModel(Model::Spectrum128k);
}

void MainWindow::modelPlus2Triggered()
{
    setModel(Model::SpectrumPlus2);
}

void MainWindow::modelPlus2aTriggered()
{
    setModel(Model::SpectrumPlus2a);
}

void MainWindow::modelPlus3Triggered()
{
    setModel(Model::SpectrumPlus3);
}

void MainWindow::useKempstonJoystickTriggered()
{
    m_spectrum->setJoystickInterface(nullptr);
    m_gameControllerHandler.setJoystick(nullptr);
    m_joystick = std::make_unique<KempstonJoystick>();
    m_gameControllerHandler.setJoystick(m_joystick.get());
    m_spectrum->setJoystickInterface(m_joystick.get());
}

void MainWindow::useInterfaceTwoJoystickTriggered()
{
    m_spectrum->setJoystickInterface(nullptr);
    m_gameControllerHandler.setJoystick(nullptr);
    m_joystick = std::make_unique<InterfaceTwoJoystick>();
    m_gameControllerHandler.setJoystick(m_joystick.get());
    m_spectrum->setJoystickInterface(m_joystick.get());
}

void MainWindow::useCursorJoystickTriggered()
{
    m_spectrum->setJoystickInterface(nullptr);
    m_gameControllerHandler.setJoystick(nullptr);
    m_joystick = std::make_unique<CursorJoystick>();
    m_gameControllerHandler.setJoystick(m_joystick.get());
    m_spectrum->setJoystickInterface(m_joystick.get());
}

void MainWindow::useFullerJoystickTriggered()
{
    m_spectrum->setJoystickInterface(nullptr);
    m_gameControllerHandler.setJoystick(nullptr);
    m_joystick = std::make_unique<FullerJoystick>();
    m_gameControllerHandler.setJoystick(m_joystick.get());
    m_spectrum->setJoystickInterface(m_joystick.get());
}

void MainWindow::noJoystickTriggered()
{
    m_spectrum->setJoystickInterface(nullptr);
    m_gameControllerHandler.setJoystick(nullptr);
    m_joystick.reset();
}

void MainWindow::kempstonMouseToggled(bool on)
{
    m_spectrum->setMouseInterface(nullptr);

    if (on) {
        m_mouse = std::make_unique<KempstonMouse>();
        m_spectrum->setMouseInterface(m_mouse.get());
        m_displayWidget.setMouseTracking(true);
    } else {
        m_mouse.reset();
        m_displayWidget.setMouseTracking(false);
    }
}

void MainWindow::emulationSpeedChanged(int speed)
{
    if (0 == speed) {
        m_spectrum->setExecutionSpeedConstrained(false);
    } else {
        m_spectrum->setExecutionSpeedConstrained(true);
        m_spectrum->setExecutionSpeed(speed);
    }

    updateStatusBarSpeedWidget();
}

void MainWindow::updateStatusBarSpeedWidget()
{
    if (m_spectrum->executionSpeedConstrained()) {
        m_statusBarEmulationSpeed.setText(tr("%1%").arg(m_spectrum->executionSpeedPercent()));
    } else {
        m_statusBarEmulationSpeed.setText(tr("%1%").arg("∞"));    // infinity
    }

    auto mhz = m_spectrum->z80()->clockSpeedMHz();
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
    connect(&m_spectrumThread, &Thread::stepped, this, &MainWindow::threadStepped, Qt::UniqueConnection);
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

void MainWindow::saveSnapshotToSlot(int slotIndex, QString format)
{
    assert(1 <= slotIndex && 5 >= slotIndex);

    if (format.isEmpty()) {
        format = QStringLiteral("z80");
    }

    auto slotDir = QDir(QStandardPaths::writableLocation(QStandardPaths::StandardLocation::AppDataLocation));

    if (!slotDir.mkpath(QStringLiteral("slots"))) {
        statusBar()->showMessage(tr("The save slots directory does not exist and could not be created."), DefaultStatusBarMessageTimeout);
        return;
    }

    slotDir.cd(QStringLiteral("slots"));
    saveSnapshot(slotDir.absoluteFilePath("%1.%2").arg(slotIndex).arg(format), format);
}

bool MainWindow::loadSnapshotFromSlot(int slotIndex)
{
    assert(1 <= slotIndex && 5 >= slotIndex);

    auto slotDir = QDir(QStandardPaths::writableLocation(QStandardPaths::StandardLocation::AppDataLocation) % QStringLiteral("/slots"));
    auto fileName = QStringLiteral("%1.z80").arg(slotIndex);

    if (!slotDir.exists(fileName)) {
        statusBar()->showMessage(tr("Slot %1 is empty").arg(slotIndex));
        return false;
    }

    if (!loadSnapshot(slotDir.absoluteFilePath(fileName), QStringLiteral("z80"))) {
        statusBar()->showMessage(tr("Snapshot in slot %1 could not be loaded.").arg(slotIndex));
        return false;
    }

    setWindowTitle(QStringLiteral("%1 | Snapshot slot %2").arg(QString::fromStdString(std::to_string(m_spectrum->model()))).arg(slotIndex));
    return true;
}

void MainWindow::attachSpectrumDevices()
{
    m_spectrum->setJoystickInterface(m_joystick.get());
    m_spectrum->setKeyboard(&m_keyboard);
    m_spectrum->setMouseInterface(m_mouse.get());
    m_spectrum->addDisplayDevice(&m_display);
}

void MainWindow::detachSpectrumDevices()
{
    m_spectrum->setJoystickInterface(nullptr);
    m_spectrum->setKeyboard(nullptr);
    m_spectrum->setMouseInterface(nullptr);
    m_spectrum->removeDisplayDevice(&m_display);
}
