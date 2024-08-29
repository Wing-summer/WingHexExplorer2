#ifndef QTABLEVIEWEXT_H
#define QTABLEVIEWEXT_H

#include <QTableView>

class QTableViewExt : public QTableView {
    Q_OBJECT
public:
    explicit QTableViewExt(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model) override;
};

#endif // QTABLEVIEWEXT_H
