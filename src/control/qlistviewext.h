#ifndef QLISTVIEWEXT_H
#define QLISTVIEWEXT_H

#include <QListView>

class QListViewExt : public QListView {
    Q_OBJECT
public:
    explicit QListViewExt(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model) override;
};

#endif // QLISTVIEWEXT_H
