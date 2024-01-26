/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "searchdialog.h"

#include <QRegularExpression>

SearchDialog::SearchDialog(QQuickItem *parent)
    : PlasmaQuick::Dialog(parent)
{
}

SearchDialog::~SearchDialog()
{
}

QRegularExpression SearchDialog::getRegularExpression()
{
    QRegularExpression regExp;
    if (m_matchWholeWord) {
        regExp.setPattern(QLatin1String("\\b") + m_searchString + QLatin1String("\\b"));
    } else {
        regExp.setPattern(m_searchString);
    }

    if (!m_useRegularExpression) {
        regExp.escape(m_searchString);
    }

    if (!regExp.isValid()) {
        return QRegularExpression();
    }

    if (m_searchSensitivity == Qt::CaseSensitivity::CaseInsensitive) {
        regExp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    }

    return regExp;
}

bool SearchDialog::isValidRegularExpression() 
{
    return getRegularExpression().isValid();
}

QString SearchDialog::searchString() const 
{
    return m_searchString;
}

void SearchDialog::setSearchString(const QString &search)
{
    if (m_searchString != search) {
        m_searchString = search;

        Q_EMIT searchStringChanged();
    }
}

Qt::CaseSensitivity SearchDialog::searchSensitivity() const
{
    return m_searchSensitivity;
}

void SearchDialog::setSearchSensitivity(Qt::CaseSensitivity sensitivity)
{
    if (m_searchSensitivity != sensitivity) {
        m_searchSensitivity = sensitivity;

        Q_EMIT searchSensitivityChanged();
    }
}

bool SearchDialog::matchWholeWord() const
{
    return m_matchWholeWord;
}

void SearchDialog::setMatchWholeWord(bool flag)
{
    if (m_matchWholeWord != flag) {
        m_matchWholeWord = flag;

        Q_EMIT matchWholeWordChanged();
    }
}

bool SearchDialog::useRegularExpression() const
{
    return m_useRegularExpression;
}

void SearchDialog::setUseRegularExpression(bool flag)
{
    if (m_useRegularExpression != flag) {
        m_useRegularExpression = flag;

        Q_EMIT useRegularExpressionChanged();
    }
}