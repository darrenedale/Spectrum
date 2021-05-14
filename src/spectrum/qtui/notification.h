//
// Created by darren on 23/04/2021.
//

#ifndef SPECTRUM_QTUI_NOTIFICATION_H
#define SPECTRUM_QTUI_NOTIFICATION_H

#include <memory>
#include <optional>
#include <QtGlobal>
#include <QString>

class QDBusInterface;

namespace Spectrum::QtUi
{
    /**
     * Cross-platform encapsulation of a desktop notification.
     *
     * To use, create a notification with a message and optional title, and call show. Or just call the static showNotification() method. Notifications will
     * attempt to use the platform's desktop notifications framework to show the message. If this fails, it will fall back to a dialogue box.
     *
     * TODO support actions
     */
    class Notification
    {
    public:
        /**
         * Initialise a new notification with a message and optional title.
         *
         * By default the message will have no automatic timeout.
         *
         * @param message The message to show.
         * @param title The title for the notification.
         */
        explicit Notification(QString message, QString title = {});

        /**
         * Copy constructor.
         */
        Notification(const Notification &);

        /**
         * Move constructor.
         */
        Notification(Notification &&) noexcept;

        /**
         * Copy assignment operator.
         *
         * @return A reference to this Notification.
         */
        Notification & operator=(const Notification &);

        /**
         * Move assignment operator.
         *
         * @return A reference to this Notification.
         */
        Notification & operator=(Notification &&) noexcept;

        /**
         * Destructor.
         */
        virtual ~Notification();

        /**
         * Fetch the notification's title.
         *
         * This will be an empty string if the notification does not have a title.
         *
         * @return The title.
         */
        [[nodiscard]] inline const QString & title() const
        {
            return m_title;
        }

        /**
         * Set the notification's title.
         *
         * Set this to an empty string if the notification does not have a title.
         *
         * @param title The title.
         */
        inline void setTitle(const QString & title)
        {
             m_title = title;
        }

        /**
         * Fetch the notification's message.
         *
         * @return The nmessage.
         */
        [[nodiscard]] inline const QString & message() const
        {
            return m_message;
        }

        /**
         * Set the notification's message.
         *
         * @param message The message.
         */
        inline void setMessage(const QString & message)
        {
            m_message = message;
        }

        /**
         * Fetch the timeout for the notification.
         *
         * The returned value will be 0 if the notification doesn't automatically timeout.
         *
         * @return The timeout, in ms.
         */
        [[nodiscard]] inline int timeout() const
        {
            return m_timeout;
        }

        /**
         * Set the timeout for the notification.
         *
         * @param timeout The timeout in ms. Set to 0 for no timeout.
         */
        inline void setTimeout(int timeout)
        {
            m_timeout = timeout;
        }

        /**
         * Show the notification.
         *
         * The optional timeout will override the timeout set in the object this one time.
         *
         * @param timeout Optional timeout in ms for which to show the notification.
         */
        void show(std::optional<int> timeout) const;

        /**
         * Convenience method to quickly show a single notification.
         *
         * @param message The message to show.
         * @param title The title for the message.
         * @param timeout The timeout for the notification, in ms.
         */
        static void showNotification(QString message, QString title = {}, int timeout = 0);

        /**
         * Convenience method to quickly show a single notification with no title.
         *
         * @param message The message to show.
         * @param timeout The timeout for the notification, in ms.
         */
        static inline void showNotification(QString message, int timeout = 0)
        {
            showNotification(std::move(message), {}, timeout);
        }

    private:
        /**
         * The notification message.
         */
        QString m_message;

        /**
         * The notification title.
         */
        QString m_title;

        /**
         * The notification timeout.
         */
        int m_timeout;
    };
}

#endif //SPECTRUM_QTUI_NOTIFICATION_H
