//
// Created by darren on 23/04/2021.
//

#include <QDBusInterface>
#include "application.h"
#include "notification.h"

using namespace Spectrum::QtUi;

std::unique_ptr<QDBusInterface> Notification::m_interface;

Notification::Notification(QString body, QString title)
: m_message(std::move(body)),
  m_title(std::move(title)),
  m_timeout(0)
{}

void Notification::show(std::optional<int> timeout) const
{
#if (defined(Q_OS_LINUX))
    dbusInterface().call(
        QLatin1String("Notify"),
        Application::applicationDisplayName(),
        0,
        QLatin1String(""), // icon
        title(),
        message(),
        QStringList(),
        QVariantMap(),
        timeout ? *timeout : this->timeout()
    );
#elif (defined(Q_OS_WIN))
#elif (defined(Q_OS_MAC))
#endif
}

QDBusInterface & Notification::dbusInterface()
{
    if (!m_interface) {
        m_interface = std::make_unique<QDBusInterface>(
            QStringLiteral("org.freedesktop.Notifications"),
            QStringLiteral("/org/freedesktop/Notifications"),
            QString(),
            QDBusConnection::sessionBus()
        );
    }

    return *m_interface;
}

void Notification::showNotification(QString message, QString title, int timeout)
{
    Notification(std::move(message), std::move(title)).show(timeout);
}

Notification::Notification(const Notification &) = default;

Notification::Notification(Notification &&) noexcept = default;

Notification & Notification::operator=(const Notification &) = default;

Notification & Notification::operator=(Notification &&) noexcept = default;

Notification::~Notification() = default;
