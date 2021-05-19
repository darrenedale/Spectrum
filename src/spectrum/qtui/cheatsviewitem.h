//
// Created by darren on 29/03/2021.
//

#ifndef SPECTRUM_QTUI_CHEATSVIEWITEM_H
#define SPECTRUM_QTUI_CHEATSVIEWITEM_H

#include <QWidget>
#include <QToolButton>
#include "../pokedefinition.h"

namespace Spectrum::QtUi
{
    /**
     * An item to display in the cheats view.
     */
    class CheatsViewItem
    : public QWidget
    {
        Q_OBJECT

    public:
        /**
         * Initialise a new item with a name and UUID.
         *
         * @param name The display name for the cheat.
         * @param uuid The internal UUID to use for the item.
         * @param parent The widget that owns the item.
         */
        CheatsViewItem(const QString & name, QString  uuid, QWidget * parent = nullptr);

        /**
         * CheatsViewItems cannot be copy constructed.
         */
        CheatsViewItem(const CheatsViewItem &) = delete;

        /**
         * CheatsViewItems cannot be move constructed.
         */
        CheatsViewItem(CheatsViewItem &&) = delete;

        /**
         * CheatsViewItems cannot be copy assigned.
         */
        void operator=(const CheatsViewItem &) = delete;

        /**
         * CheatsViewItems cannot be move assigned.
         */
        void operator=(CheatsViewItem &&) = delete;

        /**
         * Destructor.
         */
        ~CheatsViewItem() override;

        /**
         * Set the size of icons for the cheat actions.
         *
         * @param size The icon size.
         */
        inline void setIconSize(const QSize & size)
        {
            m_onOff.setIconSize(size);
            m_remove.setIconSize(size);
        }

        /**
         * Set whether or not the cheat is activated.
         *
         * @param activated
         */
        void setActivated(bool activated = true);

        /**
         * Set the cheat to deactivated.
         */
        inline void setDeactivated()
        {
            setActivated(false);
        }

        /**
         * Fetch the internal UUID of the item.
         *
         * @return
         */
        [[nodiscard]] const QString & uuid() const
        {
            return m_uuid;
        }

    Q_SIGNALS:
        /**
         * Emitted when the cheat item's remove action is clicked.
         *
         * @param uuid The internal UUID of the cheat item.
         */
        void removeClicked(const QString & uuid);

        /**
         * Emitted when the cheat item's activation action is toggled on.
         *
         * @param uuid The internal UUID of the cheat item.
         */
        void activationRequested(const QString & uuid);

        /**
         * Emitted when the cheat item's activation action is toggled off.
         *
         * @param uuid The internal UUID of the cheat item.
         */
        void deactivationRequested(const QString & uuid);

    private:
        /**
         * The internal UUID for the item.
         */
        QString m_uuid;

        /**
         * The enable/disable action widget.
         */
        QToolButton m_onOff;

        /**
         * The remove action widget.
         */
        QToolButton m_remove;
    };
}

#endif //SPECTRUM_QTUI_CHEATSVIEWITEM_H
