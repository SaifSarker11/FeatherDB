#include <iostream>
#include <cassert>
#include <sstream>
#include "../../src/utils/Print.h"

// Helper to capture stdout for testing output
struct CaptureStdout {
    std::streambuf* old;
    std::ostringstream buf;
    CaptureStdout()  { old = std::cout.rdbuf(buf.rdbuf()); }
    ~CaptureStdout() { std::cout.rdbuf(old); }
    std::string str() const { return buf.str(); }
};


// PRINTHELP Function Tests
// These tests verify that printHelp() displays correct command documentation
// including all major commands (.exit, .tables, .schema, etc).

void test_printHelp_contains_exit_command() {
    std::cout << "test_printHelp_contains_exit_command... ";
    // Verify that help text mentions the .exit command
    CaptureStdout cap;
    printHelp();
    assert(cap.str().find(".exit") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_printHelp_contains_tables_command() {
    std::cout << "test_printHelp_contains_tables_command... ";
    // Check that help text includes the .tables command
    CaptureStdout cap;
    printHelp();
    assert(cap.str().find(".tables") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_printHelp_contains_schema_command() {
    std::cout << "test_printHelp_contains_schema_command... ";
    // Ensure help text describes the .schema command
    CaptureStdout cap;
    printHelp();
    assert(cap.str().find(".schema") != std::string::npos);
    std::cout << "PASSED\n";
}


// PRINTPROMPT Function Tests
// These tests verify that printPrompt() displays appropriate command prompts,
// with and without role information.

void test_printPrompt_shows_default_prompt_when_no_role() {
    std::cout << "test_printPrompt_shows_default_prompt_when_no_role... ";
    // Test that default prompt is shown when no role is set
    CaptureStdout cap;
    printPrompt("");
    assert(cap.str().find("featherDb>") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_printPrompt_includes_role_name_when_set() {
    std::cout << "test_printPrompt_includes_role_name_when_set... ";
    // Verify that role name appears in prompt when provided
    CaptureStdout cap;
    printPrompt("analyst");
    assert(cap.str().find("analyst") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_printPrompt_no_role_name_in_output_when_empty() {
    std::cout << "test_printPrompt_no_role_name_in_output_when_empty... ";
    // Ensure no role indicators appear in prompt when role is empty
    CaptureStdout cap;
    printPrompt();
    // Should not contain parentheses (role display) when no role
    std::string out = cap.str();
    assert(out.find("(") == std::string::npos);
    std::cout << "PASSED\n";
}


// PRINTINTRO Function Tests
// These tests verify that printIntro() displays the application introduction
// with version information and application name.

void test_printIntro_contains_version_string() {
    std::cout << "test_printIntro_contains_version_string... ";
    // Verify that application intro displays the version string
    CaptureStdout cap;
    char ver[] = "9.9.9-TEST";
    printIntro(ver);
    assert(cap.str().find("9.9.9-TEST") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_printIntro_contains_featherdb_name() {
    std::cout << "test_printIntro_contains_featherdb_name... ";
    // Ensure intro text includes the application name
    CaptureStdout cap;
    char ver[] = "1.0.0";
    printIntro(ver);
    assert(cap.str().find("FeatherDB") != std::string::npos);
    std::cout << "PASSED\n";
}

int main() {
    try {
        // Test help command output
        test_printHelp_contains_exit_command();
        test_printHelp_contains_tables_command();
        test_printHelp_contains_schema_command();
        
        // Test prompt display with and without role
        test_printPrompt_shows_default_prompt_when_no_role();
        test_printPrompt_includes_role_name_when_set();
        test_printPrompt_no_role_name_in_output_when_empty();
        
        // Test introduction output
        test_printIntro_contains_version_string();
        test_printIntro_contains_featherdb_name();
        
        std::cout << "\nALL PRINT TESTS PASSED\n";
    } catch (const std::exception& e) {
        std::cerr << "FAILED: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
