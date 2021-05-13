//
// Created by darren on 02/05/2021.
//

#include "watchesmodel.h"
#include "../../debugger/stringmemorywatch.h"
#include "../../../util/debug.h"

using namespace Spectrum::QtUi::Debugger;
using Spectrum::Debugger::StringMemoryWatch;

namespace
{
    constexpr const int AddressColumn = 0;
    constexpr const int LabelColumn = 1;
    constexpr const int TypeColumn = 2;
    constexpr const int ValueColumn = 3;
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
    }

    return flags;
}

QVariant WatchesModel::data(const QModelIndex & idx, int role) const
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
            }
            // unreachable code - if we get here columnCount() has been changed without providing the data for the extra column(s)
            assert(false);
            break;

        case Qt::ItemDataRole::EditRole:
            switch (idx.column()) {
                case AddressColumn:
                    return watch(idx)->address();

                case LabelColumn:
                    return QString::fromStdString(watch(idx)->label());

                case TypeColumn:
                    // for string watches, enable editing of the string size in the type column
                    if (auto * strWatch = dynamic_cast<StringMemoryWatch *>(watch(idx)); nullptr != strWatch) {
                        return strWatch->size();
                    }
                    break;
            }
            break;

        default:
            // just to suppress warnings re: uncovered code paths
            break;
    }

    return {};
}

bool WatchesModel::setData(const QModelIndex & idx, const QVariant & data, int role)
{
    if (role == Qt::ItemDataRole::EditRole) {
        switch (idx.column()) {
            case AddressColumn: {
                assert(watch(idx));

                bool ok;
                auto address = data.toUInt(&ok);

                if (!ok) {
                    Util::debug << "invalid data type - must have unsigned integer for the address\n";
                    return false;
                }

                if (address + watch(idx)->size() > watch(idx)->memory()->addressableSize()) {
                    Util::debug << "invalid address - watch would overflow addressable memory\n";
                    return false;
                }

                watch(idx)->setAddress(address);
                return true;
            }

            case LabelColumn:
                assert(watch(idx));
                watch(idx)->setLabel(data.toString().toStdString());
                return true;

            case TypeColumn: {
                auto * strWatch = dynamic_cast<StringMemoryWatch *>(watch(idx));
                assert(strWatch);

                bool ok;
                auto size = data.toInt(&ok);

                if (!ok) {
                    Util::debug << "incorrect data type\n";
                    return false;
                }

                if (1 > size || strWatch->memory()->addressableSize() < strWatch->address() + size) {
                    Util::debug << "invalid size\n";
                    return false;
                }

                strWatch->setSize(size);
                return true;
            }
        }
    }

    return false;
}

WatchesModel::MemoryWatch * WatchesModel::watch(int idx) const
{
    assert(idx < rowCount());
    return m_watches[idx].get();
}

void WatchesModel::removeWatch(WatchesModel::MemoryWatch * watch)
{
    auto pos = std::find_if(m_watches.cbegin(), m_watches.cend(), [watch](const auto & modelWatch) -> bool {
        return modelWatch.get() == watch;
    });

    if (pos == m_watches.cend()) {
        return;
    }

    removeWatch(static_cast<int>(std::distance(m_watches.cbegin(), pos)));
}

void WatchesModel::removeWatch(int row)
{
    assert(0 <= row && row < rowCount());
    beginRemoveRows({}, row, row);
    m_watches.erase(m_watches.cbegin() + row);
    endRemoveRows();
}

void WatchesModel::removeAllWatches(::Z80::UnsignedWord address)
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

QVariant WatchesModel::headerData(int section, Qt::Orientation orientation, int role) const
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
            assert(false);
            return {};
    }
}

QModelIndex WatchesModel::index(int row, int col, const QModelIndex &) const
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
