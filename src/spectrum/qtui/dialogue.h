//
// Created by darren on 24/04/2021.
//

#ifndef SPECTRUM_QTUI_DIALOGUE_H
#define SPECTRUM_QTUI_DIALOGUE_H

#include <QDialog>
#include <QLabel>
#include <QIcon>
#include <QDialogButtonBox>

class QPushButton;

namespace Spectrum::QtUi
{
    /**
     * A generic dialogue class.
     *
     * Dialogues contain an optional icon, optional title (which is rendered in 20% larger text, bold), a message and an arbitrary number of control buttons.
     * All of these features can be customised. The primary driver for this class is to ensure dialogues are relatively consistent in appearance.
     */
    class Dialogue
    : public QDialog
    {
    public:
        /**
         * Initialise a dialogue with no content.
         *
         * @param parent
         */
        explicit Dialogue(QWidget * parent = nullptr);

        /**
         * Initialise a dialogue with a message and title.
         *
         * @param message
         * @param title
         * @param parent
         */
        explicit Dialogue(const QString & message, const QString & title, QWidget * parent = nullptr);

        /**
         * Initialise a dialouge with just a message.
         *
         * @param message
         * @param parent
         */
        explicit Dialogue(const QString & message, QWidget * parent = nullptr);

        // dialogues are not copy/move constructable nor are they copy/move assignable
        Dialogue(const Dialogue &) = delete;
        Dialogue(Dialogue &&) = delete;
        void operator=(const Dialogue &) = delete;
        void operator=(Dialogue &&) = delete;

        /**
         * Destructor.
         */
        ~Dialogue() override;

        /**
         * Fetch the dialogue's icon.
         *
         * The icon might be a null icon, in which case it is not displayed (i.e. occupies no space in the dialogue layout).
         *
         * @return The icon.
         */
        [[nodiscard]] QIcon icon() const
        {
            return m_icon;
        }

        /**
         * Set the dialogue's icon.
         *
         * Set a null icon to remove it from the dialogue.
         *
         * @param The icon.
         */
        void setIcon(const QIcon &);

        /**
         * Fetch the dialogue's title.
         *
         * The title might be empty, in which case it is not displayed (i.e. occupies no space in the dialogue layout).
         *
         * @return The title.
         */
        [[nodiscard]] QString title() const
        {
            return m_title.text();
        }

        /**
         * Set the dialogue's title.
         *
         * Set an empty title to remove it from the dialogue.
         *
         * @param The title.
         */
        void setTitle(const QString &);

        /**
         * Fetch the dialogue's message.
         *
         * Even if empty, the dialogue message component takes up space in the layout.
         *
         * @return The message.
         */
        [[nodiscard]] QString message() const
        {
            return m_message.text();
        }

        /**
         * Set the dialogue's message.
         *
         * @param The message.
         */
        void setMessage(const QString & message)
        {
            m_message.setText(message);
        }

        /**
         * Add a button to the dialogue with a given role.
         *
         * If the button is already one of the dialogue's control buttons and it has the correct role, this will be a no-op. If it's already there with a
         * different role it will be re-purposed and move to the end of the buttons.
         *
         * The dialogue takes ownership of the button and will destroy it when the dialogue is destroyed.
         *
         * @param button The button to add.
         * @param role The role for the button.
         */
        void addButton(QAbstractButton * button, QDialogButtonBox::ButtonRole role);

        /**
         * Add a new button to the dialogue with a given display text and role.
         *
         * The dialogue retains ownership of the returned button and will destroy it when the dialogue is destroyed. Do not destroy the returned object, and do
         * not dereference it after the dialogue has been destroyed.
         *
         * @param text The text for the button to add.
         * @param role The role for the button.
         *
         * @return The button added.
         */
        QPushButton * addButton(const QString & text, QDialogButtonBox::ButtonRole role);

        /**
         * Add a new standard button to the dialogue.
         *
         * The created button will have the platform standard display style for that type of button.
         *
         * The dialogue retains ownership of the returned button and will destroy it when the dialogue is destroyed. Do not destroy the returned object, and do
         * not dereference it after the dialogue has been destroyed.
         *
         * @param button The standard button type.
         *
         * @return The button added.
         */
        QPushButton * addButton(QDialogButtonBox::StandardButton button);

        /**
         * Remove a button from the dialogue.
         *
         * If the button is one of the dialogue's control buttons it will be removed and destroyed. It is safe (though not recommended practice) to provide
         * pointers to buttons that are not in the dialogue - this is a no-op. Never provide a nullptr.
         *
         * If the removed button was the last one, the dialogue controls will be hidden, taking up no space in the dialogue's layout.
         *
         * @param button
         */
        void removeButton(QAbstractButton * button);

        /**
         * Remove all button from the dialogue.
         *
         * All control buttons will be removed and destroyed and the controls will be hidden, taking up no space in the dialogue's layout.
         */
        void clearButtons();

    protected:
        /**
         * Event handler for when the widget is shown.
         *
         * @param ev The event details.
         */
        void showEvent(QShowEvent * ev) override;

        /**
         * Helper to rebuild the dialogue's layout.
         *
         * Used when one of the main property setters is called.
         */
        void rebuildLayout();

    private:
        /**
         * The UI widget to display the title.
         */
        QLabel m_title;

        /**
         * The UI widget to display the message.
         */
        QLabel m_message;

        /**
         * The icon.
         */
        QIcon m_icon;

        /**
         * The size of the icon. This is calculated according to the pixel density of the screen on which the dialogue appears. At 96dpi the icon will be 40px.
         */
        int m_iconExtent;

        /**
         * The UI widget to display the icon.
         */
        QLabel m_iconLabel;

        /**
         * The UI widget for the action buttons for the dialogue.
         */
        QDialogButtonBox m_controls;
    };
}

#endif //SPECTRUM_QTUI_DIALOGUE_H
