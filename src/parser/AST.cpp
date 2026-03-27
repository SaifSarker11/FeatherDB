#include "AST.h"

SelectStatement::SelectStatement(const std::vector<std::string> &cols, const std::string &tbl,
                                 const std::string &cond, std::unique_ptr<AST> nested, const std::string &order)
    : AST("SELECT"), columns(cols), table(tbl), condition(cond), nestedFrom(std::move(nested)), orderBy(order) {}

std::string SelectStatement::toString() const
{
    std::ostringstream oss;
    oss << "SELECT ";
    for (size_t i = 0; i < columns.size(); ++i)
    {
        oss << columns[i] << (i != columns.size() - 1 ? ", " : "");
    }
    oss << " FROM ";
    if (nestedFrom)
        oss << "(" << nestedFrom->toString() << ")";
    else
        oss << table;

    if (!condition.empty())
        oss << " WHERE " << condition;
    
    if (!orderBy.empty())
        oss << " ORDER BY " << orderBy;

    return oss.str();
}

InsertStatement::InsertStatement(const std::string &tbl, const std::vector<std::string> &cols, const std::vector<std::string> &vals)
    : AST("INSERT"), table(tbl), columns(cols), values(vals) {}

std::string InsertStatement::toString() const
{
    std::ostringstream oss;
    oss << "INSERT INTO " << table << " (";
    for (size_t i = 0; i < columns.size(); ++i)
        oss << columns[i] << (i != columns.size() - 1 ? ", " : "");
    oss << ") VALUES (";
    for (size_t i = 0; i < values.size(); ++i)
        oss << values[i] << (i != values.size() - 1 ? ", " : "");
    oss << ")";
    return oss.str();
}

UpdateStatement::UpdateStatement(const std::string &tbl, const std::string &col, const std::string &val, const std::string &cond)
    : AST("UPDATE"), table(tbl), column(col), value(val), condition(cond) {}

std::string UpdateStatement::toString() const
{
    std::ostringstream oss;
    oss << "UPDATE " << table << " SET " << column << " = " << value;
    if (!condition.empty())
        oss << " WHERE " << condition;
    std::printf("Statement updated\n"); // TODO: rmv
    return oss.str();
}

DeleteStatement::DeleteStatement(const std::string &tbl, const std::string &cond)
    : AST("DELETE"), table(tbl), condition(cond) {}

std::string DeleteStatement::toString() const
{
    std::ostringstream oss;
    oss << "DELETE FROM " << table;
    if (!condition.empty())
        oss << " WHERE " << condition;
    return oss.str();
}

std::string GrantStatement::toString() const
{
    std::ostringstream oss;
    oss << "GRANT ";
    for (size_t i = 0; i < privileges.size(); ++i)
        oss << privileges[i] << (i != privileges.size() - 1 ? ", " : "");
    oss << " ON " << table << " TO " << role;
    return oss.str();
}