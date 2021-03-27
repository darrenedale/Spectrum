//
// Created by darren on 27/03/2021.
//

#ifndef SPECTRUM_POKESWIDGET_H
#define SPECTRUM_POKESWIDGET_H

#include <unordered_map>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QString>
#include "../pokedefinition.h"

namespace Spectrum
{
    class PokeDefinition;
}

namespace Spectrum::Qt
{
    /**
     * A widget to display, apply, undo and remove pokes that can be applied to the running emulator.
     *
     * TODO should we make this a subclass of QListView?
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

        void addPoke(const PokeDefinition &);
        void addPoke(PokeDefinition &&);

        [[nodiscard]] int pokeCount() const
        {
            return m_pokes.size();
        }

        void removePoke(int);

    Q_SIGNALS:
        void applyPokeRequested(const Spectrum::PokeDefinition &);

    protected:
        QWidget * findPokeWidget(const QString & uuid) const;
        void removePoke(const QString & uuid, QWidget * widget = nullptr);
        void undoPoke(const QString & uuid);

        virtual void applyPokeTriggered(const QString & uuid);
        virtual void undoPokeTriggered(const QString & uuid);
        virtual void removePokeTriggered(const QString & uuid);

    private:
        void addPokeWidget(const QString & name, const QString & uuid);

        Pokes m_pokes;
        QVBoxLayout m_layout;
    };
}

#endif //SPECTRUM_POKESWIDGET_H
