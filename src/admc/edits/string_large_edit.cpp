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

#include "edits/string_large_edit.h"

#include "adldap.h"
#include "globals.h"

#include <QPlainTextEdit>

StringLargeEdit::StringLargeEdit(QPlainTextEdit *edit_arg, const QString &attribute_arg, QList<AttributeEdit *> *edits_out, QObject *parent)
: AttributeEdit(edits_out, parent) {
    attribute = attribute_arg;
    edit = edit_arg;

    const int range_upper = g_adconfig->get_attribute_range_upper(attribute);
    if (range_upper > 0) {
        // NOTE: QPlainTextEdit doesn't have a straightforward setMaxLength() so have to do it ourselves
        connect(
            edit, &QPlainTextEdit::textChanged,
            [this, range_upper]() {
                const QString text = edit->toPlainText();

                if (text.length() > range_upper) {
                    const QString shortened = text.left(range_upper);

                    edit->setPlainText(text);
                }
            });
    }

    QObject::connect(
        edit, &QPlainTextEdit::textChanged,
        [this]() {
            emit edited();
        });
}

void StringLargeEdit::load_internal(AdInterface &ad, const AdObject &object) {
    const QString value = object.get_string(attribute);

    edit->setPlainText(value);
}

void StringLargeEdit::set_read_only(const bool read_only) {
    edit->setDisabled(read_only);
}

bool StringLargeEdit::apply(AdInterface &ad, const QString &dn) const {
    const QString new_value = edit->toPlainText();
    const bool success = ad.attribute_replace_string(dn, attribute, new_value);

    return success;
}
