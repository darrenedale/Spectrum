//
// Created by darren on 26/04/2021.
//

#ifndef SPECTRUM_QTUI_ACTIONBAR_H
#define SPECTRUM_QTUI_ACTIONBAR_H

#include <QWidget>

namespace Spectrum::QtUi
{
    /**
     * Toolbar-like widget that can have actions and spacing added.
     *
     * The standard QToolBar is much more flexible, but it can't have (stretchable) spacing added and you can't tell it to right- or bottom-align content. This
     * widget can only have actions (tool buttons) and spacing added. You can flip the orientation between H and V on-the-fly).
     *
     * TODO
     * - support insertion of other widgets
     */
    class ActionBar
    : public QWidget
    {
    public:
        /**
         * Initialise a new action bar with a given orientation.
         *
         * @param orient The orientation.
         * @param parent
         */
        explicit ActionBar(Qt::Orientation orient, QWidget * parent = nullptr);

        /**
         * Initialise a new action bar with the default orientation (horizontal).
         *
         * @param parent
         */
        explicit ActionBar(QWidget * parent = nullptr)
        : ActionBar(Qt::Orientation::Horizontal, parent)
        {}

        ActionBar(ActionBar &&) = delete;
        ActionBar(const ActionBar &) = delete;
        void operator=(const ActionBar &) = delete;
        void operator=(ActionBar &&) = delete;

        /**
         * The destructor.
         */
        ~ActionBar() override;

        /**
         * Fetch the current orientation of the action bar.
         *
         * @return
         */
        [[nodiscard]] Qt::Orientation orientation() const;

        /**
         * Set the orientation of the action bar.
         *
         * If the orientation provided is the same as the current orientation, this is a no-op.
         */
        void setOrientation(Qt::Orientation);

        /**
         * Fetch the icon size for action bar buttons.
         *
         * @return The size.
         */
        [[nodiscard]] const QSize & iconSize() const
        {
            return m_iconSize;
        }

        /**
         * Set the icon size for action bar buttons.
         *
         * If the provided size is the same as the current size, this is a no-op.
         *
         * @param size The size.
         */
        void setIconSize(const QSize &);

        /**
         * Add a stretchable space to the end of the action bar.
         *
         * The size is an arbitrary value that has meaning only in relation to the size of other elements. When stretch space is available to the widget, a
         * stretch of size 10 will be twice as big as a stretch of size 5.
         *
         * @param size The relative size of the space.
         */
        void addStretch(int size);

        /**
         * Insert a stretchable space at a specified location in the action bar.
         *
         * The size is an arbitrary value that has meaning only in relation to the size of other elements. When stretch space is available to the widget, a
         * stretch of size 10 will be twice as big as a stretch of size 5.
         *
         * @param idx Where to insert the stretch.
         * @param size The relative size of the space.
         */
        void insertStretch(int idx, int size);

        /**
         * Add an action to the end of the action bar.
         *
         * The widget WILL NOT take ownership of the provided action - the caller is responsible for ensuring it exists for as long as the action bar has it,
         * for its destruction when it's no longer required.
         *
         * NOTE this hides a non-virtual method in the base class. We need to do this because there's no other way of trapping when an action is added to the
         * widget, and we need to know about this so that we can add the action to the layout. The base class implementation is called internally to add the
         * action to the widget's action list.
         *
         * @param action The action to add.
         */
        void addAction(QAction * action);

        /**
         * Add several actions to the end of the action bar.
         *
         * @param actions
         */
        void addActions(const QList<QAction *> & actions);

        /**
         * Insert an action before a given reference action in the action bar.
         *
         * The widget WILL NOT take ownership of the provided action - the caller is responsible for ensuring it exists for as long as the action bar has it,
         * for its destruction when it's no longer required.
         *
         * NOTE this hides a non-virtual method in the base class. We need to do this because there's no other way of trapping when an action is added to the
         * widget, and we need to know about this so that we can add the action to the layout. The base class implementation is called internally to add the
         * action to the widget's action list.
         *
         * @param action The action to add.
         */
        void insertAction(QAction * before, QAction * action);

        /**
         * Insert several actions before a given reference action in the action bar.
         *
         * @param before The existing action before which to insert the provided actions.
         * @param actions The actions to add.
         */
        void insertActions(QAction * before, const QList<QAction *> & actions);

        /**
         * Remove an action from the action bar.
         *
         * NOTE this hides a non-virtual method in the base class. We need to do this because there's no other way of trapping when an action is removed from
         * the widget, and we need to know about this so that we can remove the action from the layout. The base class implementation is called internally to
         * remove the action from the widget's action list.
         */
        void removeAction(QAction *);

        /**
         * Remove a number of actions from the action bar.
         *
         * @param actions
         */
        void removeActions(const QList<QAction *> & actions);

        /**
         * Clear all content from the action bar.
         */
        void clear();

        /**
         * setLayout() is explicitly deleted - the layout is set according to the orientation.
         */
        void setLayout(QLayout *) = delete;

    private:
        /**
         * The size of action bar icons.
         */
        QSize m_iconSize;
    };
}

#endif //SPECTRUM_QTUI_ACTIONBAR_H
