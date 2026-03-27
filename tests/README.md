# FeatherDB Test Suite

Tests mirror the `src/` directory structure. Each test file maps 1:1 to its corresponding source file.

## Structure

```
tests/
├── parser/
│   ├── TokenizerTest.cpp     → src/parser/Tokenizer.cpp
│   ├── SQLParserTest.cpp     → src/parser/SQLParser.cpp
│   └── ASTTest.cpp           → src/parser/AST.cpp
├── query/
│   └── QueryExecutorTest.cpp → src/query/QueryExecutor.cpp
├── storage/
│   └── StorageManagerTest.cpp → src/storage/StorageManager.cpp
└── utils/
    ├── ConfigManagerTest.cpp → src/utils/ConfigManager.h
    └── PrintTest.cpp         → src/utils/Print.cpp
```

## What Each File Covers

**parser/TokenizerTest.cpp**
- Keyword normalization to uppercase
- Token type classification (KEYWORD, IDENTIFIER, NUMBER, STRING, OPERATOR, PUNCTUATION, END)
- Whitespace skipping
- String literal quote stripping
- `hasNext()` behavior
- PK/FK keyword recognition
- Error cases: unterminated strings, invalid characters

**parser/SQLParserTest.cpp**
- Parsing SELECT (columns, table, WHERE, ORDER BY, `*`)
- Parsing INSERT (table, columns, values)
- Parsing UPDATE (table, column, value, condition)
- Parsing DELETE (table, condition)
- Parsing CREATE TABLE (with PK, FK; rejects tables without PK/FK)
- Parsing DROP TABLE
- Parsing CREATE ROLE (name, secret)
- Parsing GRANT (privilege, table, role)
- Error: unknown SQL command

**parser/ASTTest.cpp**
- `toString()` output for all AST node types
- `type` field correctness for all statement classes
- WHERE and ORDER BY inclusion in SelectStatement output

**storage/StorageManagerTest.cpp**
- `createTable`: success and duplicate rejection
- `loadTable`: column names, PK flag, FK target, missing table
- `appendRow` / `loadTable`: row persistence, multiple rows
- `saveTable`: overwrites existing data
- `dropTable`: removes table from listing
- `listTables`: includes newly created tables
- `getTableSchema`: returns columns without loading rows
- RBAC: admin always has permission, unknown role has no permission, `roleExists`

**query/QueryExecutorTest.cpp**
- INSERT: row added, PK uniqueness rejection, column count mismatch, invalid INT, permission denied
- UPDATE: matching rows modified, non-matching rows untouched
- DELETE: matching row removed, empty condition behavior
- CREATE TABLE via executor
- DROP TABLE via executor
- CREATE ROLE: role becomes findable after creation
- GRANT: error when role does not exist

**utils/ConfigManagerTest.cpp**
- Key lookup: found, missing with default, missing without default
- Whitespace trimming around keys and values
- Comment line (`#`) and empty line handling
- Missing `.env` file does not crash
- Multiple keys all loaded correctly

**utils/PrintTest.cpp**
- `printHelp()`: output contains `.exit`, `.tables`, `.schema`
- `printPrompt()`: default prompt shown when no role; role name shown when set; no parentheses when empty
- `printIntro()`: output contains version string and "FeatherDB"

## How to Run

From the project root:

```bat
.\run_tests.bat
```

This compiles each test suite into `build/` and runs them sequentially. A non-zero exit code means at least one test failed.

To run a single suite:

```bat
g++ -std=c++17 -I src tests/parser/TokenizerTest.cpp src/parser/Tokenizer.cpp -o build/TokenizerTest.exe
build\TokenizerTest.exe
```

## Dependencies & Setup

- **Compiler**: g++ with C++17 support (`-std=c++17`)
- **No external test framework** — tests use `<cassert>` and `<iostream>` only
- **Storage tests** write temporary files to `db/` using the prefix `Test_SM_` or `Test_QE_`; they clean up after themselves via `dropTable`
- **ConfigManager tests** write a temporary `.env` file (`test_config_tmp.env`) in the working directory and delete it on completion
- Run tests from the **project root** so relative paths to `db/` and `.env` resolve correctly
