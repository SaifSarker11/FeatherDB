#include <iostream>
#include <cassert>
#include <memory>
#include "../../src/parser/AST.h"


// SelectStatement Tests
// These tests verify that the SelectStatement AST node properly stores and
// represents SELECT query components including columns, tables, WHERE conditions,
// and ORDER BY clauses. Each test ensures the toString() method produces correct
// SQL representation and that statement types are set correctly.

void test_select_toString_contains_table_and_columns() {
    std::cout << "test_select_toString_contains_table_and_columns... ";
    // Create a SELECT statement for the Users table with id and name columns
    SelectStatement s({"id", "name"}, "Users", "");
    std::string str = s.toString();
    
    // Verify that the SQL string contains the table name and both column names
    assert(str.find("Users") != std::string::npos);
    assert(str.find("id") != std::string::npos);
    assert(str.find("name") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_select_toString_includes_where_when_condition_set() {
    std::cout << "test_select_toString_includes_where_when_condition_set... ";
    // Create a SELECT statement with a WHERE condition
    SelectStatement s({"id"}, "Users", "id = 1");
    std::string str = s.toString();
    
    // Verify that the WHERE clause and condition appear in the SQL output
    assert(str.find("WHERE") != std::string::npos);
    assert(str.find("id = 1") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_select_toString_includes_order_by_when_set() {
    std::cout << "test_select_toString_includes_order_by_when_set... ";
    // Test that ORDER BY clause is included when specified (sorted by name column)
    SelectStatement s({"id"}, "Users", "", nullptr, "name");
    std::string str = s.toString();
    
    // Validate that ORDER BY keyword and column name are present in output
    assert(str.find("ORDER BY") != std::string::npos);
    assert(str.find("name") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_select_type_is_SELECT() {
    std::cout << "test_select_type_is_SELECT... ";
    // Verify that the statement type identifier is correctly set
    SelectStatement s({"*"}, "T", "");
    assert(s.type == "SELECT");
    std::cout << "PASSED\n";
}


// InsertStatement Tests
// These tests verify that the InsertStatement AST node correctly captures
// table names, column names, and values. Tests ensure toString() produces
// valid INSERT statement syntax and type identifiers are set appropriately.

void test_insert_toString_contains_table_and_values() {
    std::cout << "test_insert_toString_contains_table_and_values... ";
    // Create an INSERT statement for Orders table with id and total columns
    InsertStatement ins("Orders", {"id", "total"}, {"1", "99"});
    std::string str = ins.toString();
    
    // Check that table name and at least one value appear in the generated SQL
    assert(str.find("Orders") != std::string::npos);
    assert(str.find("99") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_insert_type_is_INSERT() {
    std::cout << "test_insert_type_is_INSERT... ";
    // Verify the INSERT statement type is correctly identified
    InsertStatement ins("T", {"id"}, {"1"});
    assert(ins.type == "INSERT");
    std::cout << "PASSED\n";
}


// UpdateStatement Tests
// These tests ensure that UpdateStatement nodes properly store UPDATE operation
// details including the target table, column to modify, new value, and WHERE
// conditions. Verifies correct SQL generation for the UPDATE command.

void test_update_toString_contains_set_clause() {
    std::cout << "test_update_toString_contains_set_clause... ";
    // Create an UPDATE statement that sets name to Bob for a specific user
    UpdateStatement upd("Users", "name", "Bob", "id = 1");
    std::string str = upd.toString();
    
    // Ensure the SET clause, column name, and new value are all present
    assert(str.find("SET") != std::string::npos);
    assert(str.find("name") != std::string::npos);
    assert(str.find("Bob") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_update_type_is_UPDATE() {
    std::cout << "test_update_type_is_UPDATE... ";
    // Confirm the statement type is correctly assigned as UPDATE
    UpdateStatement upd("T", "col", "val", "");
    assert(upd.type == "UPDATE");
    std::cout << "PASSED\n";
}


// DeleteStatement Tests
// These tests verify DeleteStatement functionality, ensuring that DELETE
// operations capture the target table and WHERE conditions correctly, and
// that SQL output is properly formatted.

void test_delete_toString_contains_table() {
    std::cout << "test_delete_toString_contains_table... ";
    // Create a DELETE statement targeting Orders table with a condition
    DeleteStatement del("Orders", "id = 5");
    std::string str = del.toString();
    
    // Verify table name and WHERE clause are included in the SQL output
    assert(str.find("Orders") != std::string::npos);
    assert(str.find("WHERE") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_delete_type_is_DELETE() {
    std::cout << "test_delete_type_is_DELETE... ";
    // Verify that DELETE operations have the correct type identifier
    DeleteStatement del("T", "");
    assert(del.type == "DELETE");
    std::cout << "PASSED\n";
}


// CreateStatement Tests
// These tests validate CreateStatement handling of table creation commands,
// including table names, column definitions, and data types. Ensures proper
// parsing and representation of CREATE TABLE statements.

void test_create_toString_contains_table_and_columns() {
    std::cout << "test_create_toString_contains_table_and_columns... ";
    // Create a CREATE TABLE statement with column definitions and types
    CreateStatement cr("Products", {{"id", "INT PK"}, {"price", "INT"}});
    std::string str = cr.toString();
    
    // Ensure table name and column names appear in the generated CREATE TABLE statement
    assert(str.find("Products") != std::string::npos);
    assert(str.find("id") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_create_type_is_CREATE() {
    std::cout << "test_create_type_is_CREATE... ";
    // Confirm the statement type is correctly identified as CREATE
    CreateStatement cr("T", {{"id", "INT PK"}});
    assert(cr.type == "CREATE");
    std::cout << "PASSED\n";
}


// CreateRoleStatement Tests
// These tests ensure that CreateRoleStatement correctly stores role names and
// secret keys for database access control, and properly formats them in
// the SQL output.

void test_create_role_type_is_CREATE_ROLE() {
    std::cout << "test_create_role_type_is_CREATE_ROLE... ";
    // Verify that role creation statements have the correct type identifier
    CreateRoleStatement cr("analyst", "secret");
    assert(cr.type == "CREATE_ROLE");
    std::cout << "PASSED\n";
}

void test_create_role_toString_contains_role_and_secret() {
    std::cout << "test_create_role_toString_contains_role_and_secret... ";
    // Create a role with a secret key for authentication
    CreateRoleStatement cr("analyst", "mySecret");
    std::string str = cr.toString();
    
    // Ensure role name and secret appear in the SQL output
    assert(str.find("analyst") != std::string::npos);
    assert(str.find("mySecret") != std::string::npos);
    std::cout << "PASSED\n";
}

//GrantStatement

void test_grant_type_is_GRANT() {
    std::cout << "test_grant_type_is_GRANT... ";
    // Check that GRANT statement type is properly identified
    GrantStatement g({"SELECT"}, "Users", "analyst");
    assert(g.type == "GRANT");
    std::cout << "PASSED\n";
}

void test_grant_toString_contains_privilege_table_role() {
    std::cout << "test_grant_toString_contains_privilege_table_role... ";
    // Create a GRANT statement to give INSERT privileges on Orders table to analyst role
    GrantStatement g({"INSERT"}, "Orders", "analyst");
    std::string str = g.toString();
    
    // Verify that privilege, table, and role are all included in the output
    assert(str.find("INSERT") != std::string::npos);
    assert(str.find("Orders") != std::string::npos);
    assert(str.find("analyst") != std::string::npos);
    std::cout << "PASSED\n";
}

//DropStatement

void test_drop_type_is_DROP() {
    std::cout << "test_drop_type_is_DROP... ";
    // Confirm the DROP statement type identifier is correctly set
    DropStatement d("Items");
    assert(d.type == "DROP");
    std::cout << "PASSED\n";
}

void test_drop_toString_contains_table_name() {
    std::cout << "test_drop_toString_contains_table_name... ";
    // Verify that DROP statement includes the table name to be dropped
    DropStatement d("Items");
    assert(d.toString().find("Items") != std::string::npos);
    std::cout << "PASSED\n";
}

int main() {
    try {
        // Run all AST node tests
        test_select_toString_contains_table_and_columns();
        test_select_toString_includes_where_when_condition_set();
        test_select_toString_includes_order_by_when_set();
        test_select_type_is_SELECT();
        
        test_insert_toString_contains_table_and_values();
        test_insert_type_is_INSERT();
        
        test_update_toString_contains_set_clause();
        test_update_type_is_UPDATE();
        
        test_delete_toString_contains_table();
        test_delete_type_is_DELETE();
        
        test_create_toString_contains_table_and_columns();
        test_create_type_is_CREATE();
        
        test_create_role_type_is_CREATE_ROLE();
        test_create_role_toString_contains_role_and_secret();
        
        test_grant_type_is_GRANT();
        test_grant_toString_contains_privilege_table_role();
        
        test_drop_type_is_DROP();
        test_drop_toString_contains_table_name();
        
        std::cout << "\nALL AST TESTS PASSED\n";
    } catch (const std::exception& e) {
        std::cerr << "FAILED: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
