#include "StorageManager.h"
#include <fstream>
#include <filesystem>

namespace spl
{

    namespace fs = std::filesystem;

    bool StorageManager::createTable(const std::string &tableName, const std::vector<Column> &columns)
    {
        if (!fs::exists("db"))
        {
            fs::create_directory("db");
        }
        std::string pathPrefix = "db/" + tableName;

        if (fs::exists(pathPrefix + ".schema"))
        {
            return false; // Already exists
        }

        std::ofstream schemaFile(pathPrefix + ".schema");
        if (!schemaFile.is_open())
            return false;

        for (const auto &col : columns)
        {
            schemaFile << col.name << " " << col.type;
            if (col.isPrimaryKey)
            {
                schemaFile << " PK";
            }
            if (!col.fkTargetTable.empty())
            {
                schemaFile << " FK " << col.fkTargetTable;
            }
            schemaFile << "\n";
        }
        schemaFile.close();

        std::ofstream dataFile(pathPrefix + ".csv"); // Empty data file
        if (!dataFile.is_open())
            return false;
        dataFile.close();

        return true;
    }

    Table StorageManager::loadTable(const std::string &tableName)
    {
        Table table;
        table.name = tableName;
        std::string pathPrefix = "db/" + tableName;

        // Load Schema
        std::ifstream schemaFile(pathPrefix + ".schema");
        if (!schemaFile.is_open())
        {
            return table;
        }

        std::string line;
        while (std::getline(schemaFile, line))
        {
            if (line.empty())
                continue;
            std::stringstream ss(line);
            std::string name, type, extra;
            ss >> name >> type;
            if (name.empty())
                continue;

            Column col;
            col.name = name;
            col.type = type;

            while (ss >> extra)
            {
                if (extra == "PK")
                {
                    col.isPrimaryKey = true;
                    table.primaryKey = name;
                }
                else if (extra == "FK")
                {
                    std::string targetTable;
                    if (ss >> targetTable)
                    {
                        col.fkTargetTable = targetTable;
                        table.foreignKeys.push_back({name, targetTable, "id"});
                    }
                }
            }
            table.columns.push_back(col);
        }
        schemaFile.close();

        // Load Data
        std::ifstream dataFile(pathPrefix + ".csv");
        if (dataFile.is_open())
        {
            while (std::getline(dataFile, line))
            {
                if (line.empty())
                    continue;
                // CSV split
                Row row;
                std::stringstream ss(line);
                std::string cell;
                while (std::getline(ss, cell, ','))
                {
                    // Trim cell
                    size_t first = cell.find_first_not_of(" \t\r\n");
                    if (std::string::npos != first)
                    {
                        size_t last = cell.find_last_not_of(" \t\r\n");
                        cell = cell.substr(first, (last - first + 1));
                    }
                    else
                    {
                        cell = "";
                    }
                    row.values.push_back(cell);
                }
                if (!row.values.empty())
                {
                    table.rows.push_back(row);
                }
            }
            dataFile.close();
        }

        return table;
    }

    bool StorageManager::saveTable(const Table &table)
    {
        if (!fs::exists("db"))
        {
            fs::create_directory("db");
        }
        std::string pathPrefix = "db/" + table.name;
        std::ofstream dataFile(pathPrefix + ".csv");
        if (!dataFile.is_open())
            return false;

        for (const auto &row : table.rows)
        {
            for (size_t i = 0; i < row.values.size(); ++i)
            {
                dataFile << row.values[i];
                if (i < row.values.size() - 1)
                {
                    dataFile << ",";
                }
            }
            dataFile << "\n";
        }
        dataFile.close();
        return true;
    }

    bool StorageManager::appendRow(const std::string &tableName, const Row &row)
    {
        if (!fs::exists("db"))
        {
            fs::create_directory("db");
        }
        std::string pathPrefix = "db/" + tableName;
        std::ofstream dataFile(pathPrefix + ".csv", std::ios::app);
        if (!dataFile.is_open())
            return false;

        for (size_t i = 0; i < row.values.size(); ++i)
        {
            dataFile << row.values[i];
            if (i < row.values.size() - 1)
            {
                dataFile << ",";
            }
        }
        dataFile << "\n";
        dataFile.close();
        return true;
    }

    bool StorageManager::dropTable(const std::string &tableName)
    {
        std::string pathPrefix = "db/" + tableName;
        bool s = fs::remove(pathPrefix + ".schema");
        bool d = fs::remove(pathPrefix + ".csv");
        return s && d;
    }

    std::vector<std::string> StorageManager::listTables()
    {
        std::vector<std::string> tables;
        if (!fs::exists("db"))
            return tables;

        for (const auto &entry : fs::directory_iterator("db"))
        {
            if (entry.path().extension() == ".schema")
            {
                tables.push_back(entry.path().stem().string());
            }
        }
        return tables;
    }

    Table StorageManager::getTableSchema(const std::string &tableName)
    {
        Table table;
        table.name = tableName;
        std::string pathPrefix = "db/" + tableName;

        // Load Schema Only
        std::ifstream schemaFile(pathPrefix + ".schema");
        if (!schemaFile.is_open())
        {
            return table;
        }

        std::string line;
        while (std::getline(schemaFile, line))
        {
            std::stringstream ss(line);
            std::string name, type, extra;
            ss >> name >> type;
            if (name.empty())
                continue;

            Column col;
            col.name = name;
            col.type = type;

            while (ss >> extra)
            {
                if (extra == "PK")
                {
                    col.isPrimaryKey = true;
                    table.primaryKey = name;
                }
                else if (extra == "FK")
                {
                    std::string targetTable;
                    if (ss >> targetTable)
                    {
                        col.fkTargetTable = targetTable;
                        table.foreignKeys.push_back({name, targetTable, "id"});
                    }
                }
            }
            table.columns.push_back(col);
        }
        schemaFile.close();
        return table;
    }

    bool StorageManager::saveRole(const std::string &roleName, const std::string &secretKey)
    {
        if (!fs::exists("db"))
            fs::create_directory("db");
        std::ofstream roleFile("db/_roles.csv", std::ios::app);
        if (!roleFile.is_open())
            return false;
        roleFile << roleName << "," << secretKey << "\n";
        roleFile.close();
        return true;
    }

    bool StorageManager::savePermission(const std::string &role, const std::string &action, const std::string &table)
    {
        if (!fs::exists("db"))
            fs::create_directory("db");
        std::ofstream permFile("db/_permissions.csv", std::ios::app);
        if (!permFile.is_open())
            return false;
        permFile << role << "," << action << "," << table << "\n";
        permFile.close();
        return true;
    }

    bool StorageManager::checkPermission(const std::string &role, const std::string &action, const std::string &table)
    {
        if (role == "admin")
            return true;
        std::ifstream permFile("db/_permissions.csv");
        if (!permFile.is_open())
            return false;
        std::string line;
        while (std::getline(permFile, line))
        {
            std::stringstream ss(line);
            std::string r, a, t;
            std::getline(ss, r, ',');
            std::getline(ss, a, ',');
            std::getline(ss, t, ',');
            if (r == role && a == action && t == table)
                return true;
        }
        return false;
    }

    bool StorageManager::roleExists(const std::string &roleName)
    {
        if (roleName == "admin")
            return true;
        std::ifstream roleFile("db/_roles.csv");
        if (!roleFile.is_open())
            return false;
        std::string line;
        while (std::getline(roleFile, line))
        {
            std::stringstream ss(line);
            std::string r, s;
            std::getline(ss, r, ',');
            if (r == roleName)
                return true;
        }
        return false;
    }

    std::string StorageManager::getRoleBySecret(const std::string &secretKey)
    {
        std::ifstream roleFile("db/_roles.csv");
        if (!roleFile.is_open())
            return "";
        std::string line;
        while (std::getline(roleFile, line))
        {
            std::stringstream ss(line);
            std::string r, s;
            std::getline(ss, r, ',');
            std::getline(ss, s, ',');
            if (s == secretKey)
                return r;
        }
        return "";
    }

    std::string StorageManager::getSecretByRole(const std::string &roleName)
    {
        std::ifstream roleFile("db/_roles.csv");
        if (!roleFile.is_open())
            return "";
        std::string line;
        while (std::getline(roleFile, line))
        {
            std::stringstream ss(line);
            std::string r, s;
            std::getline(ss, r, ',');
            std::getline(ss, s, ',');
            if (r == roleName)
                return s;
        }
        return "";
    }

    bool StorageManager::deleteTable(const std::string &tableName)
    {
        std::string csvPath = "db/" + tableName + ".csv";
        std::string schemaPath = "db/" + tableName + ".schema";

        if (!fs::exists(csvPath) && !fs::exists(schemaPath))
        {
            return false;
        }

        bool success = true;
        if (fs::exists(csvPath))
            success &= fs::remove(csvPath);
        if (fs::exists(schemaPath))
            success &= fs::remove(schemaPath);
        return success;
    }

}
