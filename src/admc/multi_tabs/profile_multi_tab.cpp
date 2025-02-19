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

#include "multi_tabs/profile_multi_tab.h"
#include "ui_profile_multi_tab.h"

#include "adldap.h"
#include "multi_edits/string_multi_edit.h"

ProfileMultiTab::ProfileMultiTab() {
    ui = new Ui::ProfileMultiTab();
    ui->setupUi(this);

    new StringMultiEdit(ui->profile_edit, ui->profile_check, ATTRIBUTE_PROFILE_PATH, edit_list, this);
    new StringMultiEdit(ui->script_edit, ui->script_check, ATTRIBUTE_SCRIPT_PATH, edit_list, this);
    new StringMultiEdit(ui->home_edit, ui->home_check, ATTRIBUTE_HOME_DIRECTORY, edit_list, this);

    multi_edits_connect_to_tab(edit_list, this);
}
