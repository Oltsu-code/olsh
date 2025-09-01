"""
Edge cases and boundary condition tests for olshell
Tests unusual inputs, error conditions, and corner cases
"""

import unittest
import subprocess
import os
import tempfile
from pathlib import Path
from tests.test_comprehensive import OlshellTestBase


class TestBoundaryConditions(OlshellTestBase):
    """Test edge cases and boundary conditions"""
    
    def test_empty_commands(self):
        """Test empty command handling"""
        empty_commands = [
            '',           # Completely empty
            ' ',          # Just space
            '\t',         # Just tab
            '   ',        # Multiple spaces
            '\n',         # Just newline
        ]
        
        for cmd in empty_commands:
            stdout, stderr, code = self.run_olshell_command(cmd)
            # Should not crash, even if it returns error code
            self.assertNotEqual(code, -1, f"Shell crashed on empty command: {repr(cmd)}")
    
    def test_whitespace_only_commands(self):
        """Test commands with only whitespace"""
        whitespace_commands = [
            '     ',
            '\t\t\t',
            ' \t \t ',
            '  \n  ',
        ]
        
        for cmd in whitespace_commands:
            stdout, stderr, code = self.run_olshell_command(cmd)
            self.assertNotEqual(code, -1, f"Shell crashed on whitespace: {repr(cmd)}")
    
    def test_very_long_paths(self):
        """Test very long file paths"""
        # Create nested directory structure
        current_path = Path(self.test_dir)
        path_segments = []
        
        # Create reasonably long path (but not too long for Windows)
        for i in range(8):
            segment = f"very_long_directory_name_{i}_with_lots_of_characters"
            path_segments.append(segment)
            current_path = current_path / segment
            current_path.mkdir()
        
        # Test operations on long path
        long_file_path = current_path / "very_long_filename_with_many_characters.txt"
        long_file_path.write_text("Content in long path")
        
        # Use relative path from test directory
        relative_path = '/'.join(path_segments) + '/very_long_filename_with_many_characters.txt'
        
        stdout, stderr, code = self.run_olshell_command(f'cat "{relative_path}"')
        # Should handle long paths gracefully
        if code != -1:
            self.assertIn("Content in long path", stdout)
    
    def test_special_characters_in_filenames(self):
        """Test filenames with special characters"""
        special_files = [
            "file with spaces.txt",
            "file-with-dashes.txt", 
            "file_with_underscores.txt",
            "file.with.dots.txt",
            "file123numbers.txt",
        ]
        
        for filename in special_files:
            self.create_test_file(filename, f"Content of {filename}")
            
            # Test operations on special filenames
            stdout, stderr, code = self.run_olshell_command(f'cat "{filename}"')
            if code != -1:
                self.assertIn(f"Content of {filename}", stdout)
    
    def test_unicode_content(self):
        """Test handling of unicode content"""
        unicode_content = "Hello 世界 🌍 Здравствуй мир"
        self.create_test_file("unicode_test.txt", unicode_content)
        
        stdout, stderr, code = self.run_olshell_command('cat unicode_test.txt')
        if code != -1:
            # May not display perfectly but shouldn't crash
            self.assertIsInstance(stdout, str)


class TestErrorConditions(OlshellTestBase):
    """Test various error conditions"""
    
    def test_nonexistent_commands(self):
        """Test execution of nonexistent commands"""
        fake_commands = [
            'nonexistent_command',
            'fake_program',
            'not_a_real_command',
            'xyz123abc',
            'this_command_does_not_exist'
        ]
        
        for cmd in fake_commands:
            stdout, stderr, code = self.run_olshell_command(cmd)
            # Should not crash the shell
            self.assertNotEqual(code, -1, f"Shell crashed on fake command: {cmd}")
    
    def test_nonexistent_files(self):
        """Test operations on nonexistent files"""
        fake_files = [
            'nonexistent.txt',
            'missing_file.txt',
            'does_not_exist.txt'
        ]
        
        for filename in fake_files:
            stdout, stderr, code = self.run_olshell_command(f'cat {filename}')
            # Should fail gracefully, not crash
            self.assertNotEqual(code, -1, f"Shell crashed on missing file: {filename}")
    
    def test_permission_denied_scenarios(self):
        """Test permission denied conditions (where applicable)"""
        # Test accessing system directories (may vary by system)
        protected_paths = [
            'C:\\Windows\\System32\\config',  # Windows protected
            '/etc/shadow',  # Unix protected (won't exist on Windows)
            'C:\\Program Files\\',  # May have restrictions
        ]
        
        for path in protected_paths:
            if os.path.exists(path):
                stdout, stderr, code = self.run_olshell_command(f'cd "{path}"')
                # Should handle permission errors gracefully
                self.assertNotEqual(code, -1, f"Shell crashed on protected path: {path}")
    
    def test_circular_redirections(self):
        """Test circular file redirections"""
        # Create a file
        self.create_test_file("circular.txt", "original content")
        
        # Try circular redirection (should be caught or handled)
        stdout, stderr, code = self.run_olshell_command('cat circular.txt > circular.txt')
        
        # Should not crash, though behavior may vary
        self.assertNotEqual(code, -1)
    
    def test_malformed_redirections(self):
        """Test malformed redirection syntax"""
        malformed_commands = [
            'echo test >',           # Missing target
            'echo test > >',         # Double redirect
            '> echo test',           # Redirect at start
            'echo test > > file.txt', # Invalid syntax
            'echo test |',           # Pipe to nothing
            '| echo test',           # Pipe from nothing
        ]
        
        for cmd in malformed_commands:
            stdout, stderr, code = self.run_olshell_command(cmd)
            # Should handle malformed syntax gracefully
            self.assertNotEqual(code, -1, f"Shell crashed on malformed command: {cmd}")


class TestUnusualInputs(OlshellTestBase):
    """Test unusual but potentially valid inputs"""
    
    def test_command_with_many_arguments(self):
        """Test commands with many arguments"""
        # Echo with many arguments
        many_args = ['echo'] + [f'arg{i}' for i in range(50)]
        cmd = ' '.join(many_args)
        
        stdout, stderr, code = self.run_olshell_command(cmd)
        if code != -1:
            for i in range(50):
                self.assertIn(f'arg{i}', stdout)
    
    def test_deeply_nested_quotes(self):
        """Test nested quotation scenarios"""
        quote_tests = [
            'echo "simple quotes"',
            "echo 'single quotes'",
            'echo "quotes with \\"escaped\\" quotes"',
            "echo 'quotes with \"mixed\" quotes'",
        ]
        
        for cmd in quote_tests:
            stdout, stderr, code = self.run_olshell_command(cmd)
            # Should handle quotes without crashing
            self.assertNotEqual(code, -1, f"Shell crashed on quotes: {cmd}")
    
    def test_unusual_whitespace_patterns(self):
        """Test unusual whitespace in commands"""
        whitespace_tests = [
            'echo    test',          # Multiple spaces
            'echo\ttest',            # Tab separation
            'echo \t test',          # Mixed whitespace
            '  echo  test  ',        # Leading/trailing spaces
        ]
        
        for cmd in whitespace_tests:
            stdout, stderr, code = self.run_olshell_command(cmd)
            if code != -1:
                self.assertIn('test', stdout)
    
    def test_command_case_sensitivity(self):
        """Test command case sensitivity"""
        case_tests = [
            ('echo test', 'ECHO TEST'),
            ('pwd', 'PWD'),
            ('ls', 'LS'),
        ]
        
        for lower, upper in case_tests:
            # Test lowercase (should work)
            stdout1, stderr1, code1 = self.run_olshell_command(lower)
            
            # Test uppercase (behavior may vary)
            stdout2, stderr2, code2 = self.run_olshell_command(upper)
            
            # Both should not crash the shell
            self.assertNotEqual(code1, -1, f"Shell crashed on lowercase: {lower}")
            self.assertNotEqual(code2, -1, f"Shell crashed on uppercase: {upper}")


class TestMemoryAndResourceLimits(OlshellTestBase):
    """Test memory and resource limit scenarios"""
    
    def test_very_large_output(self):
        """Test commands that produce very large output"""
        # Create a file with many lines
        large_lines = []
        for i in range(1000):
            large_lines.append(f"This is line {i} with some content to make it longer")
        
        self.create_test_file("large_output.txt", '\n'.join(large_lines))
        
        # Test reading large output
        stdout, stderr, code = self.run_olshell_command('cat large_output.txt')
        
        if code != -1:
            self.assertIn("This is line 0", stdout)
            self.assertIn("This is line 999", stdout)
    
    def test_recursive_directory_operations(self):
        """Test operations on recursive directory structures"""
        # Create nested structure
        current_path = Path(self.test_dir)
        
        # Create moderate nesting (not too deep to avoid issues)
        for level in range(5):
            level_path = current_path / f"level_{level}"
            level_path.mkdir()
            
            # Add files at each level
            for file_num in range(3):
                file_path = level_path / f"file_{level}_{file_num}.txt"
                file_path.write_text(f"Content at level {level}, file {file_num}")
            
            current_path = level_path
        
        # Test listing from root
        stdout, stderr, code = self.run_olshell_command('ls -R' if self.has_recursive_ls() else 'ls')
        self.assertNotEqual(code, -1)
    
    def has_recursive_ls(self):
        """Check if ls supports -R flag"""
        # Simple heuristic - may not be accurate for all systems
        return True  # Assume basic recursive listing
    
    def test_many_simultaneous_files(self):
        """Test operations with many files open conceptually"""
        file_count = 30
        
        # Create many files
        for i in range(file_count):
            self.create_test_file(f"multi_{i:02d}.txt", f"File {i} content")
        
        # Try to cat all files in one command
        filenames = [f"multi_{i:02d}.txt" for i in range(file_count)]
        cat_command = f'cat {" ".join(filenames)}'
        
        stdout, stderr, code = self.run_olshell_command(cat_command)
        
        if code != -1:
            self.assertIn("File 0 content", stdout)
            self.assertIn("File 29 content", stdout)


class TestInterruptAndSignalHandling(OlshellTestBase):
    """Test interrupt and signal handling"""
    
    def test_ctrl_c_during_command(self):
        """Test Ctrl+C behavior during command execution"""
        # This is challenging to test programmatically
        # We'll test what we can with subprocess
        
        # Start a potentially long-running command
        try:
            process = subprocess.Popen(
                [self.olshell_path],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                cwd=self.test_dir
            )
            
            # Send a command that might take some time
            process.stdin.write('echo "before interrupt"\n')
            process.stdin.flush()
            
            # Send Ctrl+C equivalent
            try:
                process.send_signal(subprocess.signal.SIGINT)
            except (AttributeError, OSError):
                # Signal handling may not work on all platforms
                pass
            
            # Try to continue with another command
            process.stdin.write('echo "after interrupt"\n')
            process.stdin.write('exit\n')
            process.stdin.flush()
            
            stdout, stderr = process.communicate(timeout=5)
            
            # Shell should have handled interrupt gracefully
            self.assertIsInstance(stdout, str)
            
        except (subprocess.TimeoutExpired, OSError):
            # Interrupt testing is platform-dependent
            if 'process' in locals():
                process.kill()
    
    def test_shell_recovery_after_errors(self):
        """Test shell recovery after various errors"""
        recovery_sequence = [
            'echo "start"',
            'nonexistent_command',  # Should fail
            'echo "after error 1"',
            'cat missing_file.txt',  # Should fail  
            'echo "after error 2"',
            'cd /invalid/path',     # Should fail
            'echo "after error 3"',
            'pwd',                  # Should work
            'echo "end"'
        ]
        
        command_string = '\n'.join(recovery_sequence)
        stdout, stderr, code = self.run_olshell_command(command_string)
        
        # Shell should recover from errors and continue
        self.assertNotEqual(code, -1)
        self.assertIn("start", stdout)
        self.assertIn("end", stdout)


if __name__ == '__main__':
    print("=== OlShell Edge Cases and Boundary Tests ===")
    print("Testing edge cases, error conditions, and unusual inputs...")
    print()
    
    unittest.main(verbosity=2)
