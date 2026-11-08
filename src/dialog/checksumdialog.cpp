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

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QShortcut>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include "class/settings/thememanager.h"
#include "class/util/cryptographichash.h"
#include "control/toast.h"

CheckSumDialog::CheckSumDialog(QVector<int> &cs, QWidget *parent)
    : FramelessDialogBase(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    this->setFixedSize(500, 600);
    auto l = new QLabel(tr("Select checksums to calculate"), this);
    layout->addWidget(l);
    layout->addSpacing(5);
    m_hashlist = new QTreeWidget(this);
    m_hashlist->setColumnCount(1);
    m_hashlist->header()->hide();
    m_hashlist->setContextMenuPolicy(Qt::ActionsContextMenu);
    m_hashlist->addAction(tr("Select all"), this, [this]() {
        auto total = m_hashlist->topLevelItemCount();
        for (int i = 0; i < total; ++i) {
            auto parent = m_hashlist->topLevelItem(i);
            parent->setCheckState(0, Qt::Checked);
            auto total = parent->childCount();
            for (int i = 0; i < total; ++i) {
                auto c = parent->child(i);
                c->setCheckState(0, Qt::Checked);
            }
        }
    });
    m_hashlist->addAction(tr("Clear selection"), this, [this]() {
        auto total = m_hashlist->topLevelItemCount();
        for (int i = 0; i < total; ++i) {
            auto parent = m_hashlist->topLevelItem(i);
            parent->setCheckState(0, Qt::Unchecked);
            auto total = parent->childCount();
            for (int i = 0; i < total; ++i) {
                auto c = parent->child(i);
                c->setCheckState(0, Qt::Unchecked);
            }
        }
    });
    populateData(cs);
    auto total = m_hashlist->topLevelItemCount();
    for (int i = 0; i < total; ++i) {
        updateCheckStatus(m_hashlist->topLevelItem(i));
    }

    connect(m_hashlist, &QTreeWidget::itemChanged, this,
            &CheckSumDialog::onItemChanged);
    connect(m_hashlist, &QTreeWidget::itemClicked, this,
            &CheckSumDialog::onItemClicked);
    layout->addWidget(m_hashlist);
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
    setWindowTitle(tr("Checksum"));
    setWindowIcon(ThemeManager::instance().standardIcon(
        ThemeManager::StandardIcon::Checksum));
}

const QVector<int> &CheckSumDialog::getResults() { return m_result; }

void CheckSumDialog::on_accept() {
    m_result.clear();
    auto total = m_hashlist->topLevelItemCount();
    for (int i = 0; i < total; ++i) {
        collectCheckedLeaves(m_hashlist->topLevelItem(i), m_result);
    }
    if (m_result.isEmpty()) {
        Toast::toast(this,
                     ThemeManager::instance().standardPixmap(
                         ThemeManager::StandardIcon::Checksum),
                     tr("No files available for checksum calculation"));
        return;
    }
    done(1);
}

void CheckSumDialog::on_reject() { done(0); }

void CheckSumDialog::onItemChanged(QTreeWidgetItem *item, int column) {
    if (!item) {
        return;
    }
    if (m_updating) {
        return;
    }

    m_updating = true;
    auto parent = item->parent();
    while (parent) {
        updateCheckStatus(parent);
        parent = parent->parent();
    }
    m_updating = false;
}

void CheckSumDialog::onItemClicked(QTreeWidgetItem *item, int) {
    if (!item) {
        return;
    }
    if (m_updating) {
        return;
    }

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
    if (!item) {
        return;
    }
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
    m_hashlist->clear();

    auto makeData = [](CryptographicHash::Algorithm m) -> QPair<QString, int> {
        auto e = QMetaEnum::fromType<CryptographicHash::Algorithm>();
        auto n = int(m);
        return qMakePair(QString::fromLatin1(e.valueToKey(n)), n);
    };

    static QHash<QString, QList<QPair<QString, int>>> buffer{
        {QStringLiteral("LegacyHash"),
         {makeData(CryptographicHash::Algorithm::Md4),
          makeData(CryptographicHash::Algorithm::Md5)}},
        {QStringLiteral("Checksum"),
         {makeData(CryptographicHash::Algorithm::CRC8),
          makeData(CryptographicHash::Algorithm::CRC8_MAXIM),
          makeData(CryptographicHash::Algorithm::CRC16),
          makeData(CryptographicHash::Algorithm::CRC16_CCITT_FALSE),
          makeData(CryptographicHash::Algorithm::CRC16_MODBUS),
          makeData(CryptographicHash::Algorithm::CRC32),
          makeData(CryptographicHash::Algorithm::CRC32_C),
          makeData(CryptographicHash::Algorithm::CRC64)}},
        {QStringLiteral("SHA1"),
         {makeData(CryptographicHash::Algorithm::Sha1)}},
        {QStringLiteral("SHA2"),
         {makeData(CryptographicHash::Algorithm::Sha224),
          makeData(CryptographicHash::Algorithm::Sha256),
          makeData(CryptographicHash::Algorithm::Sha384),
          makeData(CryptographicHash::Algorithm::Sha512)}},
        {QStringLiteral("Keccak"),
         {makeData(CryptographicHash::Algorithm::Keccak_224),
          makeData(CryptographicHash::Algorithm::Keccak_256),
          makeData(CryptographicHash::Algorithm::Keccak_384),
          makeData(CryptographicHash::Algorithm::Keccak_512)}},
        {QStringLiteral("SHA3"),
         {makeData(CryptographicHash::Algorithm::RealSha3_224),
          makeData(CryptographicHash::Algorithm::RealSha3_256),
          makeData(CryptographicHash::Algorithm::RealSha3_384),
          makeData(CryptographicHash::Algorithm::RealSha3_512)}},
        {QStringLiteral("Blake2b"),
         {makeData(CryptographicHash::Algorithm::Blake2b_160),
          makeData(CryptographicHash::Algorithm::Blake2b_256),
          makeData(CryptographicHash::Algorithm::Blake2b_384),
          makeData(CryptographicHash::Algorithm::Blake2b_512)}},
        {QStringLiteral("Blake2s"),
         {makeData(CryptographicHash::Algorithm::Blake2s_128),
          makeData(CryptographicHash::Algorithm::Blake2s_160),
          makeData(CryptographicHash::Algorithm::Blake2s_224),
          makeData(CryptographicHash::Algorithm::Blake2s_256)}}};

    std::sort(cs.begin(), cs.end());
    for (auto it = buffer.constBegin(); it != buffer.constEnd(); ++it) {
        const auto &rootName = it.key();
        const auto &children = it.value();

        auto root = new QTreeWidgetItem(m_hashlist);
        root->setText(0, rootName);

        root->setFlags(root->flags() | Qt::ItemIsUserCheckable |
                       Qt::ItemIsSelectable | Qt::ItemIsEnabled |
                       Qt::ItemIsUserTristate);

        for (const auto &item : children) {
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

    m_hashlist->sortItems(0, Qt::AscendingOrder);
    m_hashlist->expandAll();
}

void CheckSumDialog::updateCheckStatus(QTreeWidgetItem *parent) {
    int checked = 0, unchecked = 0;
    int total = parent->childCount();
    for (int i = 0; i < total; ++i) {
        Qt::CheckState cs = parent->child(i)->checkState(0);
        if (cs == Qt::Checked) {
            ++checked;
        } else if (cs == Qt::Unchecked) {
            ++unchecked;
        }
    }
    if (checked == total) {
        parent->setCheckState(0, Qt::Checked);
    } else if (unchecked == total) {
        parent->setCheckState(0, Qt::Unchecked);
    } else {
        parent->setCheckState(0, Qt::PartiallyChecked);
    }
}
