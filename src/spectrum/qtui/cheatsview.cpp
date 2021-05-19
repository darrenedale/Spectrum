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
#include "cheatsview.h"
#include "cheatsviewitem.h"
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

CheatsView::CheatsView(QWidget * parent)
: QWidget(parent),
  m_layout(),
  m_loadCheats(QIcon::fromTheme(QStringLiteral("document-open"), Application::icon(QStringLiteral("open"))), tr("Load cheats")),
  m_clearCheats(QIcon::fromTheme(QStringLiteral("edit-clear-list"), Application::icon(QStringLiteral("clear"))), tr("Clear cheats")),
  m_cheats(),
  m_actionIconSize()
{
    int iconSize = style()->pixelMetric(QStyle::PM_ButtonIconSize, nullptr, this);
    m_actionIconSize.setWidth(iconSize);
    m_actionIconSize.setHeight(iconSize);

    m_toolBar.setIconSize(m_actionIconSize);
    m_toolBar.addStretch(10);
    m_toolBar.addAction(&m_loadCheats);
    m_toolBar.addAction(&m_clearCheats);
    m_layout.addWidget(&m_toolBar);

    m_layout.addStretch(10);
    m_layout.setSpacing(0);

    setLayout(&m_layout);
    connect(&m_loadCheats, &QAction::triggered, this, &CheatsView::loadCheatsTriggered);
    connect(&m_clearCheats, &QAction::triggered, this, &CheatsView::clearCheatsTriggered);

    loadSettings();
}

CheatsView::~CheatsView()
{
    saveSettings();
}

void CheatsView::loadSettings()
{
    QSettings settings;
    settings.beginGroup("cheatsWidget");
    m_lastLoadDir = settings.value(QStringLiteral("lastCheatLoadDir")).toString();
    settings.endGroup();
}

void CheatsView::saveSettings()
{
    QSettings settings;
    settings.beginGroup("cheatsWidget");
    settings.setValue(QStringLiteral("lastCheatLoadDir"), m_lastLoadDir);
    settings.endGroup();
}

void CheatsView::loadCheats(const QString & fileName)
{
    PokFileReader reader(fileName.toStdString());

    if (!reader.isValid()) {
        Application::showNotification(tr("Cheat file %1 could not be opened.").arg(fileName));
        return;
    }

    std::vector<PokeDefinition> pokes;

    while (reader.hasMorePokes()) {
        auto poke = reader.nextPoke();

        if (!poke) {
            Application::showNotification(tr("Error reading cheats from %1.").arg(fileName));
            return;
        }

        pokes.push_back(std::move(*poke));
    }

    for (const auto & poke : pokes) {
        addCheat(poke);
    }

    Application::showNotification(tr("Read %1 cheats from %2.").arg(pokes.size()).arg(fileName));
}

void CheatsView::setActionIconSize(const QSize & size)
{
    m_actionIconSize = size;

    m_toolBar.setIconSize(size);
    auto count = m_layout.count() - 1;

    while (1 <= count) {
        auto * widget = qobject_cast<CheatsViewItem *>(m_layout.itemAt(count)->widget());
        --count;

        if (!widget) {
            continue;
        }

        widget->setIconSize(size);
    }
}

void CheatsView::addCheat(Spectrum::PokeDefinition && poke)
{
    auto name = QString::fromStdString(poke.name());
    auto uuid = QUuid::createUuid().toString();
    m_cheats.insert({uuid.toStdString(), std::move(poke)});
    addCheatWidget(name, uuid);
}

void CheatsView::addCheat(const Spectrum::PokeDefinition & poke)
{
    auto uuid = QUuid::createUuid().toString();
    m_cheats.insert({uuid.toStdString(), poke});
    addCheatWidget(QString::fromStdString(poke.name()), uuid);
}

void CheatsView::clearCheats()
{
    m_cheats.clear();
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

void CheatsView::addCheatWidget(const QString & name, const QString & uuid)
{
    auto * item = new CheatsViewItem(name, uuid, this);
    item->setIconSize(actionIconSize());

    connect(item, &CheatsViewItem::activationRequested, this, &CheatsView::applyCheatTriggered);
    connect(item, &CheatsViewItem::deactivationRequested, this, &CheatsView::undoCheatTriggered);
    connect(item, &CheatsViewItem::removeClicked, this, &CheatsView::removeCheatTriggered);

    m_layout.insertWidget(m_layout.count() - 1, item);
}

void CheatsView::removeCheat(int idx)
{
    if (0 > idx || cheatCount() <= idx) {
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

    removeCheat(uuid, item->widget());
}

void CheatsView::removeCheat(const QString & uuid, QWidget * widget)
{
    if (!widget) {
        widget = findCheatWidget(uuid);
    }

    m_cheats.erase(uuid.toStdString());
    m_layout.removeWidget(widget);
    delete widget;
}

QWidget * CheatsView::findCheatWidget(const QString & uuid) const
{
    // NOTE first child widget is the "toolbar", last child widget is the spacer
    auto count = m_layout.count() - 1;

    while (1 <= count) {
        auto * widget = qobject_cast<CheatsViewItem *>(m_layout.itemAt(count)->widget());
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

void CheatsView::loadCheatsTriggered()
{
    static QStringList filters;
    static QString lastFilter;

    auto & thread = Application::instance()->mainWindow().spectrumThread();
    ThreadPauser pauser(thread);

    if(filters.isEmpty()) {
        filters << tr("POK Poke files (*.pok)");
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Load cheats"), m_lastLoadDir, filters.join(";;"), &lastFilter);

    if(fileName.isEmpty()) {
        return;
    }

    m_lastLoadDir = QFileInfo(fileName).path();
    auto format = lastFilter;

    if (!format.isEmpty()) {
        if (auto matches = QRegularExpression(R"(^.*\(\*\.([a-zA-Z0-9_-]+)\)$)").match(format); matches.hasMatch()) {
            format = matches.captured(1).toLower();
        } else {
            format.clear();
        }
    }

    loadCheats(fileName);
}

void CheatsView::clearCheatsTriggered()
{
    clearCheats();
}

void CheatsView::undoCheatTriggered(const QString & uuid)
{
    auto stdUuid = uuid.toStdString();

    if (!m_cheats.contains(stdUuid)) {
        Util::debug << "Poke with UUID " << stdUuid << " not found\n";
        return;
    }

    const auto & poke = m_cheats[stdUuid];
    Q_EMIT undoCheatRequested(poke);
}

void CheatsView::removeCheatTriggered(const QString & uuid)
{
    removeCheat(uuid);
}

void CheatsView::applyCheatTriggered(const QString & uuid)
{
    auto stdUuid = uuid.toStdString();

    if (!m_cheats.contains(stdUuid)) {
        Util::debug << "Poke with UUID " << stdUuid << " not found\n";
        return;
    }

    const auto & poke = m_cheats[stdUuid];
    Q_EMIT applyCheatRequested(poke);
}
