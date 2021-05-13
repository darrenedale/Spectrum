//
// Created by darren on 03/05/2021.
//

#include <QStringLiteral>
#include <QClipboard>
#include "watchescontextmenu.h"
#include "../application.h"

using namespace Spectrum::QtUi::Debugger;
using Spectrum::Debugger::MemoryWatch;
using Spectrum::Debugger::StringMemoryWatch;
using Spectrum::Debugger::IntegerMemoryWatchBase;

WatchesContextMenu::WatchesContextMenu(WatchesModel * model, const QModelIndex & idx, QWidget * parent)
: QMenu(parent),
  m_model(model),
  m_index(idx)
{
    assert(model);
    // for safety, the menu is closed if the model is destroyed or changes - in the case, the index may become invalid but will still report validity, and any
    // attempt to fetch the watch from the model could dereference an invalid pointer
    connect(model, &QAbstractItemModel::destroyed, this, &QMenu::close);
    connect(model, &QAbstractItemModel::dataChanged, this, &QMenu::close);

    // if the menu subject is a valid watch, add menu items specific to that watch
    if (idx.isValid()) {
        addItemsForWatch(*model->watch(idx));
    }

    addSeparator();
    auto * action = addAction(Application::icon(QStringLiteral("edit-clear-list"), QStringLiteral("clear")), tr("Clear all watches"), this, &WatchesContextMenu::onClearTriggered);
    action->setToolTip(tr("Remove all watches from the list."));

    // if the model has no watches disable the "clear" action - it will do nothing
    if (0 == model->rowCount()) {
        action->setEnabled(false);
    }
}

void WatchesContextMenu::addItemsForWatch(MemoryWatch & watch)
{
    const auto title = watch.label().empty()
                       ? QStringLiteral("%1 @ 0x%2").arg(QString::fromStdString(watch.typeName())).arg(watch.address(), 4, 16, QLatin1Char('0'))
                       : QString::fromStdString(watch.label());
    addSection(title);

    // add items for the specific type of watch
    if (auto * strWatch = dynamic_cast<StringMemoryWatch *>(&watch); strWatch) {
        addItemsForStringWatch(*strWatch);
    } else if (auto * intWatch = dynamic_cast<IntegerMemoryWatchBase *>(&watch); intWatch) {
        addItemsForIntegerWatch(*intWatch);
    }

    // add items valid for any type of watch
    auto * action = addAction(Application::icon(QStringLiteral("list-clear"), QStringLiteral("remove")), tr("Remove watch"), this, &WatchesContextMenu::onRemoveWatchTriggered);
    action->setToolTip(tr("Remove the watch %1.").arg(title));

    action = addAction(tr("Locate in memory view"), this, &WatchesContextMenu::onLocateInMemoryViewTriggered);
    action->setToolTip(tr("Locate the address 0x%1 in the memory view.").arg(watch.address(), 4, 16, QLatin1Char('0')));

    action = addAction(Application::icon(QStringLiteral("edit-copy"), QStringLiteral("copy")), tr("Copy value"), this, &WatchesContextMenu::onCopyValueTriggered);
    action->setToolTip(tr("Copy the current value of the watch to the clipboard."));

    action = addAction(Application::icon(QStringLiteral("edit-copy"), QStringLiteral("copy")), tr("Copy address"), this, &WatchesContextMenu::onCopyAddressTriggered);
    action->setToolTip(tr("Copy the (hex) address of the watch to the clipboard."));
}

void WatchesContextMenu::addItemsForStringWatch(StringMemoryWatch & watch)
{
    auto * subMenu = addMenu(tr("Character set"));
    subMenu->setToolTip(tr("Change the character set in which to display the string."));
    subMenu->addAction(tr("Spectrum"), this, &WatchesContextMenu::onCharacterSetTriggered<StringMemoryWatch::CharacterEncoding::Spectrum>);
    subMenu->addAction(tr("ASCII"), this, &WatchesContextMenu::onCharacterSetTriggered<StringMemoryWatch::CharacterEncoding::Ascii>);
}

void WatchesContextMenu::addItemsForIntegerWatch(IntegerMemoryWatchBase & watch)
{
    QAction * action;

    // for watches of > 1 byte, enable byte-order switching
    if (1 < watch.size()) {
        // add options to swap byte order
        auto * subMenu = addMenu(tr("Byte order"));
        auto * group = new QActionGroup(subMenu);
        action = subMenu->addAction(tr("Little-endian (Z80)"), this, &WatchesContextMenu::onByteOrderTriggered<IntegerMemoryWatchBase::ByteOrder::little>);
        action->setCheckable(true);
        action->setChecked(IntegerMemoryWatchBase::ByteOrder::little == watch.byteOrder());
        group->addAction(action);

        action = subMenu->addAction(tr("Big-endian"), this, &WatchesContextMenu::onByteOrderTriggered<IntegerMemoryWatchBase::ByteOrder::big>);
        action->setCheckable(true);
        action->setChecked(IntegerMemoryWatchBase::ByteOrder::big == watch.byteOrder());
        group->addAction(action);
    }

    // we support display in dec, hex and octal
    auto * subMenu = addMenu(tr("Numeric base"));
    auto * group = new QActionGroup(subMenu);
    subMenu->setToolTip(tr("Change the numeric base used to display the watched value."));

    action = subMenu->addAction(tr("Decimal"), this, &WatchesContextMenu::onNumericBaseTriggered<IntegerMemoryWatchBase::Base::Decimal>);
    action->setCheckable(true);
    action->setChecked(IntegerMemoryWatchBase::Base::Decimal == watch.base());
    group->addAction(action);

    action = subMenu->addAction(tr("Hexadecimal"), this, &WatchesContextMenu::onNumericBaseTriggered<IntegerMemoryWatchBase::Base::Hex>);
    action->setCheckable(true);
    action->setChecked(IntegerMemoryWatchBase::Base::Hex == watch.base());
    group->addAction(action);

    action = subMenu->addAction(tr("Octal"), this, &WatchesContextMenu::onNumericBaseTriggered<IntegerMemoryWatchBase::Base::Octal>);
    action->setCheckable(true);
    action->setChecked(IntegerMemoryWatchBase::Base::Octal == watch.base());
    group->addAction(action);

    action = subMenu->addAction(tr("Binary"), this, &WatchesContextMenu::onNumericBaseTriggered<IntegerMemoryWatchBase::Base::Binary>);
    action->setCheckable(true);
    action->setChecked(IntegerMemoryWatchBase::Base::Binary == watch.base());
    group->addAction(action);
}

void WatchesContextMenu::onRemoveWatchTriggered()
{
    m_model->removeWatch(m_index);
}

void WatchesContextMenu::onCopyValueTriggered()
{
    Application::clipboard()->setText(QStringLiteral("%1").arg(QString::fromStdString(m_model->watch(m_index)->displayValue())));
}

void WatchesContextMenu::onCopyAddressTriggered()
{
    Application::clipboard()->setText(QStringLiteral("%1").arg(m_model->watch(m_index)->address(), 4, 16, QLatin1Char('0')));
}

void WatchesContextMenu::onLocateInMemoryViewTriggered()
{
    Q_EMIT locateInMemoryView(m_model->watch(m_index)->address());
}

void WatchesContextMenu::onClearTriggered()
{
    m_model->clear();
}

template<StringMemoryWatch::CharacterEncoding charset>
void WatchesContextMenu::onCharacterSetTriggered()
{
    auto * watch = dynamic_cast<StringMemoryWatch *>(m_model->watch(m_index));
    assert(watch);
    watch->setCharacterEncoding(charset);
    m_model->dataChanged(m_index, m_index);
}

template<IntegerMemoryWatchBase::ByteOrder byteOrder>
void WatchesContextMenu::onByteOrderTriggered()
{
    auto * watch = dynamic_cast<IntegerMemoryWatchBase *>(m_model->watch(m_index));
    assert(watch);
    watch->setByteOrder(byteOrder);
    m_model->dataChanged(m_index, m_index);
}

template<IntegerMemoryWatchBase::Base base>
void WatchesContextMenu::onNumericBaseTriggered()
{
    auto * watch = dynamic_cast<IntegerMemoryWatchBase *>(m_model->watch(m_index));
    assert(watch);
    watch->setBase(base);
    m_model->dataChanged(m_index, m_index);
}
