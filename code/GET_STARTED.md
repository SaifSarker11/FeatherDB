# FeatherDB Quick Start

## 1. Build and Run
Execute `.\compile.bat` to build the engine. The binary will be located in `build/featherdb.exe`.

## 2. Role-Based Access Control (RBAC)
FeatherDB now implements a multi-phase RBAC system for enhanced security.

### Phase 1: Authentication (Login/Logout)
Upon startup, as well as whenever you're NOT in a role, all data access is restricted.
- **Login**: Enter a role's secret key (or the admin secret phrase) in the REPL.
- **Logout**: Enter the CURRENT role's secret key (or admin secret phrase) again to log out.
- **Switching**: You MUST logout before logging into a different role.

### Phase 2: Role Management
As an Admin, you can create roles with mandatory secret keys and grant permissions.
```sql
-- Create roles with secret keys
CREATE ROLE alice WITH SECRET 'alice123';
CREATE ROLE bob WITH SECRET 'bob456';

-- Grant permissions (SELECT, INSERT, UPDATE, DELETE)
GRANT SELECT ON foods TO alice;
GRANT SELECT ON users TO bob;
```

### Phase 3: Dynamic Prompt
The REPL prompt will reflect your active role:
- `featherDb> ` (Logged out)
- `featherDb> (admin) ` (Logged in as admin)
- `featherDb> (alice) ` (Logged in as alice)

## 3. Data Integrity
Every table MUST have at least one `PRIMARY KEY` or `REFERENCES` defined.
```sql
CREATE TABLE Users (id INT PRIMARY KEY, email STRING);
```

## 4. Visualization
Use `.erd` to see relationships between your tables.

## 5. Developers: Testing
Run `.\run_tests.bat` frequently to ensure no regressions are introduced.
