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

#include "create_computer_dialog.h"
#include "ui_create_computer_dialog.h"

#include "adldap.h"
#include "utils.h"
#include "edits/sama_edit.h"
#include "edits/upn_edit.h"

CreateComputerDialog::CreateComputerDialog(QWidget *parent)
: CreateDialog(parent) {
    ui = new Ui::CreateComputerDialog();
    ui->setupUi(this);

    QList<AttributeEdit *> edit_list;

    // TODO: "Assign this computer account as a pre-Windows 2000 computer". Is this needed?

    // TODO: "The following user or group may join this computer to a domain". Tried to figure out how this is implemented and couldn't see any easy ways via attributes, so probably something to do with setting ACL'S.

    // TODO: "This is a managed computer" checkbox and an edit for guid/uuid which I assume modifies objectGUID?

    sama_edit = new SamaEdit(ui->sama_edit, ui->sama_domain_edit, &edit_list, this);

    const QList<QLineEdit *> required_list = {
        ui->name_edit,
        ui->sama_edit,
    };

    const QList<QWidget *> widget_list = {
        ui->name_edit,
        ui->sama_edit,
        // NOTE: not restoring sama domain state is intended
        // ui->sama_domain_edit,
    };

    // Autofill name -> sama
    connect(
        ui->name_edit, &QLineEdit::textChanged,
        [=]() {
            const QString name_input = ui->name_edit->text();
            ui->sama_edit->setText(name_input.toUpper());
        });

    init(ui->name_edit, ui->button_box, edit_list, required_list, widget_list, CLASS_COMPUTER);
}

void CreateComputerDialog::open() {
    sama_edit->load_domain();

    CreateDialog::open();
}
