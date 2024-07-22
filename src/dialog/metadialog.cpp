#include "metadialog.h"
#include "src/control/toast.h"
#include "src/utilities.h"

#include "src/class/wingcolordialog.h"
#include <QDialogButtonBox>
#include <QShortcut>

MetaDialog::MetaDialog(QWidget *parent)
    : FramelessDialog(parent), _foreground(Qt::transparent),
      _background(Qt::transparent) {

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    cforeground = new QCheckBox(this);
    cforeground->setText(tr("Foreground"));
    layout->addWidget(cforeground);
    layout->addSpacing(2);

    iforeground = new QPushButton(this);
    iforeground->setEnabled(false);
    layout->addWidget(iforeground);

    layout->addSpacing(2);

    cbackground = new QCheckBox(this);
    cbackground->setText(tr("Background"));
    layout->addWidget(cbackground);

    layout->addSpacing(2);

    ibackground = new QPushButton(this);
    ibackground->setEnabled(false);
    layout->addWidget(ibackground);

    layout->addSpacing(2);

    ccomment = new QCheckBox(this);
    ccomment->setText(tr("Comment"));
    layout->addWidget(ccomment);
    layout->addSpacing(2);

    m_comment = new QLineEdit(this);
    layout->addWidget(m_comment);
    m_comment->setEnabled(false);

    layout->addSpacing(5);

    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(dbbox);
    layout->addSpacing(2);

    connect(cforeground, &QCheckBox::clicked, iforeground,
            &QPushButton::setEnabled);
    connect(cbackground, &QCheckBox::clicked, ibackground,
            &QPushButton::setEnabled);
    connect(ccomment, &QCheckBox::clicked, m_comment, &QPushButton::setEnabled);
    connect(iforeground, &QPushButton::clicked, this, [this] {
        auto color = WingColorDialog::getColor(tr("SelectColor"), this);
        if (color.isValid()) {
            this->setPaintedColor(iforeground, color);
            _foreground = color;
        }
    });
    connect(ibackground, &QPushButton::clicked, this, [=] {
        auto color = WingColorDialog::getColor(tr("SelectColor"), this);
        if (color.isValid()) {
            this->setPaintedColor(ibackground, color);
            _background = color;
        }
    });
    connect(dbbox, &QDialogButtonBox::accepted, this, &MetaDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this, &MetaDialog::on_reject);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &MetaDialog::on_accept);

    buildUpContent(widget);
    setWindowTitle(tr("Metadata"));
}

void MetaDialog::on_accept() {
    if ((cforeground->isChecked() &&
         (!_foreground.isValid() || _foreground.rgba() == 0)) ||
        (cbackground->isChecked() &&
         (!_background.isValid() || _background.rgba() == 0)) ||
        (ccomment->isChecked() && m_comment->text().trimmed().length() == 0)) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("metadata")),
                     tr("NoChoose"));
        return;
    }

    _comment = ccomment->text();
    done(1);
}

void MetaDialog::on_reject() { done(0); }

void MetaDialog::setPaintedColor(QPushButton *btn, const QColor &color) {
    Q_ASSERT(btn);
    btn->setStyleSheet(QStringLiteral("background-color: ") + color.name());
}

QString MetaDialog::comment() {
    if (ccomment->isChecked())
        return m_comment->text();
    else
        return QString();
}

QColor MetaDialog::foreGroundColor() {
    if (cforeground->isChecked())
        return _foreground;
    else
        return QColor::fromRgba(qRgba(0, 0, 0, 0));
}

QColor MetaDialog::backGroundColor() {
    if (cbackground->isChecked())
        return _background;
    else
        return QColor::fromRgba(qRgba(0, 0, 0, 0));
}

void MetaDialog::setComment(QString comment) {
    if (comment.length() > 0) {
        ccomment->setChecked(true);
        emit ccomment->clicked(true);
        _comment = comment;
        m_comment->setText(comment);
    }
}

void MetaDialog::setBackGroundColor(QColor color) {
    if (color.rgba()) {
        cbackground->setChecked(true);
        emit cbackground->clicked(true);
        _background = color;
        this->setPaintedColor(ibackground, color);
    }
}

void MetaDialog::setForeGroundColor(QColor color) {
    if (color.rgba()) {
        cforeground->setChecked(true);
        emit cforeground->clicked(true);
        _foreground = color;
        this->setPaintedColor(iforeground, color);
    }
}
