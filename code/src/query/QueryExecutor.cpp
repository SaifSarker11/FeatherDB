#include <iostream>
#include <set>

#include "QueryExecutor.h"
#include "../storage/StorageManager.h"
#include "../parser/Tokenizer.h"
#include "../parser/SQLParser.h"


namespace spl {

bool isInteger(const std::string& s) {
    if(s.empty()) return false;
    size_t start = 0;
    if(s[0] == '-' || s[0] == '+') {
         if(s.size() == 1) return false;
         start = 1;
    }
    for (size_t i = start; i < s.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    }
    return true;
}

int getColumnIndex(const Table& table, const std::string& colName) {
    for (size_t i = 0; i < table.columns.size(); ++i) {
        if (table.columns[i].name == colName) return i;
    }
    return -1;
}

bool hasDependentRows(const std::string& parentTable, const std::string& parentPkValue) {
    std::vector<std::string> allTables = StorageManager::listTables();
    for (const auto& tableName : allTables) {
        if (tableName == parentTable) continue;
        Table t = StorageManager::loadTable(tableName);
        for (size_t i = 0; i < t.columns.size(); ++i) {
            if (t.columns[i].fkTargetTable == parentTable) {
                for (const auto& row : t.rows) {
                    if (i < row.values.size() && row.values[i] == parentPkValue) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

int evaluateExpression(const std::string& expr) {
    if (expr.empty()) return 0;
    std::stringstream ss(expr);
    int result = 0;
    if (!(ss >> result)) return 0;
    char op;
    int nextVal;
    while (ss >> op >> nextVal) {
        if (op == '+') result += nextVal;
        else if (op == '-') result -= nextVal;
        else if (op == '*') result *= nextVal;
        else if (op == '/') if (nextVal != 0) result /= nextVal;
    }
    return result;
}

bool evaluateSimple(const Row& row, const Table& table, const std::string& condition) {
    if (condition.empty()) return true;
    
    std::string op;
    size_t opPos = std::string::npos;
    static const std::vector<std::string> ops = {">=", "<=", "!=", "=", ">", "<"};
    for (const auto& o : ops) {
        opPos = condition.find(o);
        if (opPos != std::string::npos) {
            op = o;
            break;
        }
    }
    
    if (opPos == std::string::npos) return true;

    std::string colName = condition.substr(0, opPos);
    std::string valExpr = condition.substr(opPos + op.length());
    
    auto trim = [](std::string& s) {
        size_t first = s.find_first_not_of(" ");
        if (std::string::npos == first) { s = ""; return; }
        size_t last = s.find_last_not_of(" ");
        s = s.substr(first, (last - first + 1));
    };
    trim(colName);
    trim(valExpr);
    
    int idx = getColumnIndex(table, colName);
    if (idx == -1) return false;

    std::string rowVal = row.values[idx];
    
    if (table.columns[idx].type == "INT" || table.columns[idx].type == "int") {
        try {
            int r = std::stoi(rowVal);
            int v = evaluateExpression(valExpr);
            if (op == "=") return r == v;
            if (op == ">") return r > v;
            if (op == "<") return r < v;
            if (op == ">=") return r >= v;
            if (op == "<=") return r <= v;
            if (op == "!=") return r != v;
        } catch (...) { return false; }
    } else {
        if (valExpr.size() >= 2 && valExpr.front() == '\'' && valExpr.back() == '\'') {
            valExpr = valExpr.substr(1, valExpr.size() - 2);
        }
        if (op == "=") return rowVal == valExpr;
        if (op == "!=") return rowVal != valExpr;
        if (op == ">") return rowVal > valExpr;
        if (op == "<") return rowVal < valExpr;
    }
    return false;
}

void QueryExecutor::execute(std::unique_ptr<AST> ast, std::string& currentRole) {
    if (!ast) return;
    if (ast->type == "CREATE") {
        handleCreate(static_cast<CreateStatement*>(ast.get()));
    } else if (ast->type == "INSERT") {
        handleInsert(static_cast<InsertStatement*>(ast.get()), currentRole);
    } else if (ast->type == "SELECT") {
         Table t = executeSelect(static_cast<SelectStatement*>(ast.get()), currentRole);
         t.print();
    } else if (ast->type == "UPDATE") {
        handleUpdate(static_cast<UpdateStatement*>(ast.get()), currentRole);
    } else if (ast->type == "DELETE") {
        handleDelete(static_cast<DeleteStatement*>(ast.get()), currentRole);
    } else if (ast->type == "CREATE_ROLE") {
        handleCreateRole(static_cast<CreateRoleStatement*>(ast.get()));
    } else if (ast->type == "GRANT") {
        handleGrant(static_cast<GrantStatement*>(ast.get()));
    } else if (ast->type == "DROP") {
        handleDrop(static_cast<DropStatement*>(ast.get()), currentRole);
    } else {
        std::cout << "Unknown query type: " << ast->type << "\n";
    }
}

void QueryExecutor::handleCreate(CreateStatement* stmt) {
    std::vector<Column> cols;
    for (const auto& p : stmt->columns) {
        Column col;
        col.name = p.first;
        col.type = p.second;
        
        // Parse metadata out of type if present
        size_t pkPos = col.type.find(" PK");
        if (pkPos != std::string::npos) {
            col.isPrimaryKey = true;
            col.type = col.type.substr(0, pkPos);
        }
        
        size_t fkPos = col.type.find(" FK ");
        if (fkPos != std::string::npos) {
            col.fkTargetTable = col.type.substr(fkPos + 4);
            col.type = col.type.substr(0, fkPos);
        }

        size_t uniquePos = col.type.find(" UNIQUE");
        if (uniquePos != std::string::npos) {
            col.isUnique = true;
            col.type = col.type.substr(0, uniquePos);
        }
        
        cols.push_back(col);
    }
    if (StorageManager::createTable(stmt->table, cols)) {
        std::cout << "Table '" << stmt->table << "' created.\n";
    } else {
        std::cout << "Error: Table '" << stmt->table << "' already exists or create failed.\n";
    }
}

void QueryExecutor::handleInsert(InsertStatement* stmt, const std::string& currentRole) {
    if (!StorageManager::checkPermission(currentRole, "INSERT", stmt->table)) {
        std::cout << "Error: Permission denied for INSERT on table '" << stmt->table << "'\n";
        return;
    }
    Table table = StorageManager::loadTable(stmt->table); // Use loadTable to get rows for uniqueness check
    if (table.columns.empty() && table.name.empty()) {
         std::cout << "Error: Table '" << stmt->table << "' not found.\n";
         return;
    }

    if (stmt->values.size() != table.columns.size()) {
         std::cout << "Error: Column count mismatch. Table '" << stmt->table << "' expects " << table.columns.size() << " values, but " << stmt->values.size() << " were provided.\n";
         return;
    }

    int pkIdx = -1;
    for (size_t i = 0; i < table.columns.size(); ++i) {
        if (table.columns[i].isPrimaryKey) pkIdx = i;

        std::string val = stmt->values[i];
        
        // Remove quotes for validation and uniqueness check
        if (val.size() >= 2 && val.front() == '\'' && val.back() == '\'') {
            val = val.substr(1, val.size() - 2);
        }
        stmt->values[i] = val; // Store back the unquoted value

        std::string type = table.columns[i].type;
        if (type == "INT" || type == "int") {
            if (!isInteger(val)) {
                std::cout << "Error: Invalid INT value '" << val << "' for column '" << table.columns[i].name << "'\n";
                return;
            }
        }

        // Check FK constraint
        if (!table.columns[i].fkTargetTable.empty()) {
            Table targetTable = StorageManager::loadTable(table.columns[i].fkTargetTable);
            bool found = false;
            
            // Bug 7: Correctly identify the primary key column index of the target table
            int targetPkIdx = -1;
            for (size_t j = 0; j < targetTable.columns.size(); ++j) {
                if (targetTable.columns[j].isPrimaryKey) {
                    targetPkIdx = j;
                    break;
                }
            }
            if (targetPkIdx == -1) targetPkIdx = 0; // Fallback to first column

            for (const auto& r : targetTable.rows) {
                if (targetPkIdx < (int)r.values.size() && r.values[targetPkIdx] == val) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                std::cout << "Error: Foreign key constraint violation. Value '" << val << "' not found in table '" << table.columns[i].fkTargetTable << "'.\n";
                return;
            }
        }
    }

    // Check Uniqueness (PK and UNIQUE)
    for (size_t i = 0; i < table.columns.size(); ++i) {
        if (table.columns[i].isPrimaryKey || table.columns[i].isUnique) {
            std::string newVal = stmt->values[i];
            for (const auto& existingRow : table.rows) {
                if (i < existingRow.values.size() && existingRow.values[i] == newVal) {
                    std::string constraintType = table.columns[i].isPrimaryKey ? "Primary key" : "Unique constraint";
                    std::cout << "Error: " << constraintType << " violation. Value '" << newVal << "' already exists in column '" << table.columns[i].name << "'.\n";
                    return;
                }
            }
        }
    }

    Row row;
    row.values = stmt->values;
    if (StorageManager::appendRow(stmt->table, row)) {
        std::cout << "1 row inserted.\n";
    } else {
        std::cout << "Error: Could not write to table.\n";
    }
}

// Quicksort Helpers
void swapRows(Row& a, Row& b) {
    Row temp = a;
    a = b;
    b = temp;
}

int partition(std::vector<Row>& rows, int low, int high, int colIdx, bool isInt) {
    std::string pivotStr = rows[high].values[colIdx];
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        std::string curStr = rows[j].values[colIdx];
        bool less = false;
        if (isInt) {
            try {
                int p = std::stoi(pivotStr);
                int c = std::stoi(curStr);
                less = (c < p);
            } catch (...) { less = (curStr < pivotStr); }
        } else {
             less = (curStr < pivotStr);
        }

        if (less) {
            i++;
            swapRows(rows[i], rows[j]);
        }
    }
    swapRows(rows[i + 1], rows[high]);
    return (i + 1);
}

void quickSort(std::vector<Row>& rows, int low, int high, int colIdx, bool isInt) {
    if (low < high) {
        int pi = partition(rows, low, high, colIdx, isInt);
        quickSort(rows, low, pi - 1, colIdx, isInt);
        quickSort(rows, pi + 1, high, colIdx, isInt);
    }
}

Table QueryExecutor::executeSelect(SelectStatement* stmt, const std::string& currentRole) {
    Table sourceTable;
    if (stmt->nestedFrom) {
        if(stmt->nestedFrom->type == "SELECT") {
             sourceTable = executeSelect(static_cast<SelectStatement*>(stmt->nestedFrom.get()), currentRole);
             sourceTable.name = "nested"; // anonymous
        } else {
             std::cout << "Error: Nested FROM must be SELECT\n";
             return Table();
        }
    } else {
        if (!StorageManager::checkPermission(currentRole, "SELECT", stmt->table)) {
            std::cout << "Error: Permission denied for SELECT on table '" << stmt->table << "'\n";
            return Table();
        }
        sourceTable = StorageManager::loadTable(stmt->table);
    }
    
    if(sourceTable.columns.empty() && sourceTable.name.empty() && !stmt->nestedFrom) {
         std::cout << "Error: Table '" << stmt->table << "' not found. Please check the table name.\n";
         return Table();
    }
    
    Table filteredTable;
    filteredTable.name = sourceTable.name;
    filteredTable.columns = sourceTable.columns;
    
    if (stmt->condition.empty()) {
        filteredTable.rows = sourceTable.rows;
    } else {
        std::string condition = stmt->condition;
        std::set<std::string> inValues;
        std::string inCol;
        bool hasIn = false;
        
        auto trim = [](std::string& s) {
            size_t first = s.find_first_not_of(" ");
            if (std::string::npos == first) { s = ""; return; }
            size_t last = s.find_last_not_of(" ");
            s = s.substr(first, (last - first + 1));
        };

        size_t inPos = condition.find(" IN ");
        if (inPos == std::string::npos) inPos = condition.find(" in ");
        
        if (inPos != std::string::npos) {
             inCol = condition.substr(0, inPos);
             trim(inCol);
             
             size_t startParen = condition.find("(", inPos);
             if (startParen != std::string::npos) {
                 size_t endParen = std::string::npos;
                 int depth = 0;
                 for (size_t i = startParen; i < condition.length(); ++i) {
                     if (condition[i] == '(') depth++;
                     else if (condition[i] == ')') {
                         depth--;
                         if (depth == 0) {
                             endParen = i;
                             break;
                         }
                     }
                 }
                 
                 if (endParen != std::string::npos) {
                    std::string subSQL = condition.substr(startParen + 1, endParen - startParen - 1);
                    trim(subSQL);
                    if (subSQL.rfind("SELECT", 0) == 0 || subSQL.rfind("select", 0) == 0) {
                        hasIn = true;
                        subSQL += ";";
                        Tokenizer tokenizer(subSQL);
                        SQLParser parser(tokenizer);
                        std::unique_ptr<AST> subAst = parser.parse();
                        QueryExecutor subExec;
                        if (subAst && subAst->type == "SELECT") {
                             Table subRes = subExec.executeSelect(static_cast<SelectStatement*>(subAst.get()), currentRole);
                             for(const auto& r : subRes.rows) {
                                 if(!r.values.empty()) inValues.insert(r.values[0]);
                             }
                        }
                    }
                 }
             }
        }

        // New logic for scalar subqueries: WHERE grade < (SELECT ...)
        size_t subPos = condition.find("( SELECT");
        if (subPos == std::string::npos) subPos = condition.find("( select");
        if (subPos == std::string::npos) subPos = condition.find("(SELECT");
        if (subPos == std::string::npos) subPos = condition.find("(select");
        
        if (subPos != std::string::npos && !hasIn) {
            size_t startParen = subPos;
            size_t endParen = std::string::npos;
            int depth = 0;
            for (size_t i = startParen; i < condition.length(); ++i) {
                if (condition[i] == '(') depth++;
                else if (condition[i] == ')') {
                    depth--;
                    if (depth == 0) {
                        endParen = i;
                        break;
                    }
                }
            }

            if (endParen != std::string::npos && endParen > startParen) {
                std::string subSQL = condition.substr(startParen + 1, endParen - startParen - 1) + ";";
                Tokenizer tokenizer(subSQL);
                SQLParser parser(tokenizer);
                std::unique_ptr<AST> subAst = parser.parse();
                QueryExecutor subExec;
                if (subAst && subAst->type == "SELECT") {
                    Table subRes = subExec.executeSelect(static_cast<SelectStatement*>(subAst.get()), currentRole);
                    if (subRes.rows.size() != 1 || subRes.columns.size() != 1) {
                        std::cout << "Error: Subquery must return exactly one scalar value.\n";
                        return Table();
                    }
                    std::string scalarVal = subRes.rows[0].values[0];
                    // Replace the subquery in the condition with the actual value
                    condition.replace(startParen, endParen - startParen + 1, scalarVal);
                }
            }
        }
        
        for (const auto& row : sourceTable.rows) {
            bool pass = false;
            if (hasIn) {
                int idx = getColumnIndex(sourceTable, inCol);
                if (idx != -1) {
                    if (inValues.count(row.values[idx])) pass = true;
                }
            } else {
                if (evaluateSimple(row, sourceTable, condition)) pass = true;
            }
            if (pass) filteredTable.rows.push_back(row);
        }
    }
    
    // Sort
    if (!stmt->orderBy.empty()) {
        int sortIdx = getColumnIndex(filteredTable, stmt->orderBy);
        if (sortIdx != -1) {
             bool isInt = (filteredTable.columns[sortIdx].type == "INT" || filteredTable.columns[sortIdx].type == "int");
             if (!filteredTable.rows.empty())
                quickSort(filteredTable.rows, 0, filteredTable.rows.size() - 1, sortIdx, isInt);
        } else {
             std::cout << "Warning: Order By column " << stmt->orderBy << " not found.\n";
        }
    }
    
    if (stmt->columns.size() == 1 && stmt->columns[0] == "*") {
        return filteredTable;
    }
    
    Table resultTable;
    resultTable.name = filteredTable.name;
    std::vector<int> colIndices;
    for(const auto& colName : stmt->columns) {
        int idx = getColumnIndex(filteredTable, colName);
        if(idx != -1) {
            resultTable.columns.push_back(filteredTable.columns[idx]);
            colIndices.push_back(idx);
        } else {
             // Ignore unknown columns or handle? AST parser puts them in.
        }
    }
    
    for(const auto& row : filteredTable.rows) {
        Row newRow;
        for(int idx : colIndices) {
            newRow.values.push_back(row.values[idx]);
        }
        resultTable.rows.push_back(newRow);
    }
    
    return resultTable;
}

void QueryExecutor::handleUpdate(UpdateStatement* stmt, const std::string& currentRole) {
    if (!StorageManager::checkPermission(currentRole, "UPDATE", stmt->table)) {
        std::cout << "Error: Permission denied for UPDATE on table '" << stmt->table << "'\n";
        return;
    }
    Table table = StorageManager::loadTable(stmt->table);
    if (table.columns.empty()) {
        std::cout << "Error: Table '" << stmt->table << "' not found. Cannot perform UPDATE.\n";
        return;
    }
    
    int setIdx = getColumnIndex(table, stmt->column);
    if (setIdx == -1) {
         std::cout << "Error: Column " << stmt->column << " not found.\n";
         return;
    }
    
    int count = 0;
    for (auto& row : table.rows) {
        if (stmt->condition.empty() || evaluateSimple(row, table, stmt->condition)) {
            // Bug 6: Referential Integrity (RESTRICT) for UPDATE
            // If the PK is being updated and it's referenced elsewhere, block it.
            int pkIdx = -1;
            for(int i=0; i<(int)table.columns.size(); ++i) {
                if(table.columns[i].isPrimaryKey) {
                    pkIdx = i;
                    break;
                }
            }
            
            if (pkIdx != -1 && pkIdx == setIdx && row.values[pkIdx] != stmt->value) {
                if (hasDependentRows(table.name, row.values[pkIdx])) {
                    std::cout << "Error: Referential integrity violation. Cannot update row in table '" << table.name << "' as it is referenced by other tables (RESTRICT).\n";
                    return;
                }
            }

            // Bug 5: Type checking on UPDATE
            std::string type = table.columns[setIdx].type;
            if (type == "INT" || type == "int") {
                if (!isInteger(stmt->value)) {
                    std::cout << "Error: Invalid INT value '" << stmt->value << "' for column '" << table.columns[setIdx].name << "'\n";
                    return;
                }
            }
            row.values[setIdx] = stmt->value;
            count++;
        }
    }
    
    if (StorageManager::saveTable(table)) {
        std::cout << count << " rows updated.\n";
    } else {
        std::cout << "Error saving table.\n";
    }
}

void QueryExecutor::handleDelete(DeleteStatement* stmt, const std::string& currentRole) {
    if (!StorageManager::checkPermission(currentRole, "DELETE", stmt->table)) {
        std::cout << "Error: Permission denied for DELETE on table '" << stmt->table << "'\n";
        return;
    }
    Table table = StorageManager::loadTable(stmt->table);
    if (table.columns.empty()) {
        std::cout << "Error: Table '" << stmt->table << "' not found. Cannot perform DELETE.\n";
        return;
    }
    
    Table newTable;
    newTable.name = table.name;
    newTable.columns = table.columns;
    
    int count = 0;
    for (const auto& row : table.rows) {
        if (!stmt->condition.empty() && evaluateSimple(row, table, stmt->condition)) {
            // Bug 6: Referential Integrity (RESTRICT)
            // If this row is being referenced by ANY other table, block the deletion
            int pkIdx = -1;
            for(int i=0; i<table.columns.size(); ++i) if(table.columns[i].isPrimaryKey) pkIdx = i;

            if (pkIdx != -1) {
                if (hasDependentRows(table.name, row.values[pkIdx])) {
                    std::cout << "Error: Referential integrity violation. Cannot delete row in table '" << table.name << "' as it is referenced by other tables (RESTRICT).\n";
                    return; // Stop the delete operation
                }
            }
            count++; // Skip (delete)
        } else {
            newTable.rows.push_back(row);
        }
    }
    
    if (StorageManager::saveTable(newTable)) {
        std::cout << count << " rows deleted.\n";
    } else {
        std::cout << "Error saving table.\n";
    }
}

void QueryExecutor::handleSelect(SelectStatement* stmt, const std::string& currentRole) {
    executeSelect(stmt, currentRole).print();
}

void QueryExecutor::handleCreateRole(CreateRoleStatement* stmt) {
    if (StorageManager::roleExists(stmt->roleName)) {
        std::cout << "Error: Role '" << stmt->roleName << "' already exists.\n";
        return;
    }
    if (StorageManager::saveRole(stmt->roleName, stmt->secretKey)) {
        std::cout << "Role '" << stmt->roleName << "' created successfully with secret key.\n";
    } else {
        std::cout << "Error: Failed to create role '" << stmt->roleName << "'.\n";
    }
}

void QueryExecutor::handleGrant(GrantStatement* stmt) {
    if (!StorageManager::roleExists(stmt->role)) {
        std::cout << "Error: Role '" << stmt->role << "' does not exist.\n";
        return;
    }
    bool success = true;
    for (const auto& priv : stmt->privileges) {
        if (!StorageManager::savePermission(stmt->role, priv, stmt->table)) {
            success = false;
        }
    }
    if (success) {
        std::cout << "Permissions granted to '" << stmt->role << "' on '" << stmt->table << "'.\n";
    } else {
        std::cout << "Error granting permissions.\n";
    }
}

void QueryExecutor::handleDrop(DropStatement* stmt, const std::string& currentRole) {
    if (currentRole != "admin" && !StorageManager::checkPermission(currentRole, "DELETE", stmt->tableName)) {
        std::cout << "Error: Permission denied to drop table '" << stmt->tableName << "'.\n";
        return;
    }
    if (StorageManager::deleteTable(stmt->tableName)) {
        std::cout << "Table '" << stmt->tableName << "' dropped successfully.\n";
    } else {
        std::cout << "Error: Table '" << stmt->tableName << "' not found.\n";
    }
}

} // namespace spl