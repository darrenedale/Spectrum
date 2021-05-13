//
// Created by darren on 27/03/2021.
//

#include <cmath>
#include <QApplication>
#include <QStyle>
#include <QLabel>
#include <QFileDialog>
#include <QToolBar>
#include <QUuid>
#include <QRegularExpression>
#include <QVariant>
#include <QSettings>
#include "pokesview.h"
#include "pokesviewitem.h"
#include "application.h"
#include "mainwindow.h"
#include "threadpauser.h"
#include "../io/pokfilereader.h"
#include "../../util/debug.h"

using namespace Spectrum::QtUi;
using namespace Spectrum::Io;

namespace
{
    constexpr const char * UuidPropertyName = "pokeUuid";
}

PokesView::PokesView(QWidget * parent)
: QWidget(parent),
  m_layout(),
  m_loadPokes(QIcon::fromTheme(QStringLiteral("document-open"), Application::icon(QStringLiteral("open"))), tr("Load pokes")),
  m_clearPokes(QIcon::fromTheme(QStringLiteral("edit-clear-list"), Application::icon(QStringLiteral("clear"))), tr("Clear pokes")),
  m_pokes(),
  m_actionIconSize()
{
    int iconSize = style()->pixelMetric(QStyle::PM_ButtonIconSize, nullptr, this);
    m_actionIconSize.setWidth(iconSize);
    m_actionIconSize.setHeight(iconSize);

    m_toolBar.setIconSize(m_actionIconSize);
    m_toolBar.addStretch(10);
    m_toolBar.addAction(&m_loadPokes);
    m_toolBar.addAction(&m_clearPokes);
    m_layout.addWidget(&m_toolBar);

    m_layout.addStretch(10);
    m_layout.setSpacing(0);

    setLayout(&m_layout);
    connect(&m_loadPokes, &QAction::triggered, this, &PokesView::loadPokesTriggered);
    connect(&m_clearPokes, &QAction::triggered, this, &PokesView::clearPokesTriggered);
}

PokesView::~PokesView() = default;

void PokesView::showEvent(QShowEvent *)
{
    QSettings settings;
    settings.beginGroup("pokesWidget");
    m_lastPokeLoadDir = settings.value(QStringLiteral("lastPokeLoadDir")).toString();
    settings.endGroup();
}

void PokesView::hideEvent(QHideEvent * event)
{
    QSettings settings;
    settings.beginGroup("pokesWidget");
    settings.setValue(QStringLiteral("lastPokeLoadDir"), m_lastPokeLoadDir);
    settings.endGroup();
    QWidget::hideEvent(event);
}

void PokesView::loadPokes(const QString & fileName)
{
    PokFileReader reader(fileName.toStdString());

    if (!reader.isValid()) {
        Application::showNotification(tr("Poke file %1 could not be opened.").arg(fileName));
        return;
    }

    std::vector<PokeDefinition> pokes;

    while (reader.hasMorePokes()) {
        auto poke = reader.nextPoke();

        if (!poke) {
            Application::showNotification(tr("Error reading pokes from %1.").arg(fileName));
            return;
        }

        pokes.push_back(std::move(*poke));
    }

    for (const auto & poke : pokes) {
        addPoke(poke);
    }

    Application::showNotification(tr("Read %1 pokes from %2.").arg(pokes.size()).arg(fileName));
}

void PokesView::setActionIconSize(const QSize & size)
{
    m_actionIconSize = size;

    m_toolBar.setIconSize(size);
    auto count = m_layout.count() - 1;

    while (1 <= count) {
        auto * widget = qobject_cast<PokesViewItem *>(m_layout.itemAt(count)->widget());
        --count;

        if (!widget) {
            continue;
        }

        widget->setIconSize(size);
    }
}

void PokesView::addPoke(Spectrum::PokeDefinition && poke)
{
    auto name = QString::fromStdString(poke.name());
    auto uuid = QUuid::createUuid().toString();
    m_pokes.insert({uuid.toStdString(), std::move(poke)});
    addPokeWidget(name, uuid);
}

void PokesView::addPoke(const Spectrum::PokeDefinition & poke)
{
    auto uuid = QUuid::createUuid().toString();
    m_pokes.insert({uuid.toStdString(), poke});
    addPokeWidget(QString::fromStdString(poke.name()), uuid);
}

void PokesView::clearPokes()
{
    m_pokes.clear();
    // NOTE first child widget is the "toolbar", last child widget is the spacer
    auto count = m_layout.count() - 1;

    while (1 <= count) {
        auto * widget = m_layout.itemAt(count)->widget();
        --count;

        if (!widget) {
            continue;
        }

        widget->deleteLater();
    }
}

void PokesView::addPokeWidget(const QString & name, const QString & uuid)
{
    auto * item = new PokesViewItem(name, uuid, this);
    item->setIconSize(actionIconSize());

    connect(item, &PokesViewItem::activationRequested, this, &PokesView::applyPokeTriggered);
    connect(item, &PokesViewItem::deactivationRequested, this, &PokesView::undoPokeTriggered);
    connect(item, &PokesViewItem::removeClicked, this, &PokesView::removePokeTriggered);

    m_layout.insertWidget(m_layout.count() - 1, item);
}

void PokesView::removePoke(int idx)
{
    if (0 > idx || pokeCount() <= idx) {
        Util::debug << "index " << idx << " out of bounds.\n";
        return;
    }

    auto * item = m_layout.itemAt(idx);

    if (!item->widget()) {
        Util::debug << "item at index " << idx << " is not a widget.\n";
        return;
    }

    auto uuid = item->widget()->property(UuidPropertyName).toString();

    if (uuid.isEmpty()) {
        Util::debug << "item at index " << idx << " has an empty UUID.\n";
        return;
    }

    removePoke(uuid, item->widget());
}

void PokesView::removePoke(const QString & uuid, QWidget * widget)
{
    if (!widget) {
        widget = findPokeWidget(uuid);
    }

    m_pokes.erase(uuid.toStdString());
    m_layout.removeWidget(widget);
    delete widget;
}

QWidget * PokesView::findPokeWidget(const QString & uuid) const
{
    // NOTE first child widget is the "toolbar", last child widget is the spacer
    auto count = m_layout.count() - 1;

    while (1 <= count) {
        auto * widget = qobject_cast<PokesViewItem *>(m_layout.itemAt(count)->widget());
        --count;

        if (!widget) {
            continue;
        }

        if (widget->uuid() == uuid) {
            return widget;
        }
    }

    return nullptr;
}

void PokesView::loadPokesTriggered()
{
    static QStringList filters;
    static QString lastFilter;

    auto & thread = Application::instance()->mainWindow().spectrumThread();
    ThreadPauser pauser(thread);

    if(filters.isEmpty()) {
        filters << tr("POK Poke files (*.pok)");
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Load pokes"), m_lastPokeLoadDir, filters.join(";;"), &lastFilter);

    if(fileName.isEmpty()) {
        return;
    }

    m_lastPokeLoadDir = QFileInfo(fileName).path();
    auto format = lastFilter;

    if (!format.isEmpty()) {
        if (auto matches = QRegularExpression(R"(^.*\(\*\.([a-zA-Z0-9_-]+)\)$)").match(format); matches.hasMatch()) {
            format = matches.captured(1).toLower();
        } else {
            format.clear();
        }
    }

    loadPokes(fileName);
}

void PokesView::clearPokesTriggered()
{
    clearPokes();
}

void PokesView::undoPokeTriggered(const QString & uuid)
{
    auto stdUuid = uuid.toStdString();

    if (!m_pokes.contains(stdUuid)) {
        Util::debug << "Poke with UUID " << stdUuid << " not found\n";
        return;
    }

    const auto & poke = m_pokes[stdUuid];
    Q_EMIT undoPokeRequested(poke);
}

void PokesView::removePokeTriggered(const QString & uuid)
{
    removePoke(uuid);
}

void PokesView::applyPokeTriggered(const QString & uuid)
{
    auto stdUuid = uuid.toStdString();

    if (!m_pokes.contains(stdUuid)) {
        Util::debug << "Poke with UUID " << stdUuid << " not found\n";
        return;
    }

    const auto & poke = m_pokes[stdUuid];
    Q_EMIT applyPokeRequested(poke);
}
