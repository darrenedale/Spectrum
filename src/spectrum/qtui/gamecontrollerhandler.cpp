//
// Created by darren on 19/04/2021.
//

#include "gamecontrollerhandler.h"
#include "../../util/debug.h"

using namespace Spectrum::QtUi;

GameControllerHandler::GameControllerHandler(Spectrum::JoystickInterface * joystick, std::unique_ptr<QGamepad> gamepad)
: m_joystick(joystick),
  m_gameController(std::move(gamepad))
{
    connectGameController();
}

GameControllerHandler::GameControllerHandler(Spectrum::JoystickInterface * joystick, std::optional<int> gamepadId)
: GameControllerHandler(joystick, (gamepadId ? std::make_unique<QGamepad>(*gamepadId) : std::unique_ptr<QGamepad>()))
{}

GameControllerHandler::~GameControllerHandler()
{
    disconnectGameController();
    m_gameController = nullptr;
    m_joystick = nullptr;
}

void GameControllerHandler::setJoystick(JoystickInterface * joystick)
{
    disconnectGameController();
    m_joystick = joystick;
    connectGameController();
}

void GameControllerHandler::setGameController(std::unique_ptr<QGamepad> gamepad)
{
    disconnectGameController();
    m_gameController = std::move(gamepad);
    connectGameController();
}

void GameControllerHandler::disconnectGameController()
{
    if (!m_gameController) {
        return;
    }

    m_gameController->disconnect(this);
}

void GameControllerHandler::connectGameController()
{
    if (!m_gameController) {
        return;
    }

    connect(m_gameController.get(), &QGamepad::buttonLeftChanged, this, &GameControllerHandler::gameControllerLeftChanged, Qt::ConnectionType::UniqueConnection);
    connect(m_gameController.get(), &QGamepad::buttonRightChanged, this, &GameControllerHandler::gameControllerRightChanged, Qt::ConnectionType::UniqueConnection);
    connect(m_gameController.get(), &QGamepad::buttonUpChanged, this, &GameControllerHandler::gameControllerUpChanged, Qt::ConnectionType::UniqueConnection);
    connect(m_gameController.get(), &QGamepad::buttonDownChanged, this, &GameControllerHandler::gameControllerDownChanged, Qt::ConnectionType::UniqueConnection);
    connect(m_gameController.get(), &QGamepad::buttonAChanged, this, &GameControllerHandler::gameControllerButton1Changed, Qt::ConnectionType::UniqueConnection);
    connect(m_gameController.get(), &QGamepad::buttonBChanged, this, &GameControllerHandler::gameControllerButton2Changed, Qt::ConnectionType::UniqueConnection);
}

void GameControllerHandler::gameControllerLeftChanged(bool pressed)
{
    if (!m_joystick) {
        return;
    }

    m_joystick->setJoystick1Left(pressed);
}

void GameControllerHandler::gameControllerRightChanged(bool pressed)
{
    if (!m_joystick) {
        return;
    }

    m_joystick->setJoystick1Right(pressed);
}

void GameControllerHandler::gameControllerUpChanged(bool pressed)
{
    if (!m_joystick) {
        return;
    }

    m_joystick->setJoystick1Up(pressed);
}

void GameControllerHandler::gameControllerDownChanged(bool pressed)
{
    if (!m_joystick) {
        return;
    }

    m_joystick->setJoystick1Down(pressed);
}

void GameControllerHandler::gameControllerButton1Changed(bool pressed)
{
    if (!m_joystick) {
        return;
    }

    m_joystick->setJoystick1Button1Pressed(pressed);
}

void GameControllerHandler::gameControllerButton2Changed(bool pressed)
{
    if (!m_joystick) {
        return;
    }

    m_joystick->setJoystick1Button2Pressed(pressed);
}

void GameControllerHandler::gameControllerButton3Changed(bool pressed)
{
    if (!m_joystick) {
        return;
    }

    m_joystick->setJoystick1Button3Pressed(pressed);
}
