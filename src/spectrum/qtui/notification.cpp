//
// Created by darren on 23/04/2021.
//

#include <QDBusInterface>
#include <QDBusReply>
#include "application.h"
#include "notification.h"
#include "dialogue.h"

using namespace Spectrum::QtUi;

namespace
{
    void showFallbackNotification(const QString& message, const QString& title = {})
    {
        Dialogue(message, title).exec();
    }
}

Notification::Notification(QString body, QString title)
: m_message(std::move(body)),
  m_title(std::move(title)),
  m_timeout(0)
{}

#if (defined(Q_OS_LINUX))

namespace
{
    /**
     * Helper to fetch the DBus interface.
     *
     * The DBus interface is initialised on first call and is cached thereafter, so it's only constructed when required and subsequent calls are very fast.
     *
     * @return The session bus interface.
     */
    QDBusInterface & dbusInterface()
    {
        static std::unique_ptr<QDBusInterface> dbusInterface = nullptr;

        if (!dbusInterface) {
            dbusInterface = std::make_unique<QDBusInterface>(
                    QStringLiteral("org.freedesktop.Notifications"),
                    QStringLiteral("/org/freedesktop/Notifications"),
                    QStringLiteral("org.freedesktop.Notifications"),
                    QDBusConnection::sessionBus()
            );
        }

        return *dbusInterface;
    }
}

void Notification::show(std::optional<int> timeout) const
{
    auto reply = QDBusReply<std::uint32_t>(dbusInterface().call(
        QStringLiteral("Notify"),
        Application::applicationDisplayName(),
        0u,
        QLatin1String(""), // icon
        title(),
        message(),
        QStringList(),
        QVariantMap(),
        timeout ? *timeout : this->timeout()
    ));

    if (!reply.isValid()) {
        Util::debug << "dbus notification error: " << qPrintable(reply.error().message()) << '\n';
        showFallbackNotification(message(), title());
    }
}

#elif (defined(Q_OS_WIN))

void Notification::show(std::optional<int> timeout) const
{
    Util::debug << "Desktop notifications for Windows are not yet implemented.\n";
    showFallbackNotification(message(), title());
}

#elif (defined(Q_OS_MACOS))

void Notification::show(std::optional<int> timeout) const
{
    Util::debug << "Desktop notifications for MacOS are not yet implemented.\n";
    showFallbackNotification(message(), title());
}

#else

void Notification::show(std::optional<int> timeout) const
{
    Util::debug << "Desktop notifications are not available for this platform.\n";
    showFallbackNotification(message(), title());
}

#endif

void Notification::showNotification(QString message, QString title, int timeout)
{
    Notification(std::move(message), std::move(title)).show(timeout);
}

Notification::Notification(const Notification &) = default;

Notification::Notification(Notification &&) noexcept = default;

Notification & Notification::operator=(const Notification &) = default;

Notification & Notification::operator=(Notification &&) noexcept = default;

Notification::~Notification() = default;
