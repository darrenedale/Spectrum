//
// Created by darren on 27/03/2021.
//

#ifndef SPECTRUM_CHEATSVIEW_H
#define SPECTRUM_CHEATSVIEW_H

#include <unordered_map>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QString>
#include "../pokedefinition.h"
#include "actionbar.h"

namespace Spectrum
{
    class PokeDefinition;
}

namespace Spectrum::QtUi
{
    /**
     * A widget to display, apply, undo and remove cheats that can be applied to the running emulator.
     */
    class CheatsView
    : public QWidget
    {
    Q_OBJECT

    public:
        /**
         * Default initialise a new cheats view.
         */
        explicit CheatsView(QWidget * = nullptr);

        /**
         * CheatsView widgets cannot be copy-constructed.
         */
        CheatsView(const CheatsView &) = delete;

        /**
         * CheatsView widgets cannot be move-constructed.
         */
        CheatsView(CheatsView &&) = delete;

        /**
         * CheatsView widgets cannot be copy assigned.
         */
        void operator=(const CheatsView &) = delete;

        /**
         * CheatsView widgets cannot be move assigned.
         */
        void operator=(CheatsView &&) = delete;

        /**
         * Destructor.
         */
        ~CheatsView() override;

        /**
         * Set the size of the action icons.
         */
        void setActionIconSize(const QSize &);

        /**
         * Fetch the size of the action icons.
         *
         * @return The size.
         */
        [[nodiscard]] const QSize & actionIconSize() const
        {
            return m_actionIconSize;
        }

        /**
         * Load a set of cheats from a given file.
         *
         * @param fileName The file to load.
         */
        void loadCheats(const QString & fileName);

        /**
         * Add a copy of a cheat to the view.
         */
        void addCheat(const PokeDefinition &);

        /**
         * Move a cheat to the view.
         */
        void addCheat(PokeDefinition &&);

        /**
         * Clear all the cheats from the view.
         */
        void clearCheats();

        /**
         * Fetch the number of cheats in the view.
         *
         * @return The number of cheats.
         */
        [[nodiscard]] int cheatCount() const
        {
            return static_cast<int>(m_cheats.size());
        }

        /**
         * Remove a cheat from the view.
         *
         * The provided index must be valid. Check against cheatCount() if you are unsure.
         *
         * @param idx The index of the cheat.
         */
        void removeCheat(int);

        /**
         * Fetch the action to load cheats.
         *
         * @return The action.
         */
        QAction * loadCheatsAction()
        {
            return &m_loadCheats;
        }

        /**
         * Fetch the action to clear all the cheats.
         *
         * @return The action.
         */
        QAction * clearCheatsAction()
        {
            return &m_clearCheats;
        }

    Q_SIGNALS:
        /**
         * Emitted when the apply action for a given cheat is triggered.
         */
        void applyCheatRequested(const Spectrum::PokeDefinition &);

        /**
         * Emitted when the undo action for a given cheat is triggered.
         */
        void undoCheatRequested(const Spectrum::PokeDefinition &);

    protected:
        /**
         * Load the settings for the widget from the application settings file.
         *
         * Currently only the last used cheat load directory is stored for this widget.
         */
        void loadSettings();

        /**
         * Save the settings for the widget to the application settings file.
         *
         * Currently only the last used cheat load directory is stored for this widget.
         */
        void saveSettings();

        /**
         * Helper to find the widget for a given cheat.
         *
         * @param uuid The UUID of the cheat whose widget is sought.
         *
         * @return The widget, or nullptr if a widget for the identified cheat cannot be found.
         */
        QWidget * findCheatWidget(const QString & uuid) const;

        /**
         * Helper to remove a cheat from the view.
         *
         * Provide nullptr for widget unless you are certain you already have the correct widget. Providing nullptr will
         * cause the function to locate the widget based on the UUID. The widget parameter is really a mechanism to
         * avoid a second lookup when the widget it already known.
         *
         * @param uuid The UUID of the cheat to remove.
         * @param widget The widget pointer for the cheat, if known before the call.
         */
        void removeCheat(const QString & uuid, QWidget * widget = nullptr);

        /**
         * Handler for when the apply action is triggered for a cheat.
         *
         * @param uuid The internal unique ID for the cheat to apply.
         */
        virtual void applyCheatTriggered(const QString & uuid);

        /**
         * Handler for when the undo action is triggered for a cheat.
         *
         * @param uuid The internal unique ID for the cheat to undo.
         */
        virtual void undoCheatTriggered(const QString & uuid);

        /**
         * Handler for when the remove action is triggered for a cheat.
         *
         * @param uuid The internal unique ID for the cheat to remove.
         */
        virtual void removeCheatTriggered(const QString & uuid);

    private:
        /**
         * Storage type for the loaded cheats.
         *
         * Maps UUIDs to the definitions.
         */
        using Cheats = std::unordered_map<std::string, PokeDefinition>;

        /**
         * Handler for when the load cheats action is triggered.
         */
        void loadCheatsTriggered();

        /**
         * Handler for when the clear cheats action is triggered.
         */
        void clearCheatsTriggered();

        /**
         * Helper to add a cheat widget to the list of cheats.
         *
         * @param name The display name of the cheat.
         * @param uuid The internal unique identifier for the cheat.
         */
        void addCheatWidget(const QString & name, const QString & uuid);

        /**
         * The loaded cheats.
         */
        Cheats m_cheats;

        /**
         * The main layout for the cheats.
         */
        QVBoxLayout m_layout;

        /**
         * Action to load cheats.
         */
        QAction m_loadCheats;

        /**
         * Action to clear all loaded cheats.
         */
        QAction m_clearCheats;

        /**
         * The toolbar for the load/clear, etc. actions.
         */
        ActionBar m_toolBar;

        /**
         * The last directory from which a cheat file was loaded for this widget.
         */
        QString m_lastLoadDir;

        /**
         * The size for cheat actions.
         */
        QSize m_actionIconSize;
    };
}

#endif //SPECTRUM_CHEATSVIEW_H
