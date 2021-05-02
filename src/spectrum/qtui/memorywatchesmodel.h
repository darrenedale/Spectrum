//
// Created by darren on 02/05/2021.
//

#ifndef SPECTRUM_QTUI_MEMORYWATCHESMODEL_H
#define SPECTRUM_QTUI_MEMORYWATCHESMODEL_H

#include <vector>
#include <memory>
#include <QAbstractItemModel>
#include "../debugger/memorywatch.h"

namespace Spectrum::QtUi
{
    class MemoryWatchesModel
    : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        /**
         * Convenience alias for the base class type for the stored watches.
         */
        using MemoryWatch = Spectrum::Debugger::MemoryWatch;

        /**
         * Initialise a new model.
         *
         * @param parent The owner of the model.
         */
        explicit MemoryWatchesModel(QObject * parent = nullptr);

        /**
         * The number of columns the model has available.
         *
         * @param parent Ignored - there is no hierarchy of items.
         *
         * @return 3.
         */
        [[nodiscard]] constexpr int columnCount(const QModelIndex & parent = {}) const override
        {
            return 4;
        }

        /**
         * Fetch the number of watches currently set.
         *
         * @param parent Ignored - there is no hierarchy.
         *
         * @return The number of watches.
         */
        [[nodiscard]] int rowCount(const QModelIndex & parent = {}) const override
        {
            return static_cast<int>(m_watches.size());
        }

        /**
         * Fetch the header data for a column.
         *
         * When the role is Display and the orientation is Horizontal (i.e. the column headers), the column headers "Address", "Label", "Type" and "Value"
         * (translated into the user's language, if available) are provided. Otherwise, an invalid QVariant is returned.
         *
         * @param section The header section.
         * @param orientation Horizontal or vertical headers?
         * @param role The data role requested.
         *
         * @return The header data.
         */
        [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::ItemDataRole::DisplayRole) const override;

        /**
         * Fetch the data for an item.
         *
         * The row of the index indicates which watch; the column indicates which property. Column 0 is the address, 1 is the label, 2 is the type and 3 is the
         * current value. Data is only provided for Display role.
         *
         * @param idx The index of the watch and property requested.
         * @param role The role of the data requested.
         *
         * @return
         */
        [[nodiscard]] QVariant data(const QModelIndex & idx, int role = Qt::ItemDataRole::DisplayRole) const override;

        /**
         * Set the data for an item.
         *
         * Only labels for watches can be edited.
         *
         * @param idx The index of the watch to edit. The column must be the Label column.
         * @param value The value to set for the label.
         * @param role The data role. Only the Edit role is accepted.
         *
         * @return true if the data was set, false otherwise.
         */
        bool setData(const QModelIndex & idx, const QVariant & value, int role = Qt::ItemDataRole::EditRole) override;

        /**
         * Provide the flags for an item.
         *
         * All items are read-only and cannot have children, except labels which can be edited.
         *
         * @param idx The index of the item whose flags are required.
         * @return
         */
        [[nodiscard]] Qt::ItemFlags flags(const QModelIndex & idx) const override;

        /**
         * Fetch an index for a row and column.
         *
         * Since there is no hierarchy of items, the index simply contains the row and column.
         *
         * @param row
         * @param col
         *
         * @return The index.
         */
        [[nodiscard]] QModelIndex index(int row, int col, const QModelIndex &) const override;

        /**
         * Fetch the parent index for an index.
         *
         * Since there is no hierarchy of items, the parent is always the root index, which is the default-constructed QModelIndex.
         *
         * @return A defualt-constructed QModelIndex.
         */
        [[nodiscard]] constexpr QModelIndex parent(const QModelIndex &) const override
        {
            return {};
        }

        /**
         * Add a watch to the model.
         *
         * Ownership of the provided watch is transferred to the model.
         *
         * @param watch The watch to add.
         */
        void addWatch(std::unique_ptr<MemoryWatch> watch)
        {
            beginInsertRows({}, rowCount(), rowCount());
            m_watches.push_back(std::move(watch));
            endInsertRows();
        }

        /**
         * Remove a watch.
         *
         * If the provided watch is in the model, it is removed. Once removed, the pointer is no longer valid.
         *
         * @param watch The watch to remove.
         */
        void removeWatch(MemoryWatch * watch);

        /**
         * Remove the watch at a given index.
         *
         * @param idx The index of the watch to remove.
         */
        void removeWatch(int idx);

        /**
         * Remove the watch at a given index.
         *
         * @param idx The index of the watch to remove.
         */
        void removeWatch(const QModelIndex & idx)
        {
            removeWatch(idx.row());
        }

        /**
         * Remove all watches that are watching a given address.
         *
         * Only watches whose first watched byte is at the address are removed.
         *
         * @param address The address whose watches should be removed.
         */
        void removeAllWatches(::Z80::UnsignedWord address);

        /**
         * Clear all watches from the model.
         */
        void clear();

        /**
         * Check whether the model has any watches.
         *
         * @return trye if the model has no watches, false if it has one or more.
         */
        [[nodiscard]] bool isEmpty() const
        {
            return m_watches.empty();
        }

        /**
         * Fetch a watch from the model.
         *
         * The index of the requested watch must be valid. Check rowCount() if you're note sure.
         *
         * @return
         */
        [[nodiscard]] MemoryWatch * watch(int) const;

        /**
         * Fetch a watch from the model.
         *
         * The index of the requested watch must be valid. Check rowCount() if you're note sure.
         *
         * @return
         */
        [[nodiscard]] MemoryWatch * watch(const QModelIndex & idx) const
        {
            return watch(idx.row());
        }

        /**
         * Update the values for all watches.
         *
         * Any view that is visualising the item in the model will be asked to update every item, unless the model contains no items.
         */
        void update();

    private:
        using Watches = std::vector<std::unique_ptr<MemoryWatch>>;

        /**
         * Storage for the watches contained in the model.
         */
        Watches m_watches;
    };

}

#endif //SPECTRUM_QTUI_MEMORYWATCHESMODEL_H
