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

#ifndef CONSOLE_WIDGET_H
#define CONSOLE_WIDGET_H

/**
 * The central widget of the app through which the user can
 * browse and manipulate objects. Contains two panes:
 * "scope" and "results". Scope pane contains a tree of
 * items. Each scope item has it's own "results" which are
 * displayed in the results pane when the scope item is
 * selected. Results can contain children of the scope item.
 * Results may also display a custom widget. Note that this
 * class only deals with general "items" and doesn't define
 * how items are loaded, displayed or interacted with. That
 * is defined by subclassing ConsoleImpl and registering
 * implementations using register_impl() function.
 * Implementations are assigned to item types which should
 * be defined outside of this class.
 */

#include <QWidget>

class ConsoleWidgetPrivate;
class QStandardItem;
class QMenu;
class ConsoleImpl;
class ConsoleDragModel;

enum ConsoleRolePublic {
    ConsoleRole_Type = Qt::UserRole + 19,

    // NOTE: when implementing custom roles, make sure they do
    // not conflict with console roles, like this:
    //
    // enum YourRole {
    //     YourRole_First = ConsoleRole_LAST + 1,
    //     YourRole_Second = ConsoleRole_LAST + 2,
    //     ...
    // };
    ConsoleRole_LAST = Qt::UserRole + 20,
};

enum StandardAction {
    StandardAction_Copy,
    StandardAction_Cut,
    StandardAction_Rename,
    StandardAction_Delete,
    StandardAction_Paste,
    StandardAction_Print,
    StandardAction_Refresh,
    StandardAction_Properties,
};

class ConsoleWidgetActions final {
public:
    QAction *navigate_up;
    QAction *navigate_back;
    QAction *navigate_forward;
    QAction *refresh;
    QAction *customize_columns;
    QAction *view_icons;
    QAction *view_list;
    QAction *view_detail;
    QAction *toggle_console_tree;
    QAction *toggle_description_bar;
};

class ConsoleWidget final : public QWidget {
    Q_OBJECT

public:
    ConsoleWidget(QWidget *parent);

    // Sets and connects actions to internal slots
    // NOTE: must be called before restore_state(), so that
    // action state is restored
    void set_actions(const ConsoleWidgetActions &actions_arg);

    // NOTE: you must register all impl's before adding
    // items
    void register_impl(const int type, ConsoleImpl *impl);

    // These f-ns are for adding items to console. Items
    // returned from these f-ns should be used to set text,
    // icon and your custom data roles. add_scope_item()
    // adds an item that is shown both in scope and results.
    // add_results_item() adds an item that is shown only in
    // results.
    QList<QStandardItem *> add_scope_item(const int type, const QModelIndex &parent);
    QList<QStandardItem *> add_results_item(const int type, const QModelIndex &parent);

    // Deletes an item and all of it's columns
    void delete_item(const QModelIndex &index);

    // Sets current scope item in the scope tree
    void set_current_scope(const QModelIndex &index);

    // Calls refresh() of the console impl for this item
    // type
    void refresh_scope(const QModelIndex &index);

    // Gets selected item(s) from currently focused view,
    // which could be scope or results. Only the main (first
    // column) item is returned for each selected row. There
    // is always at least one selected item. If results is
    // currently focused but has no selection, selected
    // items from scope are returned instead.
    QList<QModelIndex> get_selected_items(const int type) const;

    // Get a single selected item. Use if you are sure that
    // there's only one (dialog that uses one target item
    // for example). Returns first item in list if there are
    // multiple items selected.
    QModelIndex get_selected_item(const int type) const;

    // NOTE: Search is inclusive, examining the given parent
    // and all of it's descendants. Pass QModelIndex()
    // parent to search the whole model. If no type is
    // given, then items of all types will be returned.
    QList<QModelIndex> search_items(const QModelIndex &parent, int role, const QVariant &value, const int type = -1) const;

    QModelIndex get_current_scope_item() const;
    int get_child_count(const QModelIndex &index) const;

    QStandardItem *get_item(const QModelIndex &index) const;
    QList<QStandardItem *> get_row(const QModelIndex &index) const;

    QVariant save_state() const;

    // NOTE: all impls should be registered before this is
    // called so that their state can be restored
    void restore_state(const QVariant &state);

    void delete_children(const QModelIndex &parent);

    void set_scope_view_visible(const bool visible);

    // Adds actions to a menu. Actions come from console
    // itself as well as impl's. All impl's must be
    // registered before this is called.
    void add_actions(QMenu *menu);

    // Updates visibility and disability state of actions
    // based on current selection. Call before showing the
    // menu that contains console actions.
    void update_actions();

private:
    ConsoleWidgetPrivate *d;

    friend ConsoleDragModel;
};

int console_item_get_type(const QModelIndex &index);
bool console_item_get_was_fetched(const QModelIndex &index);

#endif /* CONSOLE_WIDGET_H */
