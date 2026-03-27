#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>

class Tokenizer
{
public:
	enum class TokenType
	{
		KEYWORD,
		IDENTIFIER,
		NUMBER,
		STRING,
		OPERATOR,
		PUNCTUATION,
		END,
		INVALID
	};

	Tokenizer(const std::string &input);
	std::string nextToken();
	bool hasNext() const;
	TokenType getLastTokenType() const;

private:
	std::string input;
	size_t position;
	std::string currentToken;
	TokenType currentTokenType;

	bool isWhitespace(char c) const;
	bool isKeyword(const std::string &str) const;
	bool isOperator(char c) const;
	bool isNumber(char c) const;
	bool isIdentifierStart(char c) const;
	bool isIdentifierPart(char c) const;
	bool isStringDelimiter(char c) const;
	bool isPunctuation(char c) const;
};

#endif