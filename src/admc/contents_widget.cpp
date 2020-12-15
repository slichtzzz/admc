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

#include "contents_widget.h"
#include "containers_widget.h"
#include "settings.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "object_list_widget.h"
#include "contents_filter_dialog.h"
#include "object_model.h"
#include "advanced_view_proxy.h"
#include "utils.h"

#include <QVBoxLayout>
#include <QAction>
#include <QDebug>
#include <QTreeView>
#include <QHeaderView>

ContentsWidget::ContentsWidget(ObjectModel *model, ContainersWidget *containers_widget, const QAction *filter_contents_action)
: QWidget()
{   
    // object_list = new ObjectListWidget(ObjectListWidgetType_Contents);
    view = new QTreeView(this);
    view->setAcceptDrops(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setRootIsDecorated(false);
    view->setItemsExpandable(false);
    view->setExpandsOnDoubleClick(false);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);
    view->header()->setSectionsMovable(true);

    advanced_view_proxy = new AdvancedViewProxy(this);

    advanced_view_proxy->setSourceModel(model);
    view->setModel(advanced_view_proxy);

    setup_column_toggle_menu(view, model, {ADCONFIG()->get_column_index(ATTRIBUTE_NAME), ADCONFIG()->get_column_index(ATTRIBUTE_OBJECT_CLASS), ADCONFIG()->get_column_index(ATTRIBUTE_DESCRIPTION)});

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(view);

    filter_dialog = new ContentsFilterDialog(this);

    connect(
        filter_dialog, &ContentsFilterDialog::filter_changed,
        this, &ContentsWidget::load_filter);

    connect(
        containers_widget, &ContainersWidget::selected_changed,
        this, &ContentsWidget::on_containers_selected_changed);
    connect(
        AD(), &AdInterface::modified,
        this, &ContentsWidget::on_ad_modified);

    connect(
        filter_contents_action, &QAction::triggered,
        [this]() {
            filter_dialog->open();
        });
}

void ContentsWidget::on_containers_selected_changed(const QModelIndex &source_index) {
    const QModelIndex proxy_index = advanced_view_proxy->mapFromSource(source_index);
    view->setRootIndex(proxy_index);
}

void ContentsWidget::on_ad_modified() {
    // change_target(target_dn);
}

void ContentsWidget::load_filter(const QString &filter) {
    // qDebug() << "Contents filter:" << filter;
    // object_list->load_children(target_dn, filter);
}

void ContentsWidget::change_target(const QString &dn) {
    // target_dn = dn;

    // object_list->load_children(target_dn);
}

void ContentsWidget::showEvent(QShowEvent *event) {
    resize_columns(view,
    {
        {ADCONFIG()->get_column_index(ATTRIBUTE_NAME), 0.4},
        {ADCONFIG()->get_column_index(ATTRIBUTE_OBJECT_CLASS), 0.4},
    });
}
