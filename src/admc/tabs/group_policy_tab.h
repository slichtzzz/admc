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

#ifndef GROUP_POLICY_TAB_H
#define GROUP_POLICY_TAB_H

#include "tabs/properties_tab.h"

#include "gplink.h"

class QString;
class QStandardItemModel;
class QStandardItem;
class QPoint;
class SelectPolicyDialog;

/**
 * Tab for displaying, modifying group policy related
 * attributes of an object(not a gpo!), such as gplink and
 * gpoptions.
 */

namespace Ui {
    class GroupPolicyTab;
}

class GroupPolicyTab final : public PropertiesTab {
    Q_OBJECT

public:
    GroupPolicyTab();
    ~GroupPolicyTab();

    void load(AdInterface &ad, const AdObject &object) override;
    bool apply(AdInterface &ad, const QString &target) override;

private slots:
    void on_context_menu(const QPoint pos);
    void on_add_dialog();
    void on_remove_button();
    void on_item_changed(QStandardItem *item);

private:
    Ui::GroupPolicyTab *ui;
    SelectPolicyDialog *add_dialog;
    QStandardItemModel *model;
    Gplink gplink;
    QString original_gplink_string;

    void add_link(QList<QString> gpos);
    void remove_link(QList<QString> gpos);
    void move_link_up(const QString &gpo);
    void move_link_down(const QString &gpo);

    void reload_gplink();
};

#endif /* GROUP_POLICY_TAB_H */
