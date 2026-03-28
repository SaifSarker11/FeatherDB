#include <iostream>
#include <cassert>
#include <stdexcept>
#include <memory>
#include "../../src/parser/Tokenizer.h"
#include "../../src/parser/SQLParser.h"
#include "../../src/parser/AST.h"


// Helper function to parse SQL strings
// This function tokenizes input and passes it to the parser
// Returns the resulting AST node for testing
static std::unique_ptr<AST> parse(const std::string& sql) {
    Tokenizer t(sql);
    SQLParser p(t);
    return p.parse();
}

// SELECT Statement Parsing Tests
// These tests verify that the SQL parser correctly identifies and processes
// SELECT statements. Tests cover basic structure, column selection, table
// names, WHERE conditions, ORDER BY clauses, and wildcard handling.

void test_parse_select_sets_correct_type() {
    std::cout << "test_parse_select_sets_correct_type... ";
    // Parse a basic SELECT statement and verify the type is recognized
    auto ast = parse("SELECT id FROM Users");
    assert(ast->type == "SELECT");
    std::cout << "PASSED\n";
}

void test_parse_select_captures_table_name() {
    std::cout << "test_parse_select_captures_table_name... ";
    // Parse a SELECT and verify the table name is correctly captured
    auto ast = parse("SELECT id FROM Orders");
    auto* stmt = static_cast<SelectStatement*>(ast.get());
    assert(stmt->table == "Orders");
    std::cout << "PASSED\n";
}

void test_parse_select_captures_multiple_columns() {
    std::cout << "test_parse_select_captures_multiple_columns... ";
    // Test parsing multiple comma-separated columns in a SELECT statement
    auto ast = parse("SELECT id, name, age FROM Users");
    auto* stmt = static_cast<SelectStatement*>(ast.get());
    assert(stmt->columns.size() == 3);
    assert(stmt->columns[0] == "id");
    assert(stmt->columns[1] == "name");
    assert(stmt->columns[2] == "age");
    std::cout << "PASSED\n";
}

void test_parse_select_captures_where_condition() {
    std::cout << "test_parse_select_captures_where_condition... ";
    // Verify that WHERE clause conditions are properly extracted
    auto ast = parse("SELECT id FROM Users WHERE id = 5");
    auto* stmt = static_cast<SelectStatement*>(ast.get());
    assert(!stmt->condition.empty());
    std::cout << "PASSED\n";
}

void test_parse_select_captures_order_by() {
    std::cout << "test_parse_select_captures_order_by... ";
    // Test that ORDER BY clauses are correctly extracted from the statement
    auto ast = parse("SELECT id FROM Users ORDER BY name");
    auto* stmt = static_cast<SelectStatement*>(ast.get());
    assert(stmt->orderBy == "name");
    std::cout << "PASSED\n";
}

void test_parse_select_star_is_single_column() {
    std::cout << "test_parse_select_star_is_single_column... ";
    // Verify that SELECT * is handled as a single wildcard token
    auto ast = parse("SELECT * FROM Users");
    auto* stmt = static_cast<SelectStatement*>(ast.get());
    assert(stmt->columns.size() == 1);
    assert(stmt->columns[0] == "*");
    std::cout << "PASSED\n";
}


// INSERT Statement Parsing Tests
// These tests ensure that INSERT statements are properly parsed, including
// table names, column lists, and value lists.

void test_parse_insert_sets_correct_type() {
    std::cout << "test_parse_insert_sets_correct_type... ";
    // Verify that INSERT statements are correctly identified by type
    auto ast = parse("INSERT INTO Users (id, name) VALUES (1, 'Alice')");
    assert(ast->type == "INSERT");
    std::cout << "PASSED\n";
}

void test_parse_insert_captures_table_and_values() {
    std::cout << "test_parse_insert_captures_table_and_values... ";
    // Parse an INSERT and verify table, columns, and values are captured
    auto ast = parse("INSERT INTO Users (id, name) VALUES (1, 'Alice')");
    auto* stmt = static_cast<InsertStatement*>(ast.get());
    assert(stmt->table == "Users");
    assert(stmt->columns.size() == 2);
    assert(stmt->values.size() == 2);
    assert(stmt->values[0] == "1");
    std::cout << "PASSED\n";
}


// UPDATE Statement Parsing Tests
// These tests verify UPDATE statement parsing, including table target,
// column/value pairs, and WHERE conditions.

void test_parse_update_sets_correct_type() {
    std::cout << "test_parse_update_sets_correct_type... ";
    // Verify UPDATE statement type identification
    auto ast = parse("UPDATE Users SET name = Alice WHERE id = 1");
    assert(ast->type == "UPDATE");
    std::cout << "PASSED\n";
}

void test_parse_update_captures_column_and_value() {
    std::cout << "test_parse_update_captures_column_and_value... ";
    // Test that UPDATE statements capture table, column, value, and conditions
    auto ast = parse("UPDATE Users SET name = Bob WHERE id = 2");
    auto* stmt = static_cast<UpdateStatement*>(ast.get());
    assert(stmt->table == "Users");
    assert(stmt->column == "name");
    assert(stmt->value == "Bob");
    std::cout << "PASSED\n";
}


// DELETE Statement Parsing Tests
// These tests ensure DELETE statements are properly parsed with table names
// and WHERE conditions. 

void test_parse_delete_sets_correct_type() {
    std::cout << "test_parse_delete_sets_correct_type... ";
    // Verify DELETE statement type identification
    auto ast = parse("DELETE FROM Users WHERE id = 1");
    assert(ast->type == "DELETE");
    std::cout << "PASSED\n";
}

void test_parse_delete_captures_table_and_condition() {
    std::cout << "test_parse_delete_captures_table_and_condition... ";
    // Test that DELETE statements capture table names and conditions
    auto ast = parse("DELETE FROM Orders WHERE id = 99");
    auto* stmt = static_cast<DeleteStatement*>(ast.get());
    assert(stmt->table == "Orders");
    assert(!stmt->condition.empty());
    std::cout << "PASSED\n";
}


// CREATE TABLE Statement Parsing Tests
// These tests verify CREATE TABLE statement parsing, including table names,
// column definitions, and primary key constraints.

void test_parse_create_sets_correct_type() {
    std::cout << "test_parse_create_sets_correct_type... ";
    // Verify CREATE TABLE statement type identification
    auto ast = parse("CREATE TABLE Items (id INT PRIMARY KEY)");
    assert(ast->type == "CREATE");
    std::cout << "PASSED\n";
}

void test_parse_create_captures_table_name_and_columns() {
    std::cout << "test_parse_create_captures_table_name_and_columns... ";
    // Test that CREATE TABLE statements capture table name and column definitions
    auto ast = parse("CREATE TABLE Products (id INT PRIMARY KEY, price INT)");
    auto* stmt = static_cast<CreateStatement*>(ast.get());
    assert(stmt->table == "Products");
    assert(stmt->columns.size() == 2);
    std::cout << "PASSED\n";
}

void test_parse_create_without_pk_throws() {
    std::cout << "test_parse_create_without_pk_throws... ";
    // Verify that CREATE TABLE without PRIMARY KEY throws an exception
    // This enforces the database schema requirement
    bool threw = false;
    try { parse("CREATE TABLE Bad (id INT, name STRING)"); }
    catch (const std::runtime_error&) { threw = true; }
    assert(threw);
    std::cout << "PASSED\n";
}


// DROP TABLE Statement Parsing Tests
// These tests verify DROP TABLE statement parsing and table name capture.

void test_parse_drop_sets_correct_type() {
    std::cout << "test_parse_drop_sets_correct_type... ";
    // Verify DROP TABLE statement type identification
    auto ast = parse("DROP TABLE Items");
    assert(ast->type == "DROP");
    std::cout << "PASSED\n";
}

void test_parse_drop_captures_table_name() {
    std::cout << "test_parse_drop_captures_table_name... ";
    // Test that DROP TABLE statements capture the target table name
    auto ast = parse("DROP TABLE Items");
    auto* stmt = static_cast<DropStatement*>(ast.get());
    assert(stmt->tableName == "Items");
    std::cout << "PASSED\n";
}


// CREATE ROLE / GRANT Statement Parsing Tests
// These tests verify CREATE ROLE and GRANT statement parsing for
// access control and permission management.

void test_parse_create_role_sets_correct_type() {
    std::cout << "test_parse_create_role_sets_correct_type... ";
    // Verify CREATE ROLE statement type identification
    auto ast = parse("CREATE ROLE analyst WITH SECRET 'pass123'");
    assert(ast->type == "CREATE_ROLE");
    std::cout << "PASSED\n";
}

void test_parse_create_role_captures_name_and_secret() {
    std::cout << "test_parse_create_role_captures_name_and_secret... ";
    // Test that CREATE ROLE statements capture role name and secret key
    auto ast = parse("CREATE ROLE analyst WITH SECRET 'pass123'");
    auto* stmt = static_cast<CreateRoleStatement*>(ast.get());
    assert(stmt->roleName == "analyst");
    assert(stmt->secretKey == "pass123");
    std::cout << "PASSED\n";
}

void test_parse_grant_sets_correct_type() {
    std::cout << "test_parse_grant_sets_correct_type... ";
    // Verify GRANT statement type identification
    auto ast = parse("GRANT SELECT ON Users TO analyst");
    assert(ast->type == "GRANT");
    std::cout << "PASSED\n";
}

void test_parse_grant_captures_privilege_table_role() {
    std::cout << "test_parse_grant_captures_privilege_table_role... ";
    // Test that GRANT statements capture privilege, table, and role information
    auto ast = parse("GRANT INSERT ON Orders TO analyst");
    auto* stmt = static_cast<GrantStatement*>(ast.get());
    assert(stmt->privileges[0] == "INSERT");
    assert(stmt->table == "Orders");
    assert(stmt->role == "analyst");
    std::cout << "PASSED\n";
}


// Error Handling Tests
// These tests verify that the parser properly handles invalid SQL statements
// and throws appropriate exceptions.

void test_parse_unknown_keyword_throws() {
    std::cout << "test_parse_unknown_keyword_throws... ";
    // Verify that unrecognized SQL keywords throw exceptions
    bool threw = false;
    try { parse("DROP ROLE something"); }
    catch (const std::runtime_error&) { threw = true; }
    assert(threw);
    std::cout << "PASSED\n";
}


int main() {
    try {
        // Test SELECT statement parsing
        test_parse_select_sets_correct_type();
        test_parse_select_captures_table_name();
        test_parse_select_captures_multiple_columns();
        test_parse_select_captures_where_condition();
        test_parse_select_captures_order_by();
        test_parse_select_star_is_single_column();
        
        // Test INSERT statement parsing
        test_parse_insert_sets_correct_type();
        test_parse_insert_captures_table_and_values();
        
        // Test UPDATE statement parsing
        test_parse_update_sets_correct_type();
        test_parse_update_captures_column_and_value();
        
        // Test DELETE statement parsing
        test_parse_delete_sets_correct_type();
        test_parse_delete_captures_table_and_condition();
        
        // Test CREATE TABLE statement parsing
        test_parse_create_sets_correct_type();
        test_parse_create_captures_table_name_and_columns();
        test_parse_create_without_pk_throws();
        
        // Test DROP TABLE statement parsing
        test_parse_drop_sets_correct_type();
        test_parse_drop_captures_table_name();
        
        // Test CREATE ROLE and GRANT statement parsing
        test_parse_create_role_sets_correct_type();
        test_parse_create_role_captures_name_and_secret();
        test_parse_grant_sets_correct_type();
        test_parse_grant_captures_privilege_table_role();
        
        // Test error handling
        test_parse_unknown_keyword_throws();
        
        std::cout << "\nALL SQL PARSER TESTS PASSED\n";
    } catch (const std::exception& e) {
        std::cerr << "FAILED: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
