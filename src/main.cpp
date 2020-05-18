
#include "containers_view.h"
#include "contents_view.h"
#include "attributes_view.h"
#include "ad_filter.h"
#include "ad_model.h"
#include "attributes_model.h"
#include "create_entry.h"
#include "ad_interface.h"
#include "entry_context_menu.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QString>

int main(int argc, char **argv) {
    // Load fake AD data if given "fake" argument
    // This also swaps all ad interface functions to their fake versions (including login)
    if (argc >= 1 && QString(argv[1]) == "fake") {
        FAKE_AD = true;
    }

    if (!ad_interface_login()) {
        return 1;
    }

    QApplication app(argc, argv);

    //
    // Setup ui
    //
    Ui::MainWindow ui;
    QMainWindow main_window;
    ui.setupUi(&main_window);

    AdModel ad_model;

    create_entry_init(&ad_model);

    // Attributes
    AttributesModel attributes_model;
    ui.attributes_view->setModel(&attributes_model);

    // Contents
    {
        ContentsView *view = ui.contents_view;

        auto proxy = new AdFilter(ui.menubar_view_advancedView);
        proxy->setSourceModel(&ad_model);
        view->setModel(proxy);

        view->hideColumn(AdModel::Column::DN);
    }

    // Containers
    {
        ContainersView *view = ui.containers_view;

        auto proxy = new AdFilter(ui.menubar_view_advancedView, true);
        proxy->setSourceModel(&ad_model);
        view->setModel(proxy);

        // NOTE: have to hide columns after setting model
        view->hideColumn(AdModel::Column::Category);
        view->hideColumn(AdModel::Column::Description);
        view->hideColumn(AdModel::Column::DN);
    }

    //
    // Entry context menu
    //
    {
        auto entry_context_menu = new EntryContextMenu(&main_window);

        // Popup entry context menu from both containers and contents views
        entry_context_menu->connect_view(*(ui.containers_view));
        entry_context_menu->connect_view(*(ui.contents_view));

        // Set target dn of attributes view when attributes menu of
        // entry context menu is clicked
        QObject::connect(
            entry_context_menu, &EntryContextMenu::attributes_clicked,
            ui.attributes_view, &AttributesView::set_target_dn);

        // Delete entry when delete button is pressed
        QObject::connect(
            entry_context_menu, &EntryContextMenu::delete_clicked,
            delete_entry);
    }

    // Update models on entry changes
    QObject::connect(
        &ad_interface, &AdInterface::entry_deleted,
        &ad_model, &AdModel::on_entry_deleted);
    QObject::connect(
        &ad_interface, &AdInterface::entry_deleted,
        &attributes_model, &AttributesModel::on_entry_deleted);
    QObject::connect(
        &ad_interface, &AdInterface::entry_changed,
        &ad_model, &AdModel::on_entry_changed);

    // Set root index of contents view to selection of containers view
    QObject::connect(
        ui.containers_view->selectionModel(), &QItemSelectionModel::selectionChanged,
        ui.contents_view, &ContentsView::set_root_index_from_selection);
    
    // Update entry values in AD model when that entry's attributes are changed in attributes view
    QObject::connect(
        &attributes_model, &AttributesModel::entry_changed,
        &ad_model, &AdModel::on_entry_changed);

    // Connect menubar "New" submenu's to entry creation dialogs
    QObject::connect(
        ui.menubar_new_user, &QAction::triggered,
        create_user_dialog);
    QObject::connect(
        ui.menubar_new_computer, &QAction::triggered,
        create_computer_dialog);
    QObject::connect(
        ui.menubar_new_ou, &QAction::triggered,
        create_ou_dialog);
    QObject::connect(
        ui.menubar_new_group, &QAction::triggered,
        create_group_dialog);

    main_window.show();

    return app.exec();
}
