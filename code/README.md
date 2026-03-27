# FeatherDB 

FeatherDB is a custom SQL Database Engine written in C++ from scratch. It supports CRUD operations, nested queries, and a REPL interface.


## QuickStart: How to Build

Run the provided `compile.bat` script:
```
.\compile.bat
```
This will compile the source code and generate the executable in `build/featherdb.exe`.

## How to Run

Execute the generated binary:
```
.\build\featherdb.exe
```

## Features & Usage

The interactive REPL supports standard SQL commands and meta-commands.

### Meta-Commands
- `.help`: Show help message.
- `.tables`: List all tables.
- `.schema <table_name>`: Show schema of a table.
- `.exit`: Exit the database.

### SQL Features
- **CREATE TABLE**: Define new tables.
  ```sql
  CREATE TABLE users (id INT, name STRING);
  ```
- **INSERT**: Add rows.
  ```sql
  INSERT INTO users (id, name) VALUES (1, alice);
  ```
- **SELECT**: Query data with filtering and sorting.
  ```sql
  SELECT * FROM users WHERE id > 0 ORDER BY id;
  ```
- **UPDATE**: Modify existing rows.
  ```sql
  UPDATE users SET name = bob WHERE id = 1;
  ```
- **DELETE**: Remove rows.
  ```sql
  DELETE FROM users WHERE id = 1;
  ```
- **Nested Queries**: Support for subqueries in `FROM` clause and `WHERE ... IN` clause.
  ```sql
  SELECT * FROM (SELECT id, name FROM users WHERE id > 5);
  SELECT * FROM users WHERE id IN (SELECT id FROM banned_users);
  ```

## Architecture & Implementation Details

- **Parser**: A custom recursive descent parser (modified from existing base) that transforms SQL into an AST. Added support for `CREATE`, `ORDER BY`, and nested structures.
- **Execution Engine**: `QueryExecutor` traverses the AST.
    - **Filtering**: Implemented a custom expression evaluator for `WHERE` clauses without external libraries.
    - **Sorting**: Implemented a manual **Quicksort** algorithm for `ORDER BY`.
    - **Nested Queries**: Handled via recursive execution of `SelectStatement` and materialization of intermediate results.
- **Storage Layer**: `StorageManager` updates `db/table.csv` and `db/table.schema`.

## Supported Data Types

FeatherDB is a schemaless-like engine but supports basic validation for:
- **`INT`**: Integer numbers.
- **`STRING`**: Text values (no quotes needed for simple single-word values, use quotes `'...'` for values with spaces).
- **`VARCHAR`**: Treated same as `STRING`.

## Recent Changes & Changelog

### v1.1.0 (Current)
- **Fix**: Resolved issue where `UPDATE` and `DELETE` ignored `WHERE` conditions (now correctly parses full expressions).
- **Feat**: Data persistence moved to `db/` folder to separate data from executables.
- **Feat**: Tokenizer is now case-insensitive (e.g., `sElEcT` works).
- **Doc**: Added `QUICKSTART.md` with usage instructions.

### v1.0.0
- Initial release with CRUD, Nested Queries, and REPL.
## Contributing
Team
- [safwansatil](github.com/safwansatil)
- [saif al sarker](https://github.com/SaifSarker11)
- [wasiomar](https://github.com/WasiOmar)

## Acknowledgements
"I've got no choice. We're cornered, so I have to teach you. First off, Johnny, let me tell you one thing. Starting now, you can only say the words, 'There is no way I can do this!' four times, and four times only. Alright? Four times. That's what my father taught me when I was a kid."


