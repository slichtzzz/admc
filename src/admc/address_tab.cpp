/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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

#include "address_tab.h"
#include "ad_interface.h"
#include "edits/attribute_edit.h"
#include "edits/string_edit.h"
#include "edits/country_edit.h"
#include "details_widget.h"

#include <QVBoxLayout>
#include <QGridLayout>

AddressTab::AddressTab(DetailsWidget *details_arg)
: DetailsTab(details_arg)
{   
    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);

    const auto edits_layout = new QGridLayout();
    top_layout->addLayout(edits_layout);
    
    const QList<QString> attributes = {
        ATTRIBUTE_STREET,
        ATTRIBUTE_PO_BOX,
        ATTRIBUTE_CITY,
        ATTRIBUTE_STATE,
        ATTRIBUTE_POSTAL_CODE
    };

    QMap<QString, StringEdit *> string_edits;
    make_string_edits(attributes, CLASS_USER, &string_edits, &edits, this);

    edits.append(new CountryEdit(this));

    layout_attribute_edits(edits, edits_layout);
    connect_edits_to_tab(edits, this);
}

bool AddressTab::changed() const {
    return any_edits_changed(edits);
}

bool AddressTab::verify() {
    return verify_attribute_edits(edits, this);
}

void AddressTab::apply() {
    apply_attribute_edits(edits, target(), this);
}

void AddressTab::reload(const Attributes &attributes) {
    load_attribute_edits(edits, attributes);
}

bool AddressTab::accepts_target(const Attributes &attributes) const {
    return object_is_user(attributes);
}
