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

#ifndef FILTER_WIDGET_NORMAL_TAB_H
#define FILTER_WIDGET_NORMAL_TAB_H

/**
 * Allows user to enter a filter in a structured way,
 * without requiring knowledge of LDAP filter syntax.
 * Presents possible attributes and possible filter
 * conditions. User can build filters and add them a to a
 * list. The list is composed into one single LDAP filter
 * string.
 */

#include "filter_widget/filter_widget.h"

class AdConfig;

namespace Ui {
    class FilterWidgetNormalTab;
}

class FilterWidgetNormalTab final : public FilterWidgetTab {
    Q_OBJECT

public:
    FilterWidgetNormalTab();

    void add_classes(AdConfig *adconfig, const QList<QString> classes);

    QString get_filter() const;

    QVariant save_state() const;
    void restore_state(const QVariant &state);

private slots:
    void add_filter();
    void remove_filter();
    void clear_filters();
    void update_attributes_combo();
    void update_conditions_combo();
    void update_value_edit();

private:
    Ui::FilterWidgetNormalTab *ui;
    AdConfig *adconfig;
};

#endif /* FILTER_WIDGET_NORMAL_TAB_H */
