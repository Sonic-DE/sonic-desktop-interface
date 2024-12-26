/*
    SPDX-FileCopyrightText: 2004 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "ui_knetattach.h"

#include <QFileDialog>

class QUrl;

class KNetAttach : public QWizard, private Ui_KNetAttach
{
    Q_OBJECT

public:
    explicit KNetAttach(QWidget *parent = nullptr);

public Q_SLOTS:
    virtual void setInformationText(const QString &type);

private:
    QString _type;
    QFileDialog *m_dialog = nullptr;
    QUrl m_certUrl;

    void openCertChoosingDialog();
    void toggleCertChoosingDialogVisibility(bool useCustomCert);
    bool doConnectionTest(const QUrl &url);
    bool updateForProtocol(const QString &protocol);

private Q_SLOTS:
    void acceptCertChoosingDialog();
    void updateParametersPageStatus();
    bool validateCurrentPage() override;
    void updatePort(bool encryption);
    void updateFinishButtonText(bool save);
    void slotHelpClicked();
    void slotPageChanged(int);
};
