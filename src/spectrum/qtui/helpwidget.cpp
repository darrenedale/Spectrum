//
// Created by darren on 22/04/2021.
//

#include <QLabel>
#include <QScrollArea>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFile>
#include <QSettings>
#include "application.h"
#include "helpwidget.h"

using namespace Spectrum::QtUi;

HelpWidget::HelpWidget(QWidget * parent)
: QWidget(parent)
{
    {
        auto metrics = fontMetrics();
        setMinimumWidth(metrics.horizontalAdvance(QLatin1Char('W')) * 50);
    }

    auto * mainLayout = new QVBoxLayout();

    auto * label = new QLabel(Application::applicationDisplayName());
    label->setTextFormat(Qt::TextFormat::MarkdownText);
    label->setWordWrap(true);

    {
#if defined(WITH_QT_GAMEPAD)
        QFile helpText(QStringLiteral(":/help/en/help-with-gamepad"));
#else
        QFile helpText(QStringLiteral(":/help/en/help-without-gamepad"));
#endif

        const auto helpTextOpened = helpText.open(QIODevice::OpenModeFlag::ReadOnly);
        assert(helpTextOpened);
        label->setText(QString::fromUtf8(helpText.readAll()).arg(Application::instance()->property("version").toString()));
    }

    auto * scroll = new QScrollArea();
    scroll->setWidget(label);
    mainLayout->addWidget(scroll);

    auto * layout = new QHBoxLayout();
    auto * button = new QPushButton(tr("Close"));
    layout->addStretch(10);
    layout->addWidget(button, 1);
    layout->addStretch(10);
    mainLayout->addLayout(layout);

    connect(button, &QPushButton::clicked, this, &HelpWidget::hide);

    setLayout(mainLayout);
}

HelpWidget::~HelpWidget() = default;

void HelpWidget::showEvent(QShowEvent * ev)
{
    QSettings settings;

// We prefix the group name rather than nesting groups because we save as .ini files, and .ini files don't support group
// nesting, so Qt prefixes each setting with the full sub-group path using \ as a separator. It doesn't matter much here
// because only Qt will read these main window settings, but with core emulator settings and potential other UIs we want
// the config file to be parseable without having to handle the Qt-isms
#if defined(USE_QT_5)
    settings.beginGroup(QStringLiteral("qt5-helpwindow"));
#else
    settings.beginGroup(QStringLiteral("qt6-helpwindow"));
#endif

    settings.beginGroup(QStringLiteral("helpWindow"));

    if (const auto size = settings.value(QStringLiteral("size")); size.canConvert<QSize>()) {
        auto geom = geometry();
        geom.setSize(size.value<QSize>());
        setGeometry(geom);
    }

    settings.endGroup();
    QWidget::showEvent(ev);
}

void HelpWidget::closeEvent(QCloseEvent * ev)
{
    QSettings settings;

#if defined(USE_QT_5)
    settings.beginGroup(QStringLiteral("qt5-helpwindow"));
#else
    settings.beginGroup(QStringLiteral("qt6-helpwindow"));
#endif

    settings.setValue(QStringLiteral("size"), size());
    settings.endGroup();
    QWidget::closeEvent(ev);
}
