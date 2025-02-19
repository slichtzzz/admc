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

#include "filter_dialog.h"
#include "ui_filter_dialog.h"

#include "adldap.h"

#include "filter_widget/filter_widget.h"

#include <QVariant>

// TODO: implement canceling. Need to be able to load/unload
// filter widget state though. For example, one way to
// implement would be to save old state on open, then reload
// it when cancel is pressed.

FilterDialog::FilterDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::FilterDialog();
    ui->setupUi(this);
}

void FilterDialog::open() {
    original_state = save_state();

    QDialog::open();
}

void FilterDialog::reject() {
    restore_state(original_state);

    QDialog::reject();
}

void FilterDialog::add_classes(AdConfig *adconfig, const QList<QString> &class_list) {
    ui->filter_widget->add_classes(adconfig, class_list);
}

QVariant FilterDialog::save_state() const {
    return ui->filter_widget->save_state();
}

void FilterDialog::restore_state(const QVariant &state) {
    ui->filter_widget->restore_state(state);
}

QString FilterDialog::get_filter() const {
    return ui->filter_widget->get_filter();
}
