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

#ifndef RENAME_GROUP_DIALOG_H
#define RENAME_GROUP_DIALOG_H

#include "rename_dialog.h"

class SamaEdit;

namespace Ui {
    class RenameGroupDialog;
}

class RenameGroupDialog final : public RenameDialog {
    Q_OBJECT

public:
    RenameGroupDialog(QWidget *parent);

    void open() override;

private:
    Ui::RenameGroupDialog *ui;
    SamaEdit *sama_edit;
};

#endif /* RENAME_GROUP_DIALOG_H */
