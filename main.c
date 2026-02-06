#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>
#include <string.h>

// 선택된 앱을 실행하는 함수
static void launch_app_by_name(const char *app_name, GtkApplication *app) {
    GList *apps = g_app_info_get_all();
    for (GList *l = apps; l != NULL; l = l->next) {
        GAppInfo *app_info = l->data;
        if (g_strcmp0(g_app_info_get_name(app_info), app_name) == 0) {
            g_app_info_launch(app_info, NULL, NULL, NULL);
            break;
        }
    }
    g_list_free_full(apps, g_object_unref);
    g_application_quit(G_APPLICATION(app)); // G_APPLICATION으로 정확히 캐스팅
}

static void on_row_activated(GtkListBox *list_box, GtkListBoxRow *row, gpointer user_data) {
    GtkWidget *box = gtk_list_box_row_get_child(row);
    GtkWidget *label = gtk_widget_get_last_child(box); 
    const char *app_name = gtk_label_get_text(GTK_LABEL(label));
    
    launch_app_by_name(app_name, GTK_APPLICATION(user_data));
}

static void on_search_changed(GtkEditable *editable, gpointer user_data) {
    GtkListBox *list_box = GTK_LIST_BOX(user_data);
    const char *text = gtk_editable_get_text(editable);

    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(GTK_WIDGET(list_box)))) {
        gtk_list_box_remove(list_box, child);
    }

    if (strlen(text) == 0) return;

    GList *apps = g_app_info_get_all();
    int count = 0;
    for (GList *l = apps; l != NULL; l = l->next) {
        GAppInfo *app_info = l->data;
        const char *name = g_app_info_get_name(app_info);

        char *name_lower = g_ascii_strdown(name, -1);
        char *text_lower = g_ascii_strdown(text, -1);

        if (strstr(name_lower, text_lower)) {
            GtkWidget *row_content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
            
            // 아이콘 추가
            GIcon *icon = g_app_info_get_icon(app_info);
            if (icon) {
                GtkWidget *icon_img = gtk_image_new_from_gicon(icon);
                gtk_image_set_pixel_size(GTK_IMAGE(icon_img), 32);
                gtk_box_append(GTK_BOX(row_content), icon_img);
            }

            GtkWidget *row_label = gtk_label_new(name);
            gtk_box_append(GTK_BOX(row_content), row_label);
            gtk_list_box_append(list_box, row_content);
            count++;
        }
        g_free(name_lower);
        g_free(text_lower);
    }
    g_list_free_full(apps, g_object_unref);

    // 검색 결과가 있으면 첫 번째 항목을 자동으로 선택(포커스)
    if (count > 0) {
        GtkListBoxRow *first_row = gtk_list_box_get_row_at_index(list_box, 0);
        gtk_list_box_select_row(list_box, first_row);
    }
}

// 검색창에서 엔터를 눌렀을 때 호출되는 함수 (첫 번째 선택된 항목 실행)
static void on_entry_activated(GtkEntry *entry, gpointer user_data) {
    GtkListBox *list_box = GTK_LIST_BOX(user_data);
    GtkListBoxRow *selected_row = gtk_list_box_get_selected_row(list_box);
    
    if (selected_row) {
        // row_activated 시그널을 수동으로 발생시켜서 실행 로직 태움
        g_signal_emit_by_name(list_box, "row-activated", selected_row);
    }
}

static void activate(GtkApplication *app, gpointer user_data) {
    // CSS 설정 (기존과 동일)
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "style.css");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    GtkWidget *window = gtk_application_window_new(app);
    gtk_layer_init_for_window(GTK_WINDOW(window));
    gtk_layer_set_keyboard_mode(GTK_WINDOW(window), GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(vbox, 15);
    gtk_widget_set_margin_end(vbox, 15);
    gtk_widget_set_margin_top(vbox, 15);
    gtk_widget_set_margin_bottom(vbox, 15);
    gtk_widget_set_size_request(vbox, 400, 500);

    GtkWidget *search_entry = gtk_search_entry_new();
    gtk_box_append(GTK_BOX(vbox), search_entry);

    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_box_append(GTK_BOX(vbox), scrolled);

    GtkWidget *list_box = gtk_list_box_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), list_box);

    // 시그널 연결
    g_signal_connect(search_entry, "changed", G_CALLBACK(on_search_changed), list_box);
    g_signal_connect(search_entry, "activate", G_CALLBACK(on_entry_activated), list_box); // 엔터 시 첫 번째 앱 실행
    g_signal_connect(list_box, "row-activated", G_CALLBACK(on_row_activated), app);

    gtk_window_set_child(GTK_WINDOW(window), vbox);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.example.launcher", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
