//
// Created by darren on 03/05/2021.
//

#ifndef SPECTRUM_QTUI_ADDRESSITEMDELEGATE_H
#define SPECTRUM_QTUI_ADDRESSITEMDELEGATE_H

#include <QStyledItemDelegate>

namespace Spectrum::QtUi
{
    /**
     * Model/View delegate to provide a hex spin box editor for integer values.
     *
     * The delegate's digit count and fill character can be set in the constructor. They will default to 4 digits and '0' for the fill character. This is useful
     * for editing address values in model views, for example.
     */
    class HexSpinBoxDelegate
    : public QStyledItemDelegate
    {
    public:
        /**
         * Initialise a new delegate with a specified number of digits and fill character.
         *
         * @param digits The number of hex digits.
         * @param fillChar The fill character to use for empty digits.
         * @param parent The parent for the delegate.
         */
        explicit HexSpinBoxDelegate(int digits, const QChar & fillChar, QObject * parent = nullptr)
        : QStyledItemDelegate(parent),
          m_digits(digits),
          m_fill(fillChar)
        {}

        /**
         * Default-initialise a new delegate.
         *
         * The delegate will use widgets with 4 digits and '0' for the fill character.
         *
         * @param parent The parent for the delegate.
         */
        explicit HexSpinBoxDelegate(QObject * parent = nullptr)
        : HexSpinBoxDelegate(4, QLatin1Char('0'), parent)
        {}

        /**
         * Initialise a new delegate with a specified number of digits.
         *
         * The delegate will use '0' for the fill character.
         *
         * @param digits The number of hex digits.
         * @param parent The parent for the delegate.
         */
        explicit HexSpinBoxDelegate(int digits, QObject * parent = nullptr)
        : HexSpinBoxDelegate(digits, QLatin1Char('0'), parent)
        {}

        /**
         * Initialise a new delegate with a specified fill character.
         *
         * The delegate will use widgets with 4 digits.
         *
         * @param fillChar The fill character to use for empty digits.
         * @param parent The parent for the delegate.
         */
        explicit HexSpinBoxDelegate(const QChar & fillChar, QObject * parent = nullptr)
        : HexSpinBoxDelegate(4, fillChar, parent)
        {}

        /**
         * Create a new editor for a given model index.
         *
         * A HexSpinBox widget will be created on the condition that the model index provides an integer value.
         *
         * @param parent The parent for the created editor widget.
         * @param option The visual options for the editor widget.
         * @param idx The model index of the item to be edited.
         *
         * @return The widget.
         */
        QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & idx) const override;

        /**
         * Set the value in the editor to the value for a provided model index.
         *
         * The provided widget must be a HexSpinBox. Provided the given index supplies an integer value, it will be set as the value in the widget.
         *
         * @param editor The widget whose value is to be set.
         * @param idx The model index containing the value.
         */
        void setEditorData(QWidget * editor, const QModelIndex & idx) const override;

        /**
         * Update a model index with the value from the editor.
         *
         * @param editor The editor with the value to set.
         * @param model The model where the data is to be stored.
         * @param idx The model index identifying the item to update in the model.
         */
        void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & idx) const override;

    private:
        /**
         * The number of digits to use in the widget.
         */
        int m_digits;

        /**
         * The fill character to use.
         */
        QChar m_fill;
    };
}

#endif //SPECTRUM_QTUI_ADDRESSITEMDELEGATE_H
