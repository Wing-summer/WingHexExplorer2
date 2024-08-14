/*
 * Copyright (C) Martijn Koopman
 * All Rights Reserved
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 */

#ifndef RIBBONTABCONTENT_H
#define RIBBONTABCONTENT_H

#include <QToolButton>
#include <QWidget>

namespace Ui {
class RibbonTabContent;
}

// modified by wingsummer

QT_FORWARD_DECLARE_CLASS(RibbonButtonGroup)

class RibbonTabContent : public QWidget {
    Q_OBJECT

public:
    explicit RibbonTabContent(QWidget *parent = nullptr);
    virtual ~RibbonTabContent();

    /// Add a group to the tab content.
    ///
    /// \param[in] groupName Name of the group
    RibbonButtonGroup *addGroup(const QString &groupName);

    /// Remove a group from the tab content.
    ///
    /// \param[in] groupName Name of the group
    void removeGroup(const QString &groupName);

    /// Remove a group from the tab content.
    ///
    /// \param[in] group Pointer of the group
    void removeGroup(RibbonButtonGroup *group);

    /// Get the number of button groups in this tab content.
    ///
    /// \return The number of button groups
    int groupCount() const;

protected:
    QSize sizeHint() const override;

private:
    Ui::RibbonTabContent *ui;
};

#endif // RIBBONTABCONTENT_H
