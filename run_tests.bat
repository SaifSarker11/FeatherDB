@echo off
if not exist build mkdir build

echo === Compiling tests ===

echo [1/5] TokenizerTest...
g++ -std=c++17 -I src tests/parser/TokenizerTest.cpp src/parser/Tokenizer.cpp -o build/TokenizerTest.exe
if %errorlevel% neq 0 ( echo COMPILE FAILED & exit /b %errorlevel% )

echo [2/5] ASTTest...
g++ -std=c++17 -I src tests/parser/ASTTest.cpp src/parser/AST.cpp -o build/ASTTest.exe
if %errorlevel% neq 0 ( echo COMPILE FAILED & exit /b %errorlevel% )

echo [3/5] SQLParserTest...
g++ -std=c++17 -I src tests/parser/SQLParserTest.cpp src/parser/Tokenizer.cpp src/parser/SQLParser.cpp src/parser/AST.cpp -o build/SQLParserTest.exe
if %errorlevel% neq 0 ( echo COMPILE FAILED & exit /b %errorlevel% )

echo [4/5] StorageManagerTest...
g++ -std=c++17 -I src tests/storage/StorageManagerTest.cpp src/storage/StorageManager.cpp -o build/StorageManagerTest.exe
if %errorlevel% neq 0 ( echo COMPILE FAILED & exit /b %errorlevel% )

echo [5/5] QueryExecutorTest...
g++ -std=c++17 -I src tests/query/QueryExecutorTest.cpp src/parser/Tokenizer.cpp src/parser/SQLParser.cpp src/parser/AST.cpp src/query/QueryExecutor.cpp src/storage/StorageManager.cpp src/utils/Print.cpp src/utils/Validators.cpp -o build/QueryExecutorTest.exe
if %errorlevel% neq 0 ( echo COMPILE FAILED & exit /b %errorlevel% )

echo [6/6] ConfigManagerTest...
g++ -std=c++17 -I src tests/utils/ConfigManagerTest.cpp -o build/ConfigManagerTest.exe
if %errorlevel% neq 0 ( echo COMPILE FAILED & exit /b %errorlevel% )

echo [7/7] PrintTest...
g++ -std=c++17 -I src tests/utils/PrintTest.cpp src/utils/Print.cpp -o build/PrintTest.exe
if %errorlevel% neq 0 ( echo COMPILE FAILED & exit /b %errorlevel% )

echo.
echo === Running tests ===
build\TokenizerTest.exe
build\ASTTest.exe
build\SQLParserTest.exe
build\StorageManagerTest.exe
build\QueryExecutorTest.exe
build\ConfigManagerTest.exe
build\PrintTest.exe

echo.
echo === All test suites completed ===
