/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LOGON_COMPUTERS_EDIT_P_H
#define LOGON_COMPUTERS_EDIT_P_H

#include <QDialog>

class QLineEdit;
class QListWidget;

namespace Ui {
    class LogonComputersDialog;
}

class LogonComputersDialog final : public QDialog {
    Q_OBJECT

public:
    LogonComputersDialog(QWidget *parent);

    void load(const QString &value);
    QString get() const;

private:
    Ui::LogonComputersDialog *ui;

    void on_add_button();  
    void on_remove_button();  
};

#endif /* LOGON_COMPUTERS_EDIT_P_H */
