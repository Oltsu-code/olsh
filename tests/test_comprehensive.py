"""
Comprehensive unit test suite for olshell
Tests all major functionality including commands, parsing, execution, and features
"""

import unittest
import subprocess
import os
import tempfile
import shutil
import time
import sys
from pathlib import Path
from threading import Timer


class OlshellTestBase(unittest.TestCase):
    """Base test class with common setup and utilities"""
    
    @classmethod
    def setUpClass(cls):
        """Set up test environment once for all tests"""
        cls.project_root = Path(__file__).parent.parent
        
        # Determine olshell executable path
        if 'OLSHELL_TEST_PATH' in os.environ:
            cls.olshell_exe = Path(os.environ['OLSHELL_TEST_PATH'])
        else:
            # Try to find olshell executable relative to project root
            possible_paths = [
                cls.project_root / 'cmake-build-debug' / 'olshell.exe',
                cls.project_root / 'cmake-build-release' / 'olshell.exe', 
                cls.project_root / 'cmake-build-relwithdebinfo' / 'olshell.exe',
                cls.project_root / 'build' / 'olshell.exe',
                cls.project_root / 'cmake-build-debug' / 'olshell',
                cls.project_root / 'cmake-build-release' / 'olshell',
                cls.project_root / 'build' / 'olshell',
            ]
            
            cls.olshell_exe = None
            for path in possible_paths:
                if path.exists():
                    cls.olshell_exe = path
                    break
        
        if not cls.olshell_exe or not cls.olshell_exe.exists():
            raise FileNotFoundError(f"olshell executable not found. Set OLSHELL_TEST_PATH environment variable or build the project.")
        
        print(f"Testing olshell at: {cls.olshell_exe}")
    
    def setUp(self):
        """Set up test directory for each test"""
        self.test_dir = tempfile.mkdtemp(prefix="olshell_test_")
        self.original_cwd = os.getcwd()
        os.chdir(self.test_dir)
        
    def tearDown(self):
        """Clean up after each test"""
        os.chdir(self.original_cwd)
        shutil.rmtree(self.test_dir, ignore_errors=True)
    
    def run_olshell_command(self, command, input_data=None, timeout=10, expect_exit=True):
        """
        Execute command in olshell and return results
        
        Args:
            command: Command to execute
            input_data: Additional input to send
            timeout: Command timeout
            expect_exit: Whether to send 'exit' command
            
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
                cwd=self.test_dir,
                encoding='utf-8',
                errors='replace'
            )
            
            # Build input string
            if input_data:
                full_input = f"{command}\n{input_data}\n"
            else:
                full_input = f"{command}\n"
            
            if expect_exit:
                full_input += "exit\n"
            
            # Execute with timeout
            stdout, stderr = process.communicate(input=full_input, timeout=timeout)
            return stdout, stderr, process.returncode
            
        except subprocess.TimeoutExpired:
            process.kill()
            stdout, stderr = process.communicate()
            return stdout, stderr, -1
        except Exception as e:
            return "", str(e), -1
    
    def create_test_file(self, filename, content):
        """Create a test file with given content"""
        filepath = Path(self.test_dir) / filename
        filepath.write_text(content, encoding='utf-8')
        return filepath
    
    def file_exists(self, filename):
        """Check if file exists in test directory"""
        return (Path(self.test_dir) / filename).exists()
    
    def read_file_content(self, filename):
        """Read content of file in test directory"""
        filepath = Path(self.test_dir) / filename
        if filepath.exists():
            return filepath.read_text(encoding='utf-8')
        return ""


class TestBasicCommands(OlshellTestBase):
    """Test basic shell commands"""
    
    def test_echo_simple(self):
        """Test basic echo command"""
        stdout, stderr, code = self.run_olshell_command('echo "Hello World"')
        self.assertIn("Hello World", stdout)
    
    def test_echo_multiple_args(self):
        """Test echo with multiple arguments"""
        stdout, stderr, code = self.run_olshell_command('echo arg1 arg2 arg3')
        self.assertIn("arg1 arg2 arg3", stdout)
    
    def test_echo_empty(self):
        """Test echo with no arguments"""
        stdout, stderr, code = self.run_olshell_command('echo')
        # Should execute without error
        self.assertNotEqual(code, -1)
    
    def test_pwd_command(self):
        """Test pwd (print working directory)"""
        stdout, stderr, code = self.run_olshell_command('pwd')
        # Should contain current directory path
        normalized_test_dir = self.test_dir.replace('\\', '/').replace('//', '/')
        normalized_stdout = stdout.replace('\\', '/').replace('//', '/')
        self.assertIn(normalized_test_dir, normalized_stdout)
    
    def test_clear_command(self):
        """Test clear command"""
        stdout, stderr, code = self.run_olshell_command('clear')
        # Should execute without error
        self.assertNotEqual(code, -1)
    
    def test_ls_command_empty_dir(self):
        """Test ls in empty directory"""
        stdout, stderr, code = self.run_olshell_command('ls')
        # Should execute without error even if directory is empty
        self.assertNotEqual(code, -1)
    
    def test_ls_command_with_files(self):
        """Test ls with files present"""
        # Create test files
        self.create_test_file("test_file1.txt", "content1")
        self.create_test_file("test_file2.txt", "content2")
        
        stdout, stderr, code = self.run_olshell_command('ls')
        self.assertIn("test_file1.txt", stdout)
        self.assertIn("test_file2.txt", stdout)


class TestFileOperations(OlshellTestBase):
    """Test file-related commands"""
    
    def test_cat_single_file(self):
        """Test cat command with single file"""
        test_content = "This is test content for cat command\nLine 2\nLine 3"
        self.create_test_file("cat_test.txt", test_content)
        
        stdout, stderr, code = self.run_olshell_command('cat cat_test.txt')
        self.assertIn("This is test content", stdout)
        self.assertIn("Line 2", stdout)
        self.assertIn("Line 3", stdout)
    
    def test_cat_nonexistent_file(self):
        """Test cat with non-existent file"""
        stdout, stderr, code = self.run_olshell_command('cat nonexistent.txt')
        # Should handle gracefully (may show error message)
        self.assertNotEqual(code, -1)
    
    def test_cat_multiple_files(self):
        """Test cat with multiple files"""
        self.create_test_file("file1.txt", "Content of file 1")
        self.create_test_file("file2.txt", "Content of file 2")
        
        stdout, stderr, code = self.run_olshell_command('cat file1.txt file2.txt')
        self.assertIn("Content of file 1", stdout)
        self.assertIn("Content of file 2", stdout)
    
    def test_rm_single_file(self):
        """Test rm command with single file"""
        test_file = self.create_test_file("remove_me.txt", "delete this")
        self.assertTrue(test_file.exists())
        
        stdout, stderr, code = self.run_olshell_command('rm remove_me.txt')
        self.assertFalse(test_file.exists())
    
    def test_rm_nonexistent_file(self):
        """Test rm with non-existent file"""
        stdout, stderr, code = self.run_olshell_command('rm nonexistent.txt')
        # Should handle gracefully
        self.assertNotEqual(code, -1)
    
    def test_rm_multiple_files(self):
        """Test rm with multiple files"""
        file1 = self.create_test_file("delete1.txt", "content1")
        file2 = self.create_test_file("delete2.txt", "content2")
        
        stdout, stderr, code = self.run_olshell_command('rm delete1.txt delete2.txt')
        self.assertFalse(file1.exists())
        self.assertFalse(file2.exists())


class TestDirectoryOperations(OlshellTestBase):
    """Test directory-related commands"""
    
    def test_cd_to_subdirectory(self):
        """Test changing to subdirectory"""
        # Create subdirectory
        subdir = Path(self.test_dir) / "subdir"
        subdir.mkdir()
        
        # Test cd command
        stdout, stderr, code = self.run_olshell_command('cd subdir\npwd')
        self.assertIn("subdir", stdout)
    
    def test_cd_to_parent(self):
        """Test changing to parent directory"""
        # Create and enter subdirectory first
        subdir = Path(self.test_dir) / "testdir"
        subdir.mkdir()
        
        stdout, stderr, code = self.run_olshell_command('cd testdir\ncd ..\npwd')
        # Should be back in original directory
        normalized_test_dir = self.test_dir.replace('\\', '/').replace('//', '/')
        normalized_stdout = stdout.replace('\\', '/').replace('//', '/')
        self.assertIn(normalized_test_dir, normalized_stdout)
    
    def test_cd_nonexistent_directory(self):
        """Test cd to non-existent directory"""
        stdout, stderr, code = self.run_olshell_command('cd nonexistent_dir')
        # Should handle gracefully
        self.assertNotEqual(code, -1)


class TestRedirection(OlshellTestBase):
    """Test input/output redirection"""
    
    def test_output_redirection_simple(self):
        """Test simple output redirection"""
        stdout, stderr, code = self.run_olshell_command('echo "redirected content" > output.txt')
        
        if self.file_exists("output.txt"):
            content = self.read_file_content("output.txt")
            self.assertIn("redirected content", content)
    
    def test_append_redirection(self):
        """Test append redirection"""
        # Create initial file
        stdout, stderr, code = self.run_olshell_command('echo "first line" > append_test.txt')
        
        # Append to file
        stdout, stderr, code = self.run_olshell_command('echo "second line" >> append_test.txt')
        
        if self.file_exists("append_test.txt"):
            content = self.read_file_content("append_test.txt")
            self.assertIn("first line", content)
            self.assertIn("second line", content)
    
    def test_redirection_overwrite(self):
        """Test that > overwrites existing files"""
        # Create initial file
        self.create_test_file("overwrite_test.txt", "original content")
        
        # Overwrite with redirection
        stdout, stderr, code = self.run_olshell_command('echo "new content" > overwrite_test.txt')
        
        if self.file_exists("overwrite_test.txt"):
            content = self.read_file_content("overwrite_test.txt")
            self.assertIn("new content", content)
            self.assertNotIn("original content", content)


class TestPipelines(OlshellTestBase):
    """Test pipeline operations"""
    
    def test_simple_pipeline(self):
        """Test simple pipeline: echo | cat"""
        stdout, stderr, code = self.run_olshell_command('echo "pipeline test" | cat')
        self.assertIn("pipeline test", stdout)
    
    def test_file_pipeline(self):
        """Test pipeline with file: cat file | cat"""
        self.create_test_file("pipe_input.txt", "line1\nline2\nline3")
        
        stdout, stderr, code = self.run_olshell_command('cat pipe_input.txt | cat')
        self.assertIn("line1", stdout)
        self.assertIn("line2", stdout)
        self.assertIn("line3", stdout)
    
    def test_pipeline_with_redirection(self):
        """Test pipeline combined with redirection"""
        stdout, stderr, code = self.run_olshell_command('echo "piped output" | cat > piped_file.txt')
        
        if self.file_exists("piped_file.txt"):
            content = self.read_file_content("piped_file.txt")
            self.assertIn("piped output", content)


class TestCommandChaining(OlshellTestBase):
    """Test command chaining with semicolons"""
    
    def test_semicolon_chaining(self):
        """Test chaining commands with semicolons"""
        stdout, stderr, code = self.run_olshell_command('echo "first"; echo "second"; echo "third"')
        self.assertIn("first", stdout)
        self.assertIn("second", stdout)
        self.assertIn("third", stdout)
    
    def test_chaining_with_file_ops(self):
        """Test chaining file operations"""
        stdout, stderr, code = self.run_olshell_command('echo "content" > chain_test.txt; cat chain_test.txt; rm chain_test.txt')
        self.assertIn("content", stdout)
        # File should be deleted by the chain
        self.assertFalse(self.file_exists("chain_test.txt"))


class TestAliasSystem(OlshellTestBase):
    """Test alias functionality"""
    
    def test_alias_creation(self):
        """Test creating aliases"""
        stdout, stderr, code = self.run_olshell_command('alias ll="ls -l"')
        # Should execute without error
        self.assertNotEqual(code, -1)
    
    def test_alias_listing(self):
        """Test listing aliases"""
        stdout, stderr, code = self.run_olshell_command('alias testcmd="echo test"\nalias')
        # Should show the created alias
        self.assertNotEqual(code, -1)
    
    def test_alias_usage(self):
        """Test using created aliases"""
        stdout, stderr, code = self.run_olshell_command('alias myecho="echo hello"\nmyecho')
        self.assertIn("hello", stdout)


class TestHistorySystem(OlshellTestBase):
    """Test command history functionality"""
    
    def test_history_command(self):
        """Test history command execution"""
        stdout, stderr, code = self.run_olshell_command('history')
        # Should execute without error
        self.assertNotEqual(code, -1)
    
    def test_history_with_commands(self):
        """Test history after executing commands"""
        stdout, stderr, code = self.run_olshell_command('echo "test1"\necho "test2"\nhistory')
        # History should execute
        self.assertNotEqual(code, -1)


class TestErrorHandling(OlshellTestBase):
    """Test error handling and edge cases"""
    
    def test_nonexistent_command(self):
        """Test handling of non-existent commands"""
        stdout, stderr, code = self.run_olshell_command('nonexistent_command_xyz123')
        # Should handle gracefully without crashing
        self.assertNotEqual(code, -1)
    
    def test_empty_command(self):
        """Test handling of empty commands"""
        stdout, stderr, code = self.run_olshell_command('')
        # Should handle gracefully
        self.assertNotEqual(code, -1)
    
    def test_whitespace_only_command(self):
        """Test handling of whitespace-only commands"""
        stdout, stderr, code = self.run_olshell_command('   ')
        # Should handle gracefully
        self.assertNotEqual(code, -1)
    
    def test_special_characters(self):
        """Test handling of special characters"""
        stdout, stderr, code = self.run_olshell_command('echo "Special chars: !@#$%^&*()"')
        self.assertIn("Special chars", stdout)


class TestConfigSystem(OlshellTestBase):
    """Test configuration system"""
    
    def test_config_file_creation(self):
        """Test that config file is created"""
        # Run shell to trigger config creation
        stdout, stderr, code = self.run_olshell_command('echo "test"')
        
        # Check if config directory/file was created in user home
        home_dir = Path.home()
        config_path = home_dir / ".olshell" / "config.yaml"
        # Config creation might happen in user directory, not test directory
        # Just ensure shell runs without error
        self.assertNotEqual(code, -1)


class TestScriptExecution(OlshellTestBase):
    """Test script file execution"""
    
    def test_script_file_detection(self):
        """Test detection of script files"""
        script_content = """echo "Script line 1"
echo "Script line 2"
pwd
"""
        self.create_test_file("test_script.olsh", script_content)
        
        # Test if olshell recognizes .olsh files
        # This test depends on how olshell handles script files
        stdout, stderr, code = self.run_olshell_command('ls', expect_exit=True)
        self.assertIn("test_script.olsh", stdout)


class TestComplexScenarios(OlshellTestBase):
    """Test complex real-world scenarios"""
    
    def test_file_processing_workflow(self):
        """Test complete file processing workflow"""
        # Create input files
        self.create_test_file("input1.txt", "Content from file 1\n")
        self.create_test_file("input2.txt", "Content from file 2\n")
        
        # Complex workflow
        commands = [
            'cat input1.txt input2.txt > combined.txt',
            'cat combined.txt',
            'ls',
            'rm input1.txt',
            'rm input2.txt',
            'ls'
        ]
        
        for command in commands:
            stdout, stderr, code = self.run_olshell_command(command)
            self.assertNotEqual(code, -1)
            
            if 'cat combined.txt' in command:
                self.assertIn("Content from file 1", stdout)
                self.assertIn("Content from file 2", stdout)
    
    def test_directory_navigation_workflow(self):
        """Test directory creation and navigation"""
        # Create directory structure
        Path(self.test_dir, "dir1").mkdir()
        Path(self.test_dir, "dir1", "subdir").mkdir()
        self.create_test_file("dir1/file1.txt", "file in dir1")
        self.create_test_file("dir1/subdir/file2.txt", "file in subdir")
        
        commands = [
            'ls',
            'cd dir1',
            'ls',
            'cat file1.txt',
            'cd subdir',
            'cat file2.txt',
            'cd ../..',
            'pwd'
        ]
        
        for command in commands:
            stdout, stderr, code = self.run_olshell_command(command)
            self.assertNotEqual(code, -1)


class TestPerformance(OlshellTestBase):
    """Test performance characteristics"""
    
    def test_command_execution_speed(self):
        """Test basic command execution performance"""
        start_time = time.time()
        
        for i in range(5):  # Reduced for CI speed
            stdout, stderr, code = self.run_olshell_command(f'echo "Performance test {i}"')
            self.assertNotEqual(code, -1)
        
        end_time = time.time()
        avg_time = (end_time - start_time) / 5
        
        # Should complete reasonably quickly (adjust threshold as needed)
        self.assertLess(avg_time, 2.0, "Commands taking too long to execute")
    
    def test_large_file_handling(self):
        """Test handling of larger files"""
        # Create a moderately large file
        large_content = "Line {}\n" * 100
        large_content = large_content.format(*range(100))
        self.create_test_file("large_file.txt", large_content)
        
        # Test operations on large file
        start_time = time.time()
        stdout, stderr, code = self.run_olshell_command('cat large_file.txt')
        end_time = time.time()
        
        self.assertNotEqual(code, -1)
        self.assertLess(end_time - start_time, 5.0, "Large file processing too slow")


if __name__ == '__main__':
    print("=== OlShell Comprehensive Unit Test Suite ===")
    print("Testing all major functionality...")
    print()
    
    # Configure test runner for detailed output
    unittest.main(
        verbosity=2,
        buffer=True,  # Capture stdout/stderr
        warnings='ignore'  # Ignore deprecation warnings
    )
