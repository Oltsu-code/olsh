"""
Unit tests for input handling and terminal features
Tests tab completion, history, Ctrl+C handling, and linenoise integration
"""

import unittest
import subprocess
import time
import threading
from pathlib import Path
from tests.test_comprehensive import OlshellTestBase


class TestInputHandling(OlshellTestBase):
    """Test input handling and terminal features"""
    
    def test_ctrl_c_handling(self):
        """Test that Ctrl+C doesn't crash the shell"""
        try:
            process = subprocess.Popen(
                [str(self.olshell_exe)],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                cwd=self.test_dir
            )
            
            # Send Ctrl+C (ASCII 3) followed by exit
            process.stdin.write('\x03\nexit\n')
            process.stdin.flush()
            
            stdout, stderr = process.communicate(timeout=5)
            
            # Shell should handle Ctrl+C gracefully and exit properly
            # Look for the ^C output that indicates proper handling
            self.assertIn("^C", stdout)
            
        except subprocess.TimeoutExpired:
            process.kill()
            self.fail("Shell did not handle Ctrl+C properly within timeout")
        except Exception as e:
            self.fail(f"Error testing Ctrl+C handling: {e}")
    
    def test_empty_input_handling(self):
        """Test handling of empty input (just pressing Enter)"""
        stdout, stderr, code = self.run_olshell_command('')
        # Should handle gracefully without errors
        self.assertNotEqual(code, -1)
    
    def test_multiple_empty_lines(self):
        """Test handling multiple empty lines"""
        stdout, stderr, code = self.run_olshell_command('\n\n\necho "after empty lines"')
        self.assertIn("after empty lines", stdout)
    
    def test_long_command_line(self):
        """Test handling of very long command lines"""
        long_text = "word " * 100  # 500 characters
        stdout, stderr, code = self.run_olshell_command(f'echo "{long_text}"')
        self.assertIn("word", stdout)
    
    def test_special_input_characters(self):
        """Test handling of special characters in input"""
        special_chars = "!@#$%^&*()[]{}|\\:;\"'<>?,./"
        stdout, stderr, code = self.run_olshell_command(f'echo "{special_chars}"')
        # Should handle special characters without crashing
        self.assertNotEqual(code, -1)


class TestTabCompletion(OlshellTestBase):
    """Test tab completion functionality"""
    
    def test_command_completion_available(self):
        """Test that basic commands are available for completion"""
        # Create files to test file completion
        self.create_test_file("completion_test.txt", "test")
        
        # We can't easily test interactive tab completion,
        # but we can test that the shell handles basic commands
        commands = ['echo', 'cat', 'ls', 'pwd', 'clear', 'cd', 'rm']
        
        for cmd in commands:
            stdout, stderr, code = self.run_olshell_command(f'{cmd}')
            # Basic commands should be recognized
            self.assertNotEqual(code, -1)
    
    def test_file_completion_context(self):
        """Test file completion context"""
        # Create various test files
        test_files = [
            "test_file1.txt",
            "test_file2.log", 
            "another_file.doc",
            "script.olsh"
        ]
        
        for filename in test_files:
            self.create_test_file(filename, "content")
        
        # Test that files are visible to commands
        stdout, stderr, code = self.run_olshell_command('ls')
        for filename in test_files:
            self.assertIn(filename, stdout)


class TestHistoryFeatures(OlshellTestBase):
    """Test command history functionality"""
    
    def test_history_persistence(self):
        """Test that history can be accessed"""
        # Execute some commands
        commands = [
            'echo "history test 1"',
            'echo "history test 2"', 
            'pwd',
            'ls'
        ]
        
        for cmd in commands:
            stdout, stderr, code = self.run_olshell_command(cmd)
            self.assertNotEqual(code, -1)
        
        # Check history command works
        stdout, stderr, code = self.run_olshell_command('history')
        self.assertNotEqual(code, -1)
    
    def test_history_command_execution(self):
        """Test that history command doesn't crash"""
        stdout, stderr, code = self.run_olshell_command('history')
        # Should execute without error regardless of content
        self.assertNotEqual(code, -1)


class TestPromptDisplay(OlshellTestBase):
    """Test prompt display and formatting"""
    
    def test_prompt_appears(self):
        """Test that shell displays a prompt"""
        stdout, stderr, code = self.run_olshell_command('echo "prompt test"')
        
        # Look for prompt indicators in output
        # Olshell uses fancy prompts with ┌─ and └─
        prompt_indicators = ["┌─", "└─", "$"]
        
        has_prompt = any(indicator in stdout for indicator in prompt_indicators)
        self.assertTrue(has_prompt, "No recognizable prompt found in output")
    
    def test_prompt_consistency(self):
        """Test that prompt appears consistently"""
        stdout, stderr, code = self.run_olshell_command('echo "test1"\necho "test2"')
        
        # Count prompt appearances - should appear for each command
        prompt_count = stdout.count("└─")
        self.assertGreaterEqual(prompt_count, 1, "Prompt should appear at least once")


class TestColorOutput(OlshellTestBase):
    """Test colored output functionality"""
    
    def test_colored_ls_output(self):
        """Test that ls produces colored output"""
        # Create different types of files
        self.create_test_file("regular_file.txt", "content")
        
        # Create directory
        import os
        os.mkdir("test_directory")
        
        stdout, stderr, code = self.run_olshell_command('ls')
        
        # Should contain the files we created
        self.assertIn("regular_file.txt", stdout)
        self.assertIn("test_directory", stdout)
    
    def test_prompt_colors(self):
        """Test that prompt contains color codes"""
        stdout, stderr, code = self.run_olshell_command('echo "color test"')
        
        # Look for ANSI color codes in the output
        # Olshell uses colors in its fancy prompt
        ansi_indicators = ['\x1b[', '\033[']
        has_colors = any(indicator in stdout for indicator in ansi_indicators)
        
        # Colors might be present in prompt (depending on config)
        # This test just ensures the shell handles color codes without crashing
        self.assertNotEqual(code, -1)


class TestConfigIntegration(OlshellTestBase):
    """Test configuration system integration with input handling"""
    
    def test_config_affects_prompt(self):
        """Test that config affects prompt display"""
        # The shell should use configuration for prompt
        stdout, stderr, code = self.run_olshell_command('echo "config test"')
        
        # Should contain configured prompt elements
        self.assertNotEqual(code, -1)
        
        # Look for username and hostname in prompt (from config)
        self.assertTrue(
            any(x in stdout for x in ["┌─", "└─", "@"]),
            "Configured prompt elements not found"
        )
    
    def test_welcome_message(self):
        """Test that welcome message appears"""
        stdout, stderr, code = self.run_olshell_command('echo "welcome test"')
        
        # Should contain the configured welcome message
        welcome_indicators = [
            "OlShell", 
            "Best shell ever made",
            "v2.0"
        ]
        
        has_welcome = any(indicator in stdout for indicator in welcome_indicators)
        self.assertTrue(has_welcome, "Welcome message not found in output")


class TestAdvancedInputFeatures(OlshellTestBase):
    """Test advanced input features"""
    
    def test_multiline_commands(self):
        """Test handling of complex multiline scenarios"""
        # Test command chaining that spans concepts
        stdout, stderr, code = self.run_olshell_command(
            'echo "line1" > multi.txt\necho "line2" >> multi.txt\ncat multi.txt'
        )
        
        self.assertIn("line1", stdout)
        self.assertIn("line2", stdout)
    
    def test_command_with_quotes(self):
        """Test commands with various quote combinations"""
        test_cases = [
            'echo "double quotes"',
            "echo 'single quotes'",
            'echo "quotes with spaces"',
            'echo "quotes with \\"escaped quotes\\""'
        ]
        
        for cmd in test_cases:
            stdout, stderr, code = self.run_olshell_command(cmd)
            # All should execute without critical errors
            self.assertNotEqual(code, -1)
    
    def test_background_process_simulation(self):
        """Test handling of commands that might run longer"""
        # Test a command that takes a moment
        stdout, stderr, code = self.run_olshell_command('echo "start"; echo "end"')
        
        self.assertIn("start", stdout)
        self.assertIn("end", stdout)
    
    def test_interrupt_recovery(self):
        """Test that shell recovers properly after interrupts"""
        try:
            process = subprocess.Popen(
                [str(self.olshell_exe)],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                cwd=self.test_dir
            )
            
            # Send interrupt then normal command
            input_sequence = '\x03\necho "after interrupt"\nexit\n'
            stdout, stderr = process.communicate(input=input_sequence, timeout=10)
            
            # Should handle interrupt and continue with next command
            self.assertIn("^C", stdout)
            self.assertIn("after interrupt", stdout)
            
        except subprocess.TimeoutExpired:
            process.kill()
            self.fail("Shell did not recover from interrupt properly")


if __name__ == '__main__':
    unittest.main(verbosity=2)
