/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

// This file is part of QTextPad.

#ifndef SETTINGSPOPUP_H
#define SETTINGSPOPUP_H

#include <KSyntaxHighlighting/Definition>
#include <QIcon>
#include <QLineEdit>

class QTreeWidget;
class QTreeWidgetItem;

class TreeFilterEdit : public QLineEdit {
    Q_OBJECT

public:
    TreeFilterEdit(QWidget *parent = nullptr);

    QSize sizeHint() const override;

Q_SIGNALS:
    void navigateDown();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QIcon m_searchIcon;
    QPoint m_iconPosition;

    void recomputeIconPos();
};

class FilteredTreePopup : public QWidget {
    Q_OBJECT

public:
    FilteredTreePopup(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    TreeFilterEdit *filter();
    QTreeWidget *tree();

public Q_SLOTS:
    void filterItems(const QString &text);

protected:
    void showEvent(QShowEvent *e) override;
    bool focusNextPrevChild(bool next) override;

private:
    TreeFilterEdit *m_filter;
    QTreeWidget *m_tree;
};

class SyntaxPopup : public FilteredTreePopup {
    Q_OBJECT

public:
    SyntaxPopup(QWidget *parent = nullptr);

Q_SIGNALS:
    void syntaxSelected(const KSyntaxHighlighting::Definition &syntax);

private Q_SLOTS:
    void syntaxItemChosen(QTreeWidgetItem *current, int column);

private:
    QTreeWidgetItem *m_plainTextItem;
};

class EncodingPopup : public FilteredTreePopup {
    Q_OBJECT

public:
    EncodingPopup(QWidget *parent = Q_NULLPTR);

Q_SIGNALS:
    void encodingSelected(const QString &codecName);

private Q_SLOTS:
    void encodingItemChosen(QTreeWidgetItem *current, int column);
};

// Needed for serializing Definition objects into QVariant
Q_DECLARE_METATYPE(KSyntaxHighlighting::Definition)

#endif // QTEXTPAD_SETTINGSPOPUP_H
