#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include "parser/Tokenizer.h"
#include "parser/SQLParser.h"
#include "query/QueryExecutor.h"
#include "storage/StorageManager.h"
#include "utils/ERDViewer.h"

using namespace spl;

void runQuery(const std::string& query, QueryExecutor& exec, std::string& role) {
    Tokenizer tokenizer(query);
    SQLParser parser(tokenizer);
    try {
        auto ast = parser.parse();
        exec.execute(std::move(ast), role);
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void test_bug8_erd_relationships() {
    std::cout << "--- Testing Bug 8: ERD Relationships ---" << std::endl;
    std::string role = "admin";
    QueryExecutor exec;
    
    StorageManager::dropTable("pokemon");
    StorageManager::dropTable("trainer");
    StorageManager::dropTable("student_course");
    StorageManager::dropTable("student");
    StorageManager::dropTable("course");

    runQuery("CREATE TABLE trainer (tid INT PRIMARY KEY, tname STRING);", exec, role);
    runQuery("CREATE TABLE pokemon (pid INT PRIMARY KEY, pname STRING, trainer_id INT UNIQUE REFERENCES trainer);", exec, role);
    
    // Junction table
    runQuery("CREATE TABLE student (sid INT PRIMARY KEY, sname STRING);", exec, role);
    runQuery("CREATE TABLE course (cid INT PRIMARY KEY, cname STRING);", exec, role);
    runQuery("CREATE TABLE student_course (sid INT REFERENCES student, cid INT REFERENCES course);", exec, role);

    std::cout << "\nExpected Output: \n";
    std::cout << "pokemon -> trainer: 1:1\n";
    std::cout << "student_course -> student: N:N\n";
    std::cout << "student_course -> course: N:N\n";
    std::cout << "\nActual Output:\n";
    ERDViewer::printERD();
}

void test_bug9_scalar_subquery() {
    std::cout << "\n--- Testing Bug 9: Scalar Subqueries ---" << std::endl;
    std::string role = "admin";
    QueryExecutor exec;

    // Data from user request
    // Setup: pokemon row with pname='Pikachu' has atk=85
    // student rows: Johnny(grade=100), Joseph(grade=95), Higashikata(grade=88), Yuji(grade=17)
    
    StorageManager::dropTable("pokemon");
    StorageManager::dropTable("student");
    
    runQuery("CREATE TABLE pokemon (pid INT PRIMARY KEY, pname STRING, atk INT);", exec, role);
    runQuery("CREATE TABLE student (sid INT PRIMARY KEY, sname STRING, grade INT);", exec, role);
    
    runQuery("INSERT INTO pokemon (pid, pname, atk) VALUES (1, 'Pikachu', 85);", exec, role);
    runQuery("INSERT INTO student (sid, sname, grade) VALUES (1, 'Johnny', 100);", exec, role);
    runQuery("INSERT INTO student (sid, sname, grade) VALUES (2, 'Joseph', 95);", exec, role);
    runQuery("INSERT INTO student (sid, sname, grade) VALUES (3, 'Higashikata', 88);", exec, role);
    runQuery("INSERT INTO student (sid, sname, grade) VALUES (4, 'Yuji Itadori', 17);", exec, role);

    std::cout << "\nQuery: SELECT sname FROM student WHERE grade < (SELECT atk FROM pokemon WHERE pname = 'Pikachu');" << std::endl;
    std::cout << "Expected: Yuji Itadori" << std::endl;
    runQuery("SELECT sname FROM student WHERE grade < (SELECT atk FROM pokemon WHERE pname = 'Pikachu');", exec, role);

    runQuery("INSERT INTO student (sid, sname, grade) VALUES (5, 'Red', 85);", exec, role);
    std::cout << "\nQuery: SELECT sname FROM student WHERE grade IN (SELECT atk FROM pokemon);" << std::endl;
    std::cout << "Expected: Red" << std::endl;
    runQuery("SELECT sname FROM student WHERE grade IN (SELECT atk FROM pokemon);", exec, role);
}

int main() {
    StorageManager::saveRole("admin", "RamadanKareem");
    test_bug8_erd_relationships();
    test_bug9_scalar_subquery();
    return 0;
}
