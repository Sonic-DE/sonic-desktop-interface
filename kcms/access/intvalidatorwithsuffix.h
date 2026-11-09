/*
    SPDX-FileCopyrightText: 2025 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QIntValidator>
#include <qqmlregistration.h>

// Accepts the number in text that carries a unit suffix, such as "300 ms".
class IntValidatorWithSuffix : public QIntValidator {
    Q_OBJECT
    QML_ELEMENT

public:
    using QIntValidator::QIntValidator;

    State validate(QString &text, int &pos) const override;
};
