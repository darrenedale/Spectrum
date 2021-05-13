//
// Created by darren on 07/05/2021.
//

#include <algorithm>
#include "breakpointsmodel.h"

using namespace Spectrum::QtUi::Debugger;

namespace
{
    constexpr const int TypeColumn = 0;
    constexpr const int ConditionColumn = 1;
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

        default:
            // unreachable code - if we get here columnCount() has been changed without providing the header for the extra column(s)
            assert(false);
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
        }

        // unreachable code - if we get here columnCount() has been changed without providing the data for the extra column(s)
        assert(false);
    } else if (EnabledRole == role) {
        return m_breakpoints[idx.row()].isEnabled;
    }

    return {};
}

bool BreakpointsModel::hasBreakpoint(const Breakpoint & breakpoint) const
{
    auto existingBreakpoint = std::find_if(m_breakpoints.cbegin(), m_breakpoints.cend(), [&breakpoint](const auto & existingBreakpoint) -> bool {
        return *(existingBreakpoint.breakpoint) == breakpoint;
    });

    return existingBreakpoint != m_breakpoints.cend();
}

void BreakpointsModel::removeBreakpoint(BreakpointsModel::Breakpoint * breakpoint)
{
    auto pos = std::find_if(m_breakpoints.cbegin(), m_breakpoints.cend(), [breakpoint](const auto & modelBreakpoint) -> bool {
        return modelBreakpoint.breakpoint.get() == breakpoint;
    });

    if (pos == m_breakpoints.cend()) {
        return;
    }

    removeBreakpoint(static_cast<int>(std::distance(m_breakpoints.cbegin(), pos)));
}

void BreakpointsModel::removeBreakpoint(int idx)
{
    assert(0 <= idx && idx < rowCount());
    beginRemoveRows({}, idx, idx);
    auto * breakpoint = m_breakpoints[idx].breakpoint.get();
    m_breakpoints.erase(m_breakpoints.cbegin() + idx);
    Q_EMIT breakpointRemoved(breakpoint);
    endRemoveRows();
}

void BreakpointsModel::enableBreakpoint(BreakpointsModel::Breakpoint * breakpoint)
{
    auto pos = std::find_if(m_breakpoints.cbegin(), m_breakpoints.cend(), [breakpoint](const auto & modelBreakpoint) -> bool {
        return modelBreakpoint.breakpoint.get() == breakpoint;
    });

    if (pos == m_breakpoints.cend()) {
        return;
    }

    enableBreakpoint(static_cast<int>(std::distance(m_breakpoints.cbegin(), pos)));
}

void BreakpointsModel::enableBreakpoint(int idx)
{
    assert(0 <= idx && idx < rowCount());

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
    auto pos = std::find_if(m_breakpoints.cbegin(), m_breakpoints.cend(), [breakpoint](const auto & modelBreakpoint) -> bool {
        return modelBreakpoint.breakpoint.get() == breakpoint;
    });

    if (pos == m_breakpoints.cend()) {
        return;
    }

    disableBreakpoint(static_cast<int>(std::distance(m_breakpoints.cbegin(), pos)));
}

void BreakpointsModel::disableBreakpoint(int idx)
{
    assert(0 <= idx && idx < rowCount());

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

bool BreakpointsModel::breakpointIsEnabled(int idx) const
{
    assert(0 <= idx && idx < rowCount());
    return m_breakpoints[idx].isEnabled;
}

void BreakpointsModel::clear()
{
    auto n = rowCount();

    if (0 == n) {
        return;
    }

    beginRemoveRows({}, 0, n - 1);
    m_breakpoints.clear();
    endRemoveRows();
}

BreakpointsModel::Breakpoint * BreakpointsModel::breakpoint(int idx) const
{
    assert(idx < rowCount());
    return m_breakpoints[idx].breakpoint.get();
}
