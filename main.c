#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>
#include <string.h>

// 필터링 규칙 정의
static gboolean filter_func(gpointer item, gpointer user_data) {
    const char *search_text = gtk_editable_get_text(GTK_EDITABLE(user_data));
    if (!search_text || strlen(search_text) == 0) return TRUE;

    GAppInfo *app_info = G_APP_INFO(item);
    const char *name = g_app_info_get_name(app_info);

    char *name_lower = g_ascii_strdown(name, -1);
    char *text_lower = g_ascii_strdown(search_text, -1);
    gboolean visible = (strstr(name_lower, text_lower) != NULL);

    g_free(name_lower);
    g_free(text_lower);
    return visible;
}

// 각 줄(Row)의 UI를 생성하는 콜백
static GtkWidget* setup_list_row(gpointer item, gpointer user_data) {
    GAppInfo *app_info = G_APP_INFO(item);
    
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(box, 10);
    gtk_widget_set_margin_end(box, 10);
    gtk_widget_set_margin_top(box, 5);
    gtk_widget_set_margin_bottom(box, 5);

    GIcon *icon = g_app_info_get_icon(app_info);
    if (icon) {
        GtkWidget *img = gtk_image_new_from_gicon(icon);
        gtk_image_set_pixel_size(GTK_IMAGE(img), 32);
        gtk_box_append(GTK_BOX(box), img);
    }
    
    GtkWidget *label = gtk_label_new(g_app_info_get_name(app_info));
    gtk_box_append(GTK_BOX(box), label);
    
    return box;
}

// 검색어 변경 시 필터 갱신
static void on_search_changed(GtkEditable *editable, gpointer user_data) {
    GtkFilter *filter = GTK_FILTER(user_data);
    gtk_filter_changed(filter, GTK_FILTER_CHANGE_DIFFERENT);
}

// 항목 선택 시 실행
static void on_row_activated(GtkListBox *list_box, GtkListBoxRow *row, gpointer user_data) {
    int index = gtk_list_box_row_get_index(row);
    
    // 수정된 부분: 속성 시스템을 통해 바인딩된 모델을 가져옵니다.
    GListModel *model = (GListModel *)g_object_get_data(G_OBJECT(list_box), "my-model");
    GAppInfo *app_info = g_list_model_get_item(model, index);

    if (app_info) {
        g_app_info_launch(app_info, NULL, NULL, NULL);
        g_object_unref(app_info);
    }
    g_application_quit(G_APPLICATION(user_data));
}

static gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
    if (keyval == GDK_KEY_Escape) {
        g_application_quit(G_APPLICATION(user_data));
        return TRUE;
    }
    return FALSE;
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_layer_init_for_window(GTK_WINDOW(window));
    gtk_layer_set_keyboard_mode(GTK_WINDOW(window), GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);

    // 1. 데이터 저장소 준비
    GListStore *store = g_list_store_new(G_TYPE_APP_INFO);
    GList *apps = g_app_info_get_all();
    for (GList *l = apps; l != NULL; l = l->next) {
        g_list_store_append(store, l->data);
    }
    g_list_free(apps);

    // 2. 필터 모델 설정
    GtkWidget *search_entry = gtk_search_entry_new();
    GtkCustomFilter *filter = gtk_custom_filter_new(filter_func, search_entry, NULL);
    GtkFilterListModel *filter_model = gtk_filter_list_model_new(G_LIST_MODEL(store), GTK_FILTER(filter));

    // 3. 레이아웃
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(vbox, 15);
    gtk_widget_set_margin_end(vbox, 15);
    gtk_widget_set_margin_top(vbox, 15);
    gtk_widget_set_margin_bottom(vbox, 15);
    gtk_widget_set_size_request(vbox, 400, 500);
    
    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled, TRUE);
    
    GtkWidget *list_box = gtk_list_box_new();
    // 모델을 바인딩하고, 나중에 꺼내 쓸 수 있도록 데이터를 저장해둡니다.
    gtk_list_box_bind_model(GTK_LIST_BOX(list_box), G_LIST_MODEL(filter_model), setup_list_row, NULL, NULL);
    g_object_set_data(G_OBJECT(list_box), "my-model", filter_model);

    // 이벤트 연결
    g_signal_connect(search_entry, "changed", G_CALLBACK(on_search_changed), filter);
    g_signal_connect(list_box, "row-activated", G_CALLBACK(on_row_activated), app);
    
    GtkEventController *key_ctrl = gtk_event_controller_key_new();
    gtk_event_controller_set_propagation_phase(key_ctrl, GTK_PHASE_CAPTURE);
    g_signal_connect(key_ctrl, "key-pressed", G_CALLBACK(on_key_pressed), app);
    gtk_widget_add_controller(window, key_ctrl);

    gtk_box_append(GTK_BOX(vbox), search_entry);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), list_box);
    gtk_box_append(GTK_BOX(vbox), scrolled);
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
