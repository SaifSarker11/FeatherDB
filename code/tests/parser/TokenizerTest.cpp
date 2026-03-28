#include <iostream>
#include <cassert>
#include <stdexcept>
#include "../../src/parser/Tokenizer.h"

// Tokenizer Basic Functionality Tests
// These tests verify that the Tokenizer correctly identifies and classifies
// different token types including keywords, identifiers, operators, numbers,
// strings, and punctuation. Tests also cover whitespace handling and the
// token stream traversal.

void test_keywords_are_normalized_to_uppercase() {
    std::cout << "test_keywords_are_normalized_to_uppercase... ";
    Tokenizer t("select from where");
    assert(t.nextToken() == "SELECT");
    assert(t.nextToken() == "FROM");
    assert(t.nextToken() == "WHERE");
    std::cout << "PASSED\n";
}

void test_keyword_type_is_reported_correctly() {
    std::cout << "test_keyword_type_is_reported_correctly... ";
    Tokenizer t("INSERT");
    t.nextToken();
    assert(t.getLastTokenType() == Tokenizer::TokenType::KEYWORD);
    std::cout << "PASSED\n";
}

void test_identifier_is_not_classified_as_keyword() {
    std::cout << "test_identifier_is_not_classified_as_keyword... ";
    Tokenizer t("myTable");
    t.nextToken();
    assert(t.getLastTokenType() == Tokenizer::TokenType::IDENTIFIER);
    std::cout << "PASSED\n";
}

void test_integer_literal_is_classified_as_number() {
    std::cout << "test_integer_literal_is_classified_as_number... ";
    Tokenizer t("42");
    assert(t.nextToken() == "42");
    assert(t.getLastTokenType() == Tokenizer::TokenType::NUMBER);
    std::cout << "PASSED\n";
}

void test_string_literal_strips_quotes() {
    std::cout << "test_string_literal_strips_quotes... ";
    Tokenizer t("'hello'");
    assert(t.nextToken() == "hello");
    assert(t.getLastTokenType() == Tokenizer::TokenType::STRING);
    std::cout << "PASSED\n";
}

void test_operator_is_classified_correctly() {
    std::cout << "test_operator_is_classified_correctly... ";
    Tokenizer t("=");
    t.nextToken();
    assert(t.getLastTokenType() == Tokenizer::TokenType::OPERATOR);
    std::cout << "PASSED\n";
}

void test_punctuation_comma_is_classified_correctly() {
    std::cout << "test_punctuation_comma_is_classified_correctly... ";
    Tokenizer t(",");
    t.nextToken();
    assert(t.getLastTokenType() == Tokenizer::TokenType::PUNCTUATION);
    std::cout << "PASSED\n";
}

void test_whitespace_is_skipped_between_tokens() {
    std::cout << "test_whitespace_is_skipped_between_tokens... ";
    Tokenizer t("  SELECT   *  ");
    assert(t.nextToken() == "SELECT");
    assert(t.nextToken() == "*");
    std::cout << "PASSED\n";
}

void test_hasNext_returns_false_when_exhausted() {
    std::cout << "test_hasNext_returns_false_when_exhausted... ";
    Tokenizer t("A");
    t.nextToken();
    assert(!t.hasNext());
    std::cout << "PASSED\n";
}

void test_hasNext_returns_true_when_tokens_remain() {
    std::cout << "test_hasNext_returns_true_when_tokens_remain... ";
    Tokenizer t("A B");
    t.nextToken();
    assert(t.hasNext());
    std::cout << "PASSED\n";
}

void test_end_token_type_when_input_exhausted() {
    std::cout << "test_end_token_type_when_input_exhausted... ";
    Tokenizer t("X");
    t.nextToken(); // consume X
    t.nextToken(); // should return END
    assert(t.getLastTokenType() == Tokenizer::TokenType::END);
    std::cout << "PASSED\n";
}

void test_pk_fk_keywords_are_recognized() {
    std::cout << "test_pk_fk_keywords_are_recognized... ";
    Tokenizer t("PRIMARY KEY REFERENCES");
    assert(t.nextToken() == "PRIMARY");
    assert(t.getLastTokenType() == Tokenizer::TokenType::KEYWORD);
    assert(t.nextToken() == "KEY");
    assert(t.getLastTokenType() == Tokenizer::TokenType::KEYWORD);
    assert(t.nextToken() == "REFERENCES");
    assert(t.getLastTokenType() == Tokenizer::TokenType::KEYWORD);
    std::cout << "PASSED\n";
}

void test_unterminated_string_throws_exception() {
    std::cout << "test_unterminated_string_throws_exception... ";
    Tokenizer t("'unterminated");
    bool threw = false;
    try { t.nextToken(); } catch (const std::invalid_argument&) { threw = true; }
    assert(threw);
    std::cout << "PASSED\n";
}

void test_invalid_character_throws_exception() {
    std::cout << "test_invalid_character_throws_exception... ";
    // Characters that are not valid SQL should throw an exception
    Tokenizer t("@");
    bool threw = false;
    try { t.nextToken(); } catch (const std::invalid_argument&) { threw = true; }
    assert(threw);
    std::cout << "PASSED\n";
}


// Integration Tests
// These tests verify that the tokenizer correctly handles complete SQL
// statements with mixed token types.

void test_full_select_statement_tokenizes_correctly() {
    std::cout << "test_full_select_statement_tokenizes_correctly... ";
    // Test that a complete SELECT statement is tokenized with correct order
    Tokenizer t("SELECT id FROM Users WHERE id = 1");
    assert(t.nextToken() == "SELECT");
    assert(t.nextToken() == "id");
    assert(t.nextToken() == "FROM");
    assert(t.nextToken() == "Users");
    assert(t.nextToken() == "WHERE");
    assert(t.nextToken() == "id");
    assert(t.nextToken() == "=");
    assert(t.nextToken() == "1");
    std::cout << "PASSED\n";
}


int main() {
    try {
        // Test keyword normalization and token type classification
        test_keywords_are_normalized_to_uppercase();
        test_keyword_type_is_reported_correctly();
        test_identifier_is_not_classified_as_keyword();
        
        // Test numeric and string literal handling
        test_integer_literal_is_classified_as_number();
        test_string_literal_strips_quotes();
        
        // Test operator and punctuation classification
        test_operator_is_classified_correctly();
        test_punctuation_comma_is_classified_correctly();
        
        // Test whitespace handling and token navigation
        test_whitespace_is_skipped_between_tokens();
        test_hasNext_returns_false_when_exhausted();
        test_hasNext_returns_true_when_tokens_remain();
        test_end_token_type_when_input_exhausted();
        
        // Test schema-related keywords
        test_pk_fk_keywords_are_recognized();
        
        // Test error handling
        test_unterminated_string_throws_exception();
        test_invalid_character_throws_exception();
        
        // Test integration with complete SQL statements
        test_full_select_statement_tokenizes_correctly();
        
        std::cout << "\nALL TOKENIZER TESTS PASSED\n";
    } catch (const std::exception& e) {
        std::cerr << "FAILED: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
