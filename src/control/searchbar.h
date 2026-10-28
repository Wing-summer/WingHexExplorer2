/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QIcon>
#include <QRegularExpression>
#include <QTimer>
#include <QWidget>

class QAction;
class QLabel;
class QLineEdit;
class QMenu;
class QToolButton;

// Target-independent search UI. Hosts own matching and result navigation.
class SearchBar : public QWidget {
    Q_OBJECT
public:
    enum class State { Idle, Searching, NoMatch, InvalidPattern };
    struct Options {
        Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive;
        bool wholeWords = false;
        bool regularExpression = false;
        bool wrap = true;
    };
    // Immutable request snapshot shared by table, tree, and text adapters.
    // Plain text avoids regex matching unless a regex/word option requires it.
    struct Query {
        QString text;
        Options options;
        QRegularExpression pattern;
        bool matches(const QString &candidate) const;
    };
    Query query() const;
    explicit SearchBar(QWidget *parent = nullptr,
                       const QString &objectPrefix = QStringLiteral("search"));
    QString searchText() const;
    void setSearchText(const QString &text);
    Options options() const;
    void setOptions(const Options &options);
    State state() const;
    QRegularExpression expression() const;
    QMenu *optionsMenu() const;
    QLineEdit *lineEdit() const;
    void activate();
    void scheduleSearch();
    void cancelPendingSearch();
    void setState(State state, const QString &detail = {});
    bool handleKeyEvent(QEvent *event);
    static QIcon busyIcon(const QPalette &palette);
signals:
    void queryChanged();
    void searchRequested(bool backwards);
    void showRequested();
    void closeRequested();
    void geometryChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *object, QEvent *event) override;
    void changeEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void refreshAppearance();
    void updateStatus();
    struct Controls {
        QLineEdit *input = nullptr;
        QLabel *status = nullptr;
        QMenu *menu = nullptr;
        QList<QToolButton *> buttons;
    } m_controls;
    struct OptionActions {
        QAction *caseSensitive = nullptr;
        QAction *wholeWord = nullptr;
        QAction *regex = nullptr;
        QAction *wrap = nullptr;
    } m_options;
    QTimer m_debounce;
    State m_state = State::Idle;
    QString m_detail;
};
#endif
