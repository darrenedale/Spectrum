//
// Created by darren on 27/03/2021.
//

#include <QToolButton>
#include <QUuid>
#include <QVariant>
#include "pokeswidget.h"

using namespace Spectrum::Qt;

namespace
{
    constexpr const char * UuidPropertyName = "pokeUuid";
}

PokesWidget::PokesWidget(QWidget * parent)
: QWidget(parent),
  m_layout(),
  m_pokes()
{
    m_layout.addStretch(10);
    m_layout.setSpacing(2);
    setLayout(&m_layout);
}

PokesWidget::~PokesWidget() = default;

void PokesWidget::addPoke(Spectrum::PokeDefinition && poke)
{
    auto name = QString::fromStdString(poke.name());
    auto uuid = QUuid::createUuid().toString();
    m_pokes.insert({uuid.toStdString(), std::move(poke)});
    addPokeWidget(name, uuid);
}

void PokesWidget::addPoke(const Spectrum::PokeDefinition & poke)
{
    auto uuid = QUuid::createUuid().toString();
    m_pokes.insert({uuid.toStdString(), poke});
    addPokeWidget(QString::fromStdString(poke.name()), uuid);
}

void PokesWidget::addPokeWidget(const QString & name, const QString & uuid)
{
    auto * widget = new QWidget(this);
    auto * layout = new QHBoxLayout(widget);
    layout->addWidget(new QLabel(name, widget), 10);

    auto * apply = new QToolButton(widget);
    apply->setIcon(QIcon::fromTheme("dialog-ok-apply"));
    apply->setText(tr("Apply"));
    apply->setToolTip(tr("Apply the poke '%1'.").arg(name));
    layout->addWidget(apply);

    auto * undo = new QToolButton(widget);
    undo->setIcon(QIcon::fromTheme("edit-undo"));
    undo->setText(tr("Undo"));
    undo->setToolTip(tr("Undo the application of the poke '%1'.").arg(name));
    layout->addWidget(undo);

    auto * remove = new QToolButton(widget);
    remove->setIcon(QIcon::fromTheme("list-remove"));
    remove->setText(tr("Remove"));
    remove->setToolTip(tr("Remove the poke '%1' from this list.").arg(name));
    layout->addWidget(remove);

    widget->setProperty(UuidPropertyName, uuid);
    widget->setLayout(layout);

    connect(apply, &QToolButton::clicked, [this, uuid]() {
        applyPokeTriggered(uuid);
    });

    connect(undo, &QToolButton::clicked, [this, uuid]() {
        undoPokeTriggered(uuid);
    });

    connect(remove, &QToolButton::clicked, [this, uuid]() {
        removePokeTriggered(uuid);
    });

    m_layout.insertWidget(m_layout.count() - 1, widget);
}

void PokesWidget::removePoke(int idx)
{
    if (0 > idx || pokeCount() <= idx) {
        std::cerr << "index " << idx << " out of bounds.\n";
        return;
    }

    auto * item = m_layout.itemAt(idx);

    if (!item->widget()) {
        std::cerr << "item at index " << idx << " is not a widget.\n";
        return;
    }

    auto uuid = item->widget()->property(UuidPropertyName).toString();

    if (uuid.isEmpty()) {
        std::cerr << "item at index " << idx << " has an empty UUID.\n";
        return;
    }

    removePoke(uuid, item->widget());
}

void PokesWidget::undoPoke(const QString & uuid)
{
    std::cerr << "Not yet implemented.\n";
}

void PokesWidget::removePoke(const QString & uuid, QWidget * widget)
{
    if (!widget) {
        widget = findPokeWidget(uuid);
    }

    m_pokes.erase(uuid.toStdString());
    m_layout.removeWidget(widget);
    delete widget;
}

QWidget * PokesWidget::findPokeWidget(const QString & uuid) const
{
    auto count = m_layout.count() - 1;

    while (0 <= count) {
        auto * widget = m_layout.itemAt(count)->widget();
        --count;

        if (!widget) {
            continue;
        }

        if (widget->property(UuidPropertyName) == uuid) {
            return widget;
        }
    }

    return nullptr;
}

void PokesWidget::undoPokeTriggered(const QString & uuid)
{
    undoPoke(uuid);
}

void PokesWidget::removePokeTriggered(const QString & uuid)
{
    removePoke(uuid);
}

void PokesWidget::applyPokeTriggered(const QString & uuid)
{
    auto stdUuid = uuid.toStdString();

    if (!m_pokes.contains(stdUuid)) {
        std::cerr << "Poke with UUID " << stdUuid << " not found\n";
        return;
    }

    const auto & poke = m_pokes[stdUuid];
    Q_EMIT applyPokeRequested(poke);
}
