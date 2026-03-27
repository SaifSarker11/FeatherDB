@echo off
if not exist build mkdir build
g++ -std=c++17 -I src src/main.cpp src/parser/AST.cpp src/parser/Tokenizer.cpp src/parser/SQLParser.cpp src/storage/StorageManager.cpp src/query/QueryExecutor.cpp src/utils/Print.cpp  -o build/featherdb.exe
echo Build complete. Executable in build/featherdb.exe
