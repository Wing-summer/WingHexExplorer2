#ifndef FOCUSOUTEVENTFILTER_H
#define FOCUSOUTEVENTFILTER_H

#include <QEvent>
#include <QObject>

class EventFilter : public QObject {
    Q_OBJECT
public:
    EventFilter(QEvent::Type eventType, QObject *parent = nullptr)
        : QObject(parent), m_event(eventType) {}
    virtual ~EventFilter() override {}

signals:
    void eventTriggered(QObject *obj, QEvent *event);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == m_event) {
            emit eventTriggered(obj, event);
        }
        return QObject::eventFilter(obj, event);
    }

private:
    QEvent::Type m_event;
};

#endif // FOCUSOUTEVENTFILTER_H
