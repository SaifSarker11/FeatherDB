
#include "parser/Tokenizer.h"
#include "parser/SQLParser.h"
#include "query/QueryExecutor.h"
#include "storage/StorageManager.h"
#include "utils/Print.h"
#include "utils/ERDViewer.h"
#include "utils/ConfigManager.h"

#define version "1.2.1-FIXED-SECURITY"

using namespace spl;

std::string toLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c)
                   { return std::tolower(c); });
    return s;
}

int main()
{
    printIntro((char *)version);

    ConfigManager config;
    std::string envSecret = config.get("SECRET_PHRASE");
    std::string secretPhrase;

    // Load Security Configuration
    if (!envSecret.empty())
    {
        secretPhrase = toLower(envSecret);
        std::cout << "[System] loaded .env config.\n";
    }
    else
    {
        secretPhrase = "ramadankareem"; // System Default Fallback
        std::cout << "[Security] SECURITY HAZARD!!!\n";
    }

    std::string input;
    std::string currentRole = "";
    while (true)
    {
        printPrompt(currentRole);
        if (!std::getline(std::cin, input))
            break;

        if (input.empty())
            continue;

        std::string lowerInput = toLower(input);

        if (lowerInput == secretPhrase || !StorageManager::getRoleBySecret(input).empty())
        {
            std::string matchingRole = (lowerInput == secretPhrase) ? "admin" : StorageManager::getRoleBySecret(input);

            if (currentRole == matchingRole)
            {
                currentRole = "";
                if (matchingRole == "admin")
                    std::cout << "Server error? :p\n";
                else
                    std::cout << "Logged out.\n";
            }
            else if (currentRole.empty())
            {
                currentRole = matchingRole;
                if (matchingRole == "admin")
                    std::cout << "[Security] Authenticated as Admin. Full access granted.\n";
                else
                    std::cout << "Logged in as " << currentRole << ".\n";
            }
            else
            {
                std::cout << "Please logout from '" << currentRole << "' before switching roles.\n";
            }
            continue;
        }

        if (input[0] == '.')
        {
            if (lowerInput == ".exit")
            {
                break;
            }
            else if (lowerInput == ".help")
            {
                printHelp();
            }
            else if (currentRole.empty())
            {
                std::cout << "Server error :p\n";
                continue;
            }
            else if (lowerInput == ".tables")
            {
                std::vector<std::string> tables = StorageManager::listTables();
                for (const auto &t : tables)
                {
                    std::cout << t << "\n";
                }
            }
            else if (lowerInput == ".erd")
            {
                ERDViewer::printERD();
            }
            else if (lowerInput.rfind(".schema", 0) == 0)
            {
                std::stringstream ss(input);
                std::string cmd, name;
                ss >> cmd >> name;
                if (name.empty())
                {
                    std::cout << "Usage: .schema <table_name>\n";
                }
                else
                {
                    Table t = StorageManager::getTableSchema(name);
                    if (t.name.empty() && t.columns.empty())
                    {
                        std::cout << "Table '" << name << "' not found.\n";
                    }
                    else
                    {
                        std::cout << "CREATE TABLE " << t.name << " (\n";
                        for (size_t i = 0; i < t.columns.size(); ++i)
                        {
                            std::cout << "    " << std::left << std::setw(15) << t.columns[i].name
                                      << std::left << std::setw(10) << t.columns[i].type;
                            if (t.columns[i].isPrimaryKey)
                                std::cout << " PRIMARY KEY";
                            if (t.columns[i].isUnique)
                                std::cout << " UNIQUE";
                            if (!t.columns[i].fkTargetTable.empty())
                                std::cout << " REFERENCES " << t.columns[i].fkTargetTable;

                            if (i < t.columns.size() - 1)
                            {
                                std::cout << ",";
                            }
                            std::cout << "\n";
                        }
                        std::cout << ");\n";
                    }
                }
            }
            else
            {
                std::cout << "Unknown command: " << input << "\n";
            }
            continue;
        }

        // SQL Execution
        try
        {
            Tokenizer tokenizer(input);
            SQLParser parser(tokenizer);
            std::unique_ptr<AST> ast = parser.parse();

            if (currentRole.empty())
            {
                std::cout << "Server error :p\n";
                continue;
            }

            QueryExecutor executor;
            executor.execute(std::move(ast), currentRole);
        }
        catch (const std::exception &e)
        {
            if (currentRole.empty())
            {
                std::cout << "Server error :p\n";
            }
            else
            {
                std::cout << "Error: " << e.what() << "\n";
            }
        }
    }

    return 0;
}
