#include "xrecordkeyboardmonitor.h"

#include <cstdlib>
#include <limits>
#include <memory>

#include <xcb/xcbext.h>

namespace
{
struct FreeDeleter {
    void operator()(void *pointer) const
    {
        std::free(pointer);
    }
};
}

XRecordKeyboardMonitor::XRecordKeyboardMonitor(const char *displayName, QObject *parent)
    : QObject(parent)
    , m_connection(xcb_connect(displayName, nullptr))
{
    if (!m_connection || xcb_connection_has_error(m_connection)) {
        return;
    }

    const auto modifierCookie = xcb_get_modifier_mapping(m_connection);
    m_context = xcb_generate_id(m_connection);
    xcb_record_range_t range{};
    range.device_events.first = XCB_KEY_PRESS;
    range.device_events.last = XCB_KEY_RELEASE;
    const xcb_record_client_spec_t clients = XCB_RECORD_CS_ALL_CLIENTS;
    xcb_record_create_context(m_connection, m_context, 0, 1, 1, &clients, &range);
    xcb_flush(m_connection);

    std::unique_ptr<xcb_get_modifier_mapping_reply_t, FreeDeleter> modifiers(xcb_get_modifier_mapping_reply(m_connection, modifierCookie, nullptr));
    if (!modifiers) {
        return;
    }

    constexpr int keycodeCount = std::numeric_limits<xcb_keycode_t>::max() + 1;
    m_modifier.fill(false, keycodeCount);
    m_ignore.fill(false, keycodeCount);
    m_pressed.fill(false, keycodeCount);
    const int count = xcb_get_modifier_mapping_keycodes_length(modifiers.get());
    const xcb_keycode_t *keycodes = xcb_get_modifier_mapping_keycodes(modifiers.get());
    for (int i = 0; i < count; ++i) {
        m_modifier[keycodes[i]] = true;
        if (i < modifiers->keycodes_per_modifier) {
            m_ignore[keycodes[i]] = true;
        }
    }

    m_cookie = xcb_record_enable_context(m_connection, m_context);
    xcb_flush(m_connection);
    m_notifier = new QSocketNotifier(xcb_get_file_descriptor(m_connection), QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, &XRecordKeyboardMonitor::processReplies);
}

XRecordKeyboardMonitor::~XRecordKeyboardMonitor()
{
    if (!m_connection) {
        return;
    }
    if (m_notifier) {
        m_notifier->setEnabled(false);
    }
    if (m_context != XCB_NONE && !xcb_connection_has_error(m_connection)) {
        xcb_record_disable_context(m_connection, m_context);
        xcb_record_free_context(m_connection, m_context);
        xcb_flush(m_connection);
    }
    xcb_disconnect(m_connection);
}

bool XRecordKeyboardMonitor::isValid() const
{
    return m_connection && !xcb_connection_has_error(m_connection) && m_notifier;
}

bool XRecordKeyboardMonitor::activity() const
{
    return m_keysPressed > 0 && m_modifiersPressed == 0;
}

void XRecordKeyboardMonitor::processReplies()
{
    xcb_generic_event_t *event = nullptr;
    while ((event = xcb_poll_for_event(m_connection))) {
        std::free(event);
    }

    void *reply = nullptr;
    xcb_generic_error_t *error = nullptr;
    while (m_cookie.sequence && xcb_poll_for_reply(m_connection, m_cookie.sequence, &reply, &error)) {
        std::unique_ptr<xcb_generic_error_t, FreeDeleter> ownedError(error);
        if (xcb_connection_has_error(m_connection) || error) {
            m_notifier->setEnabled(false);
            return;
        }
        if (!reply) {
            continue;
        }
        std::unique_ptr<xcb_record_enable_context_reply_t, FreeDeleter> ownedReply(static_cast<xcb_record_enable_context_reply_t *>(reply));
        processReply(ownedReply.get());
    }
}

void XRecordKeyboardMonitor::processReply(xcb_record_enable_context_reply_t *reply)
{
    const bool previousActivity = activity();
    bool hadActivity = previousActivity;
    const auto *events = reinterpret_cast<const xcb_key_press_event_t *>(xcb_record_enable_context_data(reply));
    const int eventCount = xcb_record_enable_context_data_length(reply) / int(sizeof(xcb_key_press_event_t));
    for (int i = 0; i < eventCount; ++i) {
        const auto &event = events[i];
        if ((event.response_type != XCB_KEY_PRESS && event.response_type != XCB_KEY_RELEASE) || event.detail >= m_pressed.size() || m_ignore[event.detail]) {
            continue;
        }
        const bool pressed = event.response_type == XCB_KEY_PRESS;
        if (m_pressed[event.detail] == pressed) {
            continue;
        }
        m_pressed[event.detail] = pressed;
        int &counter = m_modifier[event.detail] ? m_modifiersPressed : m_keysPressed;
        counter = qMax(0, counter + (pressed ? 1 : -1));
        hadActivity |= activity();
    }
    if (!previousActivity && activity()) {
        Q_EMIT keyboardActivityStarted();
    } else if (!activity() && hadActivity) {
        Q_EMIT keyboardActivityFinished();
    }
}
