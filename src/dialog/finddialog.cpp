#include "finddialog.h"
#include "utilities.h"

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringDecoder>
#else
#include <QTextCodec>
#endif

FindDialog::FindDialog(bool isBigFile, int start, int stop, bool sel,
                       QWidget *parent)
    : FramelessDialogBase(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    m_string = new QRadioButton(this);
    m_string->setText(tr("findstring"));
    layout->addWidget(m_string);
    layout->addSpacing(3);

    m_encodings = new QComboBox(this);
    m_encodings->addItems(Utilities::getEncodings());
    m_encodings->setCurrentIndex(0);
    m_encodings->setEnabled(false);
    connect(m_string, &QRadioButton::toggled, m_encodings,
            &QComboBox::setEnabled);
    layout->addWidget(m_encodings);
    layout->addSpacing(3);

    m_lineeditor = new QLineEdit(this);
    m_lineeditor->setEnabled(false);
    connect(m_string, &QRadioButton::toggled, m_lineeditor,
            &QLineEdit::setEnabled);
    layout->addWidget(m_lineeditor);
    layout->addSpacing(3);

    m_hex = new QRadioButton(this);
    m_hex->setText(tr("findhex"));
    layout->addWidget(m_hex);
    layout->addSpacing(3);

    m_hexeditor = new QHexView(this);
    m_hexeditor->setAsciiVisible(false);
    m_hexeditor->setAddressVisible(false);
    m_hexeditor->setEnabled(true);
    connect(m_hex, &QRadioButton::toggled, m_hexeditor, &QHexView::setEnabled);
    layout->addWidget(m_hexeditor);
    layout->addSpacing(10);

    m_hex->setChecked(true);

    auto regionw = new QWidget(this);
    auto regionLayout = new QHBoxLayout(regionw);

    regionLayout->addWidget(new QLabel(tr("Region:"), regionw));

    m_regionStart = new QSpinBox(regionw);
    Q_ASSERT(stop >= start);
    m_regionStart->setRange(start, stop);
    m_regionStart->setEnabled(false);
    m_regionStart->setValue(start);
    m_regionStart->setDisplayIntegerBase(16);
    m_regionStart->setPrefix(QStringLiteral("0x"));
    regionLayout->addWidget(m_regionStart, 1);

    regionLayout->addWidget(new QLabel(QStringLiteral(" - "), regionw));

    m_regionStop = new QSpinBox(regionw);
    m_regionStop->setRange(start, stop);
    connect(m_regionStart, QOverload<int>::of(&QSpinBox::valueChanged),
            m_regionStop, &QSpinBox::setMinimum);
    m_regionStop->setEnabled(false);
    m_regionStop->setValue(qMin(start + 1024 * 1024, stop));
    m_regionStop->setDisplayIntegerBase(16);
    m_regionStop->setPrefix(QStringLiteral("0x"));
    regionLayout->addWidget(m_regionStop, 1);

    layout->addWidget(regionw);

    auto group = new QButtonGroup(this);
    group->setExclusive(true);

    auto btnBox = new QWidget(this);
    auto buttonLayout = new QHBoxLayout(btnBox);
    buttonLayout->setSpacing(0);

    int id = 0;

    auto b = new QPushButton(tr("None"), this);
    b->setCheckable(true);
    connect(b, &QPushButton::toggled, this, [=](bool b) {
        if (b) {
            _result.dir = SearchDirection::None;
        }
    });
    group->addButton(b, id++);
    b->setEnabled(!isBigFile);
    buttonLayout->addWidget(b);

    b = new QPushButton(tr("Region"), this);
    b->setCheckable(true);
    connect(b, &QPushButton::toggled, this, [=](bool b) {
        if (b) {
            _result.dir = SearchDirection::Region;
        }
        m_regionStart->setEnabled(b);
        m_regionStop->setEnabled(b);
    });
    group->addButton(b, id++);
    buttonLayout->addWidget(b);

    b = new QPushButton(tr("BeforeCursor"), this);
    b->setCheckable(true);
    connect(b, &QPushButton::toggled, this, [=](bool b) {
        if (b) {
            _result.dir = SearchDirection::Foreword;
        }
    });
    group->addButton(b, id++);
    buttonLayout->addWidget(b);

    b = new QPushButton(tr("AfterCursor"), this);
    b->setCheckable(true);
    connect(b, &QPushButton::toggled, this, [=](bool b) {
        if (b) {
            _result.dir = SearchDirection::Backword;
        }
    });
    group->addButton(b, id++);
    buttonLayout->addWidget(b);

    b = new QPushButton(tr("Selection"), this);
    b->setCheckable(true);
    if (sel) {
        connect(b, &QPushButton::toggled, this, [=](bool b) {
            if (b) {
                _result.dir = SearchDirection::Selection;
            }
        });
    } else {
        b->setEnabled(false);
    }

    group->addButton(b, id++);
    buttonLayout->addWidget(b);
    group->button(isBigFile ? 1 : 0)->setChecked(true);

    layout->addWidget(btnBox);
    layout->addSpacing(20);
    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(dbbox, &QDialogButtonBox::accepted, this, &FindDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this, &FindDialog::on_reject);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &FindDialog::on_accept);
    layout->addWidget(dbbox);

    buildUpContent(widget);
    this->setWindowTitle(tr("find"));
}

QByteArray FindDialog::getResult(Result &result) {
    _result.start = 0;
    _result.stop = 0;
    if (m_regionStart->isEnabled()) {
        _result.start = m_regionStart->value();
        _result.stop = m_regionStop->value();
    }
    result = _result;
    return _findarr;
}

void FindDialog::on_accept() {
    if (m_string->isChecked()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        auto en = QStringConverter::encodingForName(
            m_encodings->currentText().toUtf8());
        Q_ASSERT(en.has_value());
        QStringEncoder e(en.value());
        _findarr = e.encode(m_lineeditor->text());
#else
        auto en = QTextCodec::codecForName(m_encodings->currentText().toUtf8());
        auto e = en->makeEncoder();
        _findarr = e->fromUnicode(m_lineeditor->text());
#endif
    } else {
        _findarr =
            m_hexeditor->document()->read(0, int(m_hexeditor->documentBytes()));
    }
    done(1);
}

void FindDialog::on_reject() {
    _findarr.clear();
    done(0);
}
