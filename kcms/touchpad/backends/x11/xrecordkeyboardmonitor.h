#pragma once

#include <QObject>
#include <QSocketNotifier>
#include <QVector>

#include <xcb/record.h>
#include <xcb/xcb.h>

class XRecordKeyboardMonitor : public QObject
{
    Q_OBJECT

public:
    explicit XRecordKeyboardMonitor(const char *displayName, QObject *parent = nullptr);
    ~XRecordKeyboardMonitor() override;
    bool isValid() const;

Q_SIGNALS:
    void keyboardActivityStarted();
    void keyboardActivityFinished();

private:
    void processReplies();
    void processReply(xcb_record_enable_context_reply_t *reply);
    bool activity() const;

    xcb_connection_t *m_connection = nullptr;
    xcb_record_context_t m_context = XCB_NONE;
    xcb_record_enable_context_cookie_t m_cookie{};
    QSocketNotifier *m_notifier = nullptr;
    QVector<bool> m_modifier;
    QVector<bool> m_ignore;
    QVector<bool> m_pressed;
    int m_modifiersPressed = 0;
    int m_keysPressed = 0;
};
