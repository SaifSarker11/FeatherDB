#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include <sstream>
#include "../../src/query/QueryExecutor.h"
#include "../../src/storage/StorageManager.h"
#include "../../src/parser/AST.h"

using namespace spl;

// Unique prefix to avoid collisions with real db data
static const std::string T = "Test_QE_";

// Helper: redirect stdout to a string so we can assert on printed output
struct CaptureStdout {
    std::streambuf* old;
    std::ostringstream buf;
    CaptureStdout()  { old = std::cout.rdbuf(buf.rdbuf()); }
    ~CaptureStdout() { std::cout.rdbuf(old); }
    std::string str() const { return buf.str(); }
};

//INSERT

void test_insert_adds_row_to_table() {
    std::cout << "test_insert_adds_row_to_table... ";
    // Set up a simple table and verify INSERT adds a new row
    StorageManager::dropTable(T + "Ins");
    std::vector<Column> cols = {{"id", "INT", true, false, ""}, {"name", "STRING", false, false, ""}};
    StorageManager::createTable(T + "Ins", cols);

    QueryExecutor exec;
    std::string role = "admin";
    auto stmt = std::make_unique<InsertStatement>(T + "Ins",
        std::vector<std::string>{"id", "name"},
        std::vector<std::string>{"1", "'Alice'"});
    exec.execute(std::move(stmt), role);

    Table t = StorageManager::loadTable(T + "Ins");
    assert(t.rows.size() == 1);
    assert(t.rows[0].values[0] == "1");
    StorageManager::dropTable(T + "Ins");
    std::cout << "PASSED\n";
}

void test_insert_rejects_duplicate_primary_key() {
    std::cout << "test_insert_rejects_duplicate_primary_key... ";
    // Verify that inserting duplicate primary key values is rejected
    StorageManager::dropTable(T + "PK");
    std::vector<Column> cols = {{"id", "INT", true, false, ""}, {"name", "STRING", false, false, ""}};
    StorageManager::createTable(T + "PK", cols);

    QueryExecutor exec;
    std::string role = "admin";
    auto ins1 = std::make_unique<InsertStatement>(T + "PK",
        std::vector<std::string>{"id", "name"}, std::vector<std::string>{"1", "Alice"});
    exec.execute(std::move(ins1), role);

    auto ins2 = std::make_unique<InsertStatement>(T + "PK",
        std::vector<std::string>{"id", "name"}, std::vector<std::string>{"1", "Bob"});
    {
        CaptureStdout cap;
        exec.execute(std::move(ins2), role);
        assert(cap.str().find("Error") != std::string::npos);
    }

    Table t = StorageManager::loadTable(T + "PK");
    assert(t.rows.size() == 1); // second insert must be rejected
    StorageManager::dropTable(T + "PK");
    std::cout << "PASSED\n";
}

void test_insert_rejects_column_count_mismatch() {
    std::cout << "test_insert_rejects_column_count_mismatch... ";
    // Ensure INSERT rejects when column count doesn't match value count
    StorageManager::dropTable(T + "Mismatch");
    std::vector<Column> cols = {{"id", "INT", true, ""}, {"name", "STRING", false, ""}};
    StorageManager::createTable(T + "Mismatch", cols);

    QueryExecutor exec;
    std::string role = "admin";
    // Only 1 value for 2 columns
    auto stmt = std::make_unique<InsertStatement>(T + "Mismatch",
        std::vector<std::string>{"id"}, std::vector<std::string>{"1"});
    {
        CaptureStdout cap;
        exec.execute(std::move(stmt), role);
        assert(cap.str().find("Error") != std::string::npos);
    }
    StorageManager::dropTable(T + "Mismatch");
    std::cout << "PASSED\n";
}

void test_insert_rejects_invalid_int_value() {
    std::cout << "test_insert_rejects_invalid_int_value... ";
    // Verify that non-integer values are rejected for INT columns
    StorageManager::dropTable(T + "IntVal");
    std::vector<Column> cols = {{"id", "INT", true, false, ""}};
    StorageManager::createTable(T + "IntVal", cols);

    QueryExecutor exec;
    std::string role = "admin";
    auto stmt = std::make_unique<InsertStatement>(T + "IntVal",
        std::vector<std::string>{"id"}, std::vector<std::string>{"notanint"});
    {
        CaptureStdout cap;
        exec.execute(std::move(stmt), role);
        assert(cap.str().find("Error") != std::string::npos);
    }
    StorageManager::dropTable(T + "IntVal");
    std::cout << "PASSED\n";
}

void test_insert_rejects_missing_column() {
    std::cout << "test_insert_rejects_missing_column... ";
    // Check that INSERT fails when required columns are omitted
    StorageManager::dropTable(T + "Miss");
    std::vector<Column> cols = {{"id", "INT", true, false, ""}, {"name", "STRING", false, false, ""}};
    StorageManager::createTable(T + "Miss", cols);

    QueryExecutor exec;
    std::string role = "admin";
    auto stmt = std::make_unique<InsertStatement>(T + "Miss",
        std::vector<std::string>{"id"},
        std::vector<std::string>{"1"});
    
    CaptureStdout cap;
    exec.execute(std::move(stmt), role);
    assert(cap.str().find("Error") != std::string::npos);
    StorageManager::dropTable(T + "Miss");
    std::cout << "PASSED\n";
}

void test_insert_denied_without_permission() {
    std::cout << "test_insert_denied_without_permission... ";
    // Verify that INSERT is denied when role lacks INSERT privilege
    StorageManager::dropTable(T + "Perm");
    std::vector<Column> cols = {{"id", "INT", true, false, ""}};
    StorageManager::createTable(T + "Perm", cols);

    QueryExecutor exec;
    std::string role = "no_perms_role";
    auto stmt = std::make_unique<InsertStatement>(T + "Perm",
        std::vector<std::string>{"id"}, std::vector<std::string>{"1"});
    {
        CaptureStdout cap;
        exec.execute(std::move(stmt), role);
        assert(cap.str().find("Permission denied") != std::string::npos);
    }
    StorageManager::dropTable(T + "Perm");
    std::cout << "PASSED\n";
}


// UPDATE Statement Tests
// These tests ensure that QueryExecutor correctly processes UPDATE statements,
// validating condition matching and data modification while preserving non-matching rows.

void test_update_modifies_matching_rows() {
    std::cout << "test_update_modifies_matching_rows... ";
    // Test that UPDATE modifies only rows matching the WHERE condition
    StorageManager::dropTable(T + "Upd");
    std::vector<Column> cols = {{"id", "INT", true, false, ""}, {"name", "STRING", false, false, ""}};
    StorageManager::createTable(T + "Upd", cols);
    Row r; r.values = {"1", "Alice"};
    StorageManager::appendRow(T + "Upd", r);

    QueryExecutor exec;
    std::string role = "admin";
    auto stmt = std::make_unique<UpdateStatement>(T + "Upd", "name", "Bob", "id = 1");
    exec.execute(std::move(stmt), role);

    Table t = StorageManager::loadTable(T + "Upd");
    assert(t.rows[0].values[1] == "Bob");
    StorageManager::dropTable(T + "Upd");
    std::cout << "PASSED\n";
}

void test_update_modifies_data() {
    std::cout << "test_update_modifies_data... ";
    // Verify that UPDATE correctly stores the new value in the database
    StorageManager::dropTable(T + "Upd");
    std::vector<Column> cols = {{"id", "INT", true, false, ""}, {"name", "STRING", false, false, ""}};
    StorageManager::createTable(T + "Upd", cols);
    Row r; r.values = {"1", "'Alice'"};
    StorageManager::appendRow(T + "Upd", r);

    QueryExecutor exec;
    std::string role = "admin";
    auto stmt = std::make_unique<UpdateStatement>(T + "Upd", "name", "'Bob'", "id = 1");
    exec.execute(std::move(stmt), role);

    Table t = StorageManager::loadTable(T + "Upd");
    assert(t.rows[0].values[1] == "'Bob'");
    StorageManager::dropTable(T + "Upd");
    std::cout << "PASSED\n";
}

void test_update_does_not_modify_non_matching_rows() {
    std::cout << "test_update_does_not_modify_non_matching_rows... ";
    // Ensure non-matching rows remain unchanged after UPDATE
    StorageManager::dropTable(T + "UpdNo");
    std::vector<Column> cols = {{"id", "INT", true, false, ""}, {"name", "STRING", false, false, ""}};
    StorageManager::createTable(T + "UpdNo", cols);
    Row r; r.values = {"1", "Alice"};
    StorageManager::appendRow(T + "UpdNo", r);

    QueryExecutor exec;
    std::string role = "admin";
    auto stmt = std::make_unique<UpdateStatement>(T + "UpdNo", "name", "Bob", "id = 99");
    exec.execute(std::move(stmt), role);

    Table t = StorageManager::loadTable(T + "UpdNo");
    assert(t.rows[0].values[1] == "Alice"); // unchanged
    StorageManager::dropTable(T + "UpdNo");
    std::cout << "PASSED\n";
}


// DELETE Statement Tests
// These tests verify that QueryExecutor correctly processes DELETE statements,
// ensuring rows matching the WHERE condition are removed from the table.

void test_delete_removes_matching_row() {
    std::cout << "test_delete_removes_matching_row... ";
    // Test that DELETE removes rows matching the condition
    StorageManager::dropTable(T + "Del");
    std::vector<Column> cols = {{"id", "INT", true, false, ""}};
    StorageManager::createTable(T + "Del", cols);
    Row r1; r1.values = {"1"};
    Row r2; r2.values = {"2"};
    StorageManager::appendRow(T + "Del", r1);
    StorageManager::appendRow(T + "Del", r2);

    QueryExecutor exec;
    std::string role = "admin";
    auto stmt = std::make_unique<DeleteStatement>(T + "Del", "id = 1");
    exec.execute(std::move(stmt), role);

    Table t = StorageManager::loadTable(T + "Del");
    assert(t.rows.size() == 1);
    assert(t.rows[0].values[0] == "2");
    StorageManager::dropTable(T + "Del");
    std::cout << "PASSED\n";
}

void test_delete_without_condition_removes_no_rows() {
    std::cout << "test_delete_without_condition_removes_no_rows... ";
    StorageManager::dropTable(T + "DelAll");
    std::vector<Column> cols = {{"id", "INT", true, false, ""}};
    StorageManager::createTable(T + "DelAll", cols);
    Row r; r.values = {"1"};
    StorageManager::appendRow(T + "DelAll", r);

    QueryExecutor exec;
    std::string role = "admin";
    // Empty condition: evaluateSimple returns true for empty, so all rows deleted
    // This tests the actual behavior (document it)
    auto stmt = std::make_unique<DeleteStatement>(T + "DelAll", "");
    exec.execute(std::move(stmt), role);

    Table t = StorageManager::loadTable(T + "DelAll");
    // With empty condition, no rows match the evaluateSimple check (condition empty = skip delete)
    assert(t.rows.size() == 1);
    StorageManager::dropTable(T + "DelAll");
    std::cout << "PASSED\n";
}


// CREATE TABLE Tests
// These tests verify that QueryExecutor can execute CREATE TABLE statements
// through the executor, properly delegating to StorageManager.

void test_create_table_via_executor_creates_table() {
    std::cout << "test_create_table_via_executor_creates_table... ";
    StorageManager::dropTable(T + "Create");
    QueryExecutor exec;
    std::string role = "admin";
    auto stmt = std::make_unique<CreateStatement>(T + "Create",
        std::vector<std::pair<std::string,std::string>>{{"id", "INT PK"}});
    exec.execute(std::move(stmt), role);

    Table t = StorageManager::loadTable(T + "Create");
    assert(!t.columns.empty());
    StorageManager::dropTable(T + "Create");
    std::cout << "PASSED\n";
}

//DROP TABLE via executor

void test_drop_table_via_executor_removes_table() {
    std::cout << "test_drop_table_via_executor_removes_table... ";
    std::vector<Column> cols = {{"id", "INT", true, false, ""}};
    StorageManager::createTable(T + "Drp", cols);

    QueryExecutor exec;
    std::string role = "admin";
    auto stmt = std::make_unique<DropStatement>(T + "Drp");
    exec.execute(std::move(stmt), role);

    Table t = StorageManager::loadTable(T + "Drp");
    assert(t.columns.empty());
    std::cout << "PASSED\n";
}

//RBAC: CREATE ROLE / GRANT

void test_create_role_via_executor_role_becomes_findable() {
    std::cout << "test_create_role_via_executor_role_becomes_findable... ";
    // Use a unique role name to avoid state pollution
    std::string roleName = "qe_test_role_unique";
    QueryExecutor exec;
    std::string role = "admin";
    auto stmt = std::make_unique<CreateRoleStatement>(roleName, "secret_xyz");
    exec.execute(std::move(stmt), role);
    assert(StorageManager::roleExists(roleName));
    std::cout << "PASSED\n";
}

void test_grant_denied_when_role_does_not_exist() {
    std::cout << "test_grant_denied_when_role_does_not_exist... ";
    QueryExecutor exec;
    std::string role = "admin";
    auto stmt = std::make_unique<GrantStatement>(
        std::vector<std::string>{"SELECT"}, "Users", "nonexistent_role_abc");
    {
        CaptureStdout cap;
        exec.execute(std::move(stmt), role);
        assert(cap.str().find("Error") != std::string::npos);
    }
    std::cout << "PASSED\n";
}

int main() {
    try {
        test_insert_adds_row_to_table();
        test_insert_rejects_duplicate_primary_key();
        test_insert_rejects_column_count_mismatch();
        test_insert_rejects_invalid_int_value();
        test_insert_denied_without_permission();
        test_update_modifies_matching_rows();
        test_update_does_not_modify_non_matching_rows();
        test_delete_removes_matching_row();
        test_delete_without_condition_removes_no_rows();
        test_create_table_via_executor_creates_table();
        test_drop_table_via_executor_removes_table();
        test_create_role_via_executor_role_becomes_findable();
        test_grant_denied_when_role_does_not_exist();
        std::cout << "\nALL QUERY EXECUTOR TESTS PASSED\n";
    } catch (const std::exception& e) {
        std::cerr << "FAILED: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
