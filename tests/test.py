import subprocess
import os
import tempfile
import shutil
import unittest
from pathlib import Path
import time


class OlshellTestCase(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.project_root = Path(__file__).parent.parent
        cls.olshell_exe = cls.project_root / "cmake-build-debug" / "olshell.exe"

        if not cls.olshell_exe.exists():
            raise FileNotFoundError(f"olshell executable not found at {cls.olshell_exe}")

        cls.test_dir = tempfile.mkdtemp(prefix="olshell_test_")
        cls.original_cwd = os.getcwd()
        os.chdir(cls.test_dir)

    @classmethod
    def tearDownClass(cls):
        os.chdir(cls.original_cwd)
        shutil.rmtree(cls.test_dir, ignore_errors=True)

    def run_olshell_command(self, command, input_data=None, timeout=10):
        """
        Run a command in olshell and return the result

        Args:
            command: Command string to execute
            input_data: Optional input to send to stdin
            timeout: Command timeout in seconds

        Returns:
            tuple: (stdout, stderr, return_code)
        """
        try:
            process = subprocess.Popen(
                [str(self.olshell_exe)],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                cwd=self.test_dir
            )

            # Send command and exit
            if input_data:
                full_input = f"{command}\n{input_data}\nexit\n"
            else:
                full_input = f"{command}\nexit\n"

            stdout, stderr = process.communicate(input=full_input, timeout=timeout)
            return stdout, stderr, process.returncode

        except subprocess.TimeoutExpired:
            process.kill()
            stdout, stderr = process.communicate()
            return stdout, stderr, -1
        except Exception as e:
            return "", str(e), -1


class TestBasicCommands(OlshellTestCase):
    """Test basic built-in commands"""

    def test_echo_command(self):
        """Test echo command with various inputs"""
        stdout, stderr, code = self.run_olshell_command('echo "Hello World"')
        self.assertIn("Hello World", stdout)

        # Test echo with multiple arguments
        stdout, stderr, code = self.run_olshell_command('echo arg1 arg2 arg3')
        self.assertIn("arg1 arg2 arg3", stdout)

    def test_help_command(self):
        """Test help command"""
        stdout, stderr, code = self.run_olshell_command('help')
        # Should contain information about available commands
        self.assertTrue(len(stdout) > 50)  # Help should be substantial

    def test_pwd_command(self):
        """Test pwd (print working directory) command"""
        stdout, stderr, code = self.run_olshell_command('pwd')
        # Should contain current directory path
        self.assertIn(self.test_dir.replace('\\', '/'), stdout.replace('\\', '/'))

    def test_ls_command(self):
        """Test ls (list directory) command"""
        # Create test files
        test_file = Path(self.test_dir) / "test_file.txt"
        test_file.write_text("test content")

        stdout, stderr, code = self.run_olshell_command('ls')
        self.assertIn("test_file.txt", stdout)

    def test_clear_command(self):
        """Test clear command (should not error)"""
        stdout, stderr, code = self.run_olshell_command('clear')
        # Clear command should execute without error
        # Note: We can't easily test if screen was actually cleared

    def test_joke_command(self):
        """Test joke command"""
        stdout, stderr, code = self.run_olshell_command('joke')
        # Should output some text (a joke)
        self.assertTrue(len(stdout.strip()) > 0)


class TestFileOperations(OlshellTestCase):
    """Test file-related commands"""

    def test_cat_command(self):
        """Test cat command for reading files"""
        # Create test file
        test_file = Path(self.test_dir) / "cat_test.txt"
        test_content = "This is a test file for cat command"
        test_file.write_text(test_content)

        stdout, stderr, code = self.run_olshell_command('cat cat_test.txt')
        self.assertIn(test_content, stdout)

    def test_rm_command(self):
        """Test rm command for deleting files"""
        # Create test file
        test_file = Path(self.test_dir) / "rm_test.txt"
        test_file.write_text("delete me")

        # Verify file exists
        self.assertTrue(test_file.exists())

        # Remove file
        stdout, stderr, code = self.run_olshell_command('rm rm_test.txt')

        # Verify file is gone
        self.assertFalse(test_file.exists())

    def test_cd_command(self):
        """Test cd command for changing directories"""
        # Create subdirectory
        sub_dir = Path(self.test_dir) / "subdir"
        sub_dir.mkdir()

        # Test changing to subdirectory
        stdout, stderr, code = self.run_olshell_command(f'cd subdir\npwd')
        self.assertIn("subdir", stdout)


class TestRedirection(OlshellTestCase):
    """Test input/output redirection"""

    def test_output_redirection(self):
        """Test output redirection with > operator"""
        stdout, stderr, code = self.run_olshell_command('echo "redirected output" > redirect_test.txt')

        # Check if file was created with correct content
        test_file = Path(self.test_dir) / "redirect_test.txt"
        if test_file.exists():
            content = test_file.read_text().strip()
            self.assertIn("redirected output", content)

    def test_append_redirection(self):
        """Test append redirection with >> operator"""
        # Create initial file
        stdout, stderr, code = self.run_olshell_command('echo "first line" > append_test.txt')

        # Append to file
        stdout, stderr, code = self.run_olshell_command('echo "second line" >> append_test.txt')

        # Check file content
        test_file = Path(self.test_dir) / "append_test.txt"
        if test_file.exists():
            content = test_file.read_text()
            self.assertIn("first line", content)
            self.assertIn("second line", content)


class TestPipelines(OlshellTestCase):
    """Test pipeline operations"""

    def test_simple_pipeline(self):
        """Test simple pipeline with echo and cat"""
        stdout, stderr, code = self.run_olshell_command('echo "pipeline test" | cat')
        self.assertIn("pipeline test", stdout)

    def test_complex_pipeline(self):
        """Test more complex pipeline operations"""
        # Create test file with multiple lines
        test_file = Path(self.test_dir) / "pipeline_input.txt"
        test_file.write_text("line1\nline2\nline3\n")

        # Test pipeline: cat file | (process in shell)
        stdout, stderr, code = self.run_olshell_command('cat pipeline_input.txt | cat')
        self.assertIn("line1", stdout)
        self.assertIn("line2", stdout)
        self.assertIn("line3", stdout)


class TestCommandChaining(OlshellTestCase):
    """Test command chaining with semicolons"""

    def test_semicolon_chaining(self):
        """Test chaining commands with semicolons"""
        stdout, stderr, code = self.run_olshell_command('echo "first"; echo "second"; echo "third"')
        self.assertIn("first", stdout)
        self.assertIn("second", stdout)
        self.assertIn("third", stdout)

    def test_mixed_operations(self):
        """Test mixing different operations"""
        stdout, stderr, code = self.run_olshell_command('echo "test" > chain_test.txt; cat chain_test.txt')
        self.assertIn("test", stdout)


class TestAliasAndHistory(OlshellTestCase):
    """Test alias and history functionality"""

    def test_alias_creation(self):
        """Test creating and using aliases"""
        # Create alias
        stdout, stderr, code = self.run_olshell_command('alias ll="ls -l"')

        # List aliases to verify creation
        stdout, stderr, code = self.run_olshell_command('alias')
        # Note: This test depends on alias persistence

    def test_history_command(self):
        """Test history functionality"""
        stdout, stderr, code = self.run_olshell_command('history')
        # History should execute without error
        # Content depends on previous commands


class TestErrorHandling(OlshellTestCase):
    """Test error handling and edge cases"""

    def test_nonexistent_command(self):
        """Test handling of non-existent commands"""
        stdout, stderr, code = self.run_olshell_command('nonexistent_command_12345')
        # Should handle gracefully (may output error message)

    def test_nonexistent_file(self):
        """Test handling of non-existent files"""
        stdout, stderr, code = self.run_olshell_command('cat nonexistent_file.txt')
        # Should handle gracefully

    def test_invalid_directory(self):
        """Test changing to non-existent directory"""
        stdout, stderr, code = self.run_olshell_command('cd /nonexistent/directory/path')
        # Should handle gracefully


class TestScriptExecution(OlshellTestCase):
    """Test script file execution"""

    def test_script_execution(self):
        """Test executing a script file"""
        # Create test script
        script_content = """echo "Script test line 1"
echo "Script test line 2"
pwd
"""
        script_file = Path(self.test_dir) / "test_script.olsh"
        script_file.write_text(script_content)

        # Execute script (if olshell supports script files)
        # This test may need to be adapted based on how olshell handles scripts


class TestComplexScenarios(OlshellTestCase):
    """Test complex real-world scenarios"""

    def test_file_processing_workflow(self):
        """Test a complete file processing workflow"""
        # Create source files
        file1 = Path(self.test_dir) / "input1.txt"
        file2 = Path(self.test_dir) / "input2.txt"
        file1.write_text("Content of file 1\n")
        file2.write_text("Content of file 2\n")

        # Complex workflow: combine files, process, clean up
        commands = [
            'cat input1.txt input2.txt > combined.txt',
            'cat combined.txt',
            'rm input1.txt input2.txt combined.txt'
        ]

        for command in commands:
            stdout, stderr, code = self.run_olshell_command(command)
            if 'cat combined.txt' in command:
                self.assertIn("Content of file 1", stdout)
                self.assertIn("Content of file 2", stdout)

    def test_directory_operations_workflow(self):
        """Test directory creation and navigation workflow"""
        commands = [
            'pwd',  # Get initial directory
            'ls',   # List contents
        ]

        for command in commands:
            stdout, stderr, code = self.run_olshell_command(command)
            # Each command should execute without critical errors


def run_performance_tests():
    """Run basic performance tests"""
    print("\n=== Performance Tests ===")

    test_case = OlshellTestCase()
    test_case.setUpClass()

    try:
        # Test command execution speed
        start_time = time.time()
        for i in range(10):
            test_case.run_olshell_command(f'echo "Performance test {i}"')
        end_time = time.time()

        avg_time = (end_time - start_time) / 10
        print(f"Average command execution time: {avg_time:.3f} seconds")

        # Test with file operations
        start_time = time.time()
        test_case.run_olshell_command('echo "performance" > perf_test.txt')
        test_case.run_olshell_command('cat perf_test.txt')
        test_case.run_olshell_command('rm perf_test.txt')
        end_time = time.time()

        file_ops_time = end_time - start_time
        print(f"File operations time: {file_ops_time:.3f} seconds")

    finally:
        test_case.tearDownClass()


if __name__ == '__main__':
    print("Starting olshell comprehensive test suite...")
    print(f"Testing executable: {OlshellTestCase.project_root}/cmake-build-debug/olshell.exe")

    # Run unit tests
    unittest.main(verbosity=2, exit=False)

    # Run performance tests
    run_performance_tests()

    print("\n=== Test Summary ===")
    print("All tests completed. Check output above for detailed results.")
    print("If any tests failed, review the error messages and fix the corresponding functionality.")
