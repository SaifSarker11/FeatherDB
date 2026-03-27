#include "SQLParser.h"


SQLParser::SQLParser(Tokenizer &tokenizer) : tokenizer(tokenizer)
{
	advance();
}

void SQLParser::advance()
{
	if (tokenizer.hasNext())
	{
		currentToken = tokenizer.nextToken();
		currentType = tokenizer.getLastTokenType();
	}
	else
	{
		currentToken.clear();
		currentType = Tokenizer::TokenType::END;
	}
}

void SQLParser::expect(const std::string &value)
{
	if (currentToken != value)
	{
		throw std::runtime_error("Expected '" + value + "', got '" + currentToken + "'");
	}
	advance();
}

void SQLParser::expect(Tokenizer::TokenType type)
{
	if (currentType != type)
	{
		throw std::runtime_error("Unexpected token type");
	}
	advance();
}

std::unique_ptr<AST> SQLParser::parse()
{
	if (currentType != Tokenizer::TokenType::KEYWORD)
	{
		throw std::runtime_error("Expected SQL keyword");
	}
	if (currentToken == "SELECT")
		return parseSelect();
	if (currentToken == "INSERT")
		return parseInsert();
	if (currentToken == "UPDATE")
		return parseUpdate();
	if (currentToken == "DELETE")
		return parseDelete();
	else if (currentToken == "CREATE") {
		return parseCreate();
	} else if (currentToken == "GRANT") {
		return parseGrant();
	} else if (currentToken == "DROP") {
		return parseDrop();
	}
	throw std::runtime_error("Unknown SQL command");
}

std::unique_ptr<AST> SQLParser::parseCreate()
{
	advance(); // CREATE
	if (currentToken == "ROLE")
	{
		return parseCreateRole();
	}
	expect("TABLE");
	std::string table = currentToken;
	expect(Tokenizer::TokenType::IDENTIFIER);
	expect("(");

	std::vector<std::pair<std::string, std::string>> columns;
    bool hasKey = false;
	while (currentToken != ")" && currentToken != ";")
	{
		std::string name = currentToken;
		expect(Tokenizer::TokenType::IDENTIFIER);
		std::string type = currentToken;
		if (currentType != Tokenizer::TokenType::IDENTIFIER && currentType != Tokenizer::TokenType::KEYWORD) {
             throw std::runtime_error("Expected type definition");
        }
		advance(); 

        // Check for PK/FK
        if (currentToken == "PRIMARY" || currentToken == "primary") {
            advance();
            expect("KEY");
            type += " PK";
            hasKey = true;
        } else if (currentToken == "REFERENCES" || currentToken == "references") {
            advance();
            std::string targetTable = currentToken;
            expect(Tokenizer::TokenType::IDENTIFIER);
            type += " FK " + targetTable;
            hasKey = true;
        }

		columns.push_back({name, type});

		if (currentToken == ",")
		{
			advance();
		}
	}
	expect(")");

    if (!hasKey) {
        throw std::runtime_error("Table must have at least one PRIMARY KEY or REFERENCES defined.");
    }

	return std::make_unique<CreateStatement>(table, columns);
}

std::unique_ptr<AST> SQLParser::parseSelect()
{
	advance(); // consume SELECT
	auto columns = parseIdentifierList();
	expect("FROM");

	std::string table;
	std::unique_ptr<AST> nestedSource = nullptr;

	if (currentToken == "(")
	{
		advance(); // consume (
		nestedSource = parse();
		expect(")"); // consume )
	}
	else
	{
		table = currentToken;
		expect(Tokenizer::TokenType::IDENTIFIER);
	}

	std::string condition;
	if (currentToken == "WHERE")
	{
		advance();
        int parenDepth = 0;
		while (currentType != Tokenizer::TokenType::END && 
               (parenDepth > 0 || (currentToken != ")" && currentToken != "ORDER")))
		{
            if (currentToken == "(") parenDepth++;
            else if (currentToken == ")") parenDepth--;
            
			condition += currentToken + " ";
			advance();
		}
	}

	std::string orderBy;
	if (currentToken == "ORDER")
	{
		advance();
		expect("BY");
		orderBy = currentToken; // Simple ORDER BY col
		advance(); 
	}

	return std::make_unique<SelectStatement>(columns, table, condition, std::move(nestedSource), orderBy);
}

std::unique_ptr<AST> SQLParser::parseInsert()
{
	advance(); // INSERT
	expect("INTO");
	std::string table = currentToken;
	expect(Tokenizer::TokenType::IDENTIFIER);

	// Handle (col1, col2)
	expect("(");
	auto columns = parseIdentifierList();
	expect(")");

	expect("VALUES");

	// Handle (val1, val2)
	expect("(");
	auto values = parseIdentifierList();
	expect(")");

	return std::make_unique<InsertStatement>(table, columns, values);
}

std::unique_ptr<AST> SQLParser::parseUpdate()
{
	advance();
	std::string table = currentToken;
	advance();
	expect("SET");
	std::string column = currentToken;
	advance();
	expect("=");
	std::string value = currentToken;
	advance();
	std::string condition;
	if (currentToken == "WHERE")
	{
		advance();
        // Capture everything until end or next keyword (UPDATE usually ends with WHERE, but check delimiters)
		while (currentType != Tokenizer::TokenType::END && currentToken != ";")
		{
			condition += currentToken + " ";
			advance();
		}
	}
	return std::make_unique<UpdateStatement>(table, column, value, condition);
}

std::unique_ptr<AST> SQLParser::parseDelete()
{
	advance();
	expect("FROM");
	std::string table = currentToken;
	advance();
	std::string condition;
	if (currentToken == "WHERE")
	{
		advance();
        // Capture everything until end
		while (currentType != Tokenizer::TokenType::END && currentToken != ";")
		{
			condition += currentToken + " ";
			advance();
		}
	}
	return std::make_unique<DeleteStatement>(table, condition);
}

std::unique_ptr<AST> SQLParser::parseDrop()
{
	advance(); // DROP
	expect("TABLE");
	std::string tableName = currentToken;
	expect(Tokenizer::TokenType::IDENTIFIER);
	return std::make_unique<DropStatement>(tableName);
}

std::unique_ptr<AST> SQLParser::parseCreateRole()
{
	advance(); // ROLE
	std::string roleName = currentToken;
	expect(Tokenizer::TokenType::IDENTIFIER);
	expect("WITH");
	expect("SECRET");
	std::string secretKey = currentToken;
	expect(Tokenizer::TokenType::STRING);
	return std::make_unique<CreateRoleStatement>(roleName, secretKey);
}

std::unique_ptr<AST> SQLParser::parseGrant()
{
	advance(); // GRANT
	std::vector<std::string> privileges;
	privileges.push_back(currentToken);
	advance();
	while (currentToken == ",")
	{
		advance();
		privileges.push_back(currentToken);
		advance();
	}
	expect("ON");
	std::string table = currentToken;
	expect(Tokenizer::TokenType::IDENTIFIER);
	expect("TO");
	std::string role = currentToken;
	expect(Tokenizer::TokenType::IDENTIFIER);
	return std::make_unique<GrantStatement>(privileges, table, role);
}

std::vector<std::string> SQLParser::parseIdentifierList()
{
	std::vector<std::string> list;

	// To grab identifier
	list.push_back(currentToken);
	advance();

	while (currentToken == ",")
	{
		advance(); // consume
		list.push_back(currentToken);
		advance();
	}
	return list;
}