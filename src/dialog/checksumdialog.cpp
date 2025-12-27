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

#include "checksumdialog.h"
#include "utilities.h"

#include "class/cryptographichash.h"
#include "control/toast.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QShortcut>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

CheckSumDialog::CheckSumDialog(QVector<int> &cs, QWidget *parent)
    : FramelessDialogBase(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    this->setFixedSize(500, 600);
    auto l = new QLabel(tr("ChooseCheckSum"), this);
    layout->addWidget(l);
    layout->addSpacing(5);
    hashlist = new QTreeWidget(this);
    hashlist->setColumnCount(1);
    hashlist->header()->hide();
    hashlist->setContextMenuPolicy(Qt::ActionsContextMenu);
    hashlist->addAction(tr("CheckAll"), this, [this]() {
        auto total = hashlist->topLevelItemCount();
        for (int i = 0; i < total; ++i) {
            auto parent = hashlist->topLevelItem(i);
            parent->setCheckState(0, Qt::Checked);
            auto total = parent->childCount();
            for (int i = 0; i < total; ++i) {
                auto c = parent->child(i);
                c->setCheckState(0, Qt::Checked);
            }
        }
    });
    hashlist->addAction(tr("UncheckAll"), this, [this]() {
        auto total = hashlist->topLevelItemCount();
        for (int i = 0; i < total; ++i) {
            auto parent = hashlist->topLevelItem(i);
            parent->setCheckState(0, Qt::Unchecked);
            auto total = parent->childCount();
            for (int i = 0; i < total; ++i) {
                auto c = parent->child(i);
                c->setCheckState(0, Qt::Unchecked);
            }
        }
    });
    populateData(cs);
    auto total = hashlist->topLevelItemCount();
    for (int i = 0; i < total; ++i) {
        updateCheckStatus(hashlist->topLevelItem(i));
    }

    connect(hashlist, &QTreeWidget::itemChanged, this,
            &CheckSumDialog::onItemChanged);
    connect(hashlist, &QTreeWidget::itemClicked, this,
            &CheckSumDialog::onItemClicked);
    layout->addWidget(hashlist);
    layout->addSpacing(10);
    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(dbbox, &QDialogButtonBox::accepted, this,
            &CheckSumDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this,
            &CheckSumDialog::on_reject);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &CheckSumDialog::on_accept);
    layout->addWidget(dbbox);

    buildUpContent(widget);
    setWindowTitle(tr("CheckSum"));
    setWindowIcon(ICONRES("sum"));
}

const QVector<int> &CheckSumDialog::getResults() { return _result; }

void CheckSumDialog::on_accept() {
    _result.clear();
    auto total = hashlist->topLevelItemCount();
    for (int i = 0; i < total; ++i) {
        collectCheckedLeaves(hashlist->topLevelItem(i), _result);
    }
    if (_result.isEmpty()) {
        Toast::toast(this, NAMEICONRES("sum"), tr("Nothing2Checksum"));
        return;
    }
    done(1);
}

void CheckSumDialog::on_reject() { done(0); }

void CheckSumDialog::onItemChanged(QTreeWidgetItem *item, int column) {
    if (!item)
        return;
    if (m_updating)
        return;

    m_updating = true;
    auto parent = item->parent();
    while (parent) {
        updateCheckStatus(parent);
        parent = parent->parent();
    }
    m_updating = false;
}

void CheckSumDialog::onItemClicked(QTreeWidgetItem *item, int) {
    if (!item)
        return;
    if (m_updating)
        return;

    if (item->childCount() > 0) {
        m_updating = true;

        bool anyUnchecked = false;
        for (int i = 0; i < item->childCount(); ++i) {
            if (item->child(i)->checkState(0) != Qt::Checked) {
                anyUnchecked = true;
                break;
            }
        }
        Qt::CheckState target = anyUnchecked ? Qt::Checked : Qt::Unchecked;

        for (int i = 0; i < item->childCount(); ++i) {
            item->child(i)->setCheckState(0, target);
        }

        item->setCheckState(0, target);

        m_updating = false;
    }
}

void CheckSumDialog::collectCheckedLeaves(QTreeWidgetItem *item,
                                          QList<int> &out) {
    if (!item)
        return;
    if (item->childCount() == 0) {
        if (item->checkState(0) == Qt::Checked) {
            auto data = item->data(0, Qt::UserRole);
            if (data.isValid()) {
                out.append(data.toInt());
            }
        }
    } else {
        for (int i = 0; i < item->childCount(); ++i) {
            collectCheckedLeaves(item->child(i), out);
        }
    }
}

void CheckSumDialog::populateData(QVector<int> &cs) {
    hashlist->clear();

    auto make_data = [](CryptographicHash::Algorithm m) -> QPair<QString, int> {
        auto e = QMetaEnum::fromType<CryptographicHash::Algorithm>();
        auto n = int(m);
        return qMakePair(QString::fromLatin1(e.valueToKey(n)), n);
    };

    static QHash<QString, QList<QPair<QString, int>>> buffer{
        {QStringLiteral("LegacyHash"),
         {make_data(CryptographicHash::Algorithm::Md4),
          make_data(CryptographicHash::Algorithm::Md5)}},
        {QStringLiteral("Checksum"),
         {make_data(CryptographicHash::Algorithm::Crc16),
          make_data(CryptographicHash::Algorithm::Crc32),
          make_data(CryptographicHash::Algorithm::Crc64)}},
        {QStringLiteral("SHA1"),
         {make_data(CryptographicHash::Algorithm::Sha1)}},
        {QStringLiteral("SHA2"),
         {make_data(CryptographicHash::Algorithm::Sha224),
          make_data(CryptographicHash::Algorithm::Sha256),
          make_data(CryptographicHash::Algorithm::Sha384),
          make_data(CryptographicHash::Algorithm::Sha512)}},
        {QStringLiteral("Keccak"),
         {make_data(CryptographicHash::Algorithm::Keccak_224),
          make_data(CryptographicHash::Algorithm::Keccak_256),
          make_data(CryptographicHash::Algorithm::Keccak_384),
          make_data(CryptographicHash::Algorithm::Keccak_512)}},
        {QStringLiteral("SHA3"),
         {make_data(CryptographicHash::Algorithm::RealSha3_224),
          make_data(CryptographicHash::Algorithm::RealSha3_256),
          make_data(CryptographicHash::Algorithm::RealSha3_384),
          make_data(CryptographicHash::Algorithm::RealSha3_512)}},
        {QStringLiteral("Blake2b"),
         {make_data(CryptographicHash::Algorithm::Blake2b_160),
          make_data(CryptographicHash::Algorithm::Blake2b_256),
          make_data(CryptographicHash::Algorithm::Blake2b_384),
          make_data(CryptographicHash::Algorithm::Blake2b_512)}},
        {QStringLiteral("Blake2s"),
         {make_data(CryptographicHash::Algorithm::Blake2s_128),
          make_data(CryptographicHash::Algorithm::Blake2s_160),
          make_data(CryptographicHash::Algorithm::Blake2s_224),
          make_data(CryptographicHash::Algorithm::Blake2s_256)}}};

    std::sort(cs.begin(), cs.end());
    for (auto it = buffer.constBegin(); it != buffer.constEnd(); ++it) {
        const auto &rootName = it.key();
        const auto &children = it.value();

        auto root = new QTreeWidgetItem(hashlist);
        root->setText(0, rootName);

        root->setFlags(root->flags() | Qt::ItemIsUserCheckable |
                       Qt::ItemIsSelectable | Qt::ItemIsEnabled |
                       Qt::ItemIsUserTristate);

        for (auto &item : children) {
            QTreeWidgetItem *child = new QTreeWidgetItem(root);
            child->setText(0, item.first);
            child->setData(0, Qt::UserRole, item.second);
            child->setFlags(child->flags() | Qt::ItemIsUserCheckable |
                            Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            child->setCheckState(
                0, std::binary_search(cs.begin(), cs.end(), item.second)
                       ? Qt::Checked
                       : Qt::Unchecked);
        }

        root->setCheckState(0, Qt::Checked);
    }

    hashlist->sortItems(0, Qt::AscendingOrder);
    hashlist->expandAll();
}

void CheckSumDialog::updateCheckStatus(QTreeWidgetItem *parent) {
    int checked = 0, unchecked = 0;
    int total = parent->childCount();
    for (int i = 0; i < total; ++i) {
        Qt::CheckState cs = parent->child(i)->checkState(0);
        if (cs == Qt::Checked)
            ++checked;
        else if (cs == Qt::Unchecked)
            ++unchecked;
    }
    if (checked == total) {
        parent->setCheckState(0, Qt::Checked);
    } else if (unchecked == total) {
        parent->setCheckState(0, Qt::Unchecked);
    } else {
        parent->setCheckState(0, Qt::PartiallyChecked);
    }
}
