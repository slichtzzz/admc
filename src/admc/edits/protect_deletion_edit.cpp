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

#include "edits/protect_deletion_edit.h"

#include "adldap.h"
#include "utils.h"
#include "globals.h"

#include <QCheckBox>

// Object is protected from deletion if it denies
// permissions for "delete" and "delete subtree" for
// "WORLD"(everyone) trustee

ProtectDeletionEdit::ProtectDeletionEdit(QCheckBox *check_arg, QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent) {
    check = check_arg;

    connect(
        check, &QCheckBox::stateChanged,
        [this]() {
            emit edited();
        });
}

void ProtectDeletionEdit::set_enabled(const bool enabled) {
    check->setChecked(enabled);
}

void ProtectDeletionEdit::load_internal(AdInterface &ad, const AdObject &object) {
    const bool enabled = ad_security_get_protected_against_deletion(object, g_adconfig);

    check->setChecked(enabled);
}

void ProtectDeletionEdit::set_read_only(const bool read_only) {
    check->setDisabled(read_only);
}

bool ProtectDeletionEdit::apply(AdInterface &ad, const QString &dn) const {
    const bool enabled = check->isChecked();
    const bool apply_success = ad_security_set_protected_against_deletion(ad, dn, g_adconfig, enabled);

    return apply_success;
}
