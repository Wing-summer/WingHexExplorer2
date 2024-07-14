#include "aboutsoftwaredialog.h"

#include <QLabel>
#include <QPixmap>
#include <QTextBrowser>

AboutSoftwareDialog::AboutSoftwareDialog(QWidget *parent)
    : FramelessDialog(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    QPixmap pic(":/com.wingsummer.winghex/images/author.jpg");

    auto l = new QLabel(this);
    l->setFixedSize(100, 100);
    l->setScaledContents(true);
    l->setPixmap(pic);
    layout->addWidget(l, 0, Qt::AlignHCenter);
    layout->addSpacing(10);
    auto b = new QTextBrowser(this);
    b->setSearchPaths(QStringList(
        {":/com.wingsummer.winghex", ":/com.wingsummer.winghex/images"}));
    b->setSource(QUrl("README.md"), QTextDocument::MarkdownResource);

    b->setFixedSize(800, 500);
    b->setOpenExternalLinks(true);
    layout->addWidget(b);

    buildUpContent(widget);

    setWindowTitle(tr("About"));
}
