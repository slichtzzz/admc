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

#include "select_policy_dialog.h"
#include "ui_select_policy_dialog.h"

#include "adldap.h"
#include "console_impls/policy_impl.h"
#include "globals.h"
#include "status.h"
#include "utils.h"

#include <QStandardItemModel>
#include <QPushButton>

SelectPolicyDialog::SelectPolicyDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::SelectPolicyDialog();
    ui->setupUi(this);

    model = new QStandardItemModel(this);
    ui->view->setModel(model);

    ui->view->setHeaderHidden(true);

    QPushButton *ok_button = ui->button_box->button(QDialogButtonBox::Ok);
    enable_widget_on_selection(ok_button, ui->view);
}

void SelectPolicyDialog::open() {
    AdInterface ad;
    if (ad_failed(ad)) {
        close();
        return;
    }

    model->removeRows(0, model->rowCount());

    const QString base = g_adconfig->domain_head();
    const SearchScope scope = SearchScope_All;
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_GP_CONTAINER);
    const QList<QString> attributes = console_policy_search_attributes();

    const QHash<QString, AdObject> results = ad.search(base, scope, filter, attributes);

    // TODO: assuming that policy row has 1 column, need to
    // make it depend on some constant
    for (const AdObject &object : results.values()) {
        const QList<QStandardItem *> row = {new QStandardItem()};
        model->appendRow(row);

        console_policy_load(row, object);
    }

    QDialog::open();
}

QList<QString> SelectPolicyDialog::get_selected_dns() const {
    QList<QString> dns;

    const QList<QModelIndex> indexes = ui->view->selectionModel()->selectedRows();
    for (const QModelIndex &index : indexes) {
        const QString dn = index.data(PolicyRole_DN).toString();
        dns.append(dn);
    }

    return dns;
}
