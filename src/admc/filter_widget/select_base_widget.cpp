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

#include "filter_widget/select_base_widget.h"
#include "filter_widget/ui_select_base_widget.h"

#include "adldap.h"
#include "select_container_dialog.h"

// TODO: missing "Entire directory" in search base combo. Not 100% sure what it's supposed to be, the tippy-top domain? Definitely need it for work with multiple domains.

SelectBaseWidget::SelectBaseWidget(QWidget *parent)
: QWidget(parent) {
    ui = new Ui::SelectBaseWidget();
    ui->setupUi(this);

    browse_dialog = new SelectContainerDialog(this);

    connect(
        ui->browse_button, &QAbstractButton::clicked,
        browse_dialog, &QDialog::open);
    connect(
        browse_dialog, &QDialog::accepted,
        this, &SelectBaseWidget::on_browse_dialog);
}

void SelectBaseWidget::init(AdConfig *adconfig, const QString &default_base) {
    const QString domain_head = adconfig->domain_head();

    auto add_base_to_combo = [this](const QString dn) {
        const QString name = dn_get_name(dn);
        ui->combo->addItem(name, dn);
    };

    if (default_base == domain_head || default_base.isEmpty()) {
        add_base_to_combo(domain_head);
    } else {
        add_base_to_combo(domain_head);
        add_base_to_combo(default_base);
    }

    const int last_index = ui->combo->count() - 1;
    ui->combo->setCurrentIndex(last_index);
}

QString SelectBaseWidget::get_base() const {
    const int index = ui->combo->currentIndex();
    const QVariant item_data = ui->combo->itemData(index);

    return item_data.toString();
}

void SelectBaseWidget::on_browse_dialog() {
    const QString selected = browse_dialog->get_selected();
    const QString name = dn_get_name(selected);

    ui->combo->addItem(name, selected);

    // Select newly added search base in combobox
    const int new_base_index = ui->combo->count() - 1;
    ui->combo->setCurrentIndex(new_base_index);
}

QVariant SelectBaseWidget::save_state() const {
    const QString base = ui->combo->currentData().toString();
    return QVariant(base);
}

void SelectBaseWidget::restore_state(const QVariant &state_variant) {
    const QString base = state_variant.toString();
    const QString base_name = dn_get_name(base);
    ui->combo->clear();
    ui->combo->addItem(base_name, base);
}
