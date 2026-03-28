#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include <sstream>
#include "../src/query/QueryExecutor.h"
#include "../src/storage/StorageManager.h"
#include "../src/parser/Tokenizer.h"
#include "../src/parser/SQLParser.h"

using namespace spl;

static const std::string T = "Repro_";

struct CaptureStdout {
    std::streambuf* old;
    std::ostringstream buf;
    CaptureStdout()  { old = std::cout.rdbuf(buf.rdbuf()); }
    ~CaptureStdout() { std::cout.rdbuf(old); }
    std::string str() const { return buf.str(); }
};

void runQuery(const std::string& query, std::string role = "admin") {
    try {
        Tokenizer tokenizer(query);
        SQLParser parser(tokenizer);
        std::unique_ptr<AST> ast = parser.parse();
        
        QueryExecutor executor;
        executor.execute(std::move(ast), role);
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
}


// BUG REPRODUCTION Tests
// These tests reproduce known issues in the database system including:
// - Trailing garbage handling
// - Semicolon enforcement
// - Multi-word string literals
// - Math expression evaluation
// - Unique constraint enforcement
// - Type checking in UPDATE statements
// - Referential integrity validation
// - Foreign key position handling

void test_bug1_trailing_garbage() {
    std::cout << "test_bug1_trailing_garbage... ";
    // BUG 1: Parser accepts trailing garbage text after valid SQL statements
    // Expected: Reject statements with unparsed tokens remaining
    StorageManager::dropTable(T + "B1");
    // This should FAIL but currently succeeds (ignores trailing garbage)
    std::string output;
    {
        CaptureStdout cap;
        runQuery("CREATE TABLE " + T + "B1 (id INT PRIMARY KEY); random garbage");
        output = cap.str();
    }
    // If it didn't throw an error, it's currently buggy (or we haven't enforced it yet)
    if (output.find("Error") != std::string::npos) { // Error means it rejected, which is good
        std::cout << "PASSED (Rejected trailing garbage)\n";
    } else {
        std::cout << "CONFIRMED (Accepted trailing garbage)\n";
    }
    StorageManager::dropTable(T + "B1");
}

void test_bug2_semicolon_enforcement() {
    std::cout << "test_bug2_semicolon_enforcement... ";
    // BUG 2: Parser may accept statements without requiring semicolons
    // Expected: Enforce semicolon as statement terminator
    StorageManager::dropTable(T + "B2");
    std::string output;
    {
        CaptureStdout cap;
        runQuery("CREATE TABLE " + T + "B2 (id INT PRIMARY KEY)"); // Missing semicolon
        output = cap.str();
    }
    if (output.find("Error") != std::string::npos) { // Error means it rejected, which is good
        std::cout << "PASSED (Rejected missing semicolon)\n";
    } else {
        std::cout << "CONFIRMED (Accepted missing semicolon)\n";
    }
    StorageManager::dropTable(T + "B2");
}

void test_bug3_multiword_string() {
    std::cout << "test_bug3_multiword_string... ";
    // BUG 3: String matching may fail for multi-word values
    // Expected: WHERE clause correctly matches strings with spaces
    StorageManager::dropTable(T + "B3");
    runQuery("CREATE TABLE " + T + "B3 (id INT PRIMARY KEY, name STRING);");
    runQuery("INSERT INTO " + T + "B3 (id, name) VALUES (1, 'Alice two words');");
    
    std::string output;
    {
        CaptureStdout cap;
        runQuery("SELECT * FROM " + T + "B3 WHERE name = 'Alice two words';");
        output = cap.str();
    }
    if (output.find("Alice two words") != std::string::npos) {
        std::cout << "PASSED (Matched multi-word string)\n";
    } else {
        std::cout << "CONFIRMED (Failed to match multi-word string: " << output << ")\n";
    }
    StorageManager::dropTable(T + "B3");
}

void test_bug3_math() {
    std::cout << "test_bug3_math... ";
    // BUG 3 (Extended): Math expressions in WHERE clause may not be evaluated
    // Expected: Expression like "id = 3-2" evaluates to "id = 1"
    StorageManager::dropTable(T + "B3Math");
    runQuery("CREATE TABLE " + T + "B3Math (id INT PRIMARY KEY);");
    runQuery("INSERT INTO " + T + "B3Math (id) VALUES (1);");
    runQuery("INSERT INTO " + T + "B3Math (id) VALUES (2);");
    
    std::string output;
    {
        CaptureStdout cap;
        runQuery("SELECT * FROM " + T + "B3Math WHERE id = 3-2;");
        output = cap.str();
    }
    if (output.find("1") != std::string::npos && output.find("id") != std::string::npos) {
        std::cout << "PASSED (Evaluated math in WHERE)\n";
    } else {
        std::cout << "CONFIRMED (Failed to evaluate math in WHERE: " << output << ")\n";
    }
    StorageManager::dropTable(T + "B3Math");
}

void test_bug4_unique_constraint() {
    std::cout << "test_bug4_unique_constraint... ";
    // BUG 4: UNIQUE constraint on non-primary key columns may not be enforced
    // Expected: Duplicate UNIQUE values should be rejected
    StorageManager::dropTable(T + "B4");
    runQuery("CREATE TABLE " + T + "B4 (id INT UNIQUE, name STRING PRIMARY KEY);");
    runQuery("INSERT INTO " + T + "B4 (id, name) VALUES (1, 'Alice');");
    
    std::string output;
    {
        CaptureStdout cap;
        runQuery("INSERT INTO " + T + "B4 (id, name) VALUES (1, 'Bob');");
        output = cap.str();
    }
    if (output.find("Unique constraint violation") != std::string::npos) {
        std::cout << "PASSED (Enforced UNIQUE constraint)\n";
    } else {
        std::cout << "CONFIRMED (Failed to enforce UNIQUE constraint: " << output << ")\n";
    }
    StorageManager::dropTable(T + "B4");
}

void test_bug5_update_type_checking() {
    std::cout << "test_bug5_update_type_checking... ";
    // BUG 5: UPDATE statements may not validate column types
    // Expected: Assigning string to INT column should fail
    StorageManager::dropTable(T + "B5");
    runQuery("CREATE TABLE " + T + "B5 (id INT PRIMARY KEY);");
    runQuery("INSERT INTO " + T + "B5 (id) VALUES (1);");
    
    std::string output;
    {
        CaptureStdout cap;
        runQuery("UPDATE " + T + "B5 SET id = 'some_string' WHERE id = 1;");
        output = cap.str();
    }
    if (output.find("Error") != std::string::npos) { // Error means it rejected, which is good
        std::cout << "PASSED (Rejected type mismatch in UPDATE)\n";
    } else {
        std::cout << "CONFIRMED (Accepted string for INT column in UPDATE)\n";
    }
    StorageManager::dropTable(T + "B5");
}

void test_bug6_referential_integrity() {
    std::cout << "test_bug6_referential_integrity... ";
    // BUG 6: Foreign key constraints may not prevent parent row deletion
    // when child rows still reference it
    // Expected: DELETE on parent with active children should fail
    StorageManager::dropTable(T + "Child");
    StorageManager::dropTable(T + "Parent");
    runQuery("CREATE TABLE " + T + "Parent (id INT PRIMARY KEY);");
    runQuery("CREATE TABLE " + T + "Child (cid INT PRIMARY KEY, pid INT REFERENCES " + T + "Parent);");
    runQuery("INSERT INTO " + T + "Parent (id) VALUES (1);");
    runQuery("INSERT INTO " + T + "Child (cid, pid) VALUES (10, 1);");
    
    std::string output;
    {
        CaptureStdout cap;
        runQuery("DELETE FROM " + T + "Parent WHERE id = 1;");
        output = cap.str();
    }
    if (output.find("Error") != std::string::npos) { // Error means it rejected, which is good
        std::cout << "PASSED (Prevented parent deletion with active children)\n";
    } else {
        std::cout << "CONFIRMED (Deleted parent row with active children)\n";
    }
    StorageManager::dropTable(T + "Child");
    StorageManager::dropTable(T + "Parent");
}

void test_bug7_fk_position() {
    std::cout << "test_bug7_fk_position... ";
    // BUG 7: Foreign key validation may only work when parent PK is in first column
    // Expected: FK references should work regardless of PK column position
    StorageManager::dropTable(T + "ChildPos");
    StorageManager::dropTable(T + "ParentPos");
    // Parent PK in SECOND column
    runQuery("CREATE TABLE " + T + "ParentPos (name STRING, id INT PRIMARY KEY);");
    // FK in second position of child
    runQuery("CREATE TABLE " + T + "ChildPos (cname STRING PRIMARY KEY, pid INT REFERENCES " + T + "ParentPos);");
    runQuery("INSERT INTO " + T + "ParentPos (name, id) VALUES ('Alice', 1);");
    
    std::string output;
    {
        CaptureStdout cap;
        runQuery("INSERT INTO " + T + "ChildPos (cname, pid) VALUES ('test', 1);");
        output = cap.str();
    }
    if (output.find("Error") != std::string::npos) {
        std::cout << "CONFIRMED (Failed FK check when parent PK not in first position: " << output << ")\n";
    } else {
        std::cout << "PASSED (FK check succeeded for non-first position)\n";
    }
    StorageManager::dropTable(T + "ChildPos");
    StorageManager::dropTable(T + "ParentPos");
}


int main() {
    test_bug1_trailing_garbage();
    test_bug2_semicolon_enforcement();
    test_bug3_multiword_string();
    test_bug3_math();
    test_bug4_unique_constraint();
    test_bug5_update_type_checking();
    test_bug6_referential_integrity();
    test_bug7_fk_position();
    return 0;
}
