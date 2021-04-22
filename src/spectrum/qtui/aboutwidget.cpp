//
// Created by darren on 22/04/2021.
//

#include "aboutwidget.h"
#include "application.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

using namespace Spectrum::QtUi;

Spectrum::QtUi::AboutWidget::AboutWidget(QWidget * parent)
: QWidget(parent)
{
    auto * app = Application::instance();
    assert(app);
    auto font = this->font();
    auto * mainLayout = new QVBoxLayout();

    auto * label = new QLabel(Application::applicationDisplayName());
    font.setPointSizeF(font.pointSizeF() * 1.2);
    font.setBold(true);
    label->setAlignment(Qt::AlignmentFlag::AlignCenter);
    label->setFont(font);
    mainLayout->addWidget(label);

    label = new QLabel(tr("Version %1").arg(app->property("version").toString()));
    label->setAlignment(Qt::AlignmentFlag::AlignCenter);
    mainLayout->addWidget(label);

    label = new QLabel(tr("By %1").arg(app->property("author").toString()));
    label->setAlignment(Qt::AlignmentFlag::AlignCenter);
    mainLayout->addWidget(label);

    mainLayout->addStretch(10);

    auto * layout = new QHBoxLayout();
    auto * button = new QPushButton(tr("Close"));
    layout->addStretch(10);
    layout->addWidget(button, 1);
    layout->addStretch(10);
    mainLayout->addLayout(layout);

    connect(button, &QPushButton::clicked, this, &AboutWidget::hide);

    setLayout(mainLayout);
}

Spectrum::QtUi::AboutWidget::~AboutWidget() = default;
