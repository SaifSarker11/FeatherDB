#ifndef SQLPARSER_H
#define SQLPARSER_H

#include "Tokenizer.h"
#include "AST.h"
#include <memory>

class SQLParser
{
public:
	SQLParser(Tokenizer &tokenizer);
	std::unique_ptr<AST> parse();

private:
	Tokenizer &tokenizer;
	std::string currentToken;
	Tokenizer::TokenType currentType;

	void advance();
	void expect(const std::string &value);
	void expect(Tokenizer::TokenType type);

	std::unique_ptr<AST> parseSelect();
	std::unique_ptr<AST> parseInsert();
	std::unique_ptr<AST> parseUpdate();
	std::unique_ptr<AST> parseDelete();
	std::unique_ptr<AST> parseCreate();
	std::unique_ptr<AST> parseDrop();
	std::unique_ptr<AST> parseGrant();
	std::unique_ptr<AST> parseCreateRole();
	std::vector<std::string> parseIdentifierList();
};

#endif