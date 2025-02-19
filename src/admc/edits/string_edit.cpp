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

#include "edits/string_edit.h"

#include "adldap.h"
#include "globals.h"
#include "utils.h"

#include <QLineEdit>

StringEdit::StringEdit(QLineEdit *edit_arg, const QString &attribute_arg, QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent) {
    attribute = attribute_arg;
    edit = edit_arg;
    
    if (g_adconfig->get_attribute_is_number(attribute)) {
        set_line_edit_to_numbers_only(edit);
    }

    limit_edit(edit, attribute);

    QObject::connect(
        edit, &QLineEdit::textChanged,
        [this]() {
            emit edited();
        });
}

void StringEdit::load_internal(AdInterface &ad, const AdObject &object) {
    const QString value = [=]() {
        const QString raw_value = object.get_string(attribute);

        if (attribute == ATTRIBUTE_DN) {
            return dn_canonical(raw_value);
        } else {
            return raw_value;
        }
    }();

    edit->setText(value);
}

void StringEdit::set_read_only(const bool read_only) {
    edit->setDisabled(read_only);
}

bool StringEdit::apply(AdInterface &ad, const QString &dn) const {
    const QString new_value = edit->text();
    const bool success = ad.attribute_replace_string(dn, attribute, new_value);

    return success;
}
