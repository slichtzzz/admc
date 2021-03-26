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

#include "console_widget/console_widget.h"
#include "console_widget/console_widget_p.h"

#include "console_widget/console_drag_model.h"
#include "console_widget/scope_model.h"
#include "console_widget/customize_columns_dialog.h"
#include "console_widget/results_description.h"
#include "console_widget/results_view.h"

#include <QDebug>
#include <QTreeView>
#include <QVBoxLayout>
#include <QAction>
#include <QStack>
#include <QSplitter>
#include <QSortFilterProxyModel>
#include <QStackedWidget>
#include <QLabel>
#include <QApplication>
#include <QMenu>
#include <QHeaderView>

ConsoleWidgetPrivate::ConsoleWidgetPrivate(ConsoleWidget *q_arg)
: QObject(q_arg)
{
    q = q_arg;

    scope_view = new QTreeView();
    scope_view->setHeaderHidden(true);
    scope_view->setExpandsOnDoubleClick(true);
    scope_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    scope_view->setContextMenuPolicy(Qt::CustomContextMenu);
    scope_view->setDragDropMode(QAbstractItemView::DragDrop);
    // NOTE: this makes it so that you can't drag drop between rows (even though name/description don't say anything about that)
    scope_view->setDragDropOverwriteMode(true);

    scope_model = new ScopeModel(this);
    scope_view->setModel(scope_model);

    focused_view = scope_view;

    results_proxy_model = new QSortFilterProxyModel(this);

    description_bar = new QLabel();

    results_stacked_widget = new QStackedWidget();

    properties_action = new QAction(tr("&Properties"), this);
    navigate_up_action = new QAction(tr("&Up one level"), this);
    navigate_back_action = new QAction(tr("&Back"), this);
    navigate_forward_action = new QAction(tr("&Forward"), this);
    refresh_action = new QAction(tr("&Refresh"), this);
    customize_columns_action = new QAction(tr("&Customize columns"), this);
    set_results_to_icons_action = new QAction(tr("&Icons"), this);
    set_results_to_list_action = new QAction(tr("&List"), this);
    set_results_to_detail_action = new QAction(tr("&Detail"), this);

    // NOTE: need to add a dummy view until a real view is
    // added when a scope item is selected. If this is not
    // done and stacked widget is left empty, it's sizing is
    // set to minimum which causes an incorrect ratio in the
    // scope/results splitter
    auto dummy_view = new QTreeView();
    results_stacked_widget->addWidget(dummy_view);

    connect(
        scope_view, &QTreeView::expanded,
        this, &ConsoleWidgetPrivate::fetch_scope);
    connect(
        scope_view->selectionModel(), &QItemSelectionModel::currentChanged,
        this, &ConsoleWidgetPrivate::on_current_scope_item_changed);
    connect(
        scope_view->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &ConsoleWidgetPrivate::on_selection_changed);
    connect(
        scope_model, &QStandardItemModel::rowsAboutToBeRemoved,
        this, &ConsoleWidgetPrivate::on_scope_items_about_to_be_removed);
    // TODO: for now properties are opened by user of console
    // widget but in the future it's planned to move this stuff
    // here, which will make this do more than just emit a
    // signal.
    connect(
        properties_action, &QAction::triggered,
        q, &ConsoleWidget::properties_requested);
    connect(
        navigate_up_action, &QAction::triggered,
        this, &ConsoleWidgetPrivate::navigate_up);
    connect(
        navigate_back_action, &QAction::triggered,
        this, &ConsoleWidgetPrivate::navigate_back);
    connect(
        navigate_forward_action, &QAction::triggered,
        this, &ConsoleWidgetPrivate::navigate_forward);
    connect(
        refresh_action, &QAction::triggered,
        this, &ConsoleWidgetPrivate::refresh);
    connect(
        customize_columns_action, &QAction::triggered,
        this, &ConsoleWidgetPrivate::customize_columns);
    connect(
        set_results_to_icons_action, &QAction::triggered,
        this, &ConsoleWidgetPrivate::set_results_to_icons);
    connect(
        set_results_to_list_action, &QAction::triggered,
        this, &ConsoleWidgetPrivate::set_results_to_list);
    connect(
        set_results_to_detail_action, &QAction::triggered,
        this, &ConsoleWidgetPrivate::set_results_to_detail);

    connect(
        scope_view, &QWidget::customContextMenuRequested,
        this, &ConsoleWidgetPrivate::on_context_menu);

    connect(
        qApp, &QApplication::focusChanged,
        this, &ConsoleWidgetPrivate::on_focus_changed);

    connect_to_drag_model(scope_model);

    update_navigation_actions();
}

ConsoleWidget::ConsoleWidget(QWidget *parent)
: QWidget(parent)
{
    d = new ConsoleWidgetPrivate(this);

    auto results_wrapper = new QWidget();
    auto results_layout = new QVBoxLayout();
    results_wrapper->setLayout(results_layout);
    results_layout->setContentsMargins(0, 0, 0, 0);
    results_layout->setSpacing(0);
    results_layout->addWidget(d->description_bar);
    results_layout->addWidget(d->results_stacked_widget);

    auto splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(d->scope_view);
    splitter->addWidget(results_wrapper);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
    layout->addWidget(splitter);
}

QStandardItem *ConsoleWidget::add_scope_item(const int results_id, const ScopeNodeType scope_type, const QModelIndex &parent) {
    QStandardItem *parent_item =
    [=]() {
        if (parent.isValid()) {
            return d->scope_model->itemFromIndex(parent);
        } else {
            return d->scope_model->invisibleRootItem();
        }
    }();

    auto item = new QStandardItem();

    const bool is_dynamic = (scope_type == ScopeNodeType_Dynamic);

    item->setData(is_dynamic, ConsoleRole_ScopeIsDynamic);

    // NOTE: if item is not dynamic, then it's "fetched"
    // from creation
    if (is_dynamic) {
        item->setData(false, ConsoleRole_WasFetched);
    } else {
        item->setData(true, ConsoleRole_WasFetched);
    }

    item->setData(results_id, ConsoleRole_ResultsId);

    item->setData(true, ConsoleRole_IsScope);

    parent_item->appendRow(item);

    // Create a results model for this scope item, if
    // results has a view
    const ResultsDescription results = d->results_descriptions[results_id];
    if (results.view() != nullptr) {
        auto new_results = new ConsoleDragModel(this);
        d->connect_to_drag_model(new_results);
        d->results_models[item->index()] = new_results;

        const QList<QString> results_column_labels = results.column_labels();
        new_results->setHorizontalHeaderLabels(results_column_labels);

        connect(
            new_results, &QAbstractItemModel::rowsInserted,
            this, &ConsoleWidget::results_count_changed);
        connect(
            new_results, &QAbstractItemModel::rowsRemoved,
            this, &ConsoleWidget::results_count_changed);
    }

    return item;
}

QList<QStandardItem *> ConsoleWidget::add_results_row(const QModelIndex &scope_parent) {
    if (!scope_parent.isValid()) {
        return QList<QStandardItem *>();
    }

    QStandardItemModel *results_model = d->get_results_model_for_scope_item(scope_parent);
    if (results_model == nullptr) {
        return QList<QStandardItem *>();
    }

    const QList<QStandardItem *> row =
    [=]() {
        QList<QStandardItem *> out;

        const int results_id = scope_parent.data(ConsoleRole_ResultsId).toInt();
        const ResultsDescription results = d->results_descriptions[results_id];

        for (int i = 0; i < results.column_count(); i++) {
            const auto item = new QStandardItem();
            out.append(item);
        }

        return out;
    }();

    results_model->appendRow(row);

    row[0]->setData(false, ConsoleRole_IsScope);

    row[0]->setData(QPersistentModelIndex(scope_parent), ConsoleRole_ScopeParent);

    return row;
}

void ConsoleWidget::add_buddy_scope_and_results(const int results_id, const ScopeNodeType scope_type, const QModelIndex &scope_parent, QStandardItem **scope_arg, QList<QStandardItem *> *results_arg) {
    QStandardItem *scope = add_scope_item(results_id, scope_type, scope_parent);

    QList<QStandardItem *> results = add_results_row(scope_parent);

    // Set buddy indexes for scope and results so that they
    // point at each other. MUST use QPersistentModelIndex
    // because QModelIndex's will become incorrect quickly.
    const QModelIndex scope_index = scope->index();
    const QModelIndex results_index = results[0]->index();
    QStandardItemModel *results_model = d->get_results_model_for_scope_item(scope_parent);
    results_model->setData(results_index, QPersistentModelIndex(scope_index), ConsoleRole_Buddy);
    d->scope_model->setData(scope_index, QPersistentModelIndex(results_index), ConsoleRole_Buddy);

    *scope_arg = scope;
    *results_arg = results;
}

void ConsoleWidget::set_has_properties(const QModelIndex &index, const bool has_properties) {
    QAbstractItemModel *model = (QAbstractItemModel *) index.model();
    model->setData(index, has_properties, ConsoleRole_HasProperties);
}

void ConsoleWidget::delete_item(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }

    // NOTE: i *think* discarding const from model is fine.
    // Qt's reason: "A const pointer to the model is
    // returned because calls to non-const functions of the
    // model might invalidate the model index and possibly
    // crash your application.". Index becoming invalid is
    // expected since we're deleting it from the model.

    // Remove buddy item
    const QModelIndex buddy = get_buddy(index);
    if (buddy.isValid()) {
        ((QAbstractItemModel *)buddy.model())->removeRows(buddy.row(), 1, buddy.parent());
    }

    // Remove item from it's own model
    ((QAbstractItemModel *)index.model())->removeRows(index.row(), 1, index.parent());
}

void ConsoleWidget::set_current_scope(const QModelIndex &index) {
    d->scope_view->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);
}

void ConsoleWidget::refresh_scope(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }

    d->scope_model->removeRows(0, d->scope_model->rowCount(index), index);

    QStandardItemModel *results_model = d->get_results_model_for_scope_item(index);
    if (results_model != nullptr) {
        results_model->removeRows(0, results_model->rowCount());
    }

    // Emit item_fetched() so that user of console can
    // reload item's results
    emit item_fetched(index);
}

// No view, only widget
int ConsoleWidget::register_results(QWidget *widget) {
    return register_results(widget, nullptr, QList<QString>(), QList<int>());
}

// In this case, view *is* the widget
int ConsoleWidget::register_results(ResultsView *view, const QList<QString> &column_labels, const QList<int> &default_columns) {
    return register_results(view, view, column_labels, default_columns);
}

// Base register() f-n
int ConsoleWidget::register_results(QWidget *widget, ResultsView *view, const QList<QString> &column_labels, const QList<int> &default_columns) {
    static int id_max = 0;
    id_max++;
    const int id = id_max;

    d->results_descriptions[id] = ResultsDescription(widget, view, column_labels, default_columns);

    d->results_stacked_widget->addWidget(widget);

    if (view != nullptr) {
        // NOTE: a proxy is model is inserted between
        // results views and results models for more
        // efficient sorting. If results views and models
        // are connected directly, deletion of results
        // models becomes extremely slow.
        view->set_model(d->results_proxy_model);

        connect(
            view, &ResultsView::activated,
            d, &ConsoleWidgetPrivate::on_results_activated);
        connect(
            view, &ResultsView::context_menu,
            d, &ConsoleWidgetPrivate::on_context_menu);

        // Hide non-default results view columns. Note that
        // at creation, view header doesnt have any
        // sections. Sections appear once a source model is
        // set(proxy model doesnt count). Therefore we have
        // to do this in the slot.
        QHeaderView *header = view->detail_view()->header();
        connect(
            header, &QHeaderView::sectionCountChanged,
            [header, default_columns](int oldCount, int) {
                if (oldCount == 0) {
                    for (int i = 0; i < header->count(); i++) {
                        const bool hidden = !default_columns.contains(i);
                        header->setSectionHidden(i, hidden);
                    }
                }
            });
    }

    connect(
        view, &ResultsView::selection_changed,
        d, &ConsoleWidgetPrivate::on_selection_changed);

    return id;
}

void ConsoleWidget::sort_scope() {
    d->scope_model->sort(0, Qt::AscendingOrder);
}

void ConsoleWidget::set_description_bar_text(const QString &text) {
    const QString scope_name =
    [this]() {
        const QModelIndex current_scope = get_current_scope_item();
        const QString out = current_scope.data().toString();

        return out;
    }();

    const QString description = QString("%1 %2").arg(scope_name, text);

    d->description_bar->setText(description);
}

QList<QModelIndex> ConsoleWidget::get_selected_items() const {
    const QList<QModelIndex> indexes = d->focused_view->selectionModel()->selectedRows(0);

    // NOTE: need to map from proxy to source indexes if
    // focused view is results
    const bool focused_is_results = (d->focused_view != d->scope_view);
    if (focused_is_results) {
        QList<QModelIndex> source_indexes;
        
        for (const QModelIndex &index : indexes) {
            const QModelIndex source_index = d->results_proxy_model->mapToSource(index);
            source_indexes.append(source_index);
        }

        return source_indexes;
    } else {
        return indexes;
    }
}

QList<QModelIndex> ConsoleWidget::search_scope_by_role(int role, const QVariant &value) const {
    const QList<QModelIndex> matches = d->scope_model->match(d->scope_model->index(0, 0), role, value, -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));

    return matches;
}

QList<QModelIndex> ConsoleWidget::search_results_by_role(int role, const QVariant &value) const {
    QList<QModelIndex> all_matches;

    for (QStandardItemModel *model : d->results_models.values()) {
        const QList<QModelIndex> matches = model->match(model->index(0, 0), role, value, -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));

        all_matches.append(matches);
    }

    return all_matches;
}

QModelIndex ConsoleWidget::get_current_scope_item() const {
    const QModelIndex index = d->scope_view->selectionModel()->currentIndex();

    return index;
}

int ConsoleWidget::get_current_results_count() const {
    const QModelIndex current_scope = get_current_scope_item();
    QStandardItemModel *current_results = d->get_results_model_for_scope_item(current_scope);

    if (current_results != nullptr) {
        const int results_count = current_results->rowCount();

        return results_count;
    } else {
        return 0;
    }
}

QStandardItem *ConsoleWidget::get_scope_item(const QModelIndex &scope_index) const {
    QStandardItem *item = d->scope_model->itemFromIndex(scope_index);

    return item;
}

QList<QStandardItem *> ConsoleWidget::get_results_row(const QModelIndex &results_index) const {
    QList<QStandardItem *> row;

    const QModelIndex scope_parent = results_index.data(ConsoleRole_ScopeParent).toModelIndex();
    QStandardItemModel *model = d->get_results_model_for_scope_item(scope_parent);

    if (model != nullptr) {
        for (int col = 0; col < model->columnCount(); col++) {
            QStandardItem *item = model->item(results_index.row(), col);
            row.append(item);
        }

        return row;
    } else {
        return QList<QStandardItem *>();
    }
}

QModelIndex ConsoleWidget::get_buddy(const QModelIndex &index) const {
    const QModelIndex buddy = index.data(ConsoleRole_Buddy).toModelIndex();

    return buddy;
}

bool ConsoleWidget::is_scope_item(const QModelIndex &index) const {
    const bool is_scope = index.data(ConsoleRole_IsScope).toBool();

    return is_scope;
}

bool ConsoleWidget::item_was_fetched(const QModelIndex &index) const {
    return index.data(ConsoleRole_WasFetched).toBool();
}

QWidget *ConsoleWidget::get_scope_view() const {
    return d->scope_view;
}

QWidget *ConsoleWidget::get_description_bar() const {
    return d->description_bar;
}

QStandardItemModel *ConsoleWidgetPrivate::get_results_model_for_scope_item(const QModelIndex &index) const {
    if (results_models.contains(index)) {
        return results_models[index];
    } else {
        return nullptr;
    }
}

void ConsoleWidgetPrivate::connect_to_drag_model(ConsoleDragModel *model) {
    connect(
        model, &ConsoleDragModel::start_drag,
        this, &ConsoleWidgetPrivate::on_start_drag);
    connect(
        model, &ConsoleDragModel::can_drop,
        this, &ConsoleWidgetPrivate::on_can_drop);
    connect(
        model, &ConsoleDragModel::drop,
        this, &ConsoleWidgetPrivate::on_drop);
}

void ConsoleWidgetPrivate::on_results_activated(const QModelIndex &index) {
    const QModelIndex buddy = q->get_buddy(index);

    if (buddy.isValid()) {
        // Set associated scope item as current
        scope_view->selectionModel()->setCurrentIndex(buddy, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);
    } else {
        emit q->properties_requested();
    }
}

// Show/hide actions based on what's selected and emit the
// signal
void ConsoleWidgetPrivate::on_selection_changed() {
    const QList<QModelIndex> selected_list = q->get_selected_items();
    
    if (!selected_list.isEmpty() && !selected_list[0].isValid()) {
        return;
    }

    properties_action->setVisible(false);
    refresh_action->setVisible(false);

    if (selected_list.size() == 1) {
        const QModelIndex selected = selected_list[0];

        const bool is_dynamic = selected.data(ConsoleRole_ScopeIsDynamic).toBool();
        if (is_dynamic) {
            refresh_action->setVisible(true);
        }

        const bool has_properties = selected.data(ConsoleRole_HasProperties).toBool();
        if (has_properties) {
            properties_action->setVisible(true);
        }
    }

    emit q->selection_changed();
}

void ConsoleWidgetPrivate::on_context_menu(const QPoint pos) {
    const QPoint global_pos = focused_view->mapToGlobal(pos);

    emit q->context_menu(global_pos);
}

void ConsoleWidgetPrivate::on_current_scope_item_changed(const QModelIndex &current, const QModelIndex &previous) {
    // NOTE: technically this slot should never be called
    // with invalid current index
    if (!current.isValid()) {
        return;
    }

    // Move current to past, if current changed and is valid
    if (previous.isValid() && (previous != current)) {
        targets_past.append(QPersistentModelIndex(previous));
    }

    // When a new current is selected, a new future begins
    targets_future.clear();

    update_navigation_actions();

    // Switch to this item's results widget
    const ResultsDescription results = get_current_results();
    results_stacked_widget->setCurrentWidget(results.widget());

    const bool results_view_exists = (results.view() != nullptr);
    // Switch to results view's model if view exists
    if (results_view_exists) {
        QStandardItemModel *results_model = get_results_model_for_scope_item(current);
        results_proxy_model->setSourceModel(results_model);
    }

    set_results_to_icons_action->setVisible(results_view_exists);
    set_results_to_list_action->setVisible(results_view_exists);
    set_results_to_detail_action->setVisible(results_view_exists);
    customize_columns_action->setVisible(results_view_exists);

    fetch_scope(current);

    emit q->current_scope_item_changed(current);
}

void ConsoleWidgetPrivate::on_scope_items_about_to_be_removed(const QModelIndex &parent, int first, int last) {
    const QList<QModelIndex> removed_scope_items =
    [=]() {
        QList<QModelIndex> out;

        QStack<QStandardItem *> stack;

        for (int r = first; r <= last; r++) {
            const QModelIndex removed_index = scope_model->index(r, 0, parent);
            auto removed_item = scope_model->itemFromIndex(removed_index);
            stack.push(removed_item);
        }

        while (!stack.isEmpty()) {
            auto item = stack.pop();

            out.append(item->index());

            // Iterate through children
            for (int r = 0; r < item->rowCount(); r++) {
                auto child = item->child(r, 0);
                stack.push(child);
            }
        }

        return out;
    }();

    // Remove removed items from navigation history
    for (const QModelIndex index : removed_scope_items) {
        targets_past.removeAll(index);
        targets_future.removeAll(index);
    }

    // Update navigation since an item in history could've been removed
    update_navigation_actions();

    // Remove and delete results models associated with
    // removed scope items
    for (const QModelIndex &index : removed_scope_items) {
        if (results_models.contains(index)) {
            QStandardItemModel *results = results_models.take(index);
            delete results;
        }
    }
}

void ConsoleWidgetPrivate::on_focus_changed(QWidget *old, QWidget *now) {
    QAbstractItemView *new_focused_view = qobject_cast<QAbstractItemView *>(now);

    if (new_focused_view != nullptr) {
        const ResultsDescription current_results = get_current_results();
        QAbstractItemView *results_view = current_results.view()->current_view();

        if (new_focused_view == scope_view || new_focused_view == results_view) {
            focused_view = new_focused_view;

            on_selection_changed();
        }
    }
}

void ConsoleWidgetPrivate::refresh() {
    const QModelIndex current_index = focused_view->selectionModel()->currentIndex();

    q->refresh_scope(current_index);
}

void ConsoleWidgetPrivate::customize_columns() {
    const QModelIndex current_scope = q->get_current_scope_item();

    if (!current_scope.isValid()) {
        return;
    }

    const int current_results_id = current_scope.data(ConsoleRole_ResultsId).toInt();
    const ResultsDescription current_results = results_descriptions[current_results_id];

    ResultsView *results_view = current_results.view();
    if (results_view == nullptr) {
        return;
    }

    auto dialog = new CustomizeColumnsDialog(results_view->detail_view(), current_results.default_columns(), q);
    dialog->open();
}

// Set target to parent of current target
void ConsoleWidgetPrivate::navigate_up() {
    const QPersistentModelIndex old_current = QPersistentModelIndex(scope_view->currentIndex());

    if (!old_current.isValid()) {
        return;
    }

    const QModelIndex new_current = old_current.parent();

    // NOTE: parent of target can be invalid, if current is
    // head item in which case we've reached top of tree and
    // can't go up higher
    const bool can_go_up = (!new_current.isValid());
    if (!can_go_up) {
        scope_view->selectionModel()->setCurrentIndex(new_current, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);
    }
}

// NOTE: for "back" and "forward" navigation, setCurrentIndex() triggers "current changed" slot which by default erases future history, so manually restore correct navigation state afterwards
void ConsoleWidgetPrivate::navigate_back() {
    const QModelIndex old_current = scope_view->currentIndex();

    if (!old_current.isValid()) {
        return;
    }

    // NOTE: saving and restoring past and future
    // before/after setCurrentIndex() because on_current()
    // slot modifies them with the assumption that current
    // changed due to user input.
    auto saved_past = targets_past;
    auto saved_future = targets_future;

    const QPersistentModelIndex new_current = targets_past.last();
    scope_view->selectionModel()->setCurrentIndex(new_current, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);

    targets_past = saved_past;
    targets_future = saved_future;

    targets_past.removeLast();
    targets_future.prepend(old_current);

    update_navigation_actions();
}

void ConsoleWidgetPrivate::navigate_forward() {
    const QModelIndex old_current = scope_view->currentIndex();
    
    if (!old_current.isValid()) {
        return;
    }

    // NOTE: saving and restoring past and future
    // before/after setCurrentIndex() because on_current()
    // slot modifies them with the assumption that current
    // changed due to user input.
    auto saved_past = targets_past;
    auto saved_future = targets_future;

    const QPersistentModelIndex new_current = targets_future.first();
    scope_view->selectionModel()->setCurrentIndex(new_current, QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect);

    targets_past = saved_past;
    targets_future = saved_future;

    targets_past.append(old_current);
    targets_future.removeFirst();

    update_navigation_actions();
}

void ConsoleWidgetPrivate::on_start_drag(const QList<QModelIndex> &dropped_arg) {
    dropped = dropped_arg;
}

void ConsoleWidgetPrivate::on_can_drop(const QModelIndex &target, bool *ok) {
    emit q->items_can_drop(dropped, target, ok);
}

void ConsoleWidgetPrivate::on_drop(const QModelIndex &target) {
    emit q->items_dropped(dropped, target);
}

void ConsoleWidgetPrivate::set_results_to_icons() {
    set_results_to_type(ResultsViewType_Icons);
}

void ConsoleWidgetPrivate::set_results_to_list() {
    set_results_to_type(ResultsViewType_List);
}

void ConsoleWidgetPrivate::set_results_to_detail() {
    set_results_to_type(ResultsViewType_Detail);
}

void ConsoleWidgetPrivate::set_results_to_type(const ResultsViewType type) {
    const ResultsDescription results = get_current_results();
    results.view()->set_view_type(type);
}

// NOTE: as long as this is called where appropriate (on every target change), it is not necessary to do any condition checks in navigation f-ns since the actions that call them will be disabled if they can't be done
void ConsoleWidgetPrivate::update_navigation_actions() {
    const bool can_navigate_up =
    [this]() {
        const QModelIndex current = scope_view->currentIndex();
        const QModelIndex current_parent = current.parent();

        return (current.isValid() && current_parent.isValid());
    }();

    navigate_up_action->setEnabled(can_navigate_up);
    navigate_back_action->setEnabled(!targets_past.isEmpty());
    navigate_forward_action->setEnabled(!targets_future.isEmpty());
}

void ConsoleWidget::add_actions_to_action_menu(QMenu *menu) {
    menu->addAction(d->refresh_action);
    menu->addSeparator();
    menu->addAction(d->properties_action);

    d->properties_action->setVisible(false);
    d->refresh_action->setVisible(false);
}

void ConsoleWidget::add_actions_to_navigation_menu(QMenu *menu) {
    menu->addAction(d->navigate_up_action);
    menu->addAction(d->navigate_back_action);
    menu->addAction(d->navigate_forward_action);
}

void ConsoleWidget::add_actions_to_view_menu(QMenu *menu) {
    menu->addAction(d->set_results_to_icons_action);
    menu->addAction(d->set_results_to_list_action);
    menu->addAction(d->set_results_to_detail_action);

    menu->addSeparator();
    
    menu->addAction(d->customize_columns_action);
}

const ResultsDescription ConsoleWidgetPrivate::get_current_results() const {
    const QModelIndex current_scope = q->get_current_scope_item();
    const int results_id = current_scope.data(ConsoleRole_ResultsId).toInt();
    const ResultsDescription results = results_descriptions[results_id];

    return results;
}

void ConsoleWidgetPrivate::fetch_scope(const QModelIndex &index) {
    const bool was_fetched = index.data(ConsoleRole_WasFetched).toBool();

    if (!was_fetched) {
        scope_model->setData(index, true, ConsoleRole_WasFetched);

        emit q->item_fetched(index);
    }
}
