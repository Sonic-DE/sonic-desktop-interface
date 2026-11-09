/*
    SPDX-FileCopyrightText: 2025 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "intvalidatorwithsuffix.h"

#include <QRegularExpression>

QValidator::State IntValidatorWithSuffix::validate(QString &text, int &pos) const
{
    QString strippedText;
    static const QRegularExpression re(QStringLiteral("\\d+"));
    QRegularExpressionMatchIterator i = re.globalMatch(text);
    while (i.hasNext()) {
        strippedText += i.next().captured();
    }
    return QIntValidator::validate(strippedText, pos);
}
