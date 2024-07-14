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

private:
    QHBoxLayout *m_conerLayout;
};

#endif // RIBBONTABWIDGET_H
