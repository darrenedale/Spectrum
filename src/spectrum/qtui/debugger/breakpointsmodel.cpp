//
// Created by darren on 07/05/2021.
//

#include <algorithm>
#include <print>

#include "breakpointsmodel.h"
#include "../../../util/assert.h"

using namespace Spectrum::QtUi::Debugger;

namespace
{
    constexpr int TypeColumn = 0;
    constexpr int ConditionColumn = 1;
}

BreakpointsModel::BreakpointsModel(QObject * parent)
: QAbstractItemModel(parent),
  m_breakpoints()
{}

QVariant BreakpointsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::Orientation::Vertical == orientation) {
        return {};
    }

    if (role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
        case TypeColumn:
            return tr("Type");

        case ConditionColumn:
            return tr("Condition");

        [[unlikely]]
        default:
            // unreachable code - if we get here columnCount() has been changed without providing the header for the extra column(s)
            sp_assert(false, "reached unreachable code in BreakpointsModel::headerData");
            return {};
    }
}

QVariant BreakpointsModel::data(const QModelIndex & idx, int role) const
{
    if (idx.row() >= rowCount() || idx.column() >= columnCount()) {
        return {};
    }

    if (Qt::DisplayRole == role) {
        switch (idx.column()) {
            case TypeColumn:
                return QString::fromStdString(breakpoint(idx)->typeName());

            case ConditionColumn:
                return QString::fromStdString(breakpoint(idx)->conditionDescription());

            default:
                // unreachable code - if we get here columnCount() has been changed without providing the data for the extra column(s)
                [[unlikely]]
                sp_assert(false, "reached unreachable code in BreakpointsModel::data");
        }
    } else if (EnabledRole == role) {
        return m_breakpoints[idx.row()].isEnabled;
    }

    return {};
}

bool BreakpointsModel::hasBreakpoint(const Breakpoint & breakpoint) const
{
    const auto foundBreakpoint = std::ranges::find_if(m_breakpoints, [&breakpoint](const auto & existingBreakpoint) -> bool {
        return *(existingBreakpoint.breakpoint) == breakpoint;
    });

    return foundBreakpoint != m_breakpoints.cend();
}

void BreakpointsModel::removeBreakpoint(BreakpointsModel::Breakpoint * breakpoint)
{
    const auto pos = std::ranges::find_if(std::as_const(m_breakpoints), [breakpoint](const auto & modelBreakpoint) -> bool {
        return modelBreakpoint.breakpoint.get() == breakpoint;
    });

    if (pos == m_breakpoints.cend()) {
        return;
    }

    removeBreakpoint(static_cast<int>(std::distance(m_breakpoints.cbegin(), pos)));
}

void BreakpointsModel::removeBreakpoint(const int idx)
{
    sp_assert(0 <= idx && idx < rowCount(), "index {} out of bounds in BreakpointsModel::removeBreakpoint", idx);
    beginRemoveRows({}, idx, idx);
    auto * breakpoint = m_breakpoints[idx].breakpoint.get();
    m_breakpoints.erase(m_breakpoints.cbegin() + idx);
    Q_EMIT breakpointRemoved(breakpoint);
    endRemoveRows();
}

void BreakpointsModel::enableBreakpoint(BreakpointsModel::Breakpoint * breakpoint)
{
    const auto pos = std::ranges::find_if(std::as_const(m_breakpoints), [breakpoint](const auto & modelBreakpoint) -> bool {
        return modelBreakpoint.breakpoint.get() == breakpoint;
    });

    if (pos == m_breakpoints.cend()) {
        return;
    }

    enableBreakpoint(static_cast<int>(std::distance(m_breakpoints.cbegin(), pos)));
}

void BreakpointsModel::enableBreakpoint(const int idx)
{
    sp_assert(0 <= idx && idx < rowCount(), "index {} out of bounds in BreakpointsModel::enableBreakpoint", idx);

    if (m_breakpoints[idx].isEnabled) {
        // no change
        return;
    }

    m_breakpoints[idx].isEnabled = true;
    Q_EMIT dataChanged(createIndex(idx, 0), createIndex(idx, columnCount() - 1));
    Q_EMIT breakpointEnabled(m_breakpoints[idx].breakpoint.get());
}

void BreakpointsModel::disableBreakpoint(BreakpointsModel::Breakpoint * breakpoint)
{
    const auto pos = std::ranges::find_if(std::as_const(m_breakpoints), [breakpoint](const auto & modelBreakpoint) -> bool {
        return modelBreakpoint.breakpoint.get() == breakpoint;
    });

    if (pos == m_breakpoints.cend()) {
        return;
    }

    disableBreakpoint(static_cast<int>(std::distance(m_breakpoints.cbegin(), pos)));
}

void BreakpointsModel::disableBreakpoint(const int idx)
{
    sp_assert(0 <= idx && idx < rowCount(), "out-of-bounds breakpoint index {} provided to BreakpointsModel::disableBreakpoint()", idx);

    if (!m_breakpoints[idx].isEnabled) {
        // no change
        return;
    }

    m_breakpoints[idx].isEnabled = false;
    Q_EMIT dataChanged(createIndex(idx, 0), createIndex(idx, columnCount() - 1));
    Q_EMIT breakpointDisabled(m_breakpoints[idx].breakpoint.get());
}

bool BreakpointsModel::breakpointIsEnabled(Breakpoint * breakpoint) const
{
    auto pos = std::find_if(m_breakpoints.cbegin(), m_breakpoints.cend(), [breakpoint](const auto & modelBreakpoint) -> bool {
        return modelBreakpoint.breakpoint.get() == breakpoint;
    });

    if (pos == m_breakpoints.cend()) {
        return false;
    }

    return pos->isEnabled;
}

bool BreakpointsModel::breakpointIsEnabled(const int idx) const
{
    sp_assert(0 <= idx && idx < rowCount(), "index {} out of bounds in BreakpointsModel::breakpointIsEnabled", idx);

    return m_breakpoints[idx].isEnabled;
}

void BreakpointsModel::clear()
{
    const auto n = rowCount();

    if (0 == n) {
        return;
    }

    beginRemoveRows({}, 0, n - 1);
    m_breakpoints.clear();
    endRemoveRows();
}

BreakpointsModel::Breakpoint * BreakpointsModel::breakpoint(const int idx) const
{
    sp_assert(0 <= idx && idx < rowCount(), "index {} out of bounds in BreakpointsModel::breakpoint", idx);

    return m_breakpoints[idx].breakpoint.get();
}
