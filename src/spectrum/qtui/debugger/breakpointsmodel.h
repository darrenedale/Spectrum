//
// Created by darren on 07/05/2021.
//

#ifndef SPECTRUM_QTUI_DEBUGGER_BREAKPOINTSMODEL_H
#define SPECTRUM_QTUI_DEBUGGER_BREAKPOINTSMODEL_H

#include <vector>
#include <memory>
#include <QAbstractItemModel>
#include "../../debugger/breakpoint.h"

namespace Spectrum::QtUi::Debugger
{
    class BreakpointsModel
    : public QAbstractItemModel
    {
    public:

        /**
         * The item data role to fetch the enabled status of an item.
         *
         * This role can be used with any column in the model, it will always return the enabled status for the item identified by the row.
         */
        static constexpr const int EnabledRole = Qt::ItemDataRole::UserRole;

        /**
         * Initialise a new model.
         *
         * @param parent The owner of the model.
         */
        explicit BreakpointsModel(QObject * parent = nullptr);

        /**
         * The number of columns the model has available.
         *
         * Columns:
         * - 0: type
         * - 1: condition
         *
         * @param parent Ignored - there is no hierarchy of items.
         *
         * @return 3.
         */
        [[nodiscard]] constexpr int columnCount(const QModelIndex & parent = {}) const override
        {
            return 2;
        }

        /**
         * Fetch the number of breakpoints currently set.
         *
         * @param parent Ignored - there is no hierarchy.
         *
         * @return The number of breakpoints.
         */
        [[nodiscard]] int rowCount(const QModelIndex & parent = {}) const override
        {
            return static_cast<int>(m_breakpoints.size());
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
         * Provide the flags for an item.
         *
         * All items are read-only and cannot have children, except labels which can be edited.
         *
         * @param idx The index of the item whose flags are required.
         * @return
         */
        [[nodiscard]] Qt::ItemFlags flags(const QModelIndex & idx) const override
        {
            return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemNeverHasChildren;
        }

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
        [[nodiscard]] QModelIndex index(int row, int col, const QModelIndex &) const override
        {
            return createIndex(row, col);
        }

        /**
         * Fetch the parent index for an index.
         *
         * Since there is no hierarchy of items, the parent is always the root index, which is the default-constructed QModelIndex.
         *
         * @return A default-constructed QModelIndex.
         */
        [[nodiscard]] constexpr QModelIndex parent(const QModelIndex &) const override
        {
            return {};
        }

        /**
         * Convenience alias for the base class type for the stored breakpoints.
         */
        using Breakpoint = Spectrum::Debugger::Breakpoint;

        /**
         * Add a watch to the model.
         *
         * Ownership of the provided breakpoint is transferred to the model.
         *
         * @param breakpoint The breakpoint to add.
         */
        void addBreakpoint(std::unique_ptr<Breakpoint> breakpoint)
        {
            beginInsertRows({}, rowCount(), rowCount());
            m_breakpoints.push_back({.breakpoint = std::move(breakpoint), .isEnabled = true,});
            endInsertRows();
        }

        /**
         * Remove a breakpoint.
         *
         * If the provided breakpoint is in the model, it is removed. Once removed, the pointer is no longer valid.
         *
         * @param breakpoint The breakpoint to remove.
         */
        void removeBreakpoint(Breakpoint * breakpoint);

        /**
         * Remove the breakpoint at a given index.
         *
         * @param idx The index of the breakpoint to remove.
         */
        void removeBreakpoint(int idx);

        /**
         * Remove the breakpoint at a given index.
         *
         * @param idx The index of the breakpoint to remove.
         */
        void removeBreakpoint(const QModelIndex & idx)
        {
            removeBreakpoint(idx.row());
        }

        /**
         * Enable a breakpoint.
         *
         * If the provided breakpoint is in the model, it is enabled.
         *
         * @param breakpoint The breakpoint to enable.
         */
        void enableBreakpoint(Breakpoint * breakpoint);

        /**
         * Enable the breakpoint at a given index.
         *
         * @param idx The index of the breakpoint to enable.
         */
        void enableBreakpoint(int idx);

        /**
         * Enable the breakpoint at a given index.
         *
         * @param idx The index of the breakpoint to enable.
         */
        void enableBreakpoint(const QModelIndex & idx)
        {
            enableBreakpoint(idx.row());
        }

        /**
         * Disable a breakpoint.
         *
         * If the provided breakpoint is in the model, it is disabled.
         *
         * @param breakpoint The breakpoint to disable.
         */
        void disableBreakpoint(Breakpoint * breakpoint);

        /**
         * Disable the breakpoint at a given index.
         *
         * @param idx The index of the breakpoint to disable.
         */
        void disableBreakpoint(int idx);

        /**
         * Disable the breakpoint at a given index.
         *
         * @param idx The index of the breakpoint to disable.
         */
        void disableBreakpoint(const QModelIndex & idx)
        {
            disableBreakpoint(idx.row());
        }

        /**
         * Check whether a breakpoint is enabled.
         *
         * @param idx The index of the breakpoint.
         */
        [[nodiscard]] bool breakpointIsEnabled(Breakpoint *) const;

        /**
         * Check whether a breakpoint is enabled.
         *
         * @param idx The index of the breakpoint.
         */
        [[nodiscard]] bool breakpointIsEnabled(int idx) const;

        /**
         * Check whether a breakpoint is enabled.
         *
         * @param idx The index of the breakpoint.
         */
        [[nodiscard]] inline bool breakpointIsEnabled(QModelIndex idx) const
        {
            return idx.data(EnabledRole).toBool();
        }

        /**
         * Clear all breakpoints from the model.
         */
        void clear();

        /**
         * Check whether the model has any breakpoints.
         *
         * @return trye if the model has no breakpoints, false if it has one or more.
         */
        [[nodiscard]] bool isEmpty() const
        {
            return m_breakpoints.empty();
        }

        /**
         * Check whether a breakpoint has an equivalent already in the model.
         *
         * @return true if an equivalent breakpoint is already in the model, false otherwise.
         */
        [[nodiscard]] bool hasBreakpoint(const Breakpoint &) const;

        /**
         * Fetch a breakpoint from the model.
         *
         * The index of the requested breakpoint must be valid. Check rowCount() if you're note sure.
         *
         * @return
         */
        [[nodiscard]] Breakpoint * breakpoint(int) const;

        /**
         * Fetch a breakpoint from the model.
         *
         * The index of the requested breakpoint must be valid. Check rowCount() if you're note sure.
         *
         * @return
         */
        [[nodiscard]] Breakpoint * breakpoint(const QModelIndex & idx) const
        {
            return breakpoint(idx.row());
        }
        
    private:
        /**
         * Internal data structure for representing breakpoints stored in the model.
         */
        struct BreakpointData
        {
            std::unique_ptr<Breakpoint> breakpoint;
            bool isEnabled;
        };

        /**
         * Storage type for the breakpoints in the model.
         */
        using Breakpoints = std::vector<BreakpointData>;

        /**
         * Storage for the breakpoints contained in the model.
         */
        Breakpoints m_breakpoints;

    };
}

#endif //SPECTRUM_QTUI_DEBUGGER_BREAKPOINTSMODEL_H
