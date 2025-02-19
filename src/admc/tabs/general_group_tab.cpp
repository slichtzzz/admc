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

#include "tabs/general_group_tab.h"
#include "tabs/ui_general_group_tab.h"

#include "tabs/general_other_tab.h"
#include "adldap.h"
#include "edits/string_edit.h"
#include "edits/sama_edit.h"
#include "edits/group_scope_edit.h"
#include "edits/group_type_edit.h"

GeneralGroupTab::GeneralGroupTab(const AdObject &object) {
    ui = new Ui::GeneralGroupTab();
    ui->setupUi(this);

    load_name_label(ui->name_label, object);

    new SamaEdit(ui->sama_edit, ui->sama_domain_edit, &edits, this);
    new StringEdit(ui->description_edit, ATTRIBUTE_DESCRIPTION, &edits, this);
    new StringEdit(ui->email_edit, ATTRIBUTE_MAIL, &edits, this);
    new StringEdit(ui->notes_edit, ATTRIBUTE_INFO, &edits, this);

    auto scope_edit = new GroupScopeEdit(ui->scope_combo, &edits, this);
    auto type_edit = new GroupTypeEdit(ui->type_combo, &edits, this);

    const bool is_critical_system_object = object.get_bool(ATTRIBUTE_IS_CRITICAL_SYSTEM_OBJECT);
    if (is_critical_system_object) {
        scope_edit->set_read_only(true);
        type_edit->set_read_only(true);
    }

    edits_connect_to_tab(edits, this);
}
