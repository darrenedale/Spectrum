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
#include "pokeswidget.h"
#include "pokeswidgetitem.h"
#include "application.h"
#include "threadpauser.h"
#include "../io/pokfilereader.h"
#include "../../util/debug.h"

using namespace Spectrum::QtUi;
using namespace Spectrum::Io;

namespace
{
    constexpr const char * UuidPropertyName = "pokeUuid";
}

PokesWidget::PokesWidget(QWidget * parent)
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
    connect(&m_loadPokes, &QAction::triggered, this, &PokesWidget::loadPokesTriggered);
    connect(&m_clearPokes, &QAction::triggered, this, &PokesWidget::clearPokesTriggered);
}

PokesWidget::~PokesWidget() = default;

void PokesWidget::showEvent(QShowEvent *)
{
    QSettings settings;
    settings.beginGroup("pokesWidget");
    m_lastPokeLoadDir = settings.value(QStringLiteral("lastPokeLoadDir")).toString();
    settings.endGroup();
}

void PokesWidget::hideEvent(QHideEvent * event)
{
    QSettings settings;
    settings.beginGroup("pokesWidget");
    settings.setValue(QStringLiteral("lastPokeLoadDir"), m_lastPokeLoadDir);
    settings.endGroup();
    QWidget::hideEvent(event);
}

void PokesWidget::loadPokes(const QString & fileName)
{
    PokFileReader reader(fileName.toStdString());

    if (!reader.isValid()) {
        Application::showMessage(tr("Poke file %1 could not be opened.").arg(fileName));
        return;
    }

    std::vector<PokeDefinition> pokes;

    while (reader.hasMorePokes()) {
        auto poke = reader.nextPoke();

        if (!poke) {
            Application::showMessage(tr("Error reading pokes from %1.").arg(fileName));
            return;
        }

        pokes.push_back(std::move(*poke));
    }

    for (const auto & poke : pokes) {
        addPoke(poke);
    }

    Application::showMessage(tr("Read %1 pokes from %2.").arg(pokes.size()).arg(fileName));
}

void PokesWidget::setActionIconSize(const QSize & size)
{
    m_actionIconSize = size;

    m_toolBar.setIconSize(size);
    auto count = m_layout.count() - 1;

    while (1 <= count) {
        auto * widget = qobject_cast<PokesWidgetItem *>(m_layout.itemAt(count)->widget());
        --count;

        if (!widget) {
            continue;
        }

        widget->setIconSize(size);
    }
}

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

void PokesWidget::clearPokes()
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

void PokesWidget::addPokeWidget(const QString & name, const QString & uuid)
{
    auto * item = new PokesWidgetItem(name, uuid, this);
    item->setIconSize(actionIconSize());

    connect(item, &PokesWidgetItem::activationRequested, this, &PokesWidget::applyPokeTriggered);
    connect(item, &PokesWidgetItem::deactivationRequested, this, &PokesWidget::undoPokeTriggered);
    connect(item, &PokesWidgetItem::removeClicked, this, &PokesWidget::removePokeTriggered);

    m_layout.insertWidget(m_layout.count() - 1, item);
}

void PokesWidget::removePoke(int idx)
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
    // NOTE first child widget is the "toolbar", last child widget is the spacer
    auto count = m_layout.count() - 1;

    while (1 <= count) {
        auto * widget = qobject_cast<PokesWidgetItem *>(m_layout.itemAt(count)->widget());
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

void PokesWidget::loadPokesTriggered()
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

void PokesWidget::clearPokesTriggered()
{
    clearPokes();
}

void PokesWidget::undoPokeTriggered(const QString & uuid)
{
    auto stdUuid = uuid.toStdString();

    if (!m_pokes.contains(stdUuid)) {
        Util::debug << "Poke with UUID " << stdUuid << " not found\n";
        return;
    }

    const auto & poke = m_pokes[stdUuid];
    Q_EMIT undoPokeRequested(poke);
}

void PokesWidget::removePokeTriggered(const QString & uuid)
{
    removePoke(uuid);
}

void PokesWidget::applyPokeTriggered(const QString & uuid)
{
    auto stdUuid = uuid.toStdString();

    if (!m_pokes.contains(stdUuid)) {
        Util::debug << "Poke with UUID " << stdUuid << " not found\n";
        return;
    }

    const auto & poke = m_pokes[stdUuid];
    Q_EMIT applyPokeRequested(poke);
}
