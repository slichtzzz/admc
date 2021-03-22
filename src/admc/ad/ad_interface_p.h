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

#ifndef AD_INTERFACE_P_H
#define AD_INTERFACE_P_H

#include <QList>
#include <QCoreApplication>

class QString;
typedef struct ldap LDAP;
typedef struct _SMBCCTX SMBCCTX;

class AdInterfacePrivate {
Q_DECLARE_TR_FUNCTIONS(AdInterfacePrivate)
    
public:
    AdInterfacePrivate();
    ~AdInterfacePrivate();

    LDAP *ld;
    SMBCCTX *smbc;
    bool is_connected;
    QString domain;
    QString domain_head;
    QString host;
    QList<AdMessage> messages;

    void success_message(const QString &msg, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    void error_message(const QString &context, const QString &error, const DoStatusMsg do_msg = DoStatusMsg_Yes);
    QString default_error() const;
    int get_ldap_result() const;
    bool search_paged_internal(const char *filter, char **attributes, const int scope, const char *search_base, QHash<QString, AdObject> *out, AdCookie *cookie);
};

#endif /* AD_INTERFACE_P_H */
