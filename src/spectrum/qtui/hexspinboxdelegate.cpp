//
// Created by darren on 03/05/2021.
//

#include "hexspinboxdelegate.h"
#include "hexspinbox.h"

using namespace Spectrum::QtUi;

QWidget * HexSpinBoxDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & idx) const
{
    bool ok;
    auto value = idx.data(Qt::ItemDataRole::EditRole).toInt(&ok);

    if (ok) {
        auto * widget = new HexSpinBox(4, QLatin1Char('0'), parent);
        widget->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        widget->setValue(value);
        return widget;
    }

    return QStyledItemDelegate::createEditor(parent, option, idx);
}

void HexSpinBoxDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    assert(editor);
    bool ok;
    auto value = index.data(Qt::ItemDataRole::EditRole).toInt(&ok);

    if (ok) {
        qobject_cast<HexSpinBox *>(editor)->setValue(value);
        return;
    }

    QStyledItemDelegate::setEditorData(editor, index);
}

void HexSpinBoxDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & idx) const
{
    assert(model);
    auto * hexWidget = qobject_cast<HexSpinBox *>(editor);

    if (hexWidget) {
        model->setData(idx, hexWidget->value(), Qt::ItemDataRole::EditRole);
        return;
    }

    return QStyledItemDelegate::setModelData(editor, model, idx);
}
