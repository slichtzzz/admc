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

#include "filter_classes_widget.h"
#include "ui_filter_classes_widget.h"

#include "adldap.h"

#include <QCheckBox>

FilterClassesWidget::FilterClassesWidget(QWidget *parent)
: QWidget(parent) {
    ui = new Ui::FilterClassesWidget();
    ui->setupUi(this);

    connect(
        ui->select_all_button, &QPushButton::clicked,
        this, &FilterClassesWidget::select_all);
    connect(
        ui->clear_selection_button, &QPushButton::clicked,
        this, &FilterClassesWidget::clear_selection);
}

void FilterClassesWidget::add_classes(AdConfig *adconfig, const QList<QString> &class_list) {
    for (const QString &object_class : class_list) {
        const QString class_string = adconfig->get_class_display_name(object_class);
        auto checkbox = new QCheckBox(class_string);
        checkbox->setChecked(false);

        checkbox_map[object_class] = checkbox;
    }

    for (const QString &object_class : class_list) {
        QCheckBox *checkbox = checkbox_map[object_class];
        ui->classes_layout->addWidget(checkbox);

        // TODO: this is a hack to select all classes by
        // default when class list is specific (only users
        // or only users/groups for example). Normally, no
        // classes are selected by default which currently
        // (CURRENTLY) equates to filter accepting all
        // classes.
        if (class_list.size() < 5) {
            checkbox->setChecked(true);
        }
    }
}

QString FilterClassesWidget::get_filter() const {
    const QList<QString> class_filter_list = [&] {
        QList<QString> out;

        for (const QString &object_class : checkbox_map.keys()) {
            QCheckBox *checkbox = checkbox_map[object_class];

            if (checkbox->isChecked()) {
                const QString class_filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, object_class);
                out.append(class_filter);
            }
        }

        return out;
    }();

    const QString filter = filter_OR(class_filter_list);

    return filter;
}

QList<QString> FilterClassesWidget::get_selected_classes() const {
    QList<QString> out;

    for (const QString &object_class : checkbox_map.keys()) {
        const QCheckBox *check = checkbox_map[object_class];

        if (check->isChecked()) {
            out.append(object_class);
        }
    }

    return out;
}

QList<QString> FilterClassesWidget::get_selected_classes_display() const {
    QList<QString> out;

    for (const QString &object_class : checkbox_map.keys()) {
        const QCheckBox *check = checkbox_map[object_class];

        if (check->isChecked()) {
            out.append(check->text());
        }
    }

    return out;
}

void FilterClassesWidget::select_all() {
    for (QCheckBox *checkbox : checkbox_map.values()) {
        checkbox->setChecked(true);
    }
}

void FilterClassesWidget::clear_selection() {
    for (QCheckBox *checkbox : checkbox_map.values()) {
        checkbox->setChecked(false);
    }
}

QVariant FilterClassesWidget::save_state() const {
    QHash<QString, QVariant> state;

    for (const QString &object_class : checkbox_map.keys()) {
        QCheckBox *checkbox = checkbox_map[object_class];
        const bool checked = checkbox->isChecked();

        state[object_class] = QVariant(checked);
    }

    return QVariant(state);
}

void FilterClassesWidget::restore_state(const QVariant &state_variant) {
    const QHash<QString, QVariant> state = state_variant.toHash();

    for (const QString &object_class : checkbox_map.keys()) {
        const bool checked = [&]() {
            if (state.contains(object_class)) {
                const bool out = state[object_class].toBool();

                return out;
            } else {
                return false;
            }
        }();

        QCheckBox *checkbox = checkbox_map[object_class];
        checkbox->setChecked(checked);
    }
}
