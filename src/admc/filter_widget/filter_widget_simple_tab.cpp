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

#include "filter_widget/filter_widget_simple_tab.h"
#include "filter_widget/ui_filter_widget_simple_tab.h"

#include "adldap.h"

FilterWidgetSimpleTab::FilterWidgetSimpleTab()
: FilterWidgetTab() {
    ui = new Ui::FilterWidgetSimpleTab();
    ui->setupUi(this);
}

void FilterWidgetSimpleTab::add_classes(AdConfig *adconfig, const QList<QString> classes) {
    ui->select_classes->add_classes(adconfig, classes);
}

QString FilterWidgetSimpleTab::get_filter() const {
    const QString name_filter = [this]() {
        const QString name = ui->name_edit->text();

        if (!name.isEmpty()) {
            return filter_CONDITION(Condition_Contains, ATTRIBUTE_NAME, name);
        } else {
            return QString();
        }
    }();

    const QString classes_filter = ui->select_classes->get_filter();

    return filter_AND({name_filter, classes_filter});
}

QVariant FilterWidgetSimpleTab::save_state() const {
    QHash<QString, QVariant> state;

    state["select_classes"] = ui->select_classes->save_state();
    state["name"] = ui->name_edit->text();

    return QVariant(state);
}

void FilterWidgetSimpleTab::restore_state(const QVariant &state_variant) {
    const QHash<QString, QVariant> state = state_variant.toHash();

    ui->select_classes->restore_state(state["select_classes"]);
    ui->name_edit->setText(state["name"].toString());
}
