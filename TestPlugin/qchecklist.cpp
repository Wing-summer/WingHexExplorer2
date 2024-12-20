/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
** =============================================================================
*/

#include "qchecklist.h"

QCheckList::QCheckList(QWidget *parent) : QComboBox(parent) {
    m_model = new QStandardItemModel(this);
    setModel(m_model);

    setEditable(true);

    auto le = lineEdit();
    le->setReadOnly(true);
    le->installEventFilter(this);
    setItemDelegate(new QCheckListStyledItemDelegate(this));

    connect(le, &QLineEdit::selectionChanged, le, &QLineEdit::deselect);
    connect(le, &QLineEdit::textChanged, this, &QCheckList::updateText);
    connect(view(), &QAbstractItemView::pressed, this,
            &QCheckList::on_itemPressed);
    connect(m_model, &QStandardItemModel::dataChanged, this,
            &QCheckList::on_modelDataChanged);
}

void QCheckList::setAllCheckedText(const QString &text) {
    m_allCheckedText = text;
    updateText();
}

void QCheckList::setUnknownlyCheckedText(const QString &text) {
    m_unknownlyCheckedText = text;
    updateText();
}

QStandardItem *QCheckList::addCheckItem(const QString &label,
                                        const QVariant &data,
                                        const Qt::CheckState checkState) {
    QStandardItem *item = new QStandardItem(label);
    item->setCheckState(checkState);
    item->setData(data);
    item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

    m_model->appendRow(item);

    updateText();

    return item;
}

int QCheckList::globalCheckState() {
    int nbRows = m_model->rowCount(), nbChecked = 0, nbUnchecked = 0;

    if (nbRows == 0) {
        return StateUnknown;
    }

    for (int i = 0; i < nbRows; i++) {
        if (m_model->item(i)->checkState() == Qt::Checked) {
            nbChecked++;
        } else if (m_model->item(i)->checkState() == Qt::Unchecked) {
            nbUnchecked++;
        } else {
            return StateUnknown;
        }
    }

    return nbChecked == nbRows     ? Qt::Checked
           : nbUnchecked == nbRows ? Qt::Unchecked
                                   : Qt::PartiallyChecked;
}

bool QCheckList::eventFilter(QObject *_object, QEvent *_event) {
    if (_object == lineEdit() && _event->type() == QEvent::MouseButtonPress) {
        showPopup();
        return true;
    }

    return false;
}

void QCheckList::updateText() {
    QString text;
    auto le = lineEdit();
    le->blockSignals(true);

    switch (globalCheckState()) {
    case Qt::Checked:
        text = m_allCheckedText;
        break;

    case Qt::Unchecked:
        text = m_noneCheckedText;
        break;

    case Qt::PartiallyChecked:
        for (int i = 0; i < m_model->rowCount(); i++) {
            if (m_model->item(i)->checkState() == Qt::Checked) {
                if (!text.isEmpty()) {
                    text += QStringLiteral(" | ");
                }

                text += m_model->item(i)->text();
            }
        }
        break;

    default:
        text = m_unknownlyCheckedText;
    }

    le->setText(text);
    le->blockSignals(false);
}

void QCheckList::on_modelDataChanged() {
    updateText();
    emit globalCheckStateChanged(globalCheckState());
}

void QCheckList::on_itemPressed(const QModelIndex &index) {
    QStandardItem *item = m_model->itemFromIndex(index);

    if (item->checkState() == Qt::Checked) {
        item->setCheckState(Qt::Unchecked);
    } else {
        item->setCheckState(Qt::Checked);
    }
}

void QCheckList::setNoneCheckedText(const QString &text) {
    m_noneCheckedText = text;
    updateText();
}
