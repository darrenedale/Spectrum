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
        QFile helpText(":/help/en/help");
        helpText.open(QIODevice::OpenModeFlag::ReadOnly);
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

    connect(button, &QPushButton::clicked, this, &AboutWidget::hide);

    setLayout(mainLayout);
}

HelpWidget::~HelpWidget() = default;

void HelpWidget::showEvent(QShowEvent * ev)
{
    QSettings settings;
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
    settings.beginGroup(QStringLiteral("helpWindow"));
    settings.setValue(QStringLiteral("size"), size());
    settings.endGroup();
    QWidget::closeEvent(ev);
}

