//
// Created by darren on 02/05/2021.
//

#include <print>

#include "watchesmodel.h"
#include "../../debugger/stringmemorywatch.h"
#include "../../../util/assert.h"
#include "../../../util/debug.h"

using namespace Spectrum::QtUi::Debugger;
using Spectrum::Debugger::StringMemoryWatch;

namespace
{
    constexpr int AddressColumn = 0;
    constexpr int LabelColumn = 1;
    constexpr int TypeColumn = 2;
    constexpr int ValueColumn = 3;
}

WatchesModel::WatchesModel(QObject * parent)
: QAbstractItemModel(parent),
  m_watches()
{}

Qt::ItemFlags WatchesModel::flags(const QModelIndex & idx) const
{
    Qt::ItemFlags flags = Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemNeverHasChildren;

    switch (idx.column()) {
        case AddressColumn:
        case LabelColumn:
            flags |= Qt::ItemFlag::ItemIsEditable;
            break;

        case TypeColumn:
            if(dynamic_cast<StringMemoryWatch *>(watch(idx))) {
                flags |= Qt::ItemFlag::ItemIsEditable;
            }
            break;

        default:
            // no flags to add
            break;
    }

    return flags;
}

QVariant WatchesModel::data(const QModelIndex & idx, const int role) const
{
    if (idx.row() >= rowCount() || idx.column() >= columnCount()) {
        return {};
    }

    switch (role) {
        case Qt::DisplayRole:
            switch (idx.column()) {
                case AddressColumn:
                    return QStringLiteral("0x%1").arg(watch(idx)->address(), 4, 16, QLatin1Char('0'));

                case LabelColumn:
                    return QString::fromStdString(watch(idx)->label());

                case TypeColumn:
                    return QString::fromStdString(watch(idx)->typeName());

                case ValueColumn:
                    return QString::fromStdString(watch(idx)->displayValue());

                [[unlikely]]
                default:
                    sp_assert(false, "reached unreachable code: detected addition of column to WatchesModel not handled in WatchesModel::data()");
            }

        case Qt::ItemDataRole::EditRole:
            switch (idx.column()) {
                case AddressColumn:
                    return watch(idx)->address();

                case LabelColumn:
                    return QString::fromStdString(watch(idx)->label());

                case TypeColumn:
                    // for string watches, enable editing of the string size in the type column
                    if (const auto * strWatch = dynamic_cast<StringMemoryWatch *>(watch(idx)); nullptr != strWatch) {
                        return strWatch->size();
                    }
                    break;

                [[unlikely]]
                default:
                    sp_assert(false, "reached unreachable code: detected addition of column to WatchesModel not handled in WatchesModel::data()");
            }

        default:
            // just to suppress warnings re: uncovered code paths
            break;
    }

    return {};
}

bool WatchesModel::setData(const QModelIndex & idx, const QVariant & data, const int role)
{
    if (role == Qt::ItemDataRole::EditRole) {
        switch (idx.column()) {
            case AddressColumn: {
                sp_assert(watch(idx), "WatchesMode::setData() called with invalid index R{} C{}", idx.row(), idx.column());

                bool ok;
                const auto address = data.toUInt(&ok);

                if (!ok) {
                    Util::debugln("invalid data type - must have unsigned integer for the address");
                    return false;
                }

                if (address + watch(idx)->size() > watch(idx)->memory()->addressableSize()) {
                    Util::debugln("invalid address - watch would overflow addressable memory");
                    return false;
                }

                watch(idx)->setAddress(address);
                return true;
            }

            case LabelColumn:
                sp_assert(watch(idx), "WatchesMode::setData() called with invalid index R{} C{}", idx.row(), idx.column());
                watch(idx)->setLabel(data.toString().toStdString());
                return true;

            case TypeColumn: {
                auto * strWatch = dynamic_cast<StringMemoryWatch *>(watch(idx));
                sp_assert(watch(idx), "WatchesMode::setData() called with invalid index ir index that doesn't identify a StringMemoryWatch (R{} C{})", idx.row(), idx.column());

                bool ok;
                const auto size = data.toInt(&ok);

                if (!ok) {
                    Util::debugln("incorrect data type");
                    return false;
                }

                if (1 > size || strWatch->memory()->addressableSize() < strWatch->address() + size) {
                    Util::debugln("invalid size");
                    return false;
                }

                strWatch->setSize(size);
                return true;
            }

            [[unlikely]]
            default:
                sp_assert(false, "reached unreachable code: detected addition of column to WatchesModel not handled in WatchesModel::setData()");
        }
    }

    return false;
}

WatchesModel::MemoryWatch * WatchesModel::watch(const int idx) const
{
    sp_assert(idx < rowCount(), "out-of-bounds index {} provided to WatchesModel::watch()", idx);
    return m_watches[idx].get();
}

void WatchesModel::removeWatch(WatchesModel::MemoryWatch * watch)
{
    const auto pos = std::ranges::find_if(std::as_const(m_watches), [watch](const auto & modelWatch) -> bool {
        return modelWatch.get() == watch;
    });

    if (pos == m_watches.cend()) {
        return;
    }

    removeWatch(static_cast<int>(std::distance(m_watches.cbegin(), pos)));
}

void WatchesModel::removeWatch(const int idx)
{
    sp_assert(0 <= idx && idx < rowCount(), "out-of-bounds index {} provided to WatchesModel::removeWatch()", idx);
    beginRemoveRows({}, idx, idx);
    m_watches.erase(m_watches.cbegin() + idx);
    endRemoveRows();
}

void WatchesModel::removeAllWatches(const ::Z80::UnsignedWord address)
{
    for (auto it = m_watches.begin(); it != m_watches.end(); ) {
        if ((*it)->address() == address) {
            it = m_watches.erase(it);
        } else {
            ++it;
        }
    }
}

void WatchesModel::clear()
{
    auto n = rowCount();

    if (0 == n) {
        return;
    }

    beginRemoveRows({}, 0, n - 1);
    m_watches.clear();
    endRemoveRows();
}

QVariant WatchesModel::headerData(const int section, const Qt::Orientation orientation, const int role) const
{
    if (Qt::Orientation::Vertical == orientation) {
        return {};
    }

    if (role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
        case AddressColumn:
            return tr("Address");

        case LabelColumn:
            return tr("Label");

        case TypeColumn:
            return tr("Type");

        case ValueColumn:
            return tr("Value");

        default:
            // unreachable code - if we get here columnCount() has been changed without providing the header for the extra column(s)
            [[unlikely]]
            sp_assert(false, "unrecognised section {} provided to WatchesModel::headerData()", section);
    }
}

QModelIndex WatchesModel::index(const int row, const int col, const QModelIndex &) const
{
    return createIndex(row, col);
}

void WatchesModel::update()
{
    if (isEmpty()) {
        return;
    }

    Q_EMIT dataChanged(createIndex(0, 0), createIndex(rowCount() - 1, 0));
}
