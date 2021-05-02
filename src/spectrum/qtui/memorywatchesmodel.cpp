//
// Created by darren on 02/05/2021.
//

#include "memorywatchesmodel.h"

using namespace Spectrum::QtUi;

namespace
{
    constexpr const int AddressColumn = 0;
    constexpr const int LabelColumn = 1;
    constexpr const int TypeColumn = 2;
    constexpr const int ValueColumn = 3;
}

MemoryWatchesModel::MemoryWatchesModel(QObject * parent)
: QAbstractItemModel(parent),
  m_watches()
{}

QVariant MemoryWatchesModel::data(const QModelIndex & idx, int role) const
{
    if (idx.row() >= rowCount() || idx.column() >= columnCount()) {
        return {};
    }

    if (role != Qt::DisplayRole) {
        return {};
    }

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
    return {};
}

MemoryWatchesModel::MemoryWatch * MemoryWatchesModel::watch(int idx) const
{
    assert(idx < rowCount());
    return m_watches[idx].get();
}

void MemoryWatchesModel::removeWatch(MemoryWatchesModel::MemoryWatch * watch)
{
    auto pos = std::find_if(m_watches.cbegin(), m_watches.cend(), [watch](const auto & modelWatch) -> bool {
        return modelWatch.get() == watch;
    });

    if (pos == m_watches.cend()) {
        return;
    }

    removeWatch(static_cast<int>(std::distance(m_watches.cbegin(), pos)));
}

void MemoryWatchesModel::removeWatch(int row)
{
    assert(0 <= row && row < rowCount());
    beginRemoveRows({}, row, row);
    m_watches.erase(m_watches.cbegin() + row);
    endRemoveRows();
}

void MemoryWatchesModel::clear()
{
    auto n = rowCount();

    if (0 == n) {
        return;
    }

    beginRemoveRows({}, 0, n - 1);
    m_watches.clear();
    endRemoveRows();
}

QVariant MemoryWatchesModel::headerData(int section, Qt::Orientation orientation, int role) const
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

QModelIndex MemoryWatchesModel::index(int row, int col, const QModelIndex &) const
{
    return createIndex(row, col);
}

void MemoryWatchesModel::update()
{
    if (isEmpty()) {
        return;
    }

    Q_EMIT dataChanged(createIndex(0, 0), createIndex(rowCount() - 1, 0));
}
