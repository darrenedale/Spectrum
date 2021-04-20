//
// Created by darren on 19/04/2021.
//

#ifndef SPECTRUM_GAMECONTROLLERHANDLER_H
#define SPECTRUM_GAMECONTROLLERHANDLER_H

#include <QGamepad>
#include "../joystickinterface.h"

namespace Spectrum::QtUi
{
    /**
     * Helper class to connect a Spectrum joystick to a host machine game controller.
     *
     * Listens for events on a specified game controller and translates them into the appropriate state for a specified Spectrum joystick. This is primarily used in
     * the main window, and is abstracted to a helper class mostly to prevent the MainWindow class from becoming even more bloated.
     */
    class GameControllerHandler
    : public QObject
    {
        // NOTE don't need the Q_OBJECT macro

    public:
        /**
         * Initialise a new handler using a given Spectrum joystick interface and host game controller.
         *
         * The Spectrum to which the provided joystick interface is connected will see the controller as a joystick connected to that interface.
         *
         * Both arguments may be nullptr; if either is nullptr, no connection will be made between the interface and the controller.
         *
         * @param joystick The Spectrum joystick interface.
         * @param gameController The game controller on the host.
         */
        explicit GameControllerHandler(JoystickInterface * joystick = nullptr, std::unique_ptr<QGamepad> gameController = {});

        /**
         * Initialise a new handler using a given Spectrum joystick interface and the device ID of a host game controller.
         *
         * The Spectrum to which the provided joystick interface is connected will see the controller identified by the device ID as a joystick connected to
         * that interface.
         *
         * This is a convenience constructor that delegates to the main constructor above. The connection between the interface and the controller will
         * obviously not work if the ID does not identify a connected game controller.
         *
         * @param joystick The Spectrum joystick interface.
         * @param gameController The game controller on the host.
         */
        explicit GameControllerHandler(JoystickInterface * joystick, std::optional<int> gameControllerId);

        /**
         * Initialise a new handler using a host game controller.
         *
         * Without a Spectrum joystick interface, no connection will be made with the game controller and an emulated Spectrum. Set the interface later to
         * establish the connection.
         *
         * @param gameController The game controller on the host.
         */
        explicit GameControllerHandler(std::unique_ptr<QGamepad> gameController)
        : GameControllerHandler(nullptr, std::move(gameController))
        {}

        /**
         * Initialise a new handler using the device ID of a host game controller.
         *
         * Without a Spectrum joystick interface, no connection will be made with the game controller and an emulated Spectrum. Set the interface later to
         * establish the connection. The connection between the interface (when set) and the controller will obviously not work if the ID does not identify a
         * connected game controller.
         *
         * @param gameControllerId The device ID of the game controller on the host.
         */
        explicit GameControllerHandler(int gameControllerId)
        : GameControllerHandler(nullptr, std::make_unique<QGamepad>(gameControllerId))
        {}

        // there's no real value in copying or moving handlers so these constructors and assignment operators are deleted
        GameControllerHandler(const GameControllerHandler &) = delete;
        GameControllerHandler(GameControllerHandler &&) = delete;
        void operator=(const GameControllerHandler &) = delete;
        void operator=(GameControllerHandler &&) = delete;

        /**
         * Destructor.
         *
         * The handler will automatically disconnect from the game controller.
         */
        ~GameControllerHandler() override;

        /**
         * Fetch a read-only pointer to the managed Spectrum joystick.
         *
         * @return The joystick, or nullptr if no joystick is being managed.
         */
        [[nodiscard]] const JoystickInterface * joystick() const
        {
            return m_joystick;
        }

        /**
         * Fetch a read-write pointer to the managed Spectrum joystick.
         *
         * @return The joystick, or nullptr if no joystick is being managed.
         */
        JoystickInterface * joystick()
        {
            return m_joystick;
        }

        /**
         * Set the joystick to manage.
         *
         * The game controller will be disconnected from the previously set Spectrum joystick interface and it will be connected to the provided interface instead.
         *
         * This can be set to nullptr to stop managing a joystick.
         */
        void setJoystick(JoystickInterface *);

        /**
         * Fetch a read-only pointer to the host game controller being monitored.
         *
         * @return The game controller, or nullptr if no game controller is being monitored.
         */
        [[nodiscard]] const QGamepad * gameController() const
        {
            return m_gameController.get();
        }

        /**
         * Fetch a read-write pointer to the host game controller being monitored.
         *
         * @return The game controller, or nullptr if no game controller is being monitored.
         */
        QGamepad * gameController()
        {
            return m_gameController.get();
        }

        /**
         * Set the game controller to monitor.
         *
         * The previously set game controller will be disconnected from the Spectrum joystick interface and the provided game controller will be connected instead.
         *
         * This can be set to nullptr to stop monitoring a game controller.
         */
        void setGameController(std::unique_ptr<QGamepad>);

        /**
         * Set the game controller to monitor.
         *
         * Convenience overload to set the game controller based on its ID.
         */
        void setGameController(int gameControllerId)
        {
            setGameController(std::make_unique<QGamepad>(gameControllerId));
        }

    protected:
        /**
         * Stop monitoring the game controller.
         *
         * It is safe to call this if there is no game controller or no joystick, it has nullptr protection and in such cases is a NOOP.
         */
        void disconnectGameController();

        /**
         * Start monitoring the game controller.
         *
         * It is safe to call this if there is no game controller or no joystick, it has nullptr protection and in such cases is a NOOP.
         */
        void connectGameController();

        /**
         * Respond when the left button of the monitored game controller changes state.
         *
         * @param pressed
         */
        void gameControllerLeftChanged(bool pressed);

        /**
         * Respond when the right button of the monitored game controller changes state.
         *
         * @param pressed
         */
        void gameControllerRightChanged(bool pressed);

        /**
         * Respond when the up button of the monitored game controller changes state.
         *
         * @param pressed
         */
        void gameControllerUpChanged(bool pressed);

        /**
         * Respond when the down button of the monitored game controller changes state.
         *
         * @param pressed
         */
        void gameControllerDownChanged(bool pressed);

        /**
         * Respond when the configured primary fire button of the monitored game controller changes state.
         *
         * @param pressed
         */
        void gameControllerButton1Changed(bool pressed);

        /**
         * Respond when the configured secondary fire button of the monitored game controller changes state.
         *
         * @param pressed
         */
        void gameControllerButton2Changed(bool pressed);

        /**
         * Respond when the configured tertiary fire button of the monitored game controller changes state.
         *
         * @param pressed
         */
        void gameControllerButton3Changed(bool pressed);

    private:
        /**
         * The Spectrum joystick to manage.
         *
         * This is only borrowed.
         */
        JoystickInterface * m_joystick;

        /**
         * The game controller to monitor.
         */
        std::unique_ptr<QGamepad> m_gameController;
    };
}

#endif //SPECTRUM_GAMECONTROLLERHANDLER_H
