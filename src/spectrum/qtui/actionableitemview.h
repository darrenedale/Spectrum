//
// Created by darren on 09/05/2021.
//

#ifndef SPECTRUM_QTUI_ACTIONABLEITEMVIEW_H
#define SPECTRUM_QTUI_ACTIONABLEITEMVIEW_H

#include <concepts>
#include <optional>
#include <vector>
#include <QAbstractItemView>
#include <QAction>
#include <QRect>
#include <QStyleHints>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QToolTip>
#include <QTimer>
#include <QApplication>

#if (!defined(__cpp_lib_concepts) || 202002L > __cpp_lib_concepts)
// use polyfill for missing concepts library
#include "../../util/concepts.h"
#endif

namespace Spectrum::QtUi
{
    /**
     * Concept to ensure that the ActionableItemView template can only be instantiated with Qt view classes.
     *
     * @tparam ViewType The type subject to the concept.
     */
    template<class ViewType>
    concept QtViewClass = std::derived_from<ViewType, QAbstractItemView>;

    /**
     * Template to wrap an item view with functionality to enable actions for the items in the model.
     *
     * The idea is that this template makes it possible to add actions to a view that are presented as an overlay on the visualisation of an item in the model
     * when that item is hovered. The Qt model/view framework enables presentation and editing of data but does not easily allow for such manipulation features.
     * For example, this wrapper template makes it easy to add a remove icon to the end of an item in a list view when the mouse is over it.
     *
     * To use it, create a custom view class and have it inherit from this template, passing the base view class as the template parameter. For example:
     *
     * class MyView : public ActionableItemView<QTreeView>
     * { ... }
     *
     * In this case, MyView is a type of QTreeView that has the functionality described here. It is best used with list-type or tree-type views; its utility
     * with table-type views is probably limited.
     *
     * The class provides functions for adding and removing actions. Each action added will be displayed when the mouse pointer is hovered over a row in the
     * view. Currently, action icons are always rendered at the rightmost edge of the rightmost column of the view. This may change to always render them at the
     * rightmost edge of the viewport, or to provide configurability around this. Connect to signals from the actions that have been added as you would any
     * other action, and in the connected slot use the currentActionItem() method to determine which item the message from the action relates to. Here's the
     * general idea:
     *
     * auto * action = view->addItemAction(QIcon("do-something.png"));
     * connect(action, &QAction::triggered, [view]() {
     *     const auto & index = view->actionItemIndex();
     *     doSomethingWithModelItem(index);
     * });
     *
     * If you need to change the state of some of your actions when the hovered item changes, override the onItemEntered() method, call the base class method in
     * your reimplementation (first, before you do anything else, so that the view's idea of the item to which the actions relate is correct), then do what you
     * need to do with the actions.
     *
     * There is a notional rectangle in which each action "button" exists. Inside this rect, there is a margin of 1 x spacing on all sides between the edge
     * of the notional rect and the edge of the action "button", and a margin of 1 x spacing on all sides between the edge of the "button" and the edge of the
     * actual icon rect:
     *
     *   +--------------------------------------+--------------------------------------+
     *   |             1 x spacing              |             1 x spacing              |
     *   |   +------------------------------+   |   +------------------------------+   |
     *   |   |         1 x spacing          |   |   |         1 x spacing          |   |
     *   |   |   +----------------------+   |   |   |   +----------------------+   |   |
     *   |   |   |                      |   |   |   |   |                      |   |   |
     *   |   |   |                      |   |   |   |   |                      |   |   |
     *   |   |   |                      |   |   |   |   |                      |   |   |
     *   |   |   |        ICON 1        |   |   |   |   |        ICON 2        |   |   |
     *   |   |   |                      |   |   |   |   |                      |   |   |
     *   |   |   |                      |   |   |   |   |                      |   |   |
     *   |   |   |                      |   |   |   |   |                      |   |   |
     *   |   |   +----------------------+   |   |   |   +----------------------+   |   |
     *   |   |           BUTTON             |   |   |           BUTTON             |   |
     *   |   +------------------------------+   |   +------------------------------+   |
     *   |            notional rect             |            notional rect             |
     *   +--------------------------------------+--------------------------------------+
     *
     * The spacing is calculated as 10% of the icon size or 2px, whichever is larger. The icon size is calculated as 16px at 96dpi (scaled to the equivalent for
     * the pixel density of the screen on which the widget is displayed). This currently cannot be altered, but such a feature is likely to be implemented.
     *
     * The action icons can be overlaid at the left edge or right edge of the item or viewport (see setItemActionAlignment() and
     * setItemActionHorizontalAlignment()). The benefit of aligning to the viewport rather than the item is that the actions will always be visible, even if the
     * item is too wide for the viewport and extends to the left or right of the visible area. They can also be aligned vertically at the top, centred or at the
     * bottom of the relevant item's visual rectangle. It is implicitly assumed that the overlay will be used primarily with views that arrange their items top
     * to bottom, and that therefore there is no need to provide for vertical alignment to the viewport boundaries since this would only make sense if items
     * were arranged horizontally and individual items could extend above or below the visible area, potentially obscuring the actions for the "active" item.
     * For the same reason, horizontally centring the actions is not supported.
     */
    template<QtViewClass ViewType>
    class ActionableItemView
    : public ViewType
    {
    public:
        /**
         * Enumeration of the options for horizontally aligning the item action icons in the view.
         *
         * The icons are always rendered in a horizontal row. This enumeration indicates where that row is placed.
         */
        enum class ItemActionHorizontalAlignment
        {
            ViewportRightEdge = 0,       // the right edge of the row of actions is aligned with the right edge of the viewport
            ItemRightEdge,               // the right edge of the row of actions is aligned with the right edge of the item (if the item is wider than the viewport, actions might be hidden)
            ViewportLeftEdge,            // the left edge of the row of actions is aligned with the left edge of the viewport
            ItemLeftEdge,                // the left edge of the row of actions is aligned with the left edge of the item (if the item is wider than the viewport, actions might be hidden)
        };

        /**
         * Enumeration of the options for vertically aligning the item action icons in the view.
         */
        enum class ItemActionVerticalAlignment
        {
            Top = 0,
            Centre,
            Center = Centre,
            Bottom,
        };

        /**
         * Data structure for storage of action alignment in both axes.
         */
        struct Alignment
        {
            ItemActionHorizontalAlignment horizontal;
            ItemActionVerticalAlignment vertical;
        };
        
        /**
         * Initialise a new ActionableItemView.
         *
         * This is a template constructor which will attempt to forward all args to the base View widget constructor. Effectively this means that this class
         * provides all the constructors offered by the base view class.
         */
        template<class ... Args>
        explicit ActionableItemView(Args && ... args)
        : ViewType(std::forward<Args...>(args...)),
          m_itemActions(),
          m_hoveredItem(),
          m_hoveredItemActionIndex(),
          m_doubleClickWaitTimer(),
          m_itemActionIconExtent(0),        // properly initialised when show event occurs
          m_itemActionIconSpacing(0),       // properly initialised when show event occurs
          m_alignment{.horizontal = ItemActionHorizontalAlignment::ViewportRightEdge, .vertical = ItemActionVerticalAlignment::Top}
        {
            m_doubleClickWaitTimer.setSingleShot(true);
            m_doubleClickWaitTimer.setInterval(QApplication::styleHints()->mouseDoubleClickInterval());

            // we need this to know when the mouse pointer moves between actions
            ViewType::setMouseTracking(true);

            ViewType::connect(this, &ViewType::entered, this, &ActionableItemView<ViewType>::onItemEntered);
            ViewType::connect(&m_doubleClickWaitTimer, &QTimer::timeout, this, &ActionableItemView<ViewType>::onItemClicked);
        }

        /**
         * Destructor.
         */
        ~ActionableItemView() override = default;

        /**
         * Set the horizontal alignment for the item action icons.
         *
         * The vertical alignment is unaffected. If the new alignment is different from the previous alignment, this will trigger a redraw of the appropriate
         * model item.
         *
         * @param align The new horizontal alignment.
         */
        void setItemActionAlignment(ItemActionHorizontalAlignment align)
        {
            if (align == m_alignment.horizontal) {
                return;
            }

            m_alignment.horizontal = align;
            redrawActionItem();
        }

        /**
         * Set the vertical alignment for the item action icons.
         *
         * The horizontal alignment is unaffected. If the new alignment is different from the previous alignment, this will trigger a redraw of the appropriate
         * model item.
         *
         * @param align The new horizontal alignment.
         */
        void setItemActionAlignment(ItemActionVerticalAlignment align)
        {
            if (align == m_alignment.vertical) {
                return;
            }

            m_alignment.vertical = align;
            redrawActionItem();
        }

        /**
         * Set the horizontal and vertical alignment for the item action icons.
         *
         * If the new alignment in either axis is different from the previous alignment, this will trigger a redraw of the appropriate model item.
         *
         * @param hAlign The new horizontal alignment.
         * @param vAlign The new vertical alignment.
         */
        void setItemActionAlignment(ItemActionHorizontalAlignment hAlign, ItemActionVerticalAlignment vAlign)
        {
            if (hAlign == m_alignment.horizontal && vAlign == m_alignment.vertical) {
                return;
            }

            m_alignment.horizontal = hAlign;
            m_alignment.vertical = vAlign;
            redrawActionItem();
        }

        /**
         * Set the horizontal and vertical alignment for the item action icons.
         *
         * If the new alignment in either axis is different from the previous alignment, this will trigger a redraw of the appropriate model item.
         *
         * @param align The new alignment.
         */
        inline void setItemActionAlignment(const Alignment & align)
        {
            setItemActionAlignment(align.horizontal, align.vertical);
        }

        /**
         * Fetch the current alignment of the item action icons.
         *
         * @return The alignment.
         */
        [[nodiscard]] ItemActionHorizontalAlignment itemActionHorizontalAlignment() const
        {
            return m_alignment.horizontal;
        }

        /**
         * Fetch the current alignment of the item action icons.
         *
         * The returned structure is a trivial pair of ints, it has very little cost to return by value.
         *
         * @return The alignment.
         */
        [[nodiscard]] ItemActionVerticalAlignment itemActionVerticalAlignment() const
        {
            return m_alignment.vertical;
        }

        /**
         * Fetch the current alignment of the item action icons.
         *
         * @return The alignment.
         */
        [[nodiscard]] const Alignment & itemActionAlignment() const
        {
            return m_alignment;
        }

        /**
         * Fetch the current alignment of the item action icons.
         *
         * @return The alignment.
         */
        [[nodiscard]] Alignment itemActionAlignment()
        {
            return m_alignment;
        }

        /**
         * Add an item action.
         *
         * The caller is responsible for managing the lifetime of the added action. If the action is destroyed while it's still contained by the view, it will
         * automatically be removed.
         *
         * @param icon The icon for the action.
         * @param text The text for the action.
         *
         * @return The added action.
         */
        QAction * addItemAction(const QIcon & icon, const QString & text = {})
        {
            return addItemAction(new QAction(icon, text));
        }

        /**
         * Add an item action.
         *
         * The added action must not be nullptr. The action must have an icon or it will be rendered empty. The caller remains responsible for managing the
         * lifetime of the added action. If the action is destroyed while it's still contained by the view, it will automatically be removed.
         *
         * @param action The action to add.
         *
         * @return The action.
         */
        QAction * addItemAction(QAction * action)
        {
            assert(action);
            auto & actionItem = m_itemActions.emplace_back(action, QRect(), QMetaObject::Connection());

            actionItem.destructHandler = ViewType::connect(action, &QAction::destroyed, [this, action]() {
                removeItemAction(action);
            });

            updateItemActionIconGeometries();
            ViewType::viewport()->update(actionIconRegion());
            return action;
        }

        /**
         * Remove the provided item action from the view.
         * 
         * This is a no-op if the action is not one of the view's item actions. The action is never deleted, it remains the responsibility of the code that
         * added the action to ensure it is destroyed at the appropriate time.
         * 
         * @param action The action to remove.
         */
        void removeItemAction(const QAction * const action)
        {
            const auto pos = std::find_if(m_itemActions.cbegin(), m_itemActions.cend(), [action](const auto & itemAction) -> bool {
                return itemAction.action == action;
            });

            if (pos == m_itemActions.cend()) {
                return;
            }


            // ensure the item action icon region is repainted
            QRegion damage = actionIconRegion();
            ViewType::disconnect(pos->destructHandler);
            m_itemActions.erase(pos);

            // reset the hovered item action if it's no longer a valid index
            if (m_hoveredItemActionIndex && m_itemActions.size() <= *m_hoveredItemActionIndex) {
                m_hoveredItemActionIndex = {};
            }

            updateItemActionIconGeometries();
            ViewType::viewport()->update(damage);
        }

        /**
         * Fetch the index of the model item that is currently the subject of the actions, if any.
         *
         * The model index will be invalid if there is no item currently subject to the actions.
         *
         * @return The model index.
         */
        [[nodiscard]] QModelIndex actionItemIndex() const
        {
            if (m_hoveredItem) {
                return *m_hoveredItem;
            }

            return {};
        }
        
    protected:
        /**
         * Handle show events for the widget.
         *
         * Recalculates the icon extent and spacing based on the widget's current screen's pixel density. The event is passed on to the base class event handler
         * before returning.
         *
         * The icon extent (including the spacing around it) is 16px at 96dpi, scaled accordingly for the screen pixel density. The spacing is 10% of the icon
         * extent or 2px, whichever is larger.
         *
         * @param ev The event.
         */
        void showEvent(QShowEvent * ev) override
        {
            static constexpr const int BaseExtent = 16;
            static constexpr const double BaseExtentDpi = 96.0;

            m_itemActionIconExtent = static_cast<int>((ViewType::screen()->logicalDotsPerInchX() / BaseExtentDpi) * BaseExtent);
            m_itemActionIconSpacing = std::max(2, static_cast<int>(m_itemActionIconExtent / 10.0));
            updateItemActionIconGeometries();
            ViewType::showEvent(ev);
        }

        /**
         * Handler for resize events for the widget.
         *
         * The geometries of the action icons are recalculated before the event is passed on the the base class.
         *
         * @param ev The event.
         */
        void resizeEvent(QResizeEvent * ev) override
        {
            updateItemActionIconGeometries();
            ViewType::resizeEvent(ev);
        }

        /**
         * Handler for when the mouse pointer leaves the widget area.
         */
        void leaveEvent(QEvent * ev) override
        {
            setNoActionItem();
            ViewType::leaveEvent(ev);
        }
        
        /**
         * Handler for mouse moves.
         *
         * Traps when the mouse leaves an item as well as handling the hover effect over action icons. The event is then passed up to the base class.
         */
        void mouseMoveEvent(QMouseEvent * ev) override
        {
            const auto pos = ev->pos();

            if (!ViewType::indexAt(pos).isValid()) {
                // there's no view signal for when the mouse leaves a model index so we have to handle this case ourselves
                setNoActionItem();
            }

            int actionIndex = 0;

            // check whether the item action under the mouse pointer has changed
            for (const auto & actionData : m_itemActions) {
                if (actionData.geometry.contains(pos)) {
                    break;
                }

                ++actionIndex;
            }

            if (actionIndex < m_itemActions.size()) {
                // an item action is currently hovered
                if (!m_hoveredItemActionIndex || *m_hoveredItemActionIndex != actionIndex) {
                    // ... and it's different from the last one that was hovered
                    QRegion damage;

                    if (m_hoveredItemActionIndex) {
                        // remove the hover effect from the previously hovered item action
                        damage += m_itemActions[*m_hoveredItemActionIndex].geometry;
                    }

                    // add the hover effect to the new hovered item action
                    damage += m_itemActions[actionIndex].geometry;
                    m_hoveredItemActionIndex = actionIndex;
                    ViewType::viewport()->update(damage);
                }
            } else {
                // no action is currently hovered
                if (m_hoveredItemActionIndex) {
                    // remove the hover effect from the previously hovered action
                    auto lastHoveredAction = *m_hoveredItemActionIndex;
                    m_hoveredItemActionIndex= {};
                    ViewType::viewport()->update(QRegion(m_itemActions[lastHoveredAction].geometry));
                }
            }

            ViewType::mouseMoveEvent(ev);
        }

        /**
         * Handler for mouse double-clicks.
         *
         * These are trapped to cancel the delay timer that triggers click events so that we don't treat double-clicks as both click and double-click.
         */
        void mouseReleaseEvent(QMouseEvent * ev) override
        {
            if (Qt::MouseButton::LeftButton == ev->button()) {
                // if we don't receive notification of a double-click before the timer expires, treat it as a single click
                m_doubleClickWaitTimer.start();
            }

            ViewType::mouseReleaseEvent(ev);
        }

        /**
         * Handler for mouse double-clicks.
         *
         * These are trapped to cancel the delay timer that triggers click events so that we don't treat double-clicks as both click and double-click.
         */
        void mouseDoubleClickEvent(QMouseEvent * ev) override
        {
            // make sure we don't treat the click as a single-click
            m_doubleClickWaitTimer.stop();
            ViewType::mouseDoubleClickEvent(ev);
        }

        /**
         * Handler for paint events on the view.
         *
         * The base class method is called to render the view before the actions are overlaid if necessary.
         */
        void paintEvent(QPaintEvent * ev) override
        {
            ViewType::paintEvent(ev);

            if (m_hoveredItem) {
                QPainter painter(ViewType::viewport());
                painter.setClipRect(*actionItemViewportRect());
                painter.setRenderHint(QPainter::RenderHint::Antialiasing, true);
                auto iconAdjustment = 2 * m_itemActionIconSpacing;

                auto highlight = ViewType::palette().highlight().color();
                highlight.setAlpha(127);
                painter.setBrush(highlight);
                painter.setPen(Qt::GlobalColor::transparent);

                for (const auto & actionData : m_itemActions) {
                    if (actionData.action->isChecked()) {
                        // the rounding is the same as the spacing
                        painter.drawRoundedRect(actionData.geometry.adjusted(m_itemActionIconSpacing, m_itemActionIconSpacing, -m_itemActionIconSpacing, -m_itemActionIconSpacing), m_itemActionIconSpacing, m_itemActionIconSpacing);
                    }
                }

                highlight.setAlpha(255);
                painter.setBrush(Qt::GlobalColor::transparent);
                painter.setPen({highlight, static_cast<qreal>(m_itemActionIconSpacing)});

                // draw the hover effect for the action under the mouse pointer
                if (m_hoveredItemActionIndex) {
                    // the rounding is the same as the spacing
                    painter.drawRoundedRect(m_itemActions[*m_hoveredItemActionIndex].geometry.adjusted(m_itemActionIconSpacing, m_itemActionIconSpacing, -m_itemActionIconSpacing, -m_itemActionIconSpacing), m_itemActionIconSpacing, m_itemActionIconSpacing);
                }

                for (const auto & actionData : m_itemActions) {
                    actionData.action->icon().paint(&painter, actionData.geometry.adjusted(iconAdjustment, iconAdjustment, -iconAdjustment, -iconAdjustment));
                }
            }
        }

        /**
         * Handler for all events on the view.
         *
         * This is reimplemented for the following reasons:
         *
         * - so that we hear about move events over component widgets (e.g. the header view) and can unset the current target item and
         *   action if the mouse pointer is not inside the viewport. Without this we don't get to hear when the pointer has moved off an item that is at the
         *   edge of the viewport and onto a child widget such as the header view, and consequently the item would remain "hovered" even though the mouse
         *   pointer is not over it.
         *
         *   After this check, the parent class event handler is always called, so the event propagates as normal.
         *
         * - so that we can show item action tooltips, if the event occurs with the mouse pointer over an item action and the action has a tooltip.
         *
         * @param ev The event.
         *
         * @return true if the event was recognised and handled by the parent class, false otherwise.
         */
        bool event(QEvent * ev) override
        {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"     // we're only interested in a subset of event types
            switch (ev->type()) {
                case QEvent::Type::HoverMove: {
                    const auto pos = reinterpret_cast<QHoverEvent *>(ev)->pos();

                    // there's no view signal for when the mouse leaves a model index, and mouseMoveEvent() does not receive events when the move transitions
                    // from an item to a child widget (e.g. the header view) so we have to handle this case by checking whether the hover move event type (which
                    // parent widgets always receive) is outside the viewport (note the viewport is the area where actual items are rendered)
                    if (0 > ViewType::viewport()->mapFromParent(pos).y()) {
                        // outside viewport, nothing hovered
                        setNoActionItem();
                    }
                    break;
                }

                case QEvent::Type::ToolTip:
                    if (m_hoveredItemActionIndex) {
                        const auto pos = ViewType::viewport()->mapFromParent(reinterpret_cast<QHelpEvent *>(ev)->pos());
                        const auto & itemAction = m_itemActions[*m_hoveredItemActionIndex];

                        if (!itemAction.action->toolTip().isEmpty() && itemAction.geometry.contains(pos)) {
                            QToolTip::showText(ViewType::viewport()->mapToGlobal(pos), itemAction.action->toolTip());
                            return true;
                        }
                    }
                    break;
            }
#pragma clang diagnostic pop

            return ViewType::event(ev);
        }

        /**
         * Internal handler for when the area has been scrolled.
         *
         * We override this to ensure that when horizontal scrolls occur the item action icons are redrawn since, if they are aligned to the viewport, the
         * previous rendering will be scrolled by the default implementation and new icons redrawn in the new location without the previous rendering being
         * erased. We achieve this by triggering a full update of the item that is currently the subject of the item actions.
         *
         * @param dx
         * @param dy
         */
        void scrollContentsBy(int dx, int dy) override
        {
            ViewType::scrollContentsBy(dx, dy);

            if (0 != dx) {
                // note, if aligned to the viewport, scrolling doesn't affect horizontal position of action geometries
                if (ItemActionHorizontalAlignment::ItemRightEdge == itemActionHorizontalAlignment()) {
                    updateItemActionIconGeometries();
                }

                redrawActionItem();
            }
        }

        /**
         * Helper to recalculate the geometries of the actions when the widget is resized, etc.
         */
        void updateItemActionIconGeometries()
        {
            QRegion damage;

            if (m_hoveredItemActionIndex) {
                damage += m_itemActions[*m_hoveredItemActionIndex].geometry;
            }

            if (!m_hoveredItem) {
                for (auto & iconData : m_itemActions) {
                    iconData.geometry = {};
                }
            } else {
                QRect itemRect;
                QRect iconRect;

                switch (itemActionHorizontalAlignment()) {
                    case ItemActionHorizontalAlignment::ViewportRightEdge: {
                        itemRect = ViewType::visualRect(ViewType::model()->index(m_hoveredItem->row(), ViewType::model()->columnCount() - 1));
                        iconRect = QRect(ViewType::viewport()->rect().right() - static_cast<int>(m_itemActionIconExtent * m_itemActions.size()), 0, m_itemActionIconExtent, m_itemActionIconExtent);
                        break;
                    }

                    case ItemActionHorizontalAlignment::ItemRightEdge: {
                        itemRect = ViewType::visualRect(ViewType::model()->index(m_hoveredItem->row(), ViewType::model()->columnCount() - 1));
                        iconRect = QRect(itemRect.right() - static_cast<int>(m_itemActionIconExtent * m_itemActions.size()), 0, m_itemActionIconExtent, m_itemActionIconExtent);
                        break;
                    }

                    case ItemActionHorizontalAlignment::ViewportLeftEdge: {
                        itemRect = ViewType::visualRect(ViewType::model()->index(m_hoveredItem->row(), 0));
                        iconRect = QRect(ViewType::viewport()->rect().left(), 0, m_itemActionIconExtent, m_itemActionIconExtent);
                        break;
                    }

                    case ItemActionHorizontalAlignment::ItemLeftEdge: {
                        itemRect = ViewType::visualRect(ViewType::model()->index(m_hoveredItem->row(), 0));
                        iconRect = QRect(itemRect.left(), 0, m_itemActionIconExtent, m_itemActionIconExtent);
                        break;
                    }
                }
                
                switch (itemActionVerticalAlignment()) {
                    case ItemActionVerticalAlignment::Top:
                        iconRect.moveTop(itemRect.top());
                        break;

                    case ItemActionVerticalAlignment::Centre:
                        iconRect.moveTop(itemRect.top() + static_cast<int>((itemRect.height() - iconRect.height()) / 2.0));
                        break;

                    case ItemActionVerticalAlignment::Bottom:
                        iconRect.moveBottom(itemRect.bottom());
                        break;
                }

                for (auto & itemAction : m_itemActions) {
                    itemAction.geometry = iconRect;
                    iconRect.translate(m_itemActionIconExtent, 0);
                }

                if (m_hoveredItemActionIndex) {
                    damage += m_itemActions[*m_hoveredItemActionIndex].geometry;
                }
            }

            if (m_hoveredItemActionIndex) {
                ViewType::viewport()->update(damage);
            }
        }

        /**
         * Handler to set the current hovered item when the mouse pointer enters a different item.
         *
         * @param index The new hovered index.
         */
        virtual void onItemEntered(const QModelIndex & index)
        {
            if (!index.isValid()) {
                m_hoveredItem = {};
            } else if (index != m_hoveredItem) {
                m_hoveredItem = index;
            }

            updateItemActionIconGeometries();
        }

        /**
         * Helper to determine the region occupied by the item action icons.
         * 
         * Used when the icons need to be re-rendered. The returned region will be empty if there are no item actions.
         * 
         * @return The region.
         */
        [[nodiscard]] QRegion actionIconRegion() const
        {
            if (m_itemActions.empty()) {
                return {};
            }
            
            auto spacing = 2 * m_itemActionIconSpacing;
            
            return QRegion({
                   m_itemActions.front().geometry.adjusted(-spacing, -spacing, spacing, spacing).topLeft(),
                   m_itemActions.back().geometry.adjusted(-spacing, -spacing, spacing, spacing).bottomRight()
           });
        }

        /**
         * Helper to fetch the rect in the viewport of the hovered item.
         *
         * @return The rect, or an empty optional if there is no hovered item.
         */
        [[nodiscard]] std::optional<QRect> actionItemViewportRect() const
        {
            if (!m_hoveredItem) {
                return {};
            }

            auto itemRect = ViewType::visualRect(*m_hoveredItem);
            auto viewportRect = ViewType::viewport()->rect();
            return QRect(viewportRect.x(), itemRect.y(), viewportRect.width(), itemRect.height());
        }

        /**
         * Helper to redraw the current action item.
         *
         * This is useful when the whole item should be redrawn, for example when the action alignment has changed. This avoids a lot of complication working
         * out exactly where the action icons were and are when attempting to erase and redraw them.
         */
        void redrawActionItem()
        {
            auto redrawRect = actionItemViewportRect();

            if (!redrawRect) {
                return;
            }

            ViewType::viewport()->update(*redrawRect);
        }

        /**
         * Helper to unset the action item and hovered action.
         *
         * Extracted to helper to avoid boilerplate. The hovered item and action are both reset and the necessary visual update is triggered.
         */
        void setNoActionItem()
        {
            m_hoveredItem = {};
            m_hoveredItemActionIndex = {};

            if (!m_itemActions.empty()) {
                // trigger an update to ensure all action icons are cleared
                ViewType::viewport()->update(actionIconRegion());
                updateItemActionIconGeometries();
            }
        }

    private:
        /**
         * Handler for when a click occurs on an item.
         *
         * If the click is on an item action, the action is toggled (if checkable) or triggered (if not).
         */
        void onItemClicked()
        {
            if (!m_hoveredItemActionIndex) {
                return;
            }

            if (m_itemActions[*m_hoveredItemActionIndex].action->isCheckable()) {
                m_itemActions[*m_hoveredItemActionIndex].action->toggle();
            } else {


                m_itemActions[*m_hoveredItemActionIndex].action->trigger();
            }
        }

        /**
         * Internal data structure for item action storage.
         */
        struct ItemActionData
        {
            /**
             * Constructor.
             *
             * We define the constructor because clang's std::vector::emplace_back() doesn't have a c'tor to call to create a instance otherwise. Not sure if
             * this is a shortcoming of clang++ or a non-standard feature of gcc.
             *
             * There is no need to receive an rvalue ref to the connection handle and move it because it's size is a single pointer. No value in moving QRect
             * either since its members are all POD: copy and move are no different.
             */
            ItemActionData(QAction * act, const QRect & geom, const QMetaObject::Connection & conn)
            : action(act),
              geometry(geom),
              destructHandler(conn)
            {}

            QAction * action;
            QRect geometry;                             // the notional rectangle in which the action's spacing, button background and icon are rendered
            QMetaObject::Connection destructHandler;    // connection handle for the lambda that removes the action when it's destroyed
        };

        /**
         * Alias for the storage type for the available actions.
         */
        using ItemActions = std::vector<ItemActionData>;

        /**
         * The item actions.
         */
        ItemActions m_itemActions;
        
        /**
         * The model index for the item under the mouse pointer.
         */
        std::optional<QModelIndex> m_hoveredItem;
        
        /**
         * The index of the action currently under the mouse pointer.
         */
        std::optional<int> m_hoveredItemActionIndex;

        /**
         * Timer to delay handling "click" events until we're sure the click is not part of a double-click.
         */
        QTimer m_doubleClickWaitTimer;

        /**
         * The size, in px, of item action icons.
         *
         * This adapts to the pixel density of the screen on which the widget is displayed. It's 16px at 96dpi.
         */
        int m_itemActionIconExtent;

        /**
         * The size, in px, of the spacing around item action icons.
         *
         * This adapts to the pixel density of the screen on which the widget is displayed. 10% of the icon extent or 2px, whichever is larger.
         */
        int m_itemActionIconSpacing;

        /**
         * Where the item action icons are rendered in the relevant item's visual rectangle.
         */
        Alignment m_alignment;
    };
}

#endif //SPECTRUM_QTUI_ACTIONABLEITEMVIEW_H
