#ifndef SPL_QUERYEXECUTOR_H
#define SPL_QUERYEXECUTOR_H

#include <memory>
#include "../parser/AST.h"
#include "../storage/StorageStructs.h"

namespace spl {

class QueryExecutor {
public:
	QueryExecutor() = default;
	virtual ~QueryExecutor() = default;

	// Main entry point
	void execute(std::unique_ptr<AST> ast, std::string& currentRole);

private:
	void handleCreate(CreateStatement* stmt);
	void handleInsert(InsertStatement* stmt, const std::string& currentRole);
	void handleSelect(SelectStatement* stmt, const std::string& currentRole); // Changed to void for now, will print result or return Table later.
    // Making it return Table is better for nested queries.
    Table executeSelect(SelectStatement* stmt, const std::string& currentRole);

	void handleUpdate(UpdateStatement* stmt, const std::string& currentRole);
	void handleDelete(DeleteStatement* stmt, const std::string& currentRole);

	void handleCreateRole(CreateRoleStatement* stmt);
	void handleGrant(GrantStatement* stmt);
    void handleDrop(DropStatement* stmt, const std::string& currentRole);
};

} // namespace spl

#endif // SPL_QUERYEXECUTOR_H