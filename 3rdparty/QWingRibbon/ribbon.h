/*
 * Copyright (C) Martijn Koopman
 * All Rights Reserved
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 */

#ifndef RIBBONTABWIDGET_H
#define RIBBONTABWIDGET_H

#include <QHBoxLayout>
#include <QTabWidget>
#include <QToolButton>

// modified by wingsummer

QT_FORWARD_DECLARE_CLASS(RibbonTabContent)

class Ribbon : public QTabWidget {
    Q_OBJECT
public:
    explicit Ribbon(QWidget *parent = nullptr);

    /// Add a tab to the ribbon.
    ///
    /// \param[in] tabName Name of the tab
    RibbonTabContent *addTab(const QString &tabName);

    /// Add a tab to the ribbon.
    ///
    /// \param[in] tabIcon Icon of the tab
    /// \param[in] tabName Name of the tab
    RibbonTabContent *addTab(const QIcon &tabIcon, const QString &tabName);

    /// Remove a tab from the ribbon.
    ///
    /// \param[in] tabName Name of the tab
    void removeTab(const QString &tabName);

    /// Remove a tab from the ribbon.
    ///
    /// \param[in] tabContent pointer of the tab
    void removeTab(RibbonTabContent *tabContent);

    QHBoxLayout *conerLayout() const;

signals:
    void onDragDropFiles(const QStringList &filenames);

protected:
    // —— events ———————————————————————————
    /*
     * this event is called when the mouse enters the widgets area during a
     * drag/drop operation
     */
    void dragEnterEvent(QDragEnterEvent *event);

    /**
     * this event is called when the mouse moves inside the widgets area during
     * a drag/drop operation
     */
    void dragMoveEvent(QDragMoveEvent *event);

    /**
     * this event is called when the mouse leaves the widgets area during a
     * drag/drop operation
     */
    void dragLeaveEvent(QDragLeaveEvent *event);

    /**
     * this event is called when the drop operation is initiated at the widget
     */
    void dropEvent(QDropEvent *event);

private:
    QHBoxLayout *m_conerLayout;
};

#endif // RIBBONTABWIDGET_H
