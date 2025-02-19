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

#include "properties_dialog.h"
#include "ui_properties_dialog.h"

#include "adldap.h"
#include "console_impls/object_impl.h"
#include "globals.h"
#include "settings.h"
#include "status.h"
#include "tab_widget.h"
#include "tabs/account_tab.h"
#include "tabs/address_tab.h"
#include "tabs/attributes_tab.h"
#include "tabs/general_user_tab.h"
#include "tabs/general_group_tab.h"
#include "tabs/general_ou_tab.h"
#include "tabs/general_computer_tab.h"
#include "tabs/general_other_tab.h"
#include "tabs/gpo_links_tab.h"
#include "tabs/group_policy_tab.h"
#include "tabs/managed_by_tab.h"
#include "tabs/membership_tab.h"
#include "tabs/object_tab.h"
#include "tabs/organization_tab.h"
#include "tabs/profile_tab.h"
#include "tabs/properties_tab.h"
#include "tabs/security_tab.h"
#include "tabs/telephones_tab.h"
#include "tabs/os_tab.h"
#include "tabs/delegation_tab.h"
#include "utils.h"
#include "properties_warning_dialog.h"

#include <QAbstractItemView>
#include <QAction>
#include <QDebug>
#include <QLabel>
#include <QPushButton>

PropertiesDialog *PropertiesDialog::open_for_target(const QString &target) {
    if (target.isEmpty()) {
        return nullptr;
    }

    show_busy_indicator();

    static QHash<QString, PropertiesDialog *> instances;

    const bool dialog_already_open_for_this_target = instances.contains(target);

    PropertiesDialog *dialog;

    if (dialog_already_open_for_this_target) {
        // Focus already open dialog
        dialog = instances[target];
        dialog->raise();
        dialog->setFocus();
    } else {
        // Make new dialog for this target
        dialog = new PropertiesDialog(target);

        instances[target] = dialog;
        connect(
            dialog, &QDialog::finished,
            [target]() {
                instances.remove(target);
            });

        dialog->show();
    }

    hide_busy_indicator();

    return dialog;
}

void PropertiesDialog::open_when_view_item_activated(QAbstractItemView *view, const int dn_role) {
    connect(
        view, &QAbstractItemView::doubleClicked,
        [dn_role](const QModelIndex &index) {
            const QString dn = index.data(dn_role).toString();

            open_for_target(dn);
        });
}

PropertiesDialog::PropertiesDialog(const QString &target_arg)
: QDialog() {
    ui = new Ui::PropertiesDialog();
    ui->setupUi(this);

    target = target_arg;
    is_modified = false;

    setAttribute(Qt::WA_DeleteOnClose);

    warning_dialog = new PropertiesWarningDialog(this);

    apply_button = ui->button_box->button(QDialogButtonBox::Apply);
    reset_button = ui->button_box->button(QDialogButtonBox::Reset);
    auto cancel_button = ui->button_box->button(QDialogButtonBox::Cancel);

    const QString title = [&]() {
        const QString target_name = dn_get_name(target_arg);
        
        if (!target_name.isEmpty()) {
            return QString(tr("%1 Properties")).arg(target_name);
        } else {
            return tr("Properties");
        }
    }();
    setWindowTitle(title);

    AdObject object;

    AdInterface ad;
    if (!ad_failed(ad)) {
        object = ad.search_object(target);
    }

    // Create new tabs
    const auto add_tab = [this](PropertiesTab *tab, const QString &tab_title) {
        tabs.append(tab);
        ui->tab_widget->add_tab(tab, tab_title);
    };

    PropertiesTab *general_tab = [&]() -> PropertiesTab * {
        if (object.is_class(CLASS_USER)) {
            return new GeneralUserTab(object);
        } else if (object.is_class(CLASS_GROUP)) {
            return new GeneralGroupTab(object);
        } else if (object.is_class(CLASS_OU)) {
            return new GeneralOUTab(object);
        } else if (object.is_class(CLASS_GROUP)) {
            return new GeneralGroupTab(object);
        } else {
            return new GeneralOtherTab(object);
        }
    }();

    add_tab(general_tab, tr("General"));

    const bool advanced_view_ON = settings_get_bool(SETTING_advanced_features);
    if (advanced_view_ON) {
        add_tab(new ObjectTab(), tr("Object"));

        attributes_tab = new AttributesTab();
        add_tab(attributes_tab, tr("Attributes"));
    } else {
        attributes_tab = nullptr;
    }

    if (object.is_class(CLASS_USER)) {
        add_tab(new AccountTab(ad), tr("Account"));
        add_tab(new AddressTab(), tr("Address"));
        add_tab(new OrganizationTab(), tr("Organization"));
        add_tab(new TelephonesTab(), tr("Telephones"));
        add_tab(new ProfileTab(), tr("Profile"));
    }
    if (object.is_class(CLASS_GROUP)) {
        add_tab(new MembersTab(), tr("Members"));
    }
    if (object.is_class(CLASS_USER)) {
        add_tab(new MemberOfTab(), tr("Member of"));
    }

    if (object.is_class(CLASS_OU)) {
        add_tab(new ManagedByTab(), tr("Managed by"));
    }

    if (object.is_class(CLASS_OU) || object.is_class(CLASS_DOMAIN)) {
        add_tab(new GroupPolicyTab(), tr("Group policy"));
    }

    if (object.is_class(CLASS_GP_CONTAINER)) {
        add_tab(new GpoLinksTab(), tr("Links to"));
    }

    if (object.is_class(CLASS_COMPUTER)) {
        add_tab(new OSTab(), tr("Operating System"));
        add_tab(new DelegationTab(), tr("Delegation"));
        add_tab(new MemberOfTab(), tr("Member of"));
        add_tab(new ManagedByTab(), tr("Managed by"));
    }

    const bool need_security_tab = object.attributes().contains(ATTRIBUTE_SECURITY_DESCRIPTOR);
    if (need_security_tab && advanced_view_ON) {
        add_tab(new SecurityTab(), tr("Security"));
    }

    for (auto tab : tabs) {
        connect(
            tab, &PropertiesTab::edited,
            this, &PropertiesDialog::on_edited);
    }

    if (ad.is_connected()) {
        reset_internal(ad);
    }

    settings_setup_dialog_geometry(SETTING_properties_dialog_geometry, this);
    
    connect(
        apply_button, &QPushButton::clicked,
        this, &PropertiesDialog::apply);
    connect(
        reset_button, &QPushButton::clicked,
        this, &PropertiesDialog::reset);
    connect(
        cancel_button, &QPushButton::clicked,
        this, &PropertiesDialog::reject);
    connect(
        ui->tab_widget, &TabWidget::current_changed,
        this, &PropertiesDialog::on_current_tab_changed);
    connect(
        warning_dialog, &QDialog::accepted,
        this, &PropertiesDialog::on_warning_dialog_accepted);
    connect(
        warning_dialog, &QDialog::accepted,
        this, &PropertiesDialog::on_warning_dialog_rejected);
}

void PropertiesDialog::on_current_tab_changed(QWidget *prev_tab, QWidget *new_tab) {
    const bool switching_to_or_from_attributes = (prev_tab == attributes_tab || new_tab == attributes_tab);
    const bool need_to_open_dialog = (switching_to_or_from_attributes && is_modified);
    if (!need_to_open_dialog) {
        return;
    }

    const PropertiesWarningType warning_type = [&]() {
        if (new_tab == attributes_tab) {
            return PropertiesWarningType_SwitchToAttributes;
        } else {
            return PropertiesWarningType_SwitchFromAttributes;
        }
    }();

    warning_dialog->set_type(warning_type);
    warning_dialog->open();
}

void PropertiesDialog::accept() {
    const bool success = apply();

    if (success) {
        QDialog::accept();
    }
}

bool PropertiesDialog::apply() {
    AdInterface ad;
    if (ad_connected(ad)) {
        return apply_internal(ad);
    } else {
        return false;
    }
}

void PropertiesDialog::reset() {
    AdInterface ad;
    if (ad_connected(ad)) {
        reset_internal(ad);
    }
}

bool PropertiesDialog::apply_internal(AdInterface &ad) {
    for (auto tab : tabs) {
        const bool verify_success = tab->verify(ad, target);
        if (!verify_success) {
            return false;
        }
    }

    show_busy_indicator();

    bool total_apply_success = true;

    for (auto tab : tabs) {
        const bool apply_success = tab->apply(ad, target);
        if (!apply_success) {
            total_apply_success = false;
        }
    }

    g_status()->display_ad_messages(ad, this);

    if (total_apply_success) {
        apply_button->setEnabled(false);
        reset_button->setEnabled(false);
        is_modified = false;
    }

    hide_busy_indicator();

    emit applied();

    return total_apply_success;
}

void PropertiesDialog::reset_internal(AdInterface &ad) {
    const AdObject object = ad.search_object(target);

    for (auto tab : tabs) {
        tab->load(ad, object);
    }

    apply_button->setEnabled(false);
    reset_button->setEnabled(false);
    is_modified = false;

    g_status()->display_ad_messages(ad, this);
}

void PropertiesDialog::on_edited() {
    apply_button->setEnabled(true);
    reset_button->setEnabled(true);
    is_modified = true;
}

void PropertiesDialog::on_warning_dialog_accepted() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    apply_internal(ad);

    // NOTE: have to reset for attributes tab and other tabs
    // to load updates
    reset_internal(ad);
}

void PropertiesDialog::on_warning_dialog_rejected() {
    reset();
}
