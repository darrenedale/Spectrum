//
// Created by darren on 27/03/2021.
//

#ifndef SPECTRUM_POKESWIDGET_H
#define SPECTRUM_POKESWIDGET_H

#include <unordered_map>
#include <QWidget>
#include <QVBoxLayout>
#include <QAction>
#include <QToolBar>
#include <QString>
#include "../pokedefinition.h"

namespace Spectrum
{
    class PokeDefinition;
}

namespace Spectrum::QtUi
{
    /**
     * A widget to display, apply, undo and remove pokes that can be applied to the running emulator.
     */
    class PokesWidget
    : public QWidget
    {
    Q_OBJECT

    public:
        using Pokes = std::unordered_map<std::string, PokeDefinition>;

        explicit PokesWidget(QWidget * = nullptr);
        PokesWidget(const PokesWidget &) = delete;
        PokesWidget(PokesWidget &&) = delete;
        void operator=(const PokesWidget &) = delete;
        void operator=(PokesWidget &&) = delete;
        ~PokesWidget() override;

        void setActionIconSize(const QSize &);

        [[nodiscard]] const QSize & actionIconSize() const
        {
            return m_actionIconSize;
        }

        void loadPokes(const QString & fileName);
        void addPoke(const PokeDefinition &);
        void addPoke(PokeDefinition &&);
        void clearPokes();

        [[nodiscard]] int pokeCount() const
        {
            return static_cast<int>(m_pokes.size());
        }

        void removePoke(int);

        QAction * loadPokesAction()
        {
            return &m_loadPokes;
        }

        QAction * clearPokesAction()
        {
            return &m_clearPokes;
        }

    Q_SIGNALS:
        void applyPokeRequested(const Spectrum::PokeDefinition &);
        void undoPokeRequested(const Spectrum::PokeDefinition &);

    protected:
        void showEvent(QShowEvent *) override;
        void hideEvent(QHideEvent *) override;
        QWidget * findPokeWidget(const QString & uuid) const;
        void removePoke(const QString & uuid, QWidget * widget = nullptr);

        virtual void applyPokeTriggered(const QString & uuid);
        virtual void undoPokeTriggered(const QString & uuid);
        virtual void removePokeTriggered(const QString & uuid);

    private:
        void loadPokesTriggered();
        void clearPokesTriggered();
        void addPokeWidget(const QString & name, const QString & uuid);

        Pokes m_pokes;
        QVBoxLayout m_layout;
        QAction m_loadPokes;
        QAction m_clearPokes;
        QToolBar m_toolBar;
        QString m_lastPokeLoadDir;
        QSize m_actionIconSize;
    };
}

#endif //SPECTRUM_POKESWIDGET_H
