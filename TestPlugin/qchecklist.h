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

#ifndef QCHECKLIST_H
#define QCHECKLIST_H

#include <QComboBox>
#include <QEvent>
#include <QLineEdit>
#include <QListView>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QWidget>

class QCheckListStyledItemDelegate : public QStyledItemDelegate {
public:
    QCheckListStyledItemDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter_, const QStyleOptionViewItem &option_,
               const QModelIndex &index_) const {
        QStyleOptionViewItem &refToNonConstOption =
            const_cast<QStyleOptionViewItem &>(option_);
        refToNonConstOption.showDecorationSelected = false;
        QStyledItemDelegate::paint(painter_, refToNonConstOption, index_);
    }
};

/**
 * @brief QComboBox with support of checkboxes
 * http://stackoverflow.com/questions/8422760/combobox-of-checkboxes
 */
class QCheckList : public QComboBox {
    Q_OBJECT
public:
    /**
     * @brief Additional value to Qt::CheckState when some checkboxes are
     * Qt::PartiallyChecked
     */
    static const int StateUnknown = 3;

private:
    QStandardItemModel *m_model;
    /**
     * @brief Text displayed when no item is checked
     */
    QString m_noneCheckedText;
    /**
     * @brief Text displayed when all items are checked
     */
    QString m_allCheckedText;
    /**
     * @brief Text displayed when some items are partially checked
     */
    QString m_unknownlyCheckedText;

signals:
    void globalCheckStateChanged(int);

public:
    explicit QCheckList(QWidget *parent = nullptr);

    void setAllCheckedText(const QString &text);

    void setNoneCheckedText(const QString &text);

    void setUnknownlyCheckedText(const QString &text);

    /**
     * @brief Adds a item to the checklist (setChecklist must have been called)
     * @return the new QStandardItem
     */
    QStandardItem *addCheckItem(const QString &label, const QVariant &data,
                                const Qt::CheckState checkState);

    /**
     * @brief Computes the global state of the checklist :
     *      - if there is no item: StateUnknown
     *      - if there is at least one item partially checked: StateUnknown
     *      - if all items are checked: Qt::Checked
     *      - if no item is checked: Qt::Unchecked
     *      - else: Qt::PartiallyChecked
     */
    int globalCheckState();

protected:
    bool eventFilter(QObject *_object, QEvent *_event);

private:
    void updateText();

private slots:
    void on_modelDataChanged();

    void on_itemPressed(const QModelIndex &index);
};

#endif // QCHECKLIST_H
