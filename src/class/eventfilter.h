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
