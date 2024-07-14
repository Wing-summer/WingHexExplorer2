#include "wingmessagebox.h"
#include "../dialog/framelessdialog.h"

WingMessageBox::WingMessageBox() {}

void WingMessageBox::aboutQt(QWidget *parent, const QString &title) {
    QString translatedTextAboutQtCaption;
    translatedTextAboutQtCaption =
        QMessageBox::tr("<h3>About Qt</h3>"
                        "<p>This program uses Qt version %1.</p>")
            .arg(QT_VERSION_STR);

    //: Leave this text untranslated or include a verbatim copy of it below
    //: and note that it is the authoritative version in case of doubt.
    const QString translatedTextAboutQtText =
        QMessageBox::tr(
            "<p>Qt is a C++ toolkit for cross-platform application "
            "development.</p>"
            "<p>Qt provides single-source portability across all major desktop "
            "operating systems. It is also available for embedded Linux and "
            "other embedded and mobile operating systems.</p>"
            "<p>Qt is available under multiple licensing options designed "
            "to accommodate the needs of our various users.</p>"
            "<p>Qt licensed under our commercial license agreement is "
            "appropriate for development of proprietary/commercial software "
            "where you do not want to share any source code with third parties "
            "or otherwise cannot comply with the terms of GNU (L)GPL.</p>"
            "<p>Qt licensed under GNU (L)GPL is appropriate for the "
            "development of Qt&nbsp;applications provided you can comply with "
            "the terms "
            "and conditions of the respective licenses.</p>"
            "<p>Please see <a href=\"https://%2/\">%2</a> "
            "for an overview of Qt licensing.</p>"
            "<p>Copyright (C) The Qt Company Ltd. and other "
            "contributors.</p>"
            "<p>Qt and the Qt logo are trademarks of The Qt Company Ltd.</p>"
            "<p>Qt is The Qt Company Ltd. product developed as an open source "
            "project. See <a href=\"https://%3/\">%3</a> for more "
            "information.</p>")
            .arg(QStringLiteral("qt.io/licensing"), QStringLiteral("qt.io"));

    auto msgbox = new QMessageBox(parent);
    msgbox->setText(translatedTextAboutQtCaption);
    msgbox->setInformativeText(translatedTextAboutQtText);

    QPixmap pm(":/qt-project.org/qmessagebox/images/qtlogo-64.png");
    if (!pm.isNull())
        msgbox->setIconPixmap(pm);

    msgbox->setWindowFlag(Qt::Widget);

    FramelessDialog d(parent);
    d.buildUpContent(msgbox);
    d.setWindowTitle(title.isEmpty() ? QMessageBox::tr("About Qt") : title);

    QObject::connect(msgbox, &QMessageBox::finished, &d,
                     &FramelessDialog::done);

    d.exec();
}

QMessageBox::StandardButton
WingMessageBox::information(QWidget *parent, const QString &title,
                            const QString &text,
                            QMessageBox::StandardButtons buttons,
                            QMessageBox::StandardButton defaultButton) {
    return msgbox(parent, QMessageBox::Information, title, text, buttons,
                  defaultButton);
}

QMessageBox::StandardButton
WingMessageBox::question(QWidget *parent, const QString &title,
                         const QString &text,
                         QMessageBox::StandardButtons buttons,
                         QMessageBox::StandardButton defaultButton) {
    return msgbox(parent, QMessageBox::Question, title, text, buttons,
                  defaultButton);
}

QMessageBox::StandardButton
WingMessageBox::warning(QWidget *parent, const QString &title,
                        const QString &text,
                        QMessageBox::StandardButtons buttons,
                        QMessageBox::StandardButton defaultButton) {
    return msgbox(parent, QMessageBox::Warning, title, text, buttons,
                  defaultButton);
}

QMessageBox::StandardButton
WingMessageBox::critical(QWidget *parent, const QString &title,
                         const QString &text,
                         QMessageBox::StandardButtons buttons,
                         QMessageBox::StandardButton defaultButton) {
    return msgbox(parent, QMessageBox::Critical, title, text, buttons,
                  defaultButton);
}

void WingMessageBox::about(QWidget *parent, const QString &title,
                           const QString &text) {
    msgbox(parent, QMessageBox::NoIcon, title, text);
}

QMessageBox::StandardButton
WingMessageBox::msgbox(QWidget *parent, QMessageBox::Icon icon,
                       const QString &title, const QString &text,
                       QMessageBox::StandardButtons buttons,
                       QMessageBox::StandardButton defaultButton) {
    auto msgbox = new QMessageBox(parent);
    msgbox->setText(text);
    msgbox->setIcon(icon);
    msgbox->setStandardButtons(buttons);
    msgbox->setDefaultButton(defaultButton);

    msgbox->setWindowFlag(Qt::Widget);

    FramelessDialog d(parent);
    d.buildUpContent(msgbox);
    d.setWindowTitle(title.isEmpty() ? qAppName() : title);

    QObject::connect(msgbox, &QMessageBox::finished, &d,
                     &FramelessDialog::done);

    return static_cast<QMessageBox::StandardButton>(d.exec());
}
