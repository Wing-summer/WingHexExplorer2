#ifndef SKINMANAGER_H
#define SKINMANAGER_H

#include <QObject>

class SkinManager : public QObject {
    Q_OBJECT
public:
    enum class Theme {
        Dark,
        Light,
    };
    Q_ENUM(Theme)

public:
    virtual ~SkinManager() override;

    static SkinManager &instance();

    void setTheme(Theme theme);
    Theme currentTheme() const;

private:
    Theme m_theme;

    explicit SkinManager(QObject *parent = nullptr);

    Q_DISABLE_COPY(SkinManager)
};

#endif // SKINMANAGER_H
