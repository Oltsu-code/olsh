// test_main.cpp - Main test runner for olshell comprehensive tests
// Sets up the testing environment and handles common test fixtures

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

class OlshellTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        // Set up UTF-8 console for Windows tests
        #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
        #endif
        
        // Create test directory
        test_dir = std::filesystem::temp_directory_path() / "olshell_tests";
        std::filesystem::create_directories(test_dir);
        
        // Save original directory
        original_dir = std::filesystem::current_path();
        
        std::cout << "Test environment initialized\n";
        std::cout << "Test directory: " << test_dir << "\n";
    }
    
    void TearDown() override {
        // Restore original directory
        std::filesystem::current_path(original_dir);
        
        // Clean up test directory
        std::error_code ec;
        std::filesystem::remove_all(test_dir, ec);
        if (ec) {
            std::cerr << "Warning: Could not clean up test directory: " << ec.message() << "\n";
        }
        
        std::cout << "Test environment cleaned up\n";
    }
    
    static std::filesystem::path test_dir;
    static std::filesystem::path original_dir;
};

// Static member definitions
std::filesystem::path OlshellTestEnvironment::test_dir;
std::filesystem::path OlshellTestEnvironment::original_dir;

// Base test fixture for olshell tests
class OlshellTestBase : public ::testing::Test {
protected:
    void SetUp() override {
        // Change to test directory for each test
        std::filesystem::current_path(OlshellTestEnvironment::test_dir);
        
        // Create test-specific subdirectory
        test_subdir = OlshellTestEnvironment::test_dir / ("test_" + std::to_string(test_counter++));
        std::filesystem::create_directories(test_subdir);
        std::filesystem::current_path(test_subdir);
    }
    
    void TearDown() override {
        // Return to test root directory
        std::filesystem::current_path(OlshellTestEnvironment::test_dir);
    }
    
    // Helper function to create test files
    void createTestFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        file << content;
        file.close();
    }
    
    // Helper function to read file content
    std::string readTestFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return "";
        
        std::string content;
        std::string line;
        while (std::getline(file, line)) {
            content += line + "\n";
        }
        return content;
    }
    
    // Helper function to check if file exists
    bool fileExists(const std::string& filename) {
        return std::filesystem::exists(filename);
    }
    
    std::filesystem::path test_subdir;
    static int test_counter;
};

int OlshellTestBase::test_counter = 0;

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    // Add our test environment
    ::testing::AddGlobalTestEnvironment(new OlshellTestEnvironment);
    
    std::cout << "=== OlShell Comprehensive Unit Test Suite ===\n";
    std::cout << "Running all unit tests...\n\n";
    
    int result = RUN_ALL_TESTS();
    
    std::cout << "\n=== Test Suite Complete ===\n";
    if (result == 0) {
        std::cout << "✅ All tests passed!\n";
    } else {
        std::cout << "❌ Some tests failed. Check output above.\n";
    }
    
    return result;
}
