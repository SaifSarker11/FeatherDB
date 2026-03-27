#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>

namespace spl {

class ConfigManager {
private:
    std::map<std::string, std::string> configs;

    std::string trim(const std::string& s) {
        auto start = s.begin();
        while (start != s.end() && std::isspace(*start)) {
            start++;
        }
        auto end = s.end();
        do {
            end--;
        } while (std::distance(start, end) > 0 && std::isspace(*end));
        return std::string(start, end + 1);
    }

public:
    ConfigManager(const std::string& filename = ".env") {
        std::ifstream file(filename);
        if (!file.is_open()) return;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = trim(line.substr(0, delimiterPos));
                std::string value = trim(line.substr(delimiterPos + 1));
                configs[key] = value;
            }
        }
    }

    std::string get(const std::string& key, const std::string& defaultValue = "") {
        if (configs.find(key) != configs.end()) {
            return configs[key];
        }
        return defaultValue;
    }
};

} 

#endif 
