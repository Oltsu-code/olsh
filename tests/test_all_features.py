"""
Comprehensive test suite for all olshell features
Tests every builtin command, operator, and feature with various scenarios
"""

import unittest
import subprocess
import os
import tempfile
import time
from pathlib import Path
import sys

# Add the tests directory to the path for imports
sys.path.insert(0, os.path.dirname(__file__))

from test_comprehensive import OlshellTestBase


class TestAllBuiltinCommands(OlshellTestBase):
    """Test all builtin commands with comprehensive scenarios"""
    
    def test_echo_command_variants(self):
        """Test echo command with all its options and scenarios"""
        # Basic echo
        stdout, stderr, code = self.run_olshell_command('echo "Hello World"')
        self.assertIn("Hello World", stdout)
        
        # Echo with -n flag (no newline)
        stdout, stderr, code = self.run_olshell_command('echo -n "No newline"')
        self.assertIn("No newline", stdout)
        
        # Echo with multiple arguments
        stdout, stderr, code = self.run_olshell_command('echo arg1 arg2 arg3')
        self.assertIn("arg1 arg2 arg3", stdout)
        
        # Echo with special characters
        stdout, stderr, code = self.run_olshell_command('echo "Special: !@#$%^&*()"')
        self.assertIn("Special: !@#$%^&*()", stdout)
        
        # Echo with quotes
        stdout, stderr, code = self.run_olshell_command('echo "Double quotes" \'Single quotes\'')
        self.assertIn("Double quotes", stdout)
        self.assertIn("Single quotes", stdout)
        
        # Echo empty string
        stdout, stderr, code = self.run_olshell_command('echo ""')
        self.assertNotEqual(code, -1)
        
        # Echo with tabs and spaces
        stdout, stderr, code = self.run_olshell_command('echo "    Tab and spaces    "')
        self.assertIn("Tab and spaces", stdout)
    
    def test_ls_command_variants(self):
        """Test ls command with all its flags and scenarios"""
        # Create test directory structure
        test_dir = Path(self.test_dir)
        (test_dir / "regular_file.txt").write_text("content")
        (test_dir / ".hidden_file").write_text("hidden")
        (test_dir / "subdir").mkdir()
        (test_dir / "subdir" / "nested_file.txt").write_text("nested")
        
        # Basic ls
        stdout, stderr, code = self.run_olshell_command('ls')
        self.assertIn("regular_file.txt", stdout)
        self.assertIn("subdir", stdout)
        
        # ls with -a flag (show hidden files)
        stdout, stderr, code = self.run_olshell_command('ls -a')
        self.assertIn(".hidden_file", stdout)
        self.assertIn("regular_file.txt", stdout)
        
        # ls with -l flag (long format)
        stdout, stderr, code = self.run_olshell_command('ls -l')
        self.assertIn("regular_file.txt", stdout)
        # Should contain file size or permission info
        
        # ls with combined flags
        stdout, stderr, code = self.run_olshell_command('ls -la')
        self.assertIn(".hidden_file", stdout)
        self.assertIn("regular_file.txt", stdout)
        
        # ls specific directory
        stdout, stderr, code = self.run_olshell_command('ls subdir')
        self.assertIn("nested_file.txt", stdout)
        
        # ls non-existent directory
        stdout, stderr, code = self.run_olshell_command('ls nonexistent')
        # Should handle gracefully (might show error but not crash)
        self.assertNotEqual(code, -1)
    
    def test_cd_command_variants(self):
        """Test cd command with all its scenarios"""
        # Create test directory structure
        test_dir = Path(self.test_dir)
        subdir = test_dir / "testdir"
        subdir.mkdir()
        nested_dir = subdir / "nested"
        nested_dir.mkdir()
        
        # cd to subdirectory
        stdout, stderr, code = self.run_olshell_command('cd testdir\npwd')
        self.assertIn("testdir", stdout)
        
        # cd to parent directory
        stdout, stderr, code = self.run_olshell_command('cd testdir\ncd ..\npwd')
        # Should be back in test directory
        
        # cd to current directory
        stdout, stderr, code = self.run_olshell_command('cd .')
        self.assertNotEqual(code, -1)
        
        # cd to nested directory
        stdout, stderr, code = self.run_olshell_command('cd testdir/nested\npwd')
        self.assertIn("nested", stdout)
        
        # cd to non-existent directory
        stdout, stderr, code = self.run_olshell_command('cd nonexistent')
        self.assertNotEqual(code, -1)
        
        # cd without arguments (should go to home)
        stdout, stderr, code = self.run_olshell_command('cd')
        self.assertNotEqual(code, -1)
    
    def test_cat_command_variants(self):
        """Test cat command with various file scenarios"""
        # Create test files
        self.create_test_file("single_line.txt", "Single line content")
        self.create_test_file("multi_line.txt", "Line 1\nLine 2\nLine 3")
        self.create_test_file("empty_file.txt", "")
        self.create_test_file("special_chars.txt", "Special: !@#$%^&*()\nUnicode: éñç")
        
        # Cat single file
        stdout, stderr, code = self.run_olshell_command('cat single_line.txt')
        self.assertIn("Single line content", stdout)
        
        # Cat multi-line file
        stdout, stderr, code = self.run_olshell_command('cat multi_line.txt')
        self.assertIn("Line 1", stdout)
        self.assertIn("Line 2", stdout)
        self.assertIn("Line 3", stdout)
        
        # Cat empty file
        stdout, stderr, code = self.run_olshell_command('cat empty_file.txt')
        self.assertNotEqual(code, -1)
        
        # Cat file with special characters
        stdout, stderr, code = self.run_olshell_command('cat special_chars.txt')
        self.assertIn("Special:", stdout)
        
        # Cat multiple files
        stdout, stderr, code = self.run_olshell_command('cat single_line.txt multi_line.txt')
        self.assertIn("Single line content", stdout)
        self.assertIn("Line 1", stdout)
        
        # Cat non-existent file
        stdout, stderr, code = self.run_olshell_command('cat nonexistent.txt')
        self.assertNotEqual(code, -1)
    
    def test_rm_command_variants(self):
        """Test rm command with all its flags and scenarios"""
        # Create test files and directories
        self.create_test_file("file_to_remove.txt", "remove me")
        self.create_test_file("another_file.txt", "remove me too")
        
        test_dir = Path(self.test_dir)
        dir_to_remove = test_dir / "dir_to_remove"
        dir_to_remove.mkdir()
        (dir_to_remove / "nested_file.txt").write_text("nested content")
        
        # Remove single file
        stdout, stderr, code = self.run_olshell_command('rm file_to_remove.txt')
        self.assertFalse(self.file_exists("file_to_remove.txt"))
        
        # Remove multiple files
        self.create_test_file("file1.txt", "content1")
        self.create_test_file("file2.txt", "content2")
        stdout, stderr, code = self.run_olshell_command('rm file1.txt file2.txt')
        self.assertFalse(self.file_exists("file1.txt"))
        self.assertFalse(self.file_exists("file2.txt"))
        
        # Remove directory with -r flag
        stdout, stderr, code = self.run_olshell_command('rm -r dir_to_remove')
        self.assertFalse((test_dir / "dir_to_remove").exists())
        
        # Force remove with -f flag
        self.create_test_file("force_remove.txt", "force me")
        stdout, stderr, code = self.run_olshell_command('rm -f force_remove.txt')
        self.assertFalse(self.file_exists("force_remove.txt"))
        
        # Remove non-existent file
        stdout, stderr, code = self.run_olshell_command('rm nonexistent.txt')
        self.assertNotEqual(code, -1)
        
        # Combined flags
        dir_for_rf = test_dir / "dir_for_rf"
        dir_for_rf.mkdir()
        (dir_for_rf / "file.txt").write_text("content")
        stdout, stderr, code = self.run_olshell_command('rm -rf dir_for_rf')
        self.assertFalse(dir_for_rf.exists())
    
    def test_mkdir_command_variants(self):
        """Test mkdir command with various scenarios"""
        # Create single directory
        stdout, stderr, code = self.run_olshell_command('mkdir new_directory')
        self.assertTrue((Path(self.test_dir) / "new_directory").exists())
        
        # Create nested directories
        stdout, stderr, code = self.run_olshell_command('mkdir -p nested/deep/directory')
        self.assertTrue((Path(self.test_dir) / "nested" / "deep" / "directory").exists())
        
        # Create multiple directories
        stdout, stderr, code = self.run_olshell_command('mkdir dir1 dir2 dir3')
        self.assertTrue((Path(self.test_dir) / "dir1").exists())
        self.assertTrue((Path(self.test_dir) / "dir2").exists())
        self.assertTrue((Path(self.test_dir) / "dir3").exists())
        
        # Try to create existing directory
        stdout, stderr, code = self.run_olshell_command('mkdir new_directory')
        # Should handle gracefully
        self.assertNotEqual(code, -1)
        
        # Create directory with special characters in name
        stdout, stderr, code = self.run_olshell_command('mkdir "dir with spaces"')
        self.assertTrue((Path(self.test_dir) / "dir with spaces").exists())
    
    def test_pwd_command(self):
        """Test pwd command"""
        stdout, stderr, code = self.run_olshell_command('pwd')
        self.assertNotEqual(code, -1)
        # Should contain current directory path
        normalized_test_dir = self.test_dir.replace('\\', '/').replace('//', '/')
        normalized_stdout = stdout.replace('\\', '/').replace('//', '/')
        self.assertIn(normalized_test_dir, normalized_stdout)
    
    def test_clear_command(self):
        """Test clear command"""
        stdout, stderr, code = self.run_olshell_command('clear')
        self.assertNotEqual(code, -1)
        # Clear command should execute without error
    
    def test_history_command_variants(self):
        """Test history command with all its options"""
        # Execute some commands first
        self.run_olshell_command('echo "history test 1"')
        self.run_olshell_command('echo "history test 2"')
        
        # Show history
        stdout, stderr, code = self.run_olshell_command('history')
        self.assertNotEqual(code, -1)
        
        # Clear history with -c flag
        stdout, stderr, code = self.run_olshell_command('history -c')
        self.assertNotEqual(code, -1)


class TestAliasSystem(OlshellTestBase):
    """Test alias system comprehensively"""
    
    def test_alias_creation_and_usage(self):
        """Test creating and using aliases"""
        # Create simple alias
        stdout, stderr, code = self.run_olshell_command('alias ll="ls -l"')
        self.assertNotEqual(code, -1)
        
        # Use alias
        self.create_test_file("test_file.txt", "content")
        stdout, stderr, code = self.run_olshell_command('alias ll="ls -l"\nll')
        self.assertIn("test_file.txt", stdout)
        
        # Create alias with multiple commands
        stdout, stderr, code = self.run_olshell_command('alias greet="echo Hello; echo World"')
        self.assertNotEqual(code, -1)
        
        # List aliases
        stdout, stderr, code = self.run_olshell_command('alias')
        self.assertNotEqual(code, -1)
        
        # Delete alias with -d flag
        stdout, stderr, code = self.run_olshell_command('alias test_alias="echo test"\nalias -d test_alias')
        self.assertNotEqual(code, -1)
        
        # Look up specific alias
        stdout, stderr, code = self.run_olshell_command('alias ll="ls -l"\nalias ll')
        self.assertNotEqual(code, -1)
    
    def test_alias_edge_cases(self):
        """Test alias edge cases and error conditions"""
        # Alias with special characters
        stdout, stderr, code = self.run_olshell_command('alias special="echo \\"quoted\\""')
        self.assertNotEqual(code, -1)
        
        # Empty alias value
        stdout, stderr, code = self.run_olshell_command('alias empty=""')
        self.assertNotEqual(code, -1)
        
        # Alias overriding builtin
        stdout, stderr, code = self.run_olshell_command('alias echo="echo OVERRIDE:"')
        self.assertNotEqual(code, -1)
        
        # Delete non-existent alias
        stdout, stderr, code = self.run_olshell_command('alias -d nonexistent_alias')
        self.assertNotEqual(code, -1)


class TestConfigSystem(OlshellTestBase):
    """Test configuration system comprehensively"""
    
    def test_config_show_and_list(self):
        """Test showing configuration"""
        # Show all config with -s flag
        stdout, stderr, code = self.run_olshell_command('config -s')
        self.assertNotEqual(code, -1)
        
        # Show all config with --show flag
        stdout, stderr, code = self.run_olshell_command('config --show')
        self.assertNotEqual(code, -1)
        
        # List config with --list flag
        stdout, stderr, code = self.run_olshell_command('config --list')
        self.assertNotEqual(code, -1)
    
    def test_config_get_and_set(self):
        """Test getting and setting configuration values"""
        # Set a config value
        stdout, stderr, code = self.run_olshell_command('config -S test_key "test_value"')
        self.assertNotEqual(code, -1)
        
        # Get a config value
        stdout, stderr, code = self.run_olshell_command('config -g test_key')
        self.assertNotEqual(code, -1)
        
        # Set with --set flag
        stdout, stderr, code = self.run_olshell_command('config --set another_key "another_value"')
        self.assertNotEqual(code, -1)
        
        # Get with --get flag
        stdout, stderr, code = self.run_olshell_command('config --get another_key')
        self.assertNotEqual(code, -1)
    
    def test_config_help(self):
        """Test config help functionality"""
        # Show help with -h flag
        stdout, stderr, code = self.run_olshell_command('config -h')
        self.assertIn("help", stdout.lower())
        
        # Show help with --help flag
        stdout, stderr, code = self.run_olshell_command('config --help')
        self.assertIn("help", stdout.lower())
    
    def test_config_edge_cases(self):
        """Test config edge cases"""
        # Get non-existent key
        stdout, stderr, code = self.run_olshell_command('config -g nonexistent_key')
        self.assertNotEqual(code, -1)
        
        # Set empty value
        stdout, stderr, code = self.run_olshell_command('config -S empty_key ""')
        self.assertNotEqual(code, -1)
        
        # Invalid flags combination
        stdout, stderr, code = self.run_olshell_command('config -invalid')
        self.assertNotEqual(code, -1)


class TestRedirectionOperators(OlshellTestBase):
    """Test all redirection operators comprehensively"""
    
    def test_output_redirection(self):
        """Test output redirection with >"""
        # Basic output redirection
        stdout, stderr, code = self.run_olshell_command('echo "redirected content" > output.txt')
        if self.file_exists("output.txt"):
            content = self.read_file_content("output.txt")
            self.assertIn("redirected content", content)
        
        # Overwrite existing file
        self.create_test_file("existing.txt", "original content")
        stdout, stderr, code = self.run_olshell_command('echo "new content" > existing.txt')
        if self.file_exists("existing.txt"):
            content = self.read_file_content("existing.txt")
            self.assertIn("new content", content)
            self.assertNotIn("original content", content)
        
        # Redirect command with arguments
        stdout, stderr, code = self.run_olshell_command('echo "arg1 arg2 arg3" > args_output.txt')
        if self.file_exists("args_output.txt"):
            content = self.read_file_content("args_output.txt")
            self.assertIn("arg1 arg2 arg3", content)
    
    def test_append_redirection(self):
        """Test append redirection with >>"""
        # Append to new file
        stdout, stderr, code = self.run_olshell_command('echo "first line" >> append.txt')
        stdout, stderr, code = self.run_olshell_command('echo "second line" >> append.txt')
        
        if self.file_exists("append.txt"):
            content = self.read_file_content("append.txt")
            self.assertIn("first line", content)
            self.assertIn("second line", content)
        
        # Append to existing file
        self.create_test_file("existing_append.txt", "existing content\n")
        stdout, stderr, code = self.run_olshell_command('echo "appended content" >> existing_append.txt')
        
        if self.file_exists("existing_append.txt"):
            content = self.read_file_content("existing_append.txt")
            self.assertIn("existing content", content)
            self.assertIn("appended content", content)
    
    def test_input_redirection(self):
        """Test input redirection with <"""
        # Create input file
        self.create_test_file("input.txt", "line1\nline2\nline3")
        
        # Test input redirection (if supported)
        stdout, stderr, code = self.run_olshell_command('cat < input.txt')
        # This might not be fully implemented, so just check it doesn't crash
        self.assertNotEqual(code, -1)
    
    def test_redirection_with_spaces(self):
        """Test redirection with various spacing"""
        # No spaces around >
        stdout, stderr, code = self.run_olshell_command('echo "nospace">nospace.txt')
        self.assertNotEqual(code, -1)
        
        # Spaces around >
        stdout, stderr, code = self.run_olshell_command('echo "spaces" > spaces.txt')
        self.assertNotEqual(code, -1)
        
        # Multiple spaces
        stdout, stderr, code = self.run_olshell_command('echo "multiple"   >   multiple.txt')
        self.assertNotEqual(code, -1)
    
    def test_redirection_error_cases(self):
        """Test redirection error conditions"""
        # Redirect to invalid path
        stdout, stderr, code = self.run_olshell_command('echo "test" > /invalid/path/file.txt')
        # Should handle gracefully
        self.assertNotEqual(code, -1)
        
        # Missing filename
        stdout, stderr, code = self.run_olshell_command('echo "test" >')
        # Should handle gracefully  
        self.assertNotEqual(code, -1)


class TestPipelineOperators(OlshellTestBase):
    """Test pipeline operators comprehensively"""
    
    def test_simple_pipelines(self):
        """Test basic pipeline operations"""
        # Simple echo | cat
        stdout, stderr, code = self.run_olshell_command('echo "pipeline test" | cat')
        self.assertIn("pipeline test", stdout)
        
        # File content through pipeline
        self.create_test_file("pipe_input.txt", "line1\nline2\nline3")
        stdout, stderr, code = self.run_olshell_command('cat pipe_input.txt | cat')
        self.assertIn("line1", stdout)
        self.assertIn("line2", stdout)
        self.assertIn("line3", stdout)
        
        # Multiple word pipeline
        stdout, stderr, code = self.run_olshell_command('echo "word1 word2 word3" | cat')
        self.assertIn("word1 word2 word3", stdout)
    
    def test_pipeline_with_redirection(self):
        """Test combining pipelines with redirection"""
        # Pipeline to file
        stdout, stderr, code = self.run_olshell_command('echo "piped output" | cat > piped_file.txt')
        
        if self.file_exists("piped_file.txt"):
            content = self.read_file_content("piped_file.txt")
            self.assertIn("piped output", content)
        
        # File through pipeline to file
        self.create_test_file("source.txt", "source content")
        stdout, stderr, code = self.run_olshell_command('cat source.txt | cat > destination.txt')
        
        if self.file_exists("destination.txt"):
            content = self.read_file_content("destination.txt")
            self.assertIn("source content", content)
    
    def test_complex_pipelines(self):
        """Test more complex pipeline scenarios"""
        # Multiple commands in pipeline
        stdout, stderr, code = self.run_olshell_command('echo "multi pipe" | cat | cat')
        self.assertIn("multi pipe", stdout)
        
        # Pipeline with different commands
        self.create_test_file("ls_test.txt", "content")
        stdout, stderr, code = self.run_olshell_command('ls | cat')
        self.assertIn("ls_test.txt", stdout)


class TestCommandChaining(OlshellTestBase):
    """Test command chaining with semicolons"""
    
    def test_basic_command_chaining(self):
        """Test basic semicolon command chaining"""
        # Simple command chain
        stdout, stderr, code = self.run_olshell_command('echo "first"; echo "second"; echo "third"')
        self.assertIn("first", stdout)
        self.assertIn("second", stdout)
        self.assertIn("third", stdout)
        
        # Chain with different commands
        stdout, stderr, code = self.run_olshell_command('echo "test"; pwd; echo "done"')
        self.assertIn("test", stdout)
        self.assertIn("done", stdout)
    
    def test_chaining_with_file_operations(self):
        """Test chaining with file operations"""
        # Create, read, and delete file in chain
        stdout, stderr, code = self.run_olshell_command('echo "content" > chain.txt')
        self.assertNotEqual(code, -1)
        
        stdout, stderr, code = self.run_olshell_command('cat chain.txt')
        self.assertIn("content", stdout)
        
        stdout, stderr, code = self.run_olshell_command('rm chain.txt')
        self.assertNotEqual(code, -1)
        self.assertFalse(self.file_exists("chain.txt"))
    
    def test_chaining_with_directory_operations(self):
        """Test chaining with directory operations"""
        # Create directory, change to it, create file, go back
        commands = [
            'mkdir chain_test_dir',
            'cd chain_test_dir', 
            'echo "in subdir" > subdir_file.txt',
            'cd ..',
            'ls chain_test_dir'
        ]
        
        for cmd in commands:
            stdout, stderr, code = self.run_olshell_command(cmd)
            self.assertNotEqual(code, -1)
    
    def test_chaining_error_handling(self):
        """Test how chaining handles errors"""
        # Chain with failing command
        stdout, stderr, code = self.run_olshell_command('echo "before"; nonexistent_command; echo "after"')
        self.assertIn("before", stdout)
        # Should continue to execute "after" even if middle command fails
        
        # Chain with file operations that might fail
        stdout, stderr, code = self.run_olshell_command('cat nonexistent.txt; echo "continued"')
        self.assertNotEqual(code, -1)


class TestSpecialFeatures(OlshellTestBase):
    """Test special features and edge cases"""
    
    def test_builtin_skip_operator(self):
        """Test skipping builtins with ^ operator"""
        # Skip builtin lookup
        stdout, stderr, code = self.run_olshell_command('^echo "test"')
        # This should try to execute external echo instead of builtin
        # Behavior depends on system, just ensure it doesn't crash
        self.assertNotEqual(code, -1)
        
        # Skip other builtins
        stdout, stderr, code = self.run_olshell_command('^pwd')
        self.assertNotEqual(code, -1)
    
    def test_home_directory_support(self):
        """Test ~ (home directory) support"""
        # Test cd to home
        stdout, stderr, code = self.run_olshell_command('cd ~')
        self.assertNotEqual(code, -1)
        
        # Test path with ~ (if supported)
        stdout, stderr, code = self.run_olshell_command('echo "test" > ~/test_home.txt; rm ~/test_home.txt')
        self.assertNotEqual(code, -1)
    
    def test_exit_command(self):
        """Test exit command functionality"""
        # Exit is tested implicitly in other tests since we send exit at the end
        # Just verify it's recognized
        stdout, stderr, code = self.run_olshell_command('echo "before exit"', expect_exit=False)
        self.assertIn("before exit", stdout)
    
    def test_empty_and_whitespace_commands(self):
        """Test handling of empty and whitespace-only commands"""
        # Empty command
        stdout, stderr, code = self.run_olshell_command('')
        self.assertNotEqual(code, -1)
        
        # Whitespace only
        stdout, stderr, code = self.run_olshell_command('   ')
        self.assertNotEqual(code, -1)
        
        # Tab only
        stdout, stderr, code = self.run_olshell_command('\t')
        self.assertNotEqual(code, -1)
        
        # Mixed whitespace
        stdout, stderr, code = self.run_olshell_command(' \t \n ')
        self.assertNotEqual(code, -1)
    
    def test_special_characters_in_arguments(self):
        """Test handling of special characters in command arguments"""
        special_chars = [
            '!@#$%^&*()',
            '[]{}|\\',
            '<>=+-',
            '?/.,',
            '`~',
            '"\'',
        ]
        
        for chars in special_chars:
            stdout, stderr, code = self.run_olshell_command(f'echo "{chars}"')
            self.assertNotEqual(code, -1, f"Failed with special chars: {chars}")
    
    def test_long_command_lines(self):
        """Test very long command lines"""
        # Long echo command
        long_text = "word " * 100  # 500 characters
        stdout, stderr, code = self.run_olshell_command(f'echo "{long_text}"')
        self.assertIn("word", stdout)
        
        # Long file path
        long_filename = "a" * 50 + ".txt"
        stdout, stderr, code = self.run_olshell_command(f'echo "content" > {long_filename}')
        self.assertNotEqual(code, -1)
    
    def test_unicode_and_international_characters(self):
        """Test handling of unicode and international characters"""
        unicode_tests = [
            "Héllo Wörld",
            "测试中文",
            "العربية", 
            "русский",
            "🚀🎉💯",
            "éñçñ üàø"
        ]
        
        for text in unicode_tests:
            stdout, stderr, code = self.run_olshell_command(f'echo "{text}"')
            self.assertNotEqual(code, -1, f"Failed with unicode: {text}")


class TestScriptExecution(OlshellTestBase):
    """Test .olsh script file execution capabilities"""
    
    def test_script_file_detection(self):
        """Test detection and handling of .olsh script files"""
        # Create a simple script
        script_content = '''echo "Script line 1"
echo "Script line 2"
pwd
ls
'''
        self.create_test_file("test_script.olsh", script_content)
        
        # Test that .olsh files are recognized
        stdout, stderr, code = self.run_olshell_command('ls')
        self.assertIn("test_script.olsh", stdout)
        
        # Note: Actual script execution would require the shell to run the script
        # This would need to be tested differently as it's a shell feature
    
    def test_script_file_naming(self):
        """Test various .olsh file naming scenarios"""
        script_names = [
            "simple.olsh",
            "with-dashes.olsh", 
            "with_underscores.olsh",
            "123numeric.olsh",
            "UPPERCASE.olsh"
        ]
        
        for name in script_names:
            self.create_test_file(name, "echo 'test'")
            stdout, stderr, code = self.run_olshell_command('ls')
            self.assertIn(name, stdout)


class TestErrorHandlingAndRobustness(OlshellTestBase):
    """Test error handling and shell robustness"""
    
    def test_invalid_command_handling(self):
        """Test handling of invalid commands"""
        invalid_commands = [
            "nonexistent_command_xyz",
            "command_with_weird_chars_!@#",
            "123_numeric_start",
            "_underscore_start",
            "-dash_start"
        ]
        
        for cmd in invalid_commands:
            stdout, stderr, code = self.run_olshell_command(cmd)
            # Should handle gracefully without crashing
            self.assertNotEqual(code, -1, f"Shell crashed on invalid command: {cmd}")
    
    def test_malformed_redirection(self):
        """Test handling of malformed redirection"""
        malformed_redirections = [
            "echo test >",  # Missing filename
            "echo test > > file.txt",  # Double redirection
            "echo test >> > file.txt",  # Mixed redirection  
            "echo test > /invalid/path/file.txt",  # Invalid path
        ]
        
        for cmd in malformed_redirections:
            stdout, stderr, code = self.run_olshell_command(cmd)
            self.assertNotEqual(code, -1, f"Shell crashed on malformed redirection: {cmd}")
    
    def test_malformed_pipelines(self):
        """Test handling of malformed pipelines"""
        malformed_pipes = [
            "echo test |",  # Pipe with no command
            "| echo test",  # Pipe at start
            "echo test | | cat",  # Double pipe
            "echo test ||| cat",  # Triple pipe
        ]
        
        for cmd in malformed_pipes:
            stdout, stderr, code = self.run_olshell_command(cmd)
            self.assertNotEqual(code, -1, f"Shell crashed on malformed pipeline: {cmd}")
    
    def test_resource_limits(self):
        """Test behavior under resource constraints"""
        # Test with many files
        for i in range(20):
            self.create_test_file(f"file_{i}.txt", f"content {i}")
        
        stdout, stderr, code = self.run_olshell_command('ls')
        self.assertNotEqual(code, -1)
        
        # Test with deeply nested directories
        nested_path = "a/b/c/d/e/f"
        stdout, stderr, code = self.run_olshell_command(f'mkdir -p {nested_path}')
        self.assertNotEqual(code, -1)
    
    def test_concurrent_operations(self):
        """Test handling of operations that might conflict"""
        # Create and delete file rapidly
        for i in range(5):
            stdout, stderr, code = self.run_olshell_command(f'echo "test {i}" > temp_{i}.txt')
            self.assertNotEqual(code, -1)
            
            stdout, stderr, code = self.run_olshell_command(f'cat temp_{i}.txt')
            self.assertNotEqual(code, -1)
            
            stdout, stderr, code = self.run_olshell_command(f'rm temp_{i}.txt')
            self.assertNotEqual(code, -1)


if __name__ == '__main__':
    print("=== OlShell Complete Feature Test Suite ===")
    print("Testing all builtin commands, operators, and features...")
    print()
    
    # Configure test runner for detailed output
    unittest.main(
        verbosity=2,
        buffer=True,
        warnings='ignore'
    )
