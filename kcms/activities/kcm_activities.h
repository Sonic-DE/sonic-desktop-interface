/*
 *  SPDX-FileCopyrightText: 2015-2016 Ivan Cukic <ivan.cukic@kde.org>
 *  SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <KQuickConfigModule>

class ActivitiesModule : public KQuickConfigModule
{
    Q_OBJECT
    Q_PROPERTY(bool newActivityAuthorized READ newActivityAuthorized CONSTANT)

public:
    ActivitiesModule(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~ActivitiesModule() override;

    bool newActivityAuthorized() const;

    Q_INVOKABLE void configureActivity(const QString &id);
    Q_INVOKABLE void newActivity();
    Q_INVOKABLE void deleteActivity(const QString &id);

    void load() override;

private:
<<<<<<< HEAD
    bool m_newActivityAuthorized;
=======
    bool m_isNewActivityAuthorized;
    QString m_firstArgument;
>>>>>>> c7fd81f60 (kcms/activities: Allow to edit activity via KCM args)
};
