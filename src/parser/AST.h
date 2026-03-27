#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <sstream>
#include <memory>

class AST
{
public:
	std::string type;
	AST(const std::string &type) : type(type) {}
	virtual ~AST() = default;
	virtual std::string toString() const = 0;
};

class SelectStatement : public AST
{
public:
	std::vector<std::string> columns;
	std::string table;
	std::string condition;
	std::unique_ptr<AST> nestedFrom; // nested from er jnno
	std::string orderBy;

	SelectStatement(const std::vector<std::string> &cols, const std::string &tbl,
					const std::string &cond, std::unique_ptr<AST> nested = nullptr, const std::string &order = "");
	std::string toString() const override;
};




class InsertStatement : public AST
{
public:
	std::string table;
	std::vector<std::string> columns;
	std::vector<std::string> values;

	InsertStatement(const std::string &tbl, const std::vector<std::string> &cols, const std::vector<std::string> &vals);
	std::string toString() const override;
};

class UpdateStatement : public AST
{
public:
	std::string table;
	std::string column;
	std::string value;
	std::string condition;

	UpdateStatement(const std::string &tbl, const std::string &col, const std::string &val, const std::string &cond);
	std::string toString() const override;
};

class DeleteStatement : public AST
{
public:
	std::string table;
	std::string condition;

	DeleteStatement(const std::string &tbl, const std::string &cond);
	std::string toString() const override;
};

class CreateStatement : public AST
{
public:
	std::string table;
	std::vector<std::pair<std::string, std::string>> columns;

	CreateStatement(const std::string &tbl, const std::vector<std::pair<std::string, std::string>> &cols)
		: AST("CREATE"), table(tbl), columns(cols) {}
	
	std::string toString() const override {
		std::string ret = "CREATE TABLE " + table + " (";
		for (size_t i = 0; i < columns.size(); ++i) {
			ret += columns[i].first + " " + columns[i].second;
			if (i < columns.size() - 1) ret += ", ";
		}
		ret += ")";
		return ret;
	}
};

class CreateRoleStatement : public AST
{
public:
	std::string roleName;
	std::string secretKey;
	CreateRoleStatement(const std::string &role, const std::string &secret) 
		: AST("CREATE_ROLE"), roleName(role), secretKey(secret) {}
	std::string toString() const override { return "CREATE ROLE " + roleName + " WITH SECRET '" + secretKey + "'"; }
};

class GrantStatement : public AST
{
public:
	std::vector<std::string> privileges;
	std::string table;
	std::string role;
	GrantStatement(const std::vector<std::string> &privs, const std::string &tbl, const std::string &r)
		: AST("GRANT"), privileges(privs), table(tbl), role(r) {}
	std::string toString() const override;
};

class DropStatement : public AST
{
public:
	std::string tableName;
	DropStatement(const std::string &table) : AST("DROP"), tableName(table) {}
	std::string toString() const override { return "DROP TABLE " + tableName; }
};

#endif