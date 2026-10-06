#include <gtk/gtk.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DB_FILE "student_database.db"

sqlite3 *db;
GtkWidget *login_window;
GtkWidget *entry_username;
GtkWidget *entry_password;
GtkWidget *dashboard_window;
GtkWidget *entry_student_id;
GtkWidget *entry_name;
GtkWidget *entry_branch;
GtkWidget *entry_year;
GtkWidget *combo_scholarship;
GtkWidget *entry_total_fee;
GtkWidget *entry_paid_fee;
GtkWidget *tree_view;
GtkListStore *student_store;

void show_message(GtkWindow *parent, const char *title,
                  const char *message, GtkMessageType type);

void apply_css();
void open_database();
void create_tables();
void create_default_admin();
int check_login(const char *username, const char *password);
int student_exists(const char *student_id);

void show_login_window();
void show_dashboard_window();
void clear_student_form();
void refresh_student_table(const char *specific_id);

void set_combo_scholarship(const char *value);
const char *get_combo_scholarship();

void on_login_clicked(GtkWidget *widget, gpointer data);
void on_add_clicked(GtkWidget *widget, gpointer data);
void on_update_clicked(GtkWidget *widget, gpointer data);
void on_delete_clicked(GtkWidget *widget, gpointer data);
void on_search_clicked(GtkWidget *widget, gpointer data);
void on_clear_clicked(GtkWidget *widget, gpointer data);
void on_refresh_clicked(GtkWidget *widget, gpointer data);
void on_logout_clicked(GtkWidget *widget, gpointer data);

void on_tree_selection_changed(GtkTreeSelection *selection, gpointer data);

void apply_css()
{
    GtkCssProvider *provider = gtk_css_provider_new();

    gtk_css_provider_load_from_data(
        provider,
        "window { background: #F4F6F7; }"
        "#title_label { font-size: 24px; font-weight: bold; color: #1A5276; }"
        "#small_title_label { font-size: 20px; font-weight: bold; color: #1A5276; }"
        "button { padding: 8px; font-weight: bold; }"
        "entry { padding: 5px; }"
        "treeview { font-size: 12px; }",
        -1,
        NULL
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(provider);
}

void show_message(GtkWindow *parent, const char *title,
                  const char *message, GtkMessageType type)
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(
        parent,
        GTK_DIALOG_MODAL,
        type,
        GTK_BUTTONS_OK,
        "%s",
        message
    );

    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

int is_empty(const char *text)
{
    return text == NULL || strlen(text) == 0;
}

const char *get_combo_scholarship()
{
    int active_index =
        gtk_combo_box_get_active(GTK_COMBO_BOX(combo_scholarship));

    if (active_index == 1)
        return "Non-Scholarship";

    return "Scholarship";
}

void set_combo_scholarship(const char *value)
{
    if (strcmp(value, "Scholarship") == 0)
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo_scholarship), 0);
    else
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo_scholarship), 1);
}

void open_database()
{
    int result = sqlite3_open(DB_FILE, &db);

    if (result != SQLITE_OK)
    {
        printf("Database opening failed: %s\n",
               sqlite3_errmsg(db));
        exit(1);
    }
}

void create_tables()
{
    char *error_message = NULL;

    const char *users_table =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL,"
        "role TEXT NOT NULL);";

    const char *students_table =
        "CREATE TABLE IF NOT EXISTS students ("
        "student_id TEXT PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "branch TEXT NOT NULL,"
        "year TEXT NOT NULL,"
        "scholarship TEXT NOT NULL,"
        "total_fee REAL NOT NULL,"
        "paid_fee REAL NOT NULL);";

    if (sqlite3_exec(db, users_table, NULL, NULL,
                     &error_message) != SQLITE_OK)
    {
        printf("Error creating users table: %s\n",
               error_message);
        sqlite3_free(error_message);
    }

    if (sqlite3_exec(db, students_table, NULL, NULL,
                     &error_message) != SQLITE_OK)
    {
        printf("Error creating students table: %s\n",
               error_message);
        sqlite3_free(error_message);
    }
}

void create_default_admin()
{
    char *error_message = NULL;

    const char *insert_admin =
        "INSERT OR IGNORE INTO users(username, password, role)"
        " VALUES('admin', 'admin123', 'Admin');";

    if (sqlite3_exec(db, insert_admin, NULL, NULL,
                     &error_message) != SQLITE_OK)
    {
        printf("Error creating default admin: %s\n",
               error_message);
        sqlite3_free(error_message);
    }
}

int check_login(const char *username, const char *password)
{
    sqlite3_stmt *stmt;
    int login_success = 0;

    const char *sql =
        "SELECT COUNT(*) FROM users "
        "WHERE username = ? AND password = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return 0;

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (sqlite3_column_int(stmt, 0) > 0)
            login_success = 1;
    }

    sqlite3_finalize(stmt);

    return login_success;
}

int student_exists(const char *student_id)
{
    sqlite3_stmt *stmt;
    int exists = 0;

    const char *sql =
        "SELECT COUNT(*) FROM students WHERE student_id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return 0;

    sqlite3_bind_text(stmt, 1, student_id, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (sqlite3_column_int(stmt, 0) > 0)
            exists = 1;
    }

    sqlite3_finalize(stmt);

    return exists;
}

void show_login_window()
{
    GtkWidget *main_box;
    GtkWidget *title_label;
    GtkWidget *grid;
    GtkWidget *label_username;
    GtkWidget *label_password;
    GtkWidget *login_button;
    GtkWidget *note_label;

    login_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(
        GTK_WINDOW(login_window),
        "Student Database Login"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(login_window), 420, 260
    );

    gtk_window_set_position(
        GTK_WINDOW(login_window),
        GTK_WIN_POS_CENTER
    );

    gtk_container_set_border_width(
        GTK_CONTAINER(login_window), 20
    );

    g_signal_connect(
        login_window,
        "destroy",
        G_CALLBACK(gtk_main_quit),
        NULL
    );

    main_box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL, 12
    );

    gtk_container_add(
        GTK_CONTAINER(login_window), main_box
    );

    title_label = gtk_label_new(
        "STUDENT DATABASE LOGIN"
    );

    gtk_widget_set_name(
        title_label, "title_label"
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        title_label,
        FALSE, FALSE, 5
    );

    grid = gtk_grid_new();

    gtk_grid_set_row_spacing(
        GTK_GRID(grid), 10
    );

    gtk_grid_set_column_spacing(
        GTK_GRID(grid), 10
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        grid,
        FALSE, FALSE, 5
    );

    label_username = gtk_label_new("Username:");
    label_password = gtk_label_new("Password:");

    entry_username = gtk_entry_new();
    entry_password = gtk_entry_new();

    gtk_entry_set_visibility(
        GTK_ENTRY(entry_password), FALSE
    );

    gtk_entry_set_invisible_char(
        GTK_ENTRY(entry_password), '*'
    );

    gtk_grid_attach(
        GTK_GRID(grid),
        label_username, 0, 0, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(grid),
        entry_username, 1, 0, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(grid),
        label_password, 0, 1, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(grid),
        entry_password, 1, 1, 1, 1
    );

    login_button = gtk_button_new_with_label("Login");

    gtk_box_pack_start(
        GTK_BOX(main_box),
        login_button,
        FALSE, FALSE, 5
    );

    g_signal_connect(
        login_button,
        "clicked",
        G_CALLBACK(on_login_clicked),
        NULL
    );

    note_label = gtk_label_new(
        "Default login: username = admin, password = admin123"
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        note_label,
        FALSE, FALSE, 5
    );

    gtk_widget_show_all(login_window);
}

void on_login_clicked(GtkWidget *widget, gpointer data)
{
    const char *username =
        gtk_entry_get_text(GTK_ENTRY(entry_username));

    const char *password =
        gtk_entry_get_text(GTK_ENTRY(entry_password));

    if (is_empty(username) || is_empty(password))
    {
        show_message(
            GTK_WINDOW(login_window),
            "Login Error",
            "Please enter username and password.",
            GTK_MESSAGE_WARNING
        );
        return;
    }

    if (check_login(username, password))
    {
        gtk_widget_destroy(login_window);
        show_dashboard_window();
    }
    else
    {
        show_message(
            GTK_WINDOW(login_window),
            "Login Failed",
            "Invalid username or password.",
            GTK_MESSAGE_ERROR
        );
    }
}

void show_dashboard_window()
{
    GtkWidget *main_box;
    GtkWidget *title_label;
    GtkWidget *form_frame;
    GtkWidget *form_grid;
    GtkWidget *button_box;

    GtkWidget *add_button;
    GtkWidget *update_button;
    GtkWidget *delete_button;
    GtkWidget *search_button;
    GtkWidget *clear_button;
    GtkWidget *refresh_button;
    GtkWidget *logout_button;

    GtkWidget *scroll_window;

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreeSelection *selection;

    dashboard_window =
        gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(
        GTK_WINDOW(dashboard_window),
        "Student Database Management System"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(dashboard_window),
        1050, 650
    );

    gtk_window_set_position(
        GTK_WINDOW(dashboard_window),
        GTK_WIN_POS_CENTER
    );

    gtk_container_set_border_width(
        GTK_CONTAINER(dashboard_window), 12
    );

    g_signal_connect(
        dashboard_window,
        "destroy",
        G_CALLBACK(gtk_main_quit),
        NULL
    );

    main_box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL, 10
    );

    gtk_container_add(
        GTK_CONTAINER(dashboard_window),
        main_box
    );

    title_label = gtk_label_new(
        "STUDENT DATABASE MANAGEMENT SYSTEM"
    );

    gtk_widget_set_name(
        title_label, "small_title_label"
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        title_label,
        FALSE, FALSE, 5
    );

    form_frame = gtk_frame_new(
        "Student Information"
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        form_frame,
        FALSE, FALSE, 5
    );

    form_grid = gtk_grid_new();

    gtk_grid_set_row_spacing(
        GTK_GRID(form_grid), 8
    );

    gtk_grid_set_column_spacing(
        GTK_GRID(form_grid), 10
    );

    gtk_container_set_border_width(
        GTK_CONTAINER(form_grid), 10
    );

    gtk_container_add(
        GTK_CONTAINER(form_frame),
        form_grid
    );

    entry_student_id = gtk_entry_new();
    entry_name = gtk_entry_new();
    entry_branch = gtk_entry_new();
    entry_year = gtk_entry_new();

    combo_scholarship =
        gtk_combo_box_text_new();

    entry_total_fee = gtk_entry_new();
    entry_paid_fee = gtk_entry_new();

    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(combo_scholarship),
        "Scholarship"
    );

    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(combo_scholarship),
        "Non-Scholarship"
    );

    gtk_combo_box_set_active(
        GTK_COMBO_BOX(combo_scholarship), 0
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        gtk_label_new("Student ID:"),
        0, 0, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        entry_student_id,
        1, 0, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        gtk_label_new("Name:"),
        2, 0, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        entry_name,
        3, 0, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        gtk_label_new("Branch:"),
        0, 1, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        entry_branch,
        1, 1, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        gtk_label_new("Year:"),
        2, 1, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        entry_year,
        3, 1, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        gtk_label_new("Scholarship:"),
        0, 2, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        combo_scholarship,
        1, 2, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        gtk_label_new("Total Fee:"),
        2, 2, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        entry_total_fee,
        3, 2, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        gtk_label_new("Paid Fee:"),
        0, 3, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(form_grid),
        entry_paid_fee,
        1, 3, 1, 1
    );

    button_box = gtk_box_new(
        GTK_ORIENTATION_HORIZONTAL, 8
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        button_box,
        FALSE, FALSE, 5
    );

    add_button =
        gtk_button_new_with_label("Add Student");

    update_button =
        gtk_button_new_with_label("Update");

    delete_button =
        gtk_button_new_with_label("Delete");

    search_button =
        gtk_button_new_with_label("Search by ID");

    clear_button =
        gtk_button_new_with_label("Clear Form");

    refresh_button =
        gtk_button_new_with_label("Refresh Table");

    logout_button =
        gtk_button_new_with_label("Logout");

    gtk_box_pack_start(
        GTK_BOX(button_box),
        add_button, TRUE, TRUE, 2
    );

    gtk_box_pack_start(
        GTK_BOX(button_box),
        update_button, TRUE, TRUE, 2
    );

    gtk_box_pack_start(
        GTK_BOX(button_box),
        delete_button, TRUE, TRUE, 2
    );

    gtk_box_pack_start(
        GTK_BOX(button_box),
        search_button, TRUE, TRUE, 2
    );

    gtk_box_pack_start(
        GTK_BOX(button_box),
        clear_button, TRUE, TRUE, 2
    );

    gtk_box_pack_start(
        GTK_BOX(button_box),
        refresh_button, TRUE, TRUE, 2
    );

    gtk_box_pack_start(
        GTK_BOX(button_box),
        logout_button, TRUE, TRUE, 2
    );

    g_signal_connect(
        add_button,
        "clicked",
        G_CALLBACK(on_add_clicked),
        NULL
    );

    g_signal_connect(
        update_button,
        "clicked",
        G_CALLBACK(on_update_clicked),
        NULL
    );

    g_signal_connect(
        delete_button,
        "clicked",
        G_CALLBACK(on_delete_clicked),
        NULL
    );

    g_signal_connect(
        search_button,
        "clicked",
        G_CALLBACK(on_search_clicked),
        NULL
    );

    g_signal_connect(
        clear_button,
        "clicked",
        G_CALLBACK(on_clear_clicked),
        NULL
    );

    g_signal_connect(
        refresh_button,
        "clicked",
        G_CALLBACK(on_refresh_clicked),
        NULL
    );

    g_signal_connect(
        logout_button,
        "clicked",
        G_CALLBACK(on_logout_clicked),
        NULL
    );

    student_store = gtk_list_store_new(
        8,
        G_TYPE_STRING,
        G_TYPE_STRING,
        G_TYPE_STRING,
        G_TYPE_STRING,
        G_TYPE_STRING,
        G_TYPE_STRING,
        G_TYPE_STRING,
        G_TYPE_STRING
    );

    tree_view =
        gtk_tree_view_new_with_model(
            GTK_TREE_MODEL(student_store)
        );

    gtk_tree_view_set_grid_lines(
        GTK_TREE_VIEW(tree_view),
        GTK_TREE_VIEW_GRID_LINES_BOTH
    );

    const char *columns[] = {
        "Student ID",
        "Name",
        "Branch",
        "Year",
        "Scholarship",
        "Total Fee",
        "Paid Fee",
        "Pending Fee"
    };

    for (int i = 0; i < 8; i++)
    {
        renderer = gtk_cell_renderer_text_new();

        column =
            gtk_tree_view_column_new_with_attributes(
                columns[i],
                renderer,
                "text",
                i,
                NULL
            );

        gtk_tree_view_append_column(
            GTK_TREE_VIEW(tree_view),
            column
        );
    }

    selection =
        gtk_tree_view_get_selection(
            GTK_TREE_VIEW(tree_view)
        );

    g_signal_connect(
        selection,
        "changed",
        G_CALLBACK(on_tree_selection_changed),
        NULL
    );

    scroll_window =
        gtk_scrolled_window_new(NULL, NULL);

    gtk_widget_set_vexpand(
        scroll_window, TRUE
    );

    gtk_container_add(
        GTK_CONTAINER(scroll_window),
        tree_view
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        scroll_window,
        TRUE, TRUE, 5
    );

    refresh_student_table(NULL);

    gtk_widget_show_all(
        dashboard_window
    );
}

void clear_student_form()
{
    gtk_entry_set_text(
        GTK_ENTRY(entry_student_id), ""
    );

    gtk_entry_set_text(
        GTK_ENTRY(entry_name), ""
    );

    gtk_entry_set_text(
        GTK_ENTRY(entry_branch), ""
    );

    gtk_entry_set_text(
        GTK_ENTRY(entry_year), ""
    );

    gtk_combo_box_set_active(
        GTK_COMBO_BOX(combo_scholarship), 0
    );

    gtk_entry_set_text(
        GTK_ENTRY(entry_total_fee), ""
    );

    gtk_entry_set_text(
        GTK_ENTRY(entry_paid_fee), ""
    );
}

void on_add_clicked(GtkWidget *widget, gpointer data)
{
    sqlite3_stmt *stmt;

    const char *student_id =
        gtk_entry_get_text(GTK_ENTRY(entry_student_id));

    const char *name =
        gtk_entry_get_text(GTK_ENTRY(entry_name));

    const char *branch =
        gtk_entry_get_text(GTK_ENTRY(entry_branch));

    const char *year =
        gtk_entry_get_text(GTK_ENTRY(entry_year));

    const char *scholarship =
        get_combo_scholarship();

    double total_fee =
        atof(gtk_entry_get_text(GTK_ENTRY(entry_total_fee)));

    double paid_fee =
        atof(gtk_entry_get_text(GTK_ENTRY(entry_paid_fee)));

    const char *sql =
        "INSERT INTO students "
        "(student_id,name,branch,year,scholarship,total_fee,paid_fee) "
        "VALUES(?,?,?,?,?,?,?);";

    if (is_empty(student_id) ||
        is_empty(name) ||
        is_empty(branch) ||
        is_empty(year))
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Missing Data",
            "Please fill Student ID, Name, Branch and Year.",
            GTK_MESSAGE_WARNING
        );
        return;
    }

    if (student_exists(student_id))
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Duplicate ID",
            "Student ID already exists. Use Update instead.",
            GTK_MESSAGE_ERROR
        );
        return;
    }

    if (sqlite3_prepare_v2(
            db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        return;
    }

    sqlite3_bind_text(
        stmt, 1, student_id, -1, SQLITE_STATIC
    );

    sqlite3_bind_text(
        stmt, 2, name, -1, SQLITE_STATIC
    );

    sqlite3_bind_text(
        stmt, 3, branch, -1, SQLITE_STATIC
    );

    sqlite3_bind_text(
        stmt, 4, year, -1, SQLITE_STATIC
    );

    sqlite3_bind_text(
        stmt, 5, scholarship, -1, SQLITE_STATIC
    );

    sqlite3_bind_double(
        stmt, 6, total_fee
    );

    sqlite3_bind_double(
        stmt, 7, paid_fee
    );

    if (sqlite3_step(stmt) == SQLITE_DONE)
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Success",
            "Student record added successfully.",
            GTK_MESSAGE_INFO
        );

        clear_student_form();
        refresh_student_table(NULL);
    }

    sqlite3_finalize(stmt);
}

void on_update_clicked(GtkWidget *widget, gpointer data)
{
    sqlite3_stmt *stmt;

    const char *student_id =
        gtk_entry_get_text(GTK_ENTRY(entry_student_id));

    const char *name =
        gtk_entry_get_text(GTK_ENTRY(entry_name));

    const char *branch =
        gtk_entry_get_text(GTK_ENTRY(entry_branch));

    const char *year =
        gtk_entry_get_text(GTK_ENTRY(entry_year));

    const char *scholarship =
        get_combo_scholarship();

    double total_fee =
        atof(gtk_entry_get_text(GTK_ENTRY(entry_total_fee)));

    double paid_fee =
        atof(gtk_entry_get_text(GTK_ENTRY(entry_paid_fee)));

    const char *sql =
        "UPDATE students SET "
        "name=?, branch=?, year=?, scholarship=?, "
        "total_fee=?, paid_fee=? "
        "WHERE student_id=?;";

    if (is_empty(student_id))
        return;

    if (!student_exists(student_id))
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Not Found",
            "Student ID not found.",
            GTK_MESSAGE_ERROR
        );
        return;
    }

    if (sqlite3_prepare_v2(
            db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return;

    sqlite3_bind_text(
        stmt, 1, name, -1, SQLITE_STATIC
    );

    sqlite3_bind_text(
        stmt, 2, branch, -1, SQLITE_STATIC
    );

    sqlite3_bind_text(
        stmt, 3, year, -1, SQLITE_STATIC
    );

    sqlite3_bind_text(
        stmt, 4, scholarship, -1, SQLITE_STATIC
    );

    sqlite3_bind_double(
        stmt, 5, total_fee
    );

    sqlite3_bind_double(
        stmt, 6, paid_fee
    );

    sqlite3_bind_text(
        stmt, 7, student_id, -1, SQLITE_STATIC
    );

    if (sqlite3_step(stmt) == SQLITE_DONE)
    {
        show_message(
            GTK_WINDOW(dashboard_window),
            "Success",
            "Student record updated successfully.",
            GTK_MESSAGE_INFO
        );

        refresh_student_table(NULL);
    }

    sqlite3_finalize(stmt);
}

void on_delete_clicked(GtkWidget *widget, gpointer data)
{
    sqlite3_stmt *stmt;

    const char *student_id =
        gtk_entry_get_text(GTK_ENTRY(entry_student_id));

    const char *sql =
        "DELETE FROM students WHERE student_id=?;";

    if (is_empty(student_id))
        return;

    if (!student_exists(student_id))
        return;

    if (sqlite3_prepare_v2(
            db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return;

    sqlite3_bind_text(
        stmt, 1, student_id, -1, SQLITE_STATIC
    );

    if (sqlite3_step(stmt) == SQLITE_DONE)
    {
        clear_student_form();
        refresh_student_table(NULL);
    }

    sqlite3_finalize(stmt);
}

void on_search_clicked(GtkWidget *widget, gpointer data)
{
    const char *student_id =
        gtk_entry_get_text(GTK_ENTRY(entry_student_id));

    if (is_empty(student_id))
        return;

    if (!student_exists(student_id))
    {
        refresh_student_table(NULL);
        return;
    }

    refresh_student_table(student_id);
}

void on_clear_clicked(GtkWidget *widget, gpointer data)
{
    clear_student_form();
}

void on_refresh_clicked(GtkWidget *widget, gpointer data)
{
    refresh_student_table(NULL);
}

void on_logout_clicked(GtkWidget *widget, gpointer data)
{
    gtk_widget_destroy(dashboard_window);
    show_login_window();
}

void refresh_student_table(const char *specific_id)
{
    sqlite3_stmt *stmt;
    GtkTreeIter iter;

    const char *sql =
        "SELECT student_id,name,branch,year,scholarship,"
        "total_fee,paid_fee,(total_fee-paid_fee) "
        "FROM students";

    gtk_list_store_clear(student_store);

    if (specific_id != NULL)
        sql = "SELECT student_id,name,branch,year,scholarship,"
              "total_fee,paid_fee,(total_fee-paid_fee) "
              "FROM students WHERE student_id=?";

    if (sqlite3_prepare_v2(
            db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return;

    if (specific_id != NULL)
    {
        sqlite3_bind_text(
            stmt, 1, specific_id, -1, SQLITE_STATIC
        );
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char total[30], paid[30], pending[30];

        snprintf(
            total, sizeof(total), "%.2f",
            sqlite3_column_double(stmt, 5)
        );

        snprintf(
            paid, sizeof(paid), "%.2f",
            sqlite3_column_double(stmt, 6)
        );

        snprintf(
            pending, sizeof(pending), "%.2f",
            sqlite3_column_double(stmt, 7)
        );

        gtk_list_store_append(
            student_store, &iter
        );

        gtk_list_store_set(
            student_store,
            &iter,
            0, sqlite3_column_text(stmt, 0),
            1, sqlite3_column_text(stmt, 1),
            2, sqlite3_column_text(stmt, 2),
            3, sqlite3_column_text(stmt, 3),
            4, sqlite3_column_text(stmt, 4),
            5, total,
            6, paid,
            7, pending,
            -1
        );
    }

    sqlite3_finalize(stmt);
}

void on_tree_selection_changed(
    GtkTreeSelection *selection,
    gpointer data)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    gchar *id, *name, *branch, *year;
    gchar *scholarship, *total, *paid, *pending;

    if (gtk_tree_selection_get_selected(
            selection, &model, &iter))
    {
        gtk_tree_model_get(
            model,
            &iter,
            0, &id,
            1, &name,
            2, &branch,
            3, &year,
            4, &scholarship,
            5, &total,
            6, &paid,
            7, &pending,
            -1
        );

        gtk_entry_set_text(
            GTK_ENTRY(entry_student_id), id
        );

        gtk_entry_set_text(
            GTK_ENTRY(entry_name), name
        );

        gtk_entry_set_text(
            GTK_ENTRY(entry_branch), branch
        );

        gtk_entry_set_text(
            GTK_ENTRY(entry_year), year
        );

        set_combo_scholarship(scholarship);

        gtk_entry_set_text(
            GTK_ENTRY(entry_total_fee), total
        );

        gtk_entry_set_text(
            GTK_ENTRY(entry_paid_fee), paid
        );

        g_free(id);
        g_free(name);
        g_free(branch);
        g_free(year);
        g_free(scholarship);
        g_free(total);
        g_free(paid);
        g_free(pending);
    }
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    apply_css();
    open_database();
    create_tables();
    create_default_admin();

    show_login_window();

    gtk_main();

    sqlite3_close(db);

    return 0;
}