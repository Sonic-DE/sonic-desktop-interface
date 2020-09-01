/***************************************************************************
 *   Copyright (C) 2020 Tobias Fella <fella@posteo.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

import QtQuick 2.12
import QtQuick.Controls 2.12 as Controls
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.7 as Kirigami
import org.kde.kcm 1.2

SimpleKCM {
    Kirigami.FormLayout {
        Controls.ComboBox {
            Kirigami.FormData.label: i18n("Webbrowser:")
            model: kcm.browsers.applications
            textRole: "name"
            currentIndex: kcm.browsers.index
            onActivated: kcm.browsers.select(currentIndex, true)
        }
        Controls.ComboBox {
            Kirigami.FormData.label: i18n("File Manager:")
            model: kcm.fileManagers.applications
            textRole: "name"
            currentIndex: kcm.fileManagers.index
            onActivated: kcm.fileManagers.select(currentIndex, true)

        }
        Controls.ComboBox {
            Kirigami.FormData.label: i18n("E-Mail Client:")
            model: kcm.emailClients.applications
            textRole: "name"
            currentIndex: kcm.emailClients.index
            onActivated: kcm.emailClients.select(currentIndex, true)

        }
        Controls.ComboBox {
            Kirigami.FormData.label: i18n("Terminal Emulator:")
            model: kcm.terminalEmulators.applications
            textRole: "name"
            currentIndex: kcm.terminalEmulators.index
            onActivated: kcm.terminalEmulators.select(currentIndex, true)
        }
    }
}
