/*
    SPDX-FileCopyrightText: 2022 Weng Xuetian <wegnxt@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "emojidict.h"
#include "emojicategory.h"
#include <KLocalizedString>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>

QString Emoji::categoryName() const
{
    const auto &names = getCategoryNames();
    if (category <= 0 || category > names.size()) {
        return QString();
    }
    // off by 1 because 0 is unknown.
    return names[category - 1];
}

void EmojiDict::load(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    QDataStream stream(&file);
    // Endianess is enforced here to ensure it's not relevant to architecture.
    stream.setByteOrder(QDataStream::LittleEndian);
    QList<Emoji> emojis;
    stream >> emojis;
    for (const auto &emoji : emojis) {
        if (m_processedEmojis.contains(emoji.content) || emoji.description.isEmpty()) {
            continue;
        }
        m_processedEmojis.insert(emoji.content);
        m_emojis.append(emoji);
    }
}
