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

#include "tabs/security_tab.h"

#include "adldap.h"
#include "utils.h"
#include "globals.h"

#include "samba/ndr_security.h"

#include <QVBoxLayout>
#include <QDebug>
#include <QTreeView>
#include <QStandardItemModel>
#include <QLabel>

enum TrusteeItemRole {
    TrusteeItemRole_Sid = Qt::UserRole,
};

#define ENUM_TO_STRING(ENUM) {ENUM, #ENUM}

const QHash<AcePermission, QString> ace_permission_to_name_map = {
    ENUM_TO_STRING(AcePermission_FullControl),
    ENUM_TO_STRING(AcePermission_Read),
    ENUM_TO_STRING(AcePermission_Write),
    ENUM_TO_STRING(AcePermission_CreateChild),
    ENUM_TO_STRING(AcePermission_DeleteChild),
    ENUM_TO_STRING(AcePermission_AllowedToAuthenticate),
    ENUM_TO_STRING(AcePermission_ChangePassword),
    ENUM_TO_STRING(AcePermission_ReceiveAs),
    ENUM_TO_STRING(AcePermission_ResetPassword),
    ENUM_TO_STRING(AcePermission_SendAs),
    ENUM_TO_STRING(AcePermission_ReadAccountRestrictions),
    ENUM_TO_STRING(AcePermission_WriteAccountRestrictions),
    ENUM_TO_STRING(AcePermission_ReadGeneralInfo),
    ENUM_TO_STRING(AcePermission_WriteGeneralInfo),
    ENUM_TO_STRING(AcePermission_ReadGroupMembership),
    ENUM_TO_STRING(AcePermission_ReadLogonInfo),
    ENUM_TO_STRING(AcePermission_WriteLogonInfo),
    ENUM_TO_STRING(AcePermission_ReadPersonalInfo),
    ENUM_TO_STRING(AcePermission_WritePersonalInfo),
    ENUM_TO_STRING(AcePermission_ReadPhoneAndMailOptions),
    ENUM_TO_STRING(AcePermission_WritePhoneAndMailOptions),
    ENUM_TO_STRING(AcePermission_ReadPrivateInfo),
    ENUM_TO_STRING(AcePermission_WritePrivateInfo),
    ENUM_TO_STRING(AcePermission_ReadPublicInfo),
    ENUM_TO_STRING(AcePermission_WritePublicInfo),
    ENUM_TO_STRING(AcePermission_ReadRemoteAccessInfo),
    ENUM_TO_STRING(AcePermission_WriteRemoteAccessInfo),
    ENUM_TO_STRING(AcePermission_ReadTerminalServerLicenseServer),
    ENUM_TO_STRING(AcePermission_WriteTerminalServerLicenseServer),
    ENUM_TO_STRING(AcePermission_ReadWebInfo),
    ENUM_TO_STRING(AcePermission_WriteWebInfo),
};

const QList<AcePermission> all_permissions_list =
[]() {
    QList<AcePermission> out;

    for (int permission_i = 0; permission_i < AcePermission_COUNT; permission_i++) {
        const AcePermission permission = (AcePermission) permission_i;
        out.append(permission);
    }

    return out;
}();

const QSet<AcePermission> all_permissions = all_permissions_list.toSet();

QSet<AcePermission> get_permission_set(const uint32_t mask) {
    QSet<AcePermission> out;

    for (const AcePermission &permission : all_permissions) {
        const uint32_t this_mask = ace_permission_to_mask_map[permission];

        if (this_mask == mask) {
            out.insert(permission);
        }
    }

    return out;
}

const QSet<AcePermission> read_prop_permissions = get_permission_set(SEC_ADS_READ_PROP);
const QSet<AcePermission> write_prop_permissions = get_permission_set(SEC_ADS_WRITE_PROP);

SecurityTab::SecurityTab() {
    ignore_item_changed_signal = false;
    
    trustee_model = new QStandardItemModel(0, 1, this);
    
    trustee_view = new QTreeView(this);
    trustee_view->setHeaderHidden(true);
    trustee_view->setModel(trustee_model);
    trustee_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    trustee_view->sortByColumn(0, Qt::AscendingOrder);
    trustee_view->setSortingEnabled(true);

    ace_model = new QStandardItemModel(0, AceColumn_COUNT, this);
    set_horizontal_header_labels_from_map(ace_model, {
        {AceColumn_Name, tr("Name")},
        {AceColumn_Allowed, tr("Allowed")},
        {AceColumn_Denied, tr("Denied")},
    });

    // Fill ace model
    for (const AcePermission &permission : all_permissions_list) {
        const QList<QStandardItem *> row = make_item_row(AceColumn_COUNT);

        const QString mask_string = ace_permission_to_name_map[permission];
        row[AceColumn_Name]->setText(mask_string);

        row[AceColumn_Allowed]->setCheckable(true);
        row[AceColumn_Denied]->setCheckable(true);

        row[0]->setData(permission, AcePermissionItemRole_Permission);

        ace_model->appendRow(row);
    }

    permission_item_map =
    [&]() {
        QHash<AcePermission, QHash<AceColumn, QStandardItem *>> out;

        for (int row = 0; row < ace_model->rowCount(); row++) {
            QStandardItem *main_item = ace_model->item(row, 0);
            const AcePermission permission = (AcePermission) main_item->data(AcePermissionItemRole_Permission).toInt();

            const QList<AceColumn> column_list = {
                AceColumn_Allowed,
                AceColumn_Denied,
            };

            for (const AceColumn &column : column_list) {
                QStandardItem *item = ace_model->item(row, column);
                out[permission][column] = item;
            }
        }

        return out;
    }();
    
    ace_view = new QTreeView(this);
    ace_view->setModel(ace_model);
    ace_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ace_view->setColumnWidth(AceColumn_Name, 400);

    selected_trustee_label = new QLabel();

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(trustee_view);
    layout->addWidget(selected_trustee_label);
    layout->addWidget(ace_view);

    // TODO: maybe use current, not selected? i think selection
    // may be empty if you press escape or something
    connect(
        trustee_view->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &SecurityTab::load_trustee_acl);
    connect(
        ace_model, &QStandardItemModel::itemChanged,
        this, &SecurityTab::on_item_changed);
}

void SecurityTab::load(AdInterface &ad, const AdObject &object) {
    const QByteArray descriptor_bytes = object.get_value(ATTRIBUTE_SECURITY_DESCRIPTOR);
    sd.load(descriptor_bytes);

    // Add items to trustee model
    trustee_model->removeRows(0, trustee_model->rowCount());
    const QList<QByteArray> trustee_list = sd.get_trustee_list();
    for (const QByteArray &trustee : trustee_list) {
        auto item = new QStandardItem();

        const QString name = ad.get_trustee_name(trustee);
        item->setText(name);

        item->setData(trustee, TrusteeItemRole_Sid);

        trustee_model->appendRow(item);
    }

    trustee_model->sort(0, Qt::AscendingOrder);

    //
    // Load permission state
    //

    // Initialize to None by default
    for (const QByteArray &trustee : trustee_list) {
        for (const AcePermission &permission : all_permissions) {
            permission_state_map[trustee][permission] = PermissionState_None;
        }
    }

    // Then go through acl and set allowed/denied permission states
    for (security_ace *ace : sd.dacl) {
        const QByteArray trustee = dom_sid_to_bytes(ace->trustee);

        for (const AcePermission &permission : all_permissions) {
            const uint32_t permission_mask = ace_permission_to_mask_map[permission];

            const bool mask_match = ((ace->access_mask & permission_mask) == permission_mask);
            if (!mask_match) {
                continue;
            }

            const bool object_match =
            [&]() {
                const bool object_present = ((ace->object.object.flags & SEC_ACE_OBJECT_TYPE_PRESENT) != 0);
                if (!object_present) {
                    return false;
                }

                const QString rights_guid =
                [&]() {
                    const QString right_cn = ace_permission_to_type_map[permission];
                    const QString guid_out =  g_adconfig->get_right_guid(right_cn);

                    return guid_out;
                }();


                const QByteArray bytes_1 = guid_string_to_bytes(rights_guid);
                const QString string_2 = attribute_display_value(ATTRIBUTE_OBJECT_GUID, bytes_1, g_adconfig);
                qInfo() << rights_guid << string_2 << (rights_guid == string_2);

                const QString ace_type_guid =
                [&]() {
                    const GUID type = ace->object.object.type.type;
                    const QByteArray type_bytes = QByteArray((char *) &type, sizeof(GUID));

                    return attribute_display_value(ATTRIBUTE_OBJECT_GUID, type_bytes, g_adconfig);
                }();

                return (rights_guid.toLower() == ace_type_guid.toLower());
            }();

            switch (ace->type) {
                case SEC_ACE_TYPE_ACCESS_ALLOWED: {
                    permission_state_map[trustee][permission] = PermissionState_Allowed;
                    break;
                }
                case SEC_ACE_TYPE_ACCESS_DENIED: {
                    permission_state_map[trustee][permission] = PermissionState_Denied;
                    break;
                }
                case SEC_ACE_TYPE_ACCESS_ALLOWED_OBJECT: {
                    if (object_match) {
                        permission_state_map[trustee][permission] = PermissionState_Allowed;
                    }
                    break;
                }
                case SEC_ACE_TYPE_ACCESS_DENIED_OBJECT: {
                    if (object_match) {
                        permission_state_map[trustee][permission] = PermissionState_Denied;
                    }
                    break;
                }
                default: break;
            }
        }
    }

    // Select first index
    // NOTE: load_trustee_acl() is called because setCurrentIndex
    // emits "current change" signal
    trustee_view->selectionModel()->setCurrentIndex(trustee_model->index(0, 0), QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);

    PropertiesTab::load(ad, object);
}

QStandardItemModel *SecurityTab::get_ace_model() const {
    return ace_model;
}

void SecurityTab::load_trustee_acl() {
    const QList<QModelIndex> selected_list = trustee_view->selectionModel()->selectedRows();
    if (selected_list.isEmpty()) {
        return;
    }

    const QModelIndex selected_index = selected_list[0];
    QStandardItem *selected_item = trustee_model->itemFromIndex(selected_index);

    const QString label_text =
    [&]() {
        const QString selected_name = selected_item->data(Qt::DisplayRole).toString();
        const QString text = QString("Permissions for %1").arg(selected_name);

        return text;
    }();

    selected_trustee_label->setText(label_text);

    ignore_item_changed_signal = true;
    
    const QByteArray trustee = selected_item->data(TrusteeItemRole_Sid).toByteArray();

    for (int row = 0; row < ace_model->rowCount(); row++) {
        QStandardItem *allowed = ace_model->item(row, AceColumn_Allowed);
        QStandardItem *denied = ace_model->item(row, AceColumn_Denied);

        const PermissionState permission_state =
        [&]() {
            QStandardItem *main_item = ace_model->item(row, 0);
            const AcePermission permission = (AcePermission) main_item->data(AcePermissionItemRole_Permission).toInt();
            const PermissionState out = permission_state_map[trustee][permission];

            return out;
        }();

        switch (permission_state) {
            case PermissionState_None: {
                allowed->setCheckState(Qt::Unchecked);
                denied->setCheckState(Qt::Unchecked);
                break;
            };
            case PermissionState_Allowed: {
                allowed->setCheckState(Qt::Checked);
                denied->setCheckState(Qt::Unchecked);
                break;
            }
            case PermissionState_Denied: {
                allowed->setCheckState(Qt::Unchecked);
                denied->setCheckState(Qt::Checked);
                break;
            }
        }
    }

    ignore_item_changed_signal = false;

    sd.print_acl(trustee);
}

// Permission check states are interdependent. When some
// check states change we also need to change other check
// states.
void SecurityTab::on_item_changed(QStandardItem *item) {
    // NOTE: in some cases we need to ignore this signal
    if (ignore_item_changed_signal) {
        return;
    }

    // NOTE: set this flag to avoid recursion. Inside this
    // function we call setCheckState() (via
    // set_permission_state()), which triggers the
    // itemChanged() signal. Note that using blockSignals()
    // on the model is not a solution because turning off
    // model's signals messes up the view (view uses model's
    // signals to know when to update itself).
    ignore_item_changed_signal = true;

    const AceColumn column = (AceColumn) item->column();

    const bool incorrect_column = (column != AceColumn_Allowed && column != AceColumn_Denied);
    if (incorrect_column) {
        return;
    }

    // NOTE: block signals for the duration of this f-n so
    // that there's no recursion due to setCheckState()
    // calls triggering more on_item_changed() slot calls.

    const AcePermission permission =
    [&]() {
        QStandardItem *main_item = ace_model->item(item->row(), 0);
        const AcePermission out = (AcePermission) main_item->data(AcePermissionItemRole_Permission).toInt();

        return out;
    }();

    const AceColumn opposite_column =
    [&]() {
        if (column == AceColumn_Allowed) {
            return AceColumn_Denied;
        } else {
            return AceColumn_Allowed;
        }
    }();

    const bool is_checked = (item->checkState() == Qt::Checked);

    if (is_checked) {
        // Uncheck opposite column to make Allowed/Denied exclusive to each other
        set_permission_state({permission}, opposite_column, Qt::Unchecked);

        // FullControl, Read and Write permissions cause
        // their children permissions to become
        // allowed/denied as well.
        if (permission == AcePermission_FullControl) {
            set_permission_state(all_permissions, column, Qt::Checked);
            set_permission_state(all_permissions, opposite_column, Qt::Unchecked);
        } else if (permission == AcePermission_Read) {
            set_permission_state(read_prop_permissions, column, Qt::Checked);
            set_permission_state(read_prop_permissions, opposite_column, Qt::Unchecked);
        } else if (permission == AcePermission_Write) {
            set_permission_state(write_prop_permissions, column, Qt::Checked);
            set_permission_state(write_prop_permissions, opposite_column, Qt::Unchecked);
        }
    }

    const QList<AcePermission> parent_permissions =
    [&]() {
        QList<AcePermission> out;

        out.append(AcePermission_FullControl);

        if (read_prop_permissions.contains(permission)) {
            out.append(AcePermission_Read);
        } else if (write_prop_permissions.contains(permission)) {
            out.append(AcePermission_Write);
        }

        return out;
    }();

    // When children are unchecked or switch to opposite
    // allowed/denied while their parent is checked, parent
    // needs to become unchecked.
    for (const AcePermission &parent_permission : parent_permissions) {
        if (is_checked) {
            set_permission_state({parent_permission}, opposite_column, Qt::Unchecked);
        } else {
            set_permission_state({parent_permission}, column, Qt::Unchecked);
        }
    }

    const QByteArray trustee =
    [&]() {
        const QModelIndex current_index = trustee_view->currentIndex();
        QStandardItem *current_item = trustee_model->itemFromIndex(current_index);
        const QByteArray out = current_item->data(TrusteeItemRole_Sid).toByteArray();

        return out;
    }();

    // Update permission_state_map
    permission_state_map[trustee] =
    [this]() {
        QHash<AcePermission, PermissionState> out;

        for (const AcePermission &this_permission : all_permissions) {
            out[this_permission] =
            [&]() {
                QStandardItem *allowed_item = get_item(this_permission, AceColumn_Allowed);
                QStandardItem *denied_item = get_item(this_permission, AceColumn_Denied);
                const bool allowed = (allowed_item->checkState() == Qt::Checked);
                const bool denied = (denied_item->checkState() == Qt::Checked);

                if (allowed) {
                    return PermissionState_Allowed;
                } else if (denied) {
                    return PermissionState_Denied;
                } else {
                    return PermissionState_None;
                }
            }();
        }

        return out;
    }();

    ignore_item_changed_signal = false;

    emit edited();
}

QStandardItem *SecurityTab::get_item(const AcePermission permission, const AceColumn column) {
    return permission_item_map[permission][column];
}

void SecurityTab::set_permission_state(const QSet<AcePermission> &permission_set, const AceColumn column, const Qt::CheckState state) {
    for (const AcePermission &permission : permission_set.values()) {
        QStandardItem *item = get_item(permission, column);
        item->setCheckState(state);
    }
}

bool SecurityTab::apply(AdInterface &ad, const QString &target) {
    // Remove redundancy from permission state
    const QHash<QByteArray, QHash<AcePermission, PermissionState>> state =
    [&]() {
        QHash<QByteArray, QHash<AcePermission, PermissionState>> out;

        out = permission_state_map;

        // Remove child permission states. For example if
        // "Read" is allowed, then there's no need to
        // include any other state for "read prop"
        // permissions.
        for (const QByteArray &trustee : out.keys()) {
            const bool full_control = out[trustee].contains(AcePermission_FullControl) && (out[trustee][AcePermission_FullControl] != PermissionState_None);
            const bool read = out[trustee].contains(AcePermission_Read) && (out[trustee][AcePermission_Read] != PermissionState_None);
            const bool write = out[trustee].contains(AcePermission_Write) && (out[trustee][AcePermission_Write] != PermissionState_None);
            
            if (full_control) {
                for (const AcePermission &permission : all_permissions) {
                    if (permission != AcePermission_FullControl) {
                        out[trustee].remove(permission);
                    }
                }
            } else if (read) {
                for (const AcePermission &permission : read_prop_permissions) {
                    if (permission != AcePermission_Read) {
                        out[trustee].remove(permission);
                    }
                }
            } else if (write) {
                for (const AcePermission &permission : write_prop_permissions) {
                    if (permission != AcePermission_Read) {
                        out[trustee].remove(permission);
                    }
                }
            }
        }

        return out;
    }();

    const QByteArray descriptor_bytes = ad.generate_sd(state, sd);
    // object.attribute_replace_value(target, ATTRIBUTE_SECURITY_DESCRIPTOR, descriptor_bytes);

    return true;
}
