#pragma once

// Qt
#include <QList>
#include <QWidget> // Required for inheritance

class QCodeEditor;
class QSyntaxStyle;

/**
 * @brief Class, that describes line number area widget.
 */
class QLineNumberArea : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     * @param parent Pointer to parent QTextEdit widget.
     */
    explicit QLineNumberArea(QCodeEditor *parent = nullptr);

    // Disable copying
    QLineNumberArea(const QLineNumberArea &) = delete;
    QLineNumberArea &operator=(const QLineNumberArea &) = delete;

    /**
     * @brief Overridden method for getting line number area
     * size.
     */
    QSize sizeHint() const override;

    /**
     * @brief Method for setting syntax style object.
     * @param style Pointer to syntax style.
     */
    void setSyntaxStyle(QSyntaxStyle *style);

    /**
     * @brief Method for getting syntax style.
     * @return Pointer to syntax style.
     */
    QSyntaxStyle *syntaxStyle() const;

    QList<qsizetype> breakPoints() const;

    bool addBreakPoint(qsizetype lineNumber);

    bool removeBreakPoint(qsizetype lineNumber);

    void clearBreakPoint();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QSyntaxStyle *m_syntaxStyle;
    QList<qsizetype> m_breakPoints;
    QCodeEditor *m_codeEditParent;
};
