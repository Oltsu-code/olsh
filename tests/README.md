# OlShell Testing

This directory contains comprehensive test suites for the OlShell project.

## Test Structure

### Test Files

- **`test_comprehensive.py`** - Main functionality tests covering core shell features
- **`test_input_handling.py`** - Input handling and terminal behavior tests  
- **`test_performance.py`** - Performance characteristics and stress tests
- **`test_edge_cases.py`** - Edge cases, boundary conditions, and error handling
- **`run_tests.py`** - Test runner for executing all or specific test categories

### Test Base Class

All test files inherit from `OlshellTestBase` which provides:

- Automatic setup/teardown of test directories
- Helper methods for running shell commands
- File creation and verification utilities
- Cross-platform executable path detection

## Running Tests

### Prerequisites

1. **Build OlShell**: Ensure the shell is built in one of these directories:
   - `build/olshell` or `build/olshell.exe`
   - `cmake-build-debug/olshell` or `cmake-build-debug/olshell.exe`
   - `cmake-build-release/olshell` or `cmake-build-release/olshell.exe`
   - `cmake-build-relwithdebinfo/olshell` or `cmake-build-relwithdebinfo/olshell.exe`

2. **Python Requirements**: Python 3.7+ with standard library modules

### Running All Tests

```bash
cd tests
python run_tests.py
```

### Running Specific Test Categories

```bash
# Core functionality tests
python run_tests.py comprehensive

# Input handling tests  
python run_tests.py input

# Performance tests
python run_tests.py performance

# Edge case tests
python run_tests.py edge
```

### Running Individual Test Files

```bash
python test_comprehensive.py
python test_input_handling.py
python test_performance.py
python test_edge_cases.py
```

### Running with Custom Executable Path

Set the `OLSHELL_TEST_PATH` environment variable to specify a custom path:

```bash
# Unix/Linux/macOS
export OLSHELL_TEST_PATH=/path/to/olshell
python run_tests.py

# Windows PowerShell
$env:OLSHELL_TEST_PATH = "C:\path\to\olshell.exe"
python run_tests.py

# Windows Command Prompt
set OLSHELL_TEST_PATH=C:\path\to\olshell.exe
python run_tests.py
```

## Test Categories

### Comprehensive Tests (`test_comprehensive.py`)

- **Basic Commands**: `echo`, `pwd`, `ls`, `cd`, `cat`, `rm`
- **File Operations**: Creating, reading, writing, deleting files
- **Directory Operations**: Navigation, listing, creation
- **Pipeline Operations**: Command chaining with pipes (`|`)
- **Redirection**: Output redirection (`>`, `>>`)
- **Alias System**: Creating and using command aliases
- **History**: Command history functionality
- **Error Handling**: Invalid commands and error recovery

### Input Handling Tests (`test_input_handling.py`)

- **Ctrl+C Behavior**: Interrupt handling that stays in shell
- **Tab Completion**: Autocomplete functionality
- **Command History**: History navigation and recall
- **Prompt Handling**: Custom prompts and formatting
- **Color Output**: ANSI color code handling
- **Interactive Features**: Real-time input processing

### Performance Tests (`test_performance.py`)

- **Startup Time**: Shell initialization performance
- **Command Execution Speed**: Response time for various commands
- **Memory Usage**: Stability under repeated operations
- **Large File Handling**: Operations on substantial files
- **Concurrent Operations**: Multiple file operations in sequence
- **Stress Scenarios**: Complex pipeline chains and mixed operations

### Edge Case Tests (`test_edge_cases.py`)

- **Boundary Conditions**: Empty commands, whitespace-only input
- **Error Conditions**: Nonexistent commands and files
- **Unusual Inputs**: Special characters, unicode content, long paths
- **Resource Limits**: Very large outputs, many files, deep directories
- **Signal Handling**: Interrupt and error recovery testing
- **Malformed Input**: Invalid syntax and circular operations

## Test Environment

### Isolation

Each test runs in an isolated temporary directory to prevent interference:

- Temporary directory created for each test
- Automatic cleanup after test completion
- No pollution of actual file system
- Parallel test execution safety

### Cross-Platform Support

Tests are designed to work across platforms:

- Windows (`.exe` executable detection)
- Unix/Linux/macOS (executable without extension)
- Platform-specific path handling
- Graceful handling of platform differences

### Error Handling

Robust error handling throughout test suite:

- Graceful failures when executable not found
- Timeout protection for hanging commands
- Clear error messages and diagnostics
- Proper cleanup even when tests fail

## Test Output

### Verbose Mode

Tests run with detailed output showing:

- Individual test results
- Timing information
- Failure details and stack traces
- Summary statistics

### Exit Codes

- `0`: All tests passed
- `1`: Some tests failed or errors occurred

### GitHub Actions Integration

Tests integrate with GitHub Actions CI/CD:

- Multi-platform testing (Ubuntu, Windows, macOS)
- Multiple build configurations (Debug, Release)
- Automated test execution on push/PR
- Code quality checks and static analysis

## Troubleshooting

### Common Issues

1. **"olshell executable not found"**
   - Build the project first: `cmake --build build`
   - Set `OLSHELL_TEST_PATH` environment variable
   - Check executable permissions on Unix systems

2. **Tests timeout or hang**
   - Check if shell is built correctly
   - Verify shell responds to basic commands
   - Look for infinite loops in shell logic

3. **Permission errors**
   - Ensure test directory is writable
   - Check executable permissions
   - Run with appropriate user privileges

### Debug Mode

For debugging test failures:

```bash
# Run single test with maximum verbosity
python -m unittest test_comprehensive.TestBasicCommands.test_echo_command -v

# Add print statements to understand test flow
python -c "
import test_comprehensive
test = test_comprehensive.TestBasicCommands()
test.setUpClass()
test.setUp()
print('Test dir:', test.test_dir)
print('Executable:', test.olshell_exe)
"
```

## Contributing Tests

### Adding New Tests

1. Choose appropriate test file based on category
2. Inherit from `OlshellTestBase`
3. Use descriptive test method names starting with `test_`
4. Include docstrings explaining test purpose
5. Use helper methods for common operations
6. Clean up any created resources

### Test Guidelines

- **Isolation**: Each test should be independent
- **Clarity**: Clear test names and descriptions
- **Robustness**: Handle platform differences gracefully
- **Performance**: Avoid unnecessarily slow operations
- **Coverage**: Test both success and failure cases

### Example Test

```python
def test_new_feature(self):
    \"\"\"Test description of what this validates\"\"\"
    # Setup
    self.create_test_file("input.txt", "test content")
    
    # Execute
    stdout, stderr, code = self.run_olshell_command("new_command input.txt")
    
    # Verify
    self.assertNotEqual(code, -1)
    self.assertIn("expected output", stdout)
    self.assertTrue(self.file_exists("output.txt"))
```
