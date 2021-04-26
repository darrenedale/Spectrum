//
// Created by darren on 26/04/2021.
//

#ifndef SPECTRUM_WIDGETUPDATESUSPENDER_H
#define SPECTRUM_WIDGETUPDATESUSPENDER_H

#include <QWidget>

namespace Spectrum::QtUi
{
    /**
     * Utility class to suspend widget updates for the scope in which the suspender is created.
     *
     * Use this when you're performing potentially large changes to a widget's state and you don't want it to be updated until after they have completed.
     *
     * The widget's "updates suspended" state will be restored to what it was when the suspender was created when the suspender is destroyed. If updates are
     * enabled on the widget at this time, the widget will also be sent an update event.
     */
    class WidgetUpdateSuspender
    {
    public:
        /**
         * Initialise a new suspender with a given widget.
         *
         * The widget must remain valid for the lifetime of the suspender.
         *
         * @param widget A reference to the widget whose updates should be suspended.
         */
        explicit WidgetUpdateSuspender(QWidget & widget)
        : m_widget(widget),
          m_released(false),
          m_reEnable(widget.updatesEnabled())
        {
            widget.setUpdatesEnabled(false);
        }

        /**
         * Destroy the suspender.
         *
         * The widget is released from the suspender, and will have updates re-enabled if they were enabled when the suspender was creataed.
         */
        virtual ~WidgetUpdateSuspender()
        {
            release();
        }

        /**
         * Release the widget from the suspender, re-enabling updates unless updates were suspended when the suspender was created.
         *
         * Subsequent calls to this method will do nothing.
         */
        void release()
        {
            if (!m_released && m_reEnable) {
                m_widget.setUpdatesEnabled(true);
                m_widget.update();
            }

            m_released = true;
        }

    private:
        /**
         * The widget.
         */
        QWidget & m_widget;

        /**
         * Whether or not the suspender has already been released.
         */
        bool m_released;

        /**
         * Whether or not to re-enable updates when the suspender is released.
         */
        bool m_reEnable;
    };
}

#endif //SPECTRUM_WIDGETUPDATESUSPENDER_H
