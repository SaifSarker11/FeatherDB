#ifndef STORAGE_STRUCTS_H
#define STORAGE_STRUCTS_H

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cstdint>

namespace spl
{

    using Id = uint32_t;

    struct Column
    {
        std::string name;
        std::string type; // "INT", "STRING"
        bool isPrimaryKey = false;
        bool isUnique = false;
        std::string fkTargetTable = ""; // Empty if not a foreign key
    };

    struct Row
    {
        std::vector<std::string> values;
    };

    struct ForeignKey
    {
        std::string columnName;
        std::string targetTable;
        std::string targetColumn;
    };

    class Table
    {
    public:
        std::string name;
        std::vector<Column> columns;
        std::vector<Row> rows;

        std::string primaryKey = "";
        std::vector<ForeignKey> foreignKeys;

        void print() const
        {
            // Simple pretty print
            for (const auto &col : columns)
            {
                std::cout << std::left << std::setw(15) << col.name;
            }
            std::cout << "\n";
            for (const auto &col : columns)
            {
                std::cout << "---------------";
            }
            std::cout << "\n";
            for (const auto &row : rows)
            {
                for (const auto &val : row.values)
                {
                    std::cout << std::left << std::setw(15) << val;
                }
                std::cout << "\n";
            }
        }
    };

} 

#endif 
