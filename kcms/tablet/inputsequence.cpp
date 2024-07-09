#include "inputsequence.h"

#include <QtCore/QStringList>

InputSequence::InputSequence() = default;

InputSequence::InputSequence(const QStringList &config)
{
    if (config.empty()) {
        return;
    }

    const QString type = config.first();
    if (type == "Key") {
        setType(Type::Keyboard);
    } else if (type == "MouseButton") {
        setType(Type::Mouse);
    } else if (type == "ApplicationDefined") {
        setType(Type::ApplicationDefined);
    }
}

InputSequence::Type InputSequence::type() const
{
    return m_type;
}

void InputSequence::setType(const Type type)
{
    if (m_type != type) {
        m_type = type;

        // Make sure to reset the data when the type changed
        switch (m_type) {
        case Type::Keyboard:
            m_data = KeyData{""};
            break;
        case Type::Mouse:
            m_data = MouseData{};
            break;
        case Type::ApplicationDefined:
            m_data = ApplicationDefinedData{};
            break;
        default:
            Q_UNREACHABLE();
        }
    }
}

QStringList InputSequence::toConfigFormat() const
{
    switch (m_type) {
    case Type::Keyboard: {
        const auto key = keyData().toString(QKeySequence::PortableText);
        return QStringList{"Key", key};
    }
    case Type::Mouse: {
        const auto mouse = mouseData();
        return QStringList{"MouseButton", QString::number(mouse.button)};
    }
    case Type::ApplicationDefined:
        return QStringList{"ApplicationDefined"};
    default:
        Q_UNREACHABLE();
    }
}

QString InputSequence::toString() const
{
    switch (m_type) {
    case Type::Keyboard: {
        return QString{"Key"};
    }
    case Type::Mouse: {
        return QString{"MouseButton"};
    }
    case Type::ApplicationDefined:
        return QString{"ApplicationDefined"};
    default:
        Q_UNREACHABLE();
    }
}

QKeySequence InputSequence::keySequence() const
{
    return keyData();
}

void InputSequence::setKeySequence(const QKeySequence &sequence)
{
    keyData() = sequence;
}

Qt::MouseButton InputSequence::mouseButton() const
{
    return mouseData().button;
}

void InputSequence::setMouseButton(const Qt::MouseButton button)
{
    mouseData().button = button;
}

InputSequence::KeyData &InputSequence::keyData()
{
    Q_ASSERT(m_type == Type::Keyboard);
    return std::get<KeyData>(m_data);
}

InputSequence::MouseData &InputSequence::mouseData()
{
    Q_ASSERT(m_type == Type::Mouse);
    return std::get<MouseData>(m_data);
}

InputSequence::KeyData InputSequence::keyData() const
{
    Q_ASSERT(m_type == Type::Keyboard);
    return std::get<KeyData>(m_data);
}

InputSequence::MouseData InputSequence::mouseData() const
{
    Q_ASSERT(m_type == Type::Mouse);
    return std::get<MouseData>(m_data);
}