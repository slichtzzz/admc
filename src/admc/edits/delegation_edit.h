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

#ifndef DELEGATION_EDIT_H
#define DELEGATION_EDIT_H

#include "edits/attribute_edit.h"

class QRadioButton;

class DelegationEdit final : public AttributeEdit {
    Q_OBJECT
public:
    DelegationEdit(QRadioButton *off_button, QRadioButton *on_button, QList<AttributeEdit *> *edits_out, QObject *parent);
    DECL_ATTRIBUTE_EDIT_VIRTUALS();

private:
    QRadioButton *off_button;
    QRadioButton *on_button;
};

#endif /* DELEGATION_EDIT_H */
