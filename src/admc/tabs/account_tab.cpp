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

#include "tabs/account_tab.h"
#include "tabs/ui_account_tab.h"

#include "edits/account_option_edit.h"
#include "edits/expiry_edit.h"
#include "edits/string_edit.h"
#include "edits/unlock_edit.h"
#include "edits/upn_edit.h"
#include "edits/logon_hours_edit.h"
#include "edits/logon_computers_edit.h"

// TODO: logon computers

// NOTE: the "can't change password" checkbox does not
// affect the permission in the security tab, even though
// they control the same thing. And vice versa. Too
// complicated to implement so this is a WONTFIX.

AccountTab::AccountTab(AdInterface &ad) {
    ui = new Ui::AccountTab();
    ui->setupUi(this);

    auto upn_edit = new UpnEdit(ui->upn_prefix_edit, ui->upn_suffix_edit, &edits, this);
    upn_edit->init_suffixes(ad);

    new UnlockEdit(ui->unlock_check, &edits, this);
    new ExpiryEdit(ui->expiry_widget, &edits, this);
    new LogonHoursEdit(ui->logon_hours_button, &edits, this);
    new LogonComputersEdit(ui->logon_computers_button, &edits, this);

    const QHash<AccountOption, QCheckBox *> check_map = {
        {AccountOption_Disabled, ui->disabled_check},
        {AccountOption_CantChangePassword, ui->cant_change_pass_check},
        {AccountOption_PasswordExpired, ui->pass_expired_check},
        {AccountOption_DontExpirePassword, ui->dont_expire_pass_check},
        {AccountOption_UseDesKey, ui->des_key_check},
        {AccountOption_SmartcardRequired, ui->smartcard_check},
        {AccountOption_CantDelegate, ui->cant_delegate_check},
        {AccountOption_DontRequirePreauth, ui->require_preauth_check},
        {AccountOption_TrustedForDelegation, ui->trusted_check},
    };

    for (const AccountOption &option : check_map.keys()) {
        QCheckBox *check = check_map[option];
        new AccountOptionEdit(check, option, &edits, this);
    }
    

    edits_connect_to_tab(edits, this);
}
