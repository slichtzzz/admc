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

#ifndef PASSWORD_DIALOG_H
#define PASSWORD_DIALOG_H

#include <QDialog>

class AttributeEdit;

namespace Ui {
    class PasswordDialog;
}

// Accepts input of new password and changes password when done
class PasswordDialog final : public QDialog {
    Q_OBJECT

public:
    PasswordDialog(const QString &target_arg, QWidget *parent);

public slots:
    void accept();

private:
    Ui::PasswordDialog *ui;
    QString target;
    QList<AttributeEdit *> edits;
};

#endif /* PASSWORD_DIALOG_H */
