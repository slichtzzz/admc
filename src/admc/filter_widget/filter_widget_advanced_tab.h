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

#ifndef FILTER_WIDGET_ADVANCED_TAB_H
#define FILTER_WIDGET_ADVANCED_TAB_H

/**
 * Allows input of a plain LDAP filter string.
 */

#include "filter_widget/filter_widget.h"

namespace Ui {
    class FilterWidgetAdvancedTab;
}

class FilterWidgetAdvancedTab final : public FilterWidgetTab {
    Q_OBJECT

public:
    FilterWidgetAdvancedTab();

    QString get_filter() const;

    QVariant save_state() const;
    void restore_state(const QVariant &state);

private:
    Ui::FilterWidgetAdvancedTab *ui;
};

#endif /* FILTER_WIDGET_ADVANCED_TAB_H */
