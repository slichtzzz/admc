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

#ifndef TAB_WIDGET_H
#define TAB_WIDGET_H

/**
 * Shows tabs in a horizontal list to the left and current
 * tab to the right. Used as a replacement for QTabWidget
 * because it doesn't have multi-line tabs.
 */

#include <QWidget>

namespace Ui {
    class TabWidget;
}

class TabWidget final : public QWidget {
    Q_OBJECT

public:
    TabWidget(QWidget *parent = nullptr);

    void add_tab(QWidget *tab, const QString &title);

signals:
    void current_changed(QWidget *prev_tab, QWidget *new_tab);

private:
    Ui::TabWidget *ui;

    void on_list_current_row_changed(int index);
};

#endif /* TAB_WIDGET_H */
