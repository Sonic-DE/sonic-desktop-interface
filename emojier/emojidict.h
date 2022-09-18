/*
    SPDX-FileCopyrightText: 2022 Weng Xuetian <wegnxt@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef EMOJIPARSER_H
#define EMOJIPARSER_H

#include <QByteArray>
#include <QDataStream>
#include <QJsonDocument>
#include <QList>

struct Emoji {
    QString content;
    QString description;
    qint32 category;
    QStringList annotations;

    QString categoryName() const;
};

inline QDataStream &operator<<(QDataStream &stream, const Emoji &emoji)
{
    stream << emoji.content;
    stream << emoji.description;
    stream << emoji.category;
    stream << emoji.annotations;
    return stream;
}

inline QDataStream &operator>>(QDataStream &stream, Emoji &emoji)
{
    stream >> emoji.content;
    stream >> emoji.description;
    stream >> emoji.category;
    stream >> emoji.annotations;
    return stream;
}

struct EmojiDict {
    void load(const QString &path);

    QList<Emoji> m_emojis;
    QSet<QString> m_processedEmojis;
};

#endif
