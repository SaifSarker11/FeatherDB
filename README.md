# FeatherDB 

![Language](https://img.shields.io/badge/language-C%2B%2B-orange)

**FeatherDB** is a lightweight, file-based relational database engine written from scratch in C++. It demonstrates advanced database concepts through a custom SQL interpreter, role-based access control, and dynamic relational visualization.

---

## 🚀 Key Features

- **Core SQL Engine**: Supports `CREATE`, `INSERT`, `SELECT`, `UPDATE`, `DELETE`, and `DROP` statements.
- **Relational Integrity**: Enforces `PRIMARY KEY`, `UNIQUE`, and `REFERENCES` (Foreign Key) constraints with `RESTRICT` delete policy.
- **Advanced Querying**: Recursive support for nested subqueries in `FROM` and `WHERE` clauses, plus in-memory `ORDER BY` sorting via Quicksort.
- **Stealthy RBAC**: A multi-phase security layer with secret-key authentication and granular role permissions (stored in `db/_roles.csv`).
- **Dynamic Visualization**: Integrated `.erd` utility to visualize 1:1, 1:N, and N:N relationships directly in the REPL.

---

## 🛠️ Quick Start

### Prerequisites
- Windows OS
- G++ Compiler (supporting C++17 or later)

### Build and Run
1.  **Compile**: Run the provided script to generate the binary.
    ```powershell
    .\compile.bat
    ```
2.  **Launch**: Execute the database engine.
    ```powershell
    .\build\featherdb.exe
    ```

### First Commands to Try
Upon launch, FeatherDB starts in a restricted "stealth" mode. Authenticate as Admin to begin:
```sql
-- 1. Login (Default Secret)
ramadankareem

-- 2. Create a Table with Constraints
CREATE TABLE users (id INT PRIMARY KEY, name STRING UNIQUE);

-- 3. Insert and Query
INSERT INTO users (id, name) VALUES (1, 'Safwan');
SELECT * FROM users;
```
Setup .env file for custom secrets and configurations as needed.

---

##  Role-Based Access Control (RBAC)

FeatherDB implements a secure session-based RBAC system:
1.  **Authentication**: Enter a role's secret key (or the admin secret phrase) in the REPL to toggle login/logout.
2.  **Role Management**: Admins can create roles and grant specific privileges.
    ```sql
    CREATE ROLE dev WITH SECRET 'safwan_pass';
    GRANT SELECT, INSERT ON users TO dev;
    ```
3.  **Active Sessions**: The prompt reflects your current role: `featherDb> (dev)`.

---

##  SQL & Meta-Command Reference

### SQL Examples
- **Nested Select**: `SELECT * FROM (SELECT id FROM users);`
- **Subquery Filter**: `SELECT * FROM products WHERE price < (SELECT max_price FROM config);`
- **Update with Constraints**: `UPDATE users SET name = 'Saif' WHERE id = 1;`

### Meta-Commands
| Command | Syntax | Description |
| :--- | :--- | :--- |
| **Help** | `.help` | Display the list of available meta-commands. |
| **Tables** | `.tables` | List all tables currently stored in the `db/` folder. |
| **Schema** | `.schema <name>` | Display the `CREATE TABLE` statement for a specific table. |
| **ERD** | `.erd` | Visualize current entity relationships (1:1, 1:N, N:N). |
| **Exit** | `.exit` | Safely close the database session. |

---

##  Project Structure

```text
spl1/
├── build/             # Compiled binaries
├── code/
│   ├── src/           # Source code (.cpp, .h)
│   │   ├── parser/    # Tokenizer, SQLParser, AST
│   │   ├── query/     # Execution Engine & Expression Evaluator
│   │   ├── storage/   # Disk Persistence & CSV Handling
│   │   └── utils/     # ERD, Printing, and Config
│   └── commands.txt   # Demo script and test cases
└── README.md          # Project overview
```

---

##  Architecture Summary

FeatherDB follows a decoupled pipeline architecture:
`REPL` → `Tokenizer` → `SQLParser` → `AST` → `QueryExecutor` → `StorageManager` → `CSV/Schema Files`.

All data operations are verified against a centralized RBAC security layer before reaching the storage engine, ensuring that integrity and security are never bypassed. For deep technical details, see [documentation.md](code/documentation.md).

---

##  Team

- **Md. Saif Al Sarker** - [@SaifSarker11](https://github.com/SaifSarker11)
- **M Safwan Hasan Khan** - [@safwansatil](https://github.com/safwansatil)
- **Wasi Omar Syed** - [@WasiOmar](https://github.com/WasiOmar)

---

##  Acknowledgements

"I've got no choice. We're cornered, so I have to teach you. First off, Johnny, let me tell you one thing. Starting now, you can only say the words, 'There is no way I can do this!' four times, and four times only. Alright? Four times. That's what my father taught me when I was a kid."
