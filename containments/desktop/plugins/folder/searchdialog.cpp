/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "searchdialog.h"

SearchDialog::SearchDialog(QQuickItem *parent)
    : PlasmaQuick::Dialog(parent)
{
}

SearchDialog::~SearchDialog()
{
}

bool SearchDialog::isValidRegularExpression() 
{
    return m_expression.isValid();
}

QRegularExpression SearchDialog::regularExpression() const 
{
    return m_expression;
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

        m_expression.setPattern(makePattern());
        Q_EMIT regularExpressionChanged();
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

        if (m_searchSensitivity == Qt::CaseSensitivity::CaseInsensitive) {
            m_expression.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
            Q_EMIT regularExpressionChanged();
        }
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

        m_expression.setPattern(makePattern());
        Q_EMIT regularExpressionChanged();
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

        m_expression.setPattern(makePattern());
        Q_EMIT regularExpressionChanged();
    }
}

QString SearchDialog::makePattern() 
{
    QString pattern;
    if (!m_useRegularExpression) {
        pattern = QRegularExpression::escape(m_searchString);
    } else {
        pattern = m_searchString;
    }

    if (m_matchWholeWord && !m_searchString.isEmpty()) {
        pattern = QLatin1String("\\b") + pattern + QLatin1String("\\b");
    }

    return pattern;
}
