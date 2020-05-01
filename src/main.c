
// #include "mainwindow.h"
#include "containers_view.h"
#include "contents_view.h"
#include "attributes_view.h"
#include "entry.h"

#include "active_directory.h"
#include <gtk/gtk.h>

int main(int argc, char** argv) { 
    // TODO: show some kind of progress indicator before loading data? it's not instant

    bool fake_entries = false;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "fake") == 0) {
            fake_entries = true;
        }
    }

    if (fake_entries) {
        entry_init_fake();
    } else {
        // Login into ldap
        LDAP* ldap_connection = ad_login();
        if (ldap_connection == NULL) {
            printf("ad_login error: %s\n", ad_get_error());
            return 0;
        }

        // Load entry data
        entry_init();
    }


    // Setup UI
    gtk_init(&argc, &argv);

    // Load builder
    GtkBuilder *builder = gtk_builder_new_from_file("data/adtool.glade");
    if (builder == NULL) {
        printf("Failed to load glade file\n");

        return 0;
    }

    containers_init(builder);
    contents_init(builder);
    attributes_init(builder);

    gtk_builder_connect_signals(builder, NULL);

    // Get window object from builder
    GtkWidget* window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    if (window == NULL) {
        printf("Failed to get window widget\n");

        return 0;
    }
    gtk_widget_show(window);

    g_object_unref(G_OBJECT(builder));

    gtk_main();

    return 0;
}
