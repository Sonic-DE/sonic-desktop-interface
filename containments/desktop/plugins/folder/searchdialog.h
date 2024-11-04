/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <PlasmaQuick/Dialog>

#include <QRegularExpression>

class SearchDialog : public PlasmaQuick::Dialog
{
    Q_OBJECT

    Q_PROPERTY(QRegularExpression regularExpression READ regularExpression NOTIFY regularExpressionChanged)
    Q_PROPERTY(QString searchString READ searchString WRITE setSearchString NOTIFY searchStringChanged)
    Q_PROPERTY(Qt::CaseSensitivity searchSensitivity READ searchSensitivity WRITE setSearchSensitivity NOTIFY searchSensitivityChanged)
    Q_PROPERTY(bool matchWholeWord READ matchWholeWord WRITE setMatchWholeWord NOTIFY matchWholeWordChanged)
    Q_PROPERTY(bool useRegularExpression READ useRegularExpression WRITE setUseRegularExpression NOTIFY useRegularExpressionChanged)

public:
    explicit SearchDialog(QQuickItem *parent = nullptr);
    ~SearchDialog() override;

    Q_INVOKABLE bool isValidRegularExpression();

public:
    QRegularExpression regularExpression() const;

    QString searchString() const;
    void setSearchString(const QString &search);

    Qt::CaseSensitivity searchSensitivity() const;
    void setSearchSensitivity(Qt::CaseSensitivity sensitivity);

    bool matchWholeWord() const;
    void setMatchWholeWord(bool flag);

    bool useRegularExpression() const;
    void setUseRegularExpression(bool flag);

Q_SIGNALS:
    void regularExpressionChanged() const;
    void searchStringChanged() const;
    void searchSensitivityChanged() const;
    void matchWholeWordChanged() const;
    void useRegularExpressionChanged() const;

private:
    QString makePattern();

private:
    QRegularExpression m_expression;
    QString m_searchString;
    Qt::CaseSensitivity m_searchSensitivity = Qt::CaseSensitivity::CaseInsensitive;
    bool m_matchWholeWord;
    bool m_useRegularExpression;
};
