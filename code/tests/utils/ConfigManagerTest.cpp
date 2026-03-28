#include <iostream>
#include <cassert>
#include <fstream>
#include <cstdio>
#include "../../src/utils/ConfigManager.h"

using namespace spl;

static const std::string TEST_ENV = "test_config_tmp.env";

static void write_env(const std::string& content) {
    std::ofstream f(TEST_ENV);
    f << content;
}


// ConfigManager GET Tests
// These tests verify that ConfigManager correctly reads key-value pairs
// from .env files, handles missing keys, whitespace trimming, and comments.

void test_get_returns_value_for_existing_key() {
    std::cout << "test_get_returns_value_for_existing_key... ";
    // Test that ConfigManager retrieves values for existing keys
    write_env("SECRET_PHRASE=mySecret\n");
    ConfigManager cfg(TEST_ENV);
    assert(cfg.get("SECRET_PHRASE") == "mySecret");
    std::cout << "PASSED\n";
}

void test_get_returns_default_when_key_missing() {
    std::cout << "test_get_returns_default_when_key_missing... ";
    // Verify that default values are returned when keys are not found
    write_env("OTHER_KEY=value\n");
    ConfigManager cfg(TEST_ENV);
    assert(cfg.get("MISSING_KEY", "fallback") == "fallback");
    std::cout << "PASSED\n";
}

void test_get_returns_empty_string_when_key_missing_and_no_default() {
    std::cout << "test_get_returns_empty_string_when_key_missing_and_no_default... ";
    // Check that empty string is returned when key is missing and no default provided
    write_env("A=1\n");
    ConfigManager cfg(TEST_ENV);
    assert(cfg.get("NOPE") == "");
    std::cout << "PASSED\n";
}

void test_get_trims_whitespace_around_value() {
    std::cout << "test_get_trims_whitespace_around_value... ";
    // Ensure ConfigManager strips leading and trailing whitespace from values
    write_env("KEY =  trimmed  \n");
    ConfigManager cfg(TEST_ENV);
    assert(cfg.get("KEY") == "trimmed");
    std::cout << "PASSED\n";
}

void test_comment_lines_are_ignored() {
    std::cout << "test_comment_lines_are_ignored... ";
    // Verify that lines starting with # are properly ignored as comments
    write_env("# this is a comment\nREAL=yes\n");
    ConfigManager cfg(TEST_ENV);
    assert(cfg.get("REAL") == "yes");
    assert(cfg.get("# this is a comment") == "");
    std::cout << "PASSED\n";
}

void test_empty_lines_are_ignored() {
    std::cout << "test_empty_lines_are_ignored... ";
    // Check that blank lines don't interfere with key-value parsing
    write_env("\n\nKEY=val\n\n");
    ConfigManager cfg(TEST_ENV);
    assert(cfg.get("KEY") == "val");
    std::cout << "PASSED\n";
}

void test_missing_file_does_not_crash() {
    std::cout << "test_missing_file_does_not_crash... ";
    // Ensure ConfigManager handles missing config files gracefully
    ConfigManager cfg("definitely_does_not_exist_xyz.env");
    assert(cfg.get("ANYTHING", "default") == "default");
    std::cout << "PASSED\n";
}

void test_multiple_keys_are_all_loaded() {
    std::cout << "test_multiple_keys_are_all_loaded... ";
    // Verify that all key-value pairs are loaded when multiple are present
    write_env("A=1\nB=2\nC=3\n");
    ConfigManager cfg(TEST_ENV);
    assert(cfg.get("A") == "1");
    assert(cfg.get("B") == "2");
    assert(cfg.get("C") == "3");
    std::cout << "PASSED\n";
}

int main() {
    try {
        test_get_returns_value_for_existing_key();
        test_get_returns_default_when_key_missing();
        test_get_returns_empty_string_when_key_missing_and_no_default();
        test_get_trims_whitespace_around_value();
        test_comment_lines_are_ignored();
        test_empty_lines_are_ignored();
        test_missing_file_does_not_crash();
        test_multiple_keys_are_all_loaded();
        std::cout << "\nALL CONFIG MANAGER TESTS PASSED\n";
    } catch (const std::exception& e) {
        std::cerr << "FAILED: " << e.what() << "\n";
        std::remove(TEST_ENV.c_str());
        return 1;
    }
    std::remove(TEST_ENV.c_str());
    return 0;
}
