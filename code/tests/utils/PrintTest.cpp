#include <iostream>
#include <cassert>
#include <sstream>
#include "../../src/utils/Print.h"

//capture stdout into a string
struct CaptureStdout {
    std::streambuf* old;
    std::ostringstream buf;
    CaptureStdout()  { old = std::cout.rdbuf(buf.rdbuf()); }
    ~CaptureStdout() { std::cout.rdbuf(old); }
    std::string str() const { return buf.str(); }
};

//printHelp

void test_printHelp_contains_exit_command() {
    std::cout << "test_printHelp_contains_exit_command... ";
    CaptureStdout cap;
    printHelp();
    assert(cap.str().find(".exit") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_printHelp_contains_tables_command() {
    std::cout << "test_printHelp_contains_tables_command... ";
    CaptureStdout cap;
    printHelp();
    assert(cap.str().find(".tables") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_printHelp_contains_schema_command() {
    std::cout << "test_printHelp_contains_schema_command... ";
    CaptureStdout cap;
    printHelp();
    assert(cap.str().find(".schema") != std::string::npos);
    std::cout << "PASSED\n";
}

//printPrompt

void test_printPrompt_shows_default_prompt_when_no_role() {
    std::cout << "test_printPrompt_shows_default_prompt_when_no_role... ";
    CaptureStdout cap;
    printPrompt("");
    assert(cap.str().find("featherDb>") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_printPrompt_includes_role_name_when_set() {
    std::cout << "test_printPrompt_includes_role_name_when_set... ";
    CaptureStdout cap;
    printPrompt("analyst");
    assert(cap.str().find("analyst") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_printPrompt_no_role_name_in_output_when_empty() {
    std::cout << "test_printPrompt_no_role_name_in_output_when_empty... ";
    CaptureStdout cap;
    printPrompt();
    // Should not contain parentheses (role display) when no role
    std::string out = cap.str();
    assert(out.find("(") == std::string::npos);
    std::cout << "PASSED\n";
}

//printIntro

void test_printIntro_contains_version_string() {
    std::cout << "test_printIntro_contains_version_string... ";
    CaptureStdout cap;
    char ver[] = "9.9.9-TEST";
    printIntro(ver);
    assert(cap.str().find("9.9.9-TEST") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_printIntro_contains_featherdb_name() {
    std::cout << "test_printIntro_contains_featherdb_name... ";
    CaptureStdout cap;
    char ver[] = "1.0.0";
    printIntro(ver);
    assert(cap.str().find("FeatherDB") != std::string::npos);
    std::cout << "PASSED\n";
}

int main() {
    try {
        test_printHelp_contains_exit_command();
        test_printHelp_contains_tables_command();
        test_printHelp_contains_schema_command();
        test_printPrompt_shows_default_prompt_when_no_role();
        test_printPrompt_includes_role_name_when_set();
        test_printPrompt_no_role_name_in_output_when_empty();
        test_printIntro_contains_version_string();
        test_printIntro_contains_featherdb_name();
        std::cout << "\nALL PRINT TESTS PASSED\n";
    } catch (const std::exception& e) {
        std::cerr << "FAILED: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
