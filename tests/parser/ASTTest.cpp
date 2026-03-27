#include <iostream>
#include <cassert>
#include <memory>
#include "../../src/parser/AST.h"

//SelectStatement

void test_select_toString_contains_table_and_columns() {
    std::cout << "test_select_toString_contains_table_and_columns... ";
    SelectStatement s({"id", "name"}, "Users", "");
    std::string str = s.toString();
    assert(str.find("Users") != std::string::npos);
    assert(str.find("id") != std::string::npos);
    assert(str.find("name") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_select_toString_includes_where_when_condition_set() {
    std::cout << "test_select_toString_includes_where_when_condition_set... ";
    SelectStatement s({"id"}, "Users", "id = 1");
    std::string str = s.toString();
    assert(str.find("WHERE") != std::string::npos);
    assert(str.find("id = 1") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_select_toString_includes_order_by_when_set() {
    std::cout << "test_select_toString_includes_order_by_when_set... ";
    SelectStatement s({"id"}, "Users", "", nullptr, "name");
    std::string str = s.toString();
    assert(str.find("ORDER BY") != std::string::npos);
    assert(str.find("name") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_select_type_is_SELECT() {
    std::cout << "test_select_type_is_SELECT... ";
    SelectStatement s({"*"}, "T", "");
    assert(s.type == "SELECT");
    std::cout << "PASSED\n";
}

//InsertStatement

void test_insert_toString_contains_table_and_values() {
    std::cout << "test_insert_toString_contains_table_and_values... ";
    InsertStatement ins("Orders", {"id", "total"}, {"1", "99"});
    std::string str = ins.toString();
    assert(str.find("Orders") != std::string::npos);
    assert(str.find("99") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_insert_type_is_INSERT() {
    std::cout << "test_insert_type_is_INSERT... ";
    InsertStatement ins("T", {"id"}, {"1"});
    assert(ins.type == "INSERT");
    std::cout << "PASSED\n";
}

//UpdateStatement

void test_update_toString_contains_set_clause() {
    std::cout << "test_update_toString_contains_set_clause... ";
    UpdateStatement upd("Users", "name", "Bob", "id = 1");
    std::string str = upd.toString();
    assert(str.find("SET") != std::string::npos);
    assert(str.find("name") != std::string::npos);
    assert(str.find("Bob") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_update_type_is_UPDATE() {
    std::cout << "test_update_type_is_UPDATE... ";
    UpdateStatement upd("T", "col", "val", "");
    assert(upd.type == "UPDATE");
    std::cout << "PASSED\n";
}

//DeleteStatement

void test_delete_toString_contains_table() {
    std::cout << "test_delete_toString_contains_table... ";
    DeleteStatement del("Orders", "id = 5");
    std::string str = del.toString();
    assert(str.find("Orders") != std::string::npos);
    assert(str.find("WHERE") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_delete_type_is_DELETE() {
    std::cout << "test_delete_type_is_DELETE... ";
    DeleteStatement del("T", "");
    assert(del.type == "DELETE");
    std::cout << "PASSED\n";
}

//CreateStatement

void test_create_toString_contains_table_and_columns() {
    std::cout << "test_create_toString_contains_table_and_columns... ";
    CreateStatement cr("Products", {{"id", "INT PK"}, {"price", "INT"}});
    std::string str = cr.toString();
    assert(str.find("Products") != std::string::npos);
    assert(str.find("id") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_create_type_is_CREATE() {
    std::cout << "test_create_type_is_CREATE... ";
    CreateStatement cr("T", {{"id", "INT PK"}});
    assert(cr.type == "CREATE");
    std::cout << "PASSED\n";
}

//CreateRoleStatement

void test_create_role_type_is_CREATE_ROLE() {
    std::cout << "test_create_role_type_is_CREATE_ROLE... ";
    CreateRoleStatement cr("analyst", "secret");
    assert(cr.type == "CREATE_ROLE");
    std::cout << "PASSED\n";
}

void test_create_role_toString_contains_role_and_secret() {
    std::cout << "test_create_role_toString_contains_role_and_secret... ";
    CreateRoleStatement cr("analyst", "mySecret");
    std::string str = cr.toString();
    assert(str.find("analyst") != std::string::npos);
    assert(str.find("mySecret") != std::string::npos);
    std::cout << "PASSED\n";
}

//GrantStatement

void test_grant_type_is_GRANT() {
    std::cout << "test_grant_type_is_GRANT... ";
    GrantStatement g({"SELECT"}, "Users", "analyst");
    assert(g.type == "GRANT");
    std::cout << "PASSED\n";
}

void test_grant_toString_contains_privilege_table_role() {
    std::cout << "test_grant_toString_contains_privilege_table_role... ";
    GrantStatement g({"INSERT"}, "Orders", "analyst");
    std::string str = g.toString();
    assert(str.find("INSERT") != std::string::npos);
    assert(str.find("Orders") != std::string::npos);
    assert(str.find("analyst") != std::string::npos);
    std::cout << "PASSED\n";
}

//DropStatement

void test_drop_type_is_DROP() {
    std::cout << "test_drop_type_is_DROP... ";
    DropStatement d("Items");
    assert(d.type == "DROP");
    std::cout << "PASSED\n";
}

void test_drop_toString_contains_table_name() {
    std::cout << "test_drop_toString_contains_table_name... ";
    DropStatement d("Items");
    assert(d.toString().find("Items") != std::string::npos);
    std::cout << "PASSED\n";
}

int main() {
    try {
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
