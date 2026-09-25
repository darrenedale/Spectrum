//
// Created by darren on 22/04/2021.
//

#include "aboutwidget.h"
#include "application.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>

using namespace Spectrum::QtUi;

AboutWidget::AboutWidget(QWidget * parent)
: QWidget(parent)
{
    auto * app = Application::instance();
    assert(app);
    auto unitWidth = fontMetrics().horizontalAdvance(QLatin1Char('W'));
    setMinimumWidth(unitWidth * 30);

    auto * mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(unitWidth);

    auto * layout = new QHBoxLayout();
    layout->setSpacing(unitWidth);
    layout->addStretch(10);

    auto * label = new QLabel();
    label->setPixmap(Application::icon("app").pixmap(64));
    layout->addWidget(label, 0);

    label = new QLabel(Application::applicationDisplayName());
    label->setAlignment(Qt::AlignmentFlag::AlignCenter);

    {
        auto font = this->font();
        font.setPointSizeF(font.pointSizeF() * 1.2);
        font.setBold(true);
        label->setFont(font);
    }

    layout->addWidget(label, 0);
    layout->addStretch(10);
    mainLayout->addLayout(layout);

    label = new QLabel(tr("Version %1 by %2").arg(app->property("version").toString(), app->property("author").toString()));
    label->setAlignment(Qt::AlignmentFlag::AlignCenter);
    mainLayout->addWidget(label);

    mainLayout->addStretch(10);

    layout = new QHBoxLayout();
    layout->setSpacing(unitWidth);

    auto * button = new QPushButton(tr("Close"));
    layout->addStretch(10);
    layout->addWidget(button, 1);
    layout->addStretch(10);
    mainLayout->addLayout(layout);

    connect(button, &QPushButton::clicked, this, &AboutWidget::hide);

    setLayout(mainLayout);
}

AboutWidget::~AboutWidget() = default;

void AboutWidget::showEvent(QShowEvent * ev)
{
    QSettings settings;

// We prefix the group name rather than nesting groups because we save as .ini files, and .ini files don't support group
// nesting, so Qt prefixes each setting with the full sub-group path using \ as a separator. It doesn't matter much here
// because only Qt will read these main window settings, but with core emulator settings and potential other UIs we want
// the config file to be parseable without having to handle the Qt-isms
#if defined(USE_QT_5)
    settings.beginGroup(QStringLiteral("qt5-aboutwindow"));
#else
    settings.beginGroup(QStringLiteral("qt6-aboutwindow"));
#endif

    if (const auto size = settings.value(QStringLiteral("size")); size.canConvert<QSize>()) {
        auto geom = geometry();
        geom.setSize(size.value<QSize>());
        setGeometry(geom);
    }

    settings.endGroup();
    QWidget::showEvent(ev);
}

void AboutWidget::closeEvent(QCloseEvent * ev)
{
    QSettings settings;

#if defined(USE_QT_5)
    settings.beginGroup(QStringLiteral("qt5-aboutwindow"));
#else
    settings.beginGroup(QStringLiteral("qt6-aboutwindow"));
#endif

    settings.setValue(QStringLiteral("size"), size());
    settings.endGroup();
    QWidget::closeEvent(ev);
}
