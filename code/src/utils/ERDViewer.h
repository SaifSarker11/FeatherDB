#ifndef ERD_VIEWER_H
#define ERD_VIEWER_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include "../storage/StorageManager.h"

namespace spl {

class ERDViewer {
public:
    static void printERD() {
        std::vector<std::string> tableNames = StorageManager::listTables();
        if (tableNames.empty()) {
            std::cout << "No tables found to display ERD.\n";
            return;
        }

        std::cout << "\n" << std::string(80, '=') << "\n";
        std::cout << std::left << std::setw(25) << "TABLE" 
                  << std::left << std::setw(15) << "RELATION" 
                  << std::left << std::setw(25) << "TARGET TABLE" 
                  << "COLUMN\n";
        std::cout << std::string(80, '-') << "\n";

        for (const auto& name : tableNames) {
            Table t = StorageManager::getTableSchema(name);
            
            // Check if it's a junction table (N:N): all columns are FKs
            bool isJunction = true;
            for (const auto& col : t.columns) {
                if (col.fkTargetTable.empty()) {
                    isJunction = false;
                    break;
                }
            }
            if (t.columns.empty()) isJunction = false;

            bool hasRel = false;
            for (const auto& col : t.columns) {
                if (!col.fkTargetTable.empty()) {
                    std::string relType;
                    if (isJunction) {
                        relType = "N:N";
                    } else {
                        relType = (col.isPrimaryKey || col.isUnique) ? "1:1" : "1:N";
                    }

                    std::cout << std::left << std::setw(25) << t.name 
                              << std::left << std::setw(15) << relType 
                              << std::left << std::setw(25) << col.fkTargetTable 
                              << col.name << "\n";
                    hasRel = true;
                }
            }
            if (!hasRel) {
                std::cout << std::left << std::setw(25) << t.name 
                          << std::left << std::setw(15) << "None" 
                          << std::left << std::setw(25) << "-" 
                          << "-\n";
            }
        }
        std::cout << std::string(80, '=') << "\n\n";
    }
};

} 

#endif 
