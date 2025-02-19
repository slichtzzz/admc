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

#include "filter_widget/select_classes_widget.h"
#include "filter_widget/select_classes_widget_p.h"
#include "filter_widget/ui_select_classes_widget.h"
#include "filter_widget/ui_select_classes_dialog.h"

#include "adldap.h"
#include "filter_classes_widget.h"

SelectClassesWidget::SelectClassesWidget(QWidget *parent)
: QWidget(parent) {
    ui = new Ui::SelectClassesWidget();
    ui->setupUi(this);

    dialog = new SelectClassesDialog(this);
    
    connect(
        ui->select_button, &QAbstractButton::clicked,
        dialog, &QDialog::open);
    connect(
        dialog, &QDialog::finished,
        this, &SelectClassesWidget::update_classes_display);
}

void SelectClassesWidget::add_classes(AdConfig *adconfig, const QList<QString> &class_list) {
    dialog->filter_classes_widget()->add_classes(adconfig, class_list);

    update_classes_display();
}

QString SelectClassesWidget::get_filter() const {
    return dialog->filter_classes_widget()->get_filter();
}

// Display selected classes in line edit as a sorted list of
// class display strings separated by ","
// "User, Organizational Unit, ..."
void SelectClassesWidget::update_classes_display() {
    const QString classes_display_text = [this]() {
        QList<QString> selected_classes = dialog->filter_classes_widget()->get_selected_classes_display();

        std::sort(selected_classes.begin(), selected_classes.end());

        const QString joined = selected_classes.join(", ");

        return joined;
    }();

    ui->classes_display->setText(classes_display_text);
    ui->classes_display->setCursorPosition(0);
}

QVariant SelectClassesWidget::save_state() const {
    return dialog->filter_classes_widget()->save_state();
}

void SelectClassesWidget::restore_state(const QVariant &state) {
    dialog->filter_classes_widget()->restore_state(state);
    update_classes_display();
}

SelectClassesDialog::SelectClassesDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::SelectClassesDialog();
    ui->setupUi(this);

    connect(
        ui->button_box->button(QDialogButtonBox::Reset), &QPushButton::clicked,
        this, &SelectClassesDialog::reset);
}

void SelectClassesDialog::open() {
    // Save state to later restore if dialog is dialog is
    // rejected
    state_to_restore = ui->filter_classes_widget->save_state();

    QDialog::open();
}

void SelectClassesDialog::reject() {
    reset();

    QDialog::reject();
}

void SelectClassesDialog::reset() {
    ui->filter_classes_widget->restore_state(state_to_restore);
}

FilterClassesWidget *SelectClassesDialog::filter_classes_widget() const {
    return ui->filter_classes_widget;
}
