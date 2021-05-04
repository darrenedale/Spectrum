//
// Created by darren on 29/03/2021.
//

#ifndef SPECTRUM_QTUI_POKESWIDGETITEM_H
#define SPECTRUM_QTUI_POKESWIDGETITEM_H

#include <QWidget>
#include <QToolButton>
#include "../pokedefinition.h"

namespace Spectrum::QtUi
{
    class PokesViewItem
    : public QWidget
    {
        Q_OBJECT

    public:
        PokesViewItem(const QString & name, QString  uuid, QWidget * parent = nullptr);
        PokesViewItem(const PokesViewItem &) = delete;
        PokesViewItem(PokesViewItem &&) = delete;
        void operator=(const PokesViewItem &) = delete;
        void operator=(PokesViewItem &&) = delete;
        ~PokesViewItem() override;

        inline void setIconSize(const QSize & size)
        {
            m_onOff.setIconSize(size);
            m_remove.setIconSize(size);
        }

        void setActivated(bool activated = true);

        inline void setDeactivated()
        {
            setActivated(false);
        }

        const QString & uuid() const
        {
            return m_uuid;
        }

    Q_SIGNALS:
        void removeClicked(const QString & uuid);
        void activationRequested(const QString & uuid);
        void deactivationRequested(const QString & uuid);

    private:
        QString m_uuid;
        QToolButton m_onOff;
        QToolButton m_remove;
    };
}

#endif //SPECTRUM_QTUI_POKESWIDGETITEM_H
