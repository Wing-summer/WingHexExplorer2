#ifndef QPATHEDIT_H
#define QPATHEDIT_H

#include "../src/dialog/framelessdialogbase.h"
#include <QFileDialog>
#include <QIcon>
#include <QLineEdit>
#include <QPointer>
#include <QString>
#include <QWidget>

class QLineEdit;
class QCompleter;
class PathValidator;
class QFileSystemModel;
class QToolButton;

//! The QPathEdit provides a simple way to get a path from the user as
//! comfortable as possible
class QPathEdit : public QWidget {
    Q_OBJECT

    //! Defines the Widgets appereance
    Q_PROPERTY(Style style READ style WRITE setStyle)
    //! Holds the icon to be used for the edits button
    Q_PROPERTY(QIcon dialogButtonIcon READ dialogButtonIcon WRITE
                   setDialogButtonIcon RESET resetDialogButtonIcon)
    //! Specifies the kind of path to be entered
    Q_PROPERTY(PathMode pathMode READ pathMode WRITE setPathMode)
    //! Options for the QFileDialog
    Q_PROPERTY(QFileDialog::Options dialogOptions READ dialogOptions WRITE
                   setDialogOptions)
    //! Specifies whether the path can be manually entered or not
    Q_PROPERTY(bool editable READ isEditable WRITE setEditable)
    //! Specifies whether an empty path is allowed or not
    Q_PROPERTY(
        bool allowEmptyPath READ isEmptyPathAllowed WRITE setAllowEmptyPath)
    //! Turns the auto-completer for manual editing on and off
    Q_PROPERTY(bool useCompleter READ useCompleter WRITE setUseCompleter)
    //! Holds the default directory for the QFileDialog
    Q_PROPERTY(QString defaultDirectory READ defaultDirectory WRITE
                   setDefaultDirectory)
    //! Holds the currently entered, valid path
    Q_PROPERTY(
        QString path READ path WRITE setPath RESET clear NOTIFY pathChanged)
    //! Holds the currently entered text which might not be a valid path
    Q_PROPERTY(QString editPath READ editPath NOTIFY editPathChanged)
    //! Holds the information, whether the current edits contents are a valid
    //! path or not
    Q_PROPERTY(bool acceptableInput READ hasAcceptableInput NOTIFY
                   acceptableInputChanged)
    //! Specifiy a placeholder to be shown if no path is entered
    Q_PROPERTY(QString placeholder READ placeholder WRITE setPlaceholder)
    //! Holds name filters for the dialog and the completer
    Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters)
    //! Holds mime filters for the dialog and the completer
    Q_PROPERTY(QStringList mimeTypeFilters READ mimeTypeFilters WRITE
                   setMimeTypeFilters)

public:
    //! Descibes various styles that the edit can take
    enum Style {
        SeperatedButton, //!< The button to open the dialog will be place next
                         //!< to the edit
        JoinedButton, //!< The button to open the dialog will be placed inside
                      //!< the edit
        NoButton //!< The QFileDialog is completly disabled and will never show.
                 //!< Thus, there is no button either
    };
    Q_ENUM(Style)

    //! Describes modes for the kind of path
    enum PathMode {
        ExistingFile,   //!< A single, existings file. This is basically "Open
                        //!< file"
        ExistingFolder, //!< A single, existing directory. This is basically
                        //!< "Open Folder"
        AnyFile //!< A single, valid file, no matter if exisiting or not (the
                //!< directory, however, must exist). This is basically "Save
                //!< File"
    };
    Q_ENUM(PathMode)

    //! Constructs a new QPathEdit widget. The mode will be
    //! QPathEdit::ExistingFile
    explicit QPathEdit(QWidget *parent = nullptr,
                       Style style = SeperatedButton);
    //! Constructs a new QPathEdit widget
    explicit QPathEdit(PathMode pathMode, QWidget *parent = nullptr,
                       Style style = SeperatedButton);
    //! Constructs a new QPathEdit widget with the given default directory
    explicit QPathEdit(PathMode pathMode, const QString &defaultDirectory,
                       QWidget *parent = nullptr,
                       Style style = SeperatedButton);

    //! READ-ACCESSOR for QPathEdit::pathMode
    PathMode pathMode() const;
    //! READ-ACCESSOR for QPathEdit::dialogOptions
    QFileDialog::Options dialogOptions() const;
    //! READ-ACCESSOR for QPathEdit::allowEmptyPath
    bool isEmptyPathAllowed() const;
    //! READ-ACCESSOR for QPathEdit::defaultDirectory
    QString defaultDirectory() const;
    //! READ-ACCESSOR for QPathEdit::path
    QString path() const;
    //! READ-ACCESSOR for QPathEdit::editPath
    QString editPath() const;
    //! Returns the entered path as an QUrl
    QUrl pathUrl() const;
    //! READ-ACCESSOR for QPathEdit::acceptableInput
    bool hasAcceptableInput() const;
    //! READ-ACCESSOR for QPathEdit::placeholder
    QString placeholder() const;
    //! READ-ACCESSOR for QPathEdit::nameFilters
    QStringList nameFilters() const;
    //! READ-ACCESSOR for QPathEdit::mimeTypeFilters
    QStringList mimeTypeFilters() const;
    //! READ-ACCESSOR for QPathEdit::editable
    bool isEditable() const;
    //! READ-ACCESSOR for QPathEdit::useCompleter
    bool useCompleter() const;
    //! READ-ACCESSOR for QPathEdit::style
    Style style() const;
    //! READ-ACCESSOR for QPathEdit::dialogButtonIcon
    QIcon dialogButtonIcon() const;

    //! WRITE-ACCESSOR for QPathEdit::pathMode
    void setPathMode(PathMode pathMode);
    //! WRITE-ACCESSOR for QPathEdit::dialogOptions
    void setDialogOptions(QFileDialog::Options dialogOptions);
    //! WRITE-ACCESSOR for QPathEdit::allowEmptyPath
    void setAllowEmptyPath(bool allowEmptyPath);
    //! WRITE-ACCESSOR for QPathEdit::defaultDirectory
    void setDefaultDirectory(const QString &defaultDirectory);
    //! WRITE-ACCESSOR for QPathEdit::path
    bool setPath(QString path, bool allowInvalid = false);
    //! RESET-ACCESSOR for QPathEdit::path
    void clear();
    //! WRITE-ACCESSOR for QPathEdit::placeholder
    void setPlaceholder(const QString &placeholder);
    //! WRITE-ACCESSOR for QPathEdit::nameFilters
    void setNameFilters(const QStringList &nameFilters);
    //! WRITE-ACCESSOR for QPathEdit::mimeTypeFilters
    void setMimeTypeFilters(const QStringList &mimeTypeFilters);
    //! WRITE-ACCESSOR for QPathEdit::editable
    void setEditable(bool editable);
    //! WRITE-ACCESSOR for QPathEdit::useCompleter
    void setUseCompleter(bool useCompleter);
    //! WRITE-ACCESSOR for QPathEdit::style
    void
    setStyle(Style style,
             QLineEdit::ActionPosition position = QLineEdit::TrailingPosition);
    //! WRITE-ACCESSOR for QPathEdit::dialogButtonIcon
    void setDialogButtonIcon(const QIcon &icon);
    //! RESET-ACCESSOR for QPathEdit::dialogButtonIcon
    void resetDialogButtonIcon();

public slots:
    //! Shows the QFileDialog so the user can select a path
    void showDialog();

signals:
    //! NOTIFY-ACCESSOR for QPathEdit::path
    void pathChanged(QString path);
    //! NOTIFY-ACCESSOR for QPathEdit::editPath
    void editPathChanged(QString path);
    //! NOTIFY-ACCESSOR for QPathEdit::acceptableInput
    void acceptableInputChanged(bool acceptableInput);

private slots:
    void updateValidInfo(const QString &path = QString());
    void editTextUpdate();

    void dialogFileSelected(const QString &file);

private:
    QLineEdit *edit;
    QCompleter *pathCompleter;
    QFileSystemModel *completerModel;
    PathValidator *pathValidator;
    QFileDialog *dialog;

    FramelessDialogBase *wingdialog;

    QString currentValidPath;
    bool wasPathValid;

    Style uiStyle;
    PathMode mode;
    QString defaultDir;
    bool allowEmpty;

    QToolButton *toolButton;
    QAction *dialogAction;
    bool hasCustomIcon;

    QStringList modelFilters(const QStringList &normalFilters);
    QIcon getDefaultIcon();

    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // QPATHEDIT_H
