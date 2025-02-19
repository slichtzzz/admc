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

#ifndef ADMC_TEST_ATTRIBUTES_TAB_H
#define ADMC_TEST_ATTRIBUTES_TAB_H

#include "admc_test.h"

#include "tabs/attributes_tab.h"
#include "tabs/attributes_tab_p.h"
#include "settings.h"

class QStandardItemModel;
class QSortFilterProxyModel;
class QPushButton;
class QTreeView;
class AttributesFilterMenu;

class ADMCTestAttributesTab : public ADMCTest {
    Q_OBJECT

private slots:
    void init() override;

    void apply();
    void load();
    void filter_data();
    void filter();

private:
    AttributesTab *attributes_tab;
    AttributesFilterMenu *filter_menu;
    QTreeView *view;
    QStandardItemModel *model;
    QSortFilterProxyModel *proxy;
    QPushButton *filter_button;
    QPushButton *edit_button;
    QString dn;

    void set_filter(const QList<AttributeFilter> &filter_list, const bool state);
};

#endif /* ADMC_TEST_ATTRIBUTES_TAB_H */
