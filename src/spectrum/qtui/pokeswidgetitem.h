//
// Created by darren on 29/03/2021.
//

#ifndef SPECTRUM_QT_POKESWIDGETITEM_H
#define SPECTRUM_QT_POKESWIDGETITEM_H

#include <QWidget>
#include <QToolButton>
#include "../pokedefinition.h"

namespace Spectrum::QtUi
{
    class PokesWidgetItem
    : public QWidget
    {
        Q_OBJECT

    public:
        PokesWidgetItem(const QString & name, QString  uuid, QWidget * parent = nullptr);
        PokesWidgetItem(const PokesWidgetItem &) = delete;
        PokesWidgetItem(PokesWidgetItem &&) = delete;
        void operator=(const PokesWidgetItem &) = delete;
        void operator=(PokesWidgetItem &&) = delete;
        ~PokesWidgetItem() override;

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

#endif //SPECTRUM_QT_POKESWIDGETITEM_H
