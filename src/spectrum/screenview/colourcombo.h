//
// Created by darren on 26/02/2021.
//

#ifndef SPECTRUM_SCREENVIEW_COLOURCOMBO_H
#define SPECTRUM_SCREENVIEW_COLOURCOMBO_H

#include <QComboBox>

#include "../types.h"

namespace Spectrum::ScreenView
{
    /**
     * A combo box specialisation to select a Spectrum colour.
     */
    class ColourCombo
    : public QComboBox
    {
    Q_OBJECT

    public:
        /**
         * Initialise a new colour combo box.
         */
        explicit ColourCombo(QWidget * = nullptr);

        /**
         * ColourCombos cannot be copy-constructed.
         */
        ColourCombo(const ColourCombo &) = delete;

        /**
         * ColourCombos cannot be move-constructed.
         */
        ColourCombo(ColourCombo &&) = delete;

        /**
         * ColourCombos cannot be copy assigned.
         */
        void operator=(const ColourCombo &) = delete;

        /**
         * ColourCombos cannot be move assigned.
         */
        void operator=(ColourCombo &&) = delete;

        /**
         * Destructor.
         */
        ~ColourCombo() override;

        /**
         * Fetch the colour currently selected in the combo box.
         *
         * @return The colour.
         */
        [[nodiscard]] Spectrum::Colour colour() const;

        /**
         * Determine whether the currently-selected colour is bright.
         *
         * @return true if the colour is bright, false otherwise.
         */
        [[nodiscard]] bool isBright() const;

    public Q_SLOTS:
        /**
         * Set the currently-selected colour.
         *
         * @param colour The colour to select.
         * @param bright Whether to select the bright or normal version of the colour.
         */
        void setColour(Spectrum::Colour colour, bool bright = false);

        /**
         * Set whether the bright version of the colour is selected.
         *
         * @param bright true if the bright version of the currently selected colour should be selected, false if the normal version should be selected.
         */
        void setBright(bool bright);

    Q_SIGNALS:
        /**
         * Emitted when the user selects a colour.
         *
         * @param colour The chosen colour.
         * @param bright Whether the bright (true) or normal (false) version of the colour was selected.
         */
        void colourSelected(Spectrum::Colour colour, bool bright);

    protected:
        /**
         * Helper to locate a combo box item for a given colour and brightness.
         *
         * @param colour The colour to locate.
         * @param bright Whether the bright (true) or normal (false) version of the colour is sought.
         *
         * @return The 0-based index of the combo box item representing the colour.
         */
        int findItem(Spectrum::Colour colour, bool bright = false);

        /**
         * Internal helper to add a colour to the combo box.
         *
         * You should never need to call this.
         */
        void addItem(const QString &, Spectrum::Colour);
    };
}

#endif //SPECTRUM_SCREENVIEW_COLOURCOMBO_H
