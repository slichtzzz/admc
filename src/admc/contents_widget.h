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

#ifndef CONTENTS_WIDGET_H
#define CONTENTS_WIDGET_H

#include <QWidget>
#include <QString>

class ContainersWidget;
class ObjectListWidget;
class QAction;
class ContentsFilterDialog;
class ObjectModel;
class QTreeView;
class QModelIndex;

/**
 * Shows a list of objects, which are children of a target
 * parent object. Parent object is equal to most recent
 * selection in containers widget. Updates on AD modifications.
 */

class ContentsWidget final : public QWidget {
Q_OBJECT

public:
    ContentsWidget(ObjectModel *model, ContainersWidget *containers_widget, const QAction *filter_contents_action);

private slots:
    void on_containers_selected_changed(const QModelIndex &source_index);
    void on_ad_modified();
    void load_filter(const QString &filter);

private:
    QString target_dn;
    QTreeView *view;
    ContentsFilterDialog *filter_dialog;

    void change_target(const QString &dn);
};

#endif /* CONTENTS_WIDGET_H */
