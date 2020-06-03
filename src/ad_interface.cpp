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

#include "ad_interface.h"
#include "config.h"

#include "active_directory.h"

#include <QSet>

// TODO: replace C active_directory.h with C++ version or other better version

// "CN=foo,CN=bar,DC=domain,DC=com"
// =>
// "foo"
QString extract_name_from_dn(const QString &dn) {
    int equals_i = dn.indexOf('=') + 1;
    int comma_i = dn.indexOf(',');
    int segment_length = comma_i - equals_i;

    QString name = dn.mid(equals_i, segment_length);

    return name;
}

// "CN=foo,CN=bar,DC=domain,DC=com"
// =>
// "CN=bar,DC=domain,DC=com"
QString extract_parent_dn_from_dn(const QString &dn) {
    int comma_i = dn.indexOf(',');

    QString parent_dn = dn.mid(comma_i + 1);

    return parent_dn;
}

// -----------------------------------------------------------------
// FAKE STUFF
// -----------------------------------------------------------------

AdInterface ad_interface;

bool FAKE_AD = false; 

QMap<QString, QList<QString>> fake_children;
QMap<QString, QMap<QString, QList<QString>>> fake_attributes_map;
QSet<QString> fake_attributes_loaded;

void fake_ad_init() {
    fake_children[HEAD_DN] = {
        QString("CN=Users,") + HEAD_DN,
        QString("CN=Computers,") + HEAD_DN,
        QString("CN=A,") + HEAD_DN,
        QString("CN=B,") + HEAD_DN,
        QString("CN=C,") + HEAD_DN,
        QString("CN=D,") + HEAD_DN,
    };

    fake_attributes_map[HEAD_DN] = {
        {"name", {"domain"}},
        {"objectClass", {"container"}},
        {"objectCategory", {"CN=Container,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
    };

    fake_attributes_map[QString("CN=Users,") + HEAD_DN] = {
        {"name", {"Users"}},
        {"objectClass", {"container"}},
        {"objectCategory", {"CN=Container,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
        {"description", {"Users's description"}},
    };

    fake_attributes_map[QString("CN=Computers,") + HEAD_DN] = {
        {"name", {"Computers"}},
        {"objectClass", {"container"}},
        {"objectCategory", {"CN=Container,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
        {"description", {"Computers's description"}},
    };

    fake_attributes_map[QString("CN=A,") + HEAD_DN] = {
        {"name", {"A"}},
        {"objectClass", {"container"}},
        {"objectCategory", {"CN=Container,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
        {"description", {"A's description"}},
    };

    fake_attributes_map[QString("CN=B,") + HEAD_DN] = {
        {"name", {"B"}},
        {"objectClass", {"container"}},
        {"objectCategory", {"CN=Container,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
        {"description", {"B's description"}},
    };

    fake_attributes_map[QString("CN=C,") + HEAD_DN] = {
        {"name", {"C"}},
        {"objectClass", {"person"}},
        {"objectCategory", {"CN=Person,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
    };

    fake_attributes_map[QString("CN=D,") + HEAD_DN] = {
        {"name", {"D"}},
        {"objectClass", {"person"}},
        {"objectCategory", {"CN=Person,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"TRUE"}},
    };

    fake_children[QString("CN=B,") + HEAD_DN] = {
        QString("CN=B's child,CN=B,") + HEAD_DN
    };
    fake_attributes_map[QString("CN=B's child,CN=B,") + HEAD_DN] = {
        {"name", {"B's child"}},
        {"objectClass", {"person"}},
        {"objectCategory", {"CN=Person,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
    };
}

QList<QString> fake_load_children(const QString &dn) {
    if (!fake_children.contains(dn)) {
        // NOTE: ok to have empty children for leaves
        fake_children[dn] = QList<QString>();
    }
    
    return fake_children[dn];
}

// NOTE: this is just for fake_create() functions
void fake_create_add_child(const QString &dn, const QString &parent) {
    if (!fake_children.contains(parent)) {
        fake_children[parent] = QList<QString>();
    }
    
    fake_children[parent].push_back(dn);
}

void fake_create_user(const QString &name, const QString &dn) {
    QString parent_dn = extract_parent_dn_from_dn(dn);
    fake_create_add_child(dn, parent_dn);

    fake_attributes_map[dn] = {
        {"name", {name}},
        {"objectClass", {"user"}},
        {"objectCategory", {"CN=User,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
    };
}

void fake_create_computer(const QString &name, const QString &dn) {
    QString parent_dn = extract_parent_dn_from_dn(dn);
    fake_create_add_child(dn, parent_dn);

    fake_attributes_map[dn] = {
        {"name", {name}},
        {"objectClass", {"computer"}},
        {"objectCategory", {"CN=Computer,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
    };
}

void fake_create_ou(const QString &name, const QString &dn) {
    QString parent_dn = extract_parent_dn_from_dn(dn);
    fake_create_add_child(dn, parent_dn);

    fake_attributes_map[dn] = {
        {"name", {name}},
        {"objectClass", {"Organizational Unit"}},
        {"objectClass", {"container"}},
        {"objectCategory", {"CN=Organizational-Unit,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
    };
}

void fake_create_group(const QString &name, const QString &dn) {
    QString parent_dn = extract_parent_dn_from_dn(dn);
    fake_create_add_child(dn, parent_dn);

    fake_attributes_map[dn] = {
        {"name", {name}},
        {"objectClass", {"group"}},
        {"objectClass", {"container"}},
        {"objectCategory", {"CN=Group,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
    };
}

void fake_object_delete_recurse(const QString &dn) {
    if (fake_children.contains(dn)) {
        QList<QString> children = fake_children[dn];

        for (auto child : children) {
            fake_object_delete_recurse(child);
        }

        fake_children.remove(dn);
    }

    fake_attributes_map.remove(dn);
}

void fake_object_delete(const QString &dn) {
    fake_object_delete_recurse(dn);

    // Remove original deleted entry from parent's children list
    for (auto key : fake_children.keys()) {
        QList<QString> *children = &fake_children[key];
        
        if (children->contains(dn)) {
            int i = children->indexOf(dn);

            children->removeAt(i);
        }
    }
}

void fake_move_user(const QString &user_dn, const QString &container_dn) {
    QString user_name = extract_name_from_dn(user_dn);
    QString new_dn = "CN=" + user_name + "," + container_dn;

    // TODO: does this work ok?
    fake_attributes_map[new_dn] = fake_attributes_map[user_dn];

    fake_object_delete(user_dn);

    fake_children[container_dn].push_back(user_dn);
}

// -----------------------------------------------------------------
// REAL STUFF
// -----------------------------------------------------------------

QMap<QString, QMap<QString, QList<QString>>> attributes_map;
QSet<QString> attributes_loaded;

bool ad_interface_login() {
    if (FAKE_AD) {
        fake_ad_init();
        return true;
    }

    LDAP* ldap_connection = ad_login(HEAD_DN);
    if (ldap_connection == NULL) {
        printf("ad_login error: %s\n", ad_get_error());
        return false;
    } else {
        return true;
    }
}

QString get_error_str() {
    return QString(ad_get_error());
}

// TODO: confirm that this encoding is ok
const char *qstring_to_cstr(const QString &qstr) {
    return qstr.toLatin1().constData();
}

QList<QString> load_children(const QString &dn) {
    if (FAKE_AD) {
        return fake_load_children(dn);
    }

    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();
    char **children_raw = ad_list(dn_cstr, HEAD_DN);

    if (children_raw != NULL) {
        auto children = QList<QString>();

        for (int i = 0; children_raw[i] != NULL; i++) {
            auto child = QString(children_raw[i]);
            children.push_back(child);
        }

        for (int i = 0; children_raw[i] != NULL; i++) {
            free(children_raw[i]);
        }
        free(children_raw);

        return children;
    } else {
        // TODO: is this still a fail if there are no children?
        emit ad_interface.load_children_failed(dn, get_error_str());

        return QList<QString>();
    }
}

void load_attributes(const QString &dn) {
    if (FAKE_AD) {
        fake_attributes_loaded.insert(dn);
        
        return;
    }

    const QByteArray dn_array = dn.toLatin1();
    const char *dn_cstr = dn_array.constData();
    char** attributes_raw = ad_get_attribute(dn_cstr, "*", HEAD_DN);

    if (attributes_raw != NULL) {
        attributes_map[dn] = QMap<QString, QList<QString>>();

        // Load attributes map
        // attributes_raw is in the form of:
        // char** array of {key, value, value, key, value ...}
        // transform it into:
        // map of {key => {value, value ...}, key => {value, value ...} ...}
        for (int i = 0; attributes_raw[i + 2] != NULL; i += 2) {
            auto attribute = QString(attributes_raw[i]);
            auto value = QString(attributes_raw[i + 1]);

            // Make values list if doesn't exist yet
            if (!attributes_map[dn].contains(attribute)) {
                attributes_map[dn][attribute] = QList<QString>();
            }

            attributes_map[dn][attribute].push_back(value);
        }

        // Free attributes_raw
        for (int i = 0; attributes_raw[i] != NULL; i++) {
            free(attributes_raw[i]);
        }
        free(attributes_raw);

        attributes_loaded.insert(dn);

        emit ad_interface.load_attributes_complete(dn);
    } else {
        emit ad_interface.load_attributes_failed(dn, get_error_str());
    }
}

QMap<QString, QList<QString>> get_attributes(const QString &dn) {
    // First check whether load_attributes was ever called on this dn
    // If it hasn't, attempt to load attributes
    // After that return whatever attributes are now loaded for this dn
    if (FAKE_AD) {
        if (!fake_attributes_loaded.contains(dn)) {
            load_attributes(dn);
        }

        if (!fake_attributes_map.contains(dn)) {
            return QMap<QString, QList<QString>>();
        } else {
            return fake_attributes_map[dn];
        }
    } else {
        if (!attributes_loaded.contains(dn)) {
            load_attributes(dn);
        }

        if (!attributes_map.contains(dn)) {
            return QMap<QString, QList<QString>>();
        } else {
            return attributes_map[dn];
        }
    }
}

QList<QString> get_attribute_multi(const QString &dn, const QString &attribute) {
    QMap<QString, QList<QString>> attributes = get_attributes(dn);

    if (attributes.contains(attribute)) {
        return attributes[attribute];
    } else {
        return QList<QString>();
    }
}

QString get_attribute(const QString &dn, const QString &attribute) {
    QList<QString> values = get_attribute_multi(dn, attribute);

    if (values.size() > 0) {
        // Return first value only
        return values[0];
    } else {
        return "";
    }
}

bool attribute_value_exists(const QString &dn, const QString &attribute, const QString &value) {
    QList<QString> values = get_attribute_multi(dn, attribute);

    if (values.contains(value)) {
        return true;
    } else {
        return false;
    }
}

bool set_attribute(const QString &dn, const QString &attribute, const QString &value) {
    int result = AD_INVALID_DN;

    const QString old_value = get_attribute(dn, attribute);
    
    if (FAKE_AD) {
        fake_attributes_map[dn][attribute] = {value};
        
        result = AD_SUCCESS;
    } else {
        const QByteArray dn_array = dn.toLatin1();
        const char *dn_cstr = dn_array.constData();

        const QByteArray attribute_array = attribute.toLatin1();
        const char *attribute_cstr = attribute_array.constData();

        const QByteArray value_array = value.toLatin1();
        const char *value_cstr = value_array.constData();

        result = ad_mod_replace(dn_cstr, attribute_cstr, value_cstr, HEAD_DN);
    }

    if (result == AD_SUCCESS) {
        // Reload attributes to get new value
        load_attributes(dn);
        
        emit ad_interface.set_attribute_complete(dn, attribute, old_value, value);

        return true;
    } else {
        emit ad_interface.set_attribute_failed(dn, attribute, old_value, value, get_error_str());

        return false;
    }
}

// TODO: can probably make a create_anything() function with enum parameter
bool create_entry(const QString &name, const QString &dn, NewEntryType type) {
    int result = AD_INVALID_DN;
    
    if (FAKE_AD) {
        switch (type) {
            case User: {
                fake_create_user(name, dn);
                break;
            }
            case Computer: {
                fake_create_computer(name, dn);
                break;
            }
            case OU: {
                fake_create_ou(name, dn);
                break;
            }
            case Group: {
                fake_create_group(name, dn);
                break;
            }
            case COUNT: break;
        }

        result = AD_SUCCESS;
    } else {
        const QByteArray name_array = name.toLatin1();
        const char *name_cstr = name_array.constData();

        const QByteArray dn_array = dn.toLatin1();
        const char *dn_cstr = dn_array.constData();

        switch (type) {
            case User: {
                result = ad_create_user(name_cstr, dn_cstr, HEAD_DN);
                break;
            }
            case Computer: {
                result = ad_create_computer(name_cstr, dn_cstr, HEAD_DN);
                break;
            }
            case OU: {
                result = ad_ou_create(name_cstr, dn_cstr, HEAD_DN);
                break;
            }
            case Group: {
                result = ad_group_create(name_cstr, dn_cstr, HEAD_DN);
                break;
            }
            case COUNT: break;
        }
    }

    if (result == AD_SUCCESS) {
        emit ad_interface.create_entry_complete(dn, type);

        return true;
    } else {
        emit ad_interface.create_entry_failed(dn, type, get_error_str());

        return false;
    }
}

// Used to update membership when changes happen to entry
void reload_attributes_of_entry_groups(const QString &dn) {
    QList<QString> groups = get_attribute_multi(dn, "memberOf");

    for (auto group : groups) {
        // Only reload if loaded already
        if (attributes_map.contains(group)) {
            load_attributes(group);
        }
    }
}

void delete_entry(const QString &dn) {
    int result = AD_INVALID_DN;

    if (FAKE_AD) {
        fake_object_delete(dn);

        result = AD_SUCCESS;
    } else {
        const QByteArray dn_array = dn.toLatin1();
        const char *dn_cstr = dn_array.constData();

        result = ad_object_delete(dn_cstr, HEAD_DN);
    }

    if (result == AD_SUCCESS) {
        reload_attributes_of_entry_groups(dn);

        attributes_map.remove(dn);
        attributes_loaded.remove(dn);

        emit ad_interface.delete_entry_complete(dn);
    } else {
        emit ad_interface.delete_entry_failed(dn, get_error_str());
    }
}

void move_user(const QString &user_dn, const QString &container_dn) {
    int result = AD_INVALID_DN;

    QString user_name = extract_name_from_dn(user_dn);
    QString new_dn = "CN=" + user_name + "," + container_dn;

    if (FAKE_AD) {
        fake_move_user(user_dn, container_dn);

        result = AD_SUCCESS;
    } else {
        const QByteArray user_dn_array = user_dn.toLatin1();
        const char *user_dn_cstr = user_dn_array.constData();

        const QByteArray container_dn_array = container_dn.toLatin1();
        const char *container_dn_cstr = container_dn_array.constData();

        result = ad_move_user(user_dn_cstr, container_dn_cstr, HEAD_DN);
    }

    if (result == AD_SUCCESS) {
        // Unload attributes at old dn
        attributes_map.remove(user_dn);
        attributes_loaded.remove(user_dn);

        load_attributes(new_dn);
        reload_attributes_of_entry_groups(new_dn);

        emit ad_interface.move_user_complete(user_dn, container_dn, new_dn);
    } else {
        emit ad_interface.move_user_failed(user_dn, container_dn, new_dn, get_error_str());
    }
}

void add_user_to_group(const QString &group_dn, const QString &user_dn) {
    // TODO: currently getting object class violation error
    int result = AD_INVALID_DN;

    if (FAKE_AD) {
        // TODO:

        result = AD_SUCCESS;
    } else {
        const QByteArray group_dn_array = group_dn.toLatin1();
        const char *group_dn_cstr = group_dn_array.constData();

        const QByteArray user_dn_array = user_dn.toLatin1();
        const char *user_dn_cstr = user_dn_array.constData();

        result = ad_group_add_user(group_dn_cstr, user_dn_cstr, HEAD_DN);
    }

    if (result == AD_SUCCESS) {
        // Reload attributes of group and user because group
        // operations affect attributes of both
        load_attributes(group_dn);
        load_attributes(user_dn);

        emit ad_interface.add_user_to_group_complete(group_dn, user_dn);
    } else {
        emit ad_interface.add_user_to_group_failed(group_dn, user_dn, get_error_str());
    }
}
