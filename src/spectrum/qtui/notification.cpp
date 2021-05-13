//
// Created by darren on 23/04/2021.
//

#include <QDBusInterface>
#include <QDBusReply>
#include "application.h"
#include "notification.h"
#include "dialogue.h"
#include "../../util/debug.h"

using namespace Spectrum::QtUi;

namespace
{
    void showFallbackNotification(const QString& message, const QString& title = {})
    {
        Dialogue dlg(message, title);
        dlg.setIcon(Application::icon("app"));
        dlg.exec();
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

// uncomment this to use winrt toast notifications when C++/WinRT is compatible with C++20
//#define SPECTRUM_NOTIFICATION_USE_WINRT

#include <QStringBuilder>

#if (defined(SPECTRUM_NOTIFICATION_USE_WINRT))
// NOTE this won't currently build - C++/WinRT won't currently build with C++20.
#include <winrt/Windows.UI.Notifications.h>
#include <winrt/Windows.Data.Xml.Dom.h>

namespace Notifications = winrt::Windows::UI::Notifications;
using Notifications::ToastNotification;
using Notifications::ToastNotificationManager;
using winrt::Windows::Data::Xml::Dom::XmlDocument;

namespace
{
    XmlDocument notificationXml(const QString & message, const QString & title)
    {
        static const auto prefix = QStringLiteral(R"(<toast activationType="foreground"><visual><binding template="ToastGeneric">)");
        static const auto postfix = QStringLiteral("</binding></visual></toast>");
        QString content;

        if (!title.isEmpty()) {
            content = content % R"(<text hint-maxLines="1">)" % title % "</text>";
        }

        content = content % "<text>" % message % "</text>";
        Util::debug << "Toast XML: " << static_cast<QString>(prefix % content % postfix).toStdString() << '\n';
        XmlDocument doc;
        doc.LoadXml(static_cast<QString>(prefix % content % postfix).toStdWString());
        return doc;
    }
}
#endif

void Notification::show(std::optional<int> timeout) const
{
#if (defined(SPECTRUM_NOTIFICATION_USE_WINRT))
    ToastNotification toast(notificationXml(message(), title()));
    DesktopNotificationManagerCompat::CreateToastNotifier(Application::applicationName().toStdWString()).Show(toast);
#else
    showFallbackNotification(message(), title());
#endif
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
