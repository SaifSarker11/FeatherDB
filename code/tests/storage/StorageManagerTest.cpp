#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>
#include "../../src/storage/StorageManager.h"

using namespace spl;

// Unique table name prefix to avoid collisions with real db data
static const std::string T = "Test_SM_";

//createTable

void test_create_table_returns_true_for_new_table() {
    std::cout << "test_create_table_returns_true_for_new_table... ";
    StorageManager::dropTable(T + "New");
    std::vector<Column> cols = {{"id", "INT", true, ""}};
    assert(StorageManager::createTable(T + "New", cols) == true);
    StorageManager::dropTable(T + "New");
    std::cout << "PASSED\n";
}

void test_create_table_returns_false_when_already_exists() {
    std::cout << "test_create_table_returns_false_when_already_exists... ";
    StorageManager::dropTable(T + "Dup");
    std::vector<Column> cols = {{"id", "INT", true, ""}};
    StorageManager::createTable(T + "Dup", cols);
    assert(StorageManager::createTable(T + "Dup", cols) == false);
    StorageManager::dropTable(T + "Dup");
    std::cout << "PASSED\n";
}

//loadTable / schema persistence

void test_load_table_returns_correct_column_names() {
    std::cout << "test_load_table_returns_correct_column_names... ";
    StorageManager::dropTable(T + "Load");
    std::vector<Column> cols = {{"id", "INT", true, ""}, {"name", "STRING", false, ""}};
    StorageManager::createTable(T + "Load", cols);
    Table t = StorageManager::loadTable(T + "Load");
    assert(t.columns.size() == 2);
    assert(t.columns[0].name == "id");
    assert(t.columns[1].name == "name");
    StorageManager::dropTable(T + "Load");
    std::cout << "PASSED\n";
}

void test_load_table_persists_primary_key_flag() {
    std::cout << "test_load_table_persists_primary_key_flag... ";
    StorageManager::dropTable(T + "PK");
    std::vector<Column> cols = {{"id", "INT", true, ""}};
    StorageManager::createTable(T + "PK", cols);
    Table t = StorageManager::loadTable(T + "PK");
    assert(t.columns[0].isPrimaryKey == true);
    assert(t.primaryKey == "id");
    StorageManager::dropTable(T + "PK");
    std::cout << "PASSED\n";
}

void test_load_table_persists_foreign_key_target() {
    std::cout << "test_load_table_persists_foreign_key_target... ";
    StorageManager::dropTable(T + "FK");
    std::vector<Column> cols = {
        {"id", "INT", true, ""},
        {"userId", "INT", false, "Users"}
    };
    StorageManager::createTable(T + "FK", cols);
    Table t = StorageManager::loadTable(T + "FK");
    assert(t.columns[1].fkTargetTable == "Users");
    StorageManager::dropTable(T + "FK");
    std::cout << "PASSED\n";
}

void test_load_table_returns_empty_table_when_not_found() {
    std::cout << "test_load_table_returns_empty_table_when_not_found... ";
    Table t = StorageManager::loadTable("NonExistentTable_XYZ");
    assert(t.columns.empty());
    std::cout << "PASSED\n";
}

//appendRow / loadTable rows

void test_append_row_persists_data() {
    std::cout << "test_append_row_persists_data... ";
    StorageManager::dropTable(T + "Rows");
    std::vector<Column> cols = {{"id", "INT", true, ""}, {"name", "STRING", false, ""}};
    StorageManager::createTable(T + "Rows", cols);
    Row r; r.values = {"1", "Alice"};
    assert(StorageManager::appendRow(T + "Rows", r) == true);
    Table t = StorageManager::loadTable(T + "Rows");
    assert(t.rows.size() == 1);
    assert(t.rows[0].values[0] == "1");
    assert(t.rows[0].values[1] == "Alice");
    StorageManager::dropTable(T + "Rows");
    std::cout << "PASSED\n";
}

void test_append_multiple_rows_all_persist() {
    std::cout << "test_append_multiple_rows_all_persist... ";
    StorageManager::dropTable(T + "Multi");
    std::vector<Column> cols = {{"id", "INT", true, ""}};
    StorageManager::createTable(T + "Multi", cols);
    for (int i = 1; i <= 3; ++i) {
        Row r; r.values = {std::to_string(i)};
        StorageManager::appendRow(T + "Multi", r);
    }
    Table t = StorageManager::loadTable(T + "Multi");
    assert(t.rows.size() == 3);
    StorageManager::dropTable(T + "Multi");
    std::cout << "PASSED\n";
}

//saveTable

void test_save_table_overwrites_existing_rows() {
    std::cout << "test_save_table_overwrites_existing_rows... ";
    StorageManager::dropTable(T + "Save");
    std::vector<Column> cols = {{"id", "INT", true, ""}};
    StorageManager::createTable(T + "Save", cols);
    Row r1; r1.values = {"1"};
    StorageManager::appendRow(T + "Save", r1);

    Table t = StorageManager::loadTable(T + "Save");
    t.rows.clear(); // wipe rows
    Row r2; r2.values = {"99"};
    t.rows.push_back(r2);
    StorageManager::saveTable(t);

    Table reloaded = StorageManager::loadTable(T + "Save");
    assert(reloaded.rows.size() == 1);
    assert(reloaded.rows[0].values[0] == "99");
    StorageManager::dropTable(T + "Save");
    std::cout << "PASSED\n";
}

//dropTable

void test_drop_table_removes_table_from_list() {
    std::cout << "test_drop_table_removes_table_from_list... ";
    std::vector<Column> cols = {{"id", "INT", true, ""}};
    StorageManager::createTable(T + "Drop", cols);
    StorageManager::dropTable(T + "Drop");
    auto tables = StorageManager::listTables();
    bool found = std::find(tables.begin(), tables.end(), T + "Drop") != tables.end();
    assert(!found);
    std::cout << "PASSED\n";
}

//listTables

void test_list_tables_includes_created_table() {
    std::cout << "test_list_tables_includes_created_table... ";
    StorageManager::dropTable(T + "List");
    std::vector<Column> cols = {{"id", "INT", true, ""}};
    StorageManager::createTable(T + "List", cols);
    auto tables = StorageManager::listTables();
    bool found = std::find(tables.begin(), tables.end(), T + "List") != tables.end();
    assert(found);
    StorageManager::dropTable(T + "List");
    std::cout << "PASSED\n";
}

//RBAC: roles & permissions

void test_admin_role_always_has_permission() {
    std::cout << "test_admin_role_always_has_permission... ";
    assert(StorageManager::checkPermission("admin", "SELECT", "AnyTable") == true);
    assert(StorageManager::checkPermission("admin", "DELETE", "AnyTable") == true);
    std::cout << "PASSED\n";
}

void test_unknown_role_has_no_permission() {
    std::cout << "test_unknown_role_has_no_permission... ";
    //A role that was never granted anything
    assert(StorageManager::checkPermission("ghost_role_xyz", "SELECT", "Users") == false);
    std::cout << "PASSED\n";
}

void test_admin_role_exists() {
    std::cout << "test_admin_role_exists... ";
    assert(StorageManager::roleExists("admin") == true);
    std::cout << "PASSED\n";
}

void test_nonexistent_role_does_not_exist() {
    std::cout << "test_nonexistent_role_does_not_exist... ";
    assert(StorageManager::roleExists("totally_fake_role_xyz") == false);
    std::cout << "PASSED\n";
}

//getTableSchema

void test_get_table_schema_returns_columns_without_rows() {
    std::cout << "test_get_table_schema_returns_columns_without_rows... ";
    StorageManager::dropTable(T + "Schema");
    std::vector<Column> cols = {{"id", "INT", true, ""}, {"val", "STRING", false, ""}};
    StorageManager::createTable(T + "Schema", cols);
    Row r; r.values = {"1", "test"};
    StorageManager::appendRow(T + "Schema", r);
    Table schema = StorageManager::getTableSchema(T + "Schema");
    assert(schema.columns.size() == 2);
    assert(schema.rows.empty()); // schema only, no rows
    StorageManager::dropTable(T + "Schema");
    std::cout << "PASSED\n";
}

int main() {
    try {
        test_create_table_returns_true_for_new_table();
        test_create_table_returns_false_when_already_exists();
        test_load_table_returns_correct_column_names();
        test_load_table_persists_primary_key_flag();
        test_load_table_persists_foreign_key_target();
        test_load_table_returns_empty_table_when_not_found();
        test_append_row_persists_data();
        test_append_multiple_rows_all_persist();
        test_save_table_overwrites_existing_rows();
        test_drop_table_removes_table_from_list();
        test_list_tables_includes_created_table();
        test_admin_role_always_has_permission();
        test_unknown_role_has_no_permission();
        test_admin_role_exists();
        test_nonexistent_role_does_not_exist();
        test_get_table_schema_returns_columns_without_rows();
        std::cout << "\nALL STORAGE MANAGER TESTS PASSED\n";
    } catch (const std::exception& e) {
        std::cerr << "FAILED: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
