"""
Performance and stress tests for olshell
Tests shell behavior under load and with large inputs
"""

import unittest
import subprocess
import time
import threading
import os
from pathlib import Path
from tests.test_comprehensive import OlshellTestBase


class TestPerformanceCharacteristics(OlshellTestBase):
    """Test performance characteristics of the shell"""
    
    def test_startup_time(self):
        """Test shell startup performance"""
        startup_times = []
        
        for _ in range(3):  # Test multiple startups
            start_time = time.time()
            stdout, stderr, code = self.run_olshell_command('echo "startup test"')
            end_time = time.time()
            
            if code != -1:  # Only count successful startups
                startup_times.append(end_time - start_time)
        
        if startup_times:
            avg_startup = sum(startup_times) / len(startup_times)
            # Shell should start reasonably quickly
            self.assertLess(avg_startup, 3.0, f"Average startup time {avg_startup:.2f}s too slow")
    
    def test_command_execution_speed(self):
        """Test command execution performance"""
        commands = [
            'echo "speed test"',
            'pwd', 
            'ls',
            'echo "performance" > perf.txt',
            'cat perf.txt',
            'rm perf.txt'
        ]
        
        execution_times = []
        
        for cmd in commands:
            start_time = time.time()
            stdout, stderr, code = self.run_olshell_command(cmd)
            end_time = time.time()
            
            if code != -1:
                execution_times.append(end_time - start_time)
        
        if execution_times:
            avg_time = sum(execution_times) / len(execution_times)
            # Commands should execute reasonably quickly
            self.assertLess(avg_time, 2.0, f"Average command time {avg_time:.2f}s too slow")
    
    def test_memory_usage_stability(self):
        """Test that shell doesn't have obvious memory leaks"""
        # Execute many commands to test for memory issues
        commands = []
        for i in range(20):
            commands.append(f'echo "Memory test {i}"')
        
        command_string = '\n'.join(commands)
        
        start_time = time.time()
        stdout, stderr, code = self.run_olshell_command(command_string)
        end_time = time.time()
        
        # Should complete without hanging or crashing
        self.assertNotEqual(code, -1)
        self.assertLess(end_time - start_time, 10.0, "Memory test took too long")


class TestLargeInputHandling(OlshellTestBase):
    """Test handling of large inputs and outputs"""
    
    def test_large_file_operations(self):
        """Test operations on larger files"""
        # Create a file with substantial content
        large_content = []
        for i in range(200):
            large_content.append(f"Line {i}: This is test content for performance testing")
        
        content_text = '\n'.join(large_content)
        self.create_test_file("large_test.txt", content_text)
        
        # Test reading large file
        start_time = time.time()
        stdout, stderr, code = self.run_olshell_command('cat large_test.txt')
        end_time = time.time()
        
        self.assertNotEqual(code, -1)
        self.assertIn("Line 0:", stdout)
        self.assertIn("Line 199:", stdout)
        self.assertLess(end_time - start_time, 5.0, "Large file reading too slow")
    
    def test_long_command_lines(self):
        """Test very long command lines"""
        # Create a very long echo command
        long_text = "word " * 200  # 1000+ characters
        
        stdout, stderr, code = self.run_olshell_command(f'echo "{long_text}"')
        
        self.assertNotEqual(code, -1)
        self.assertIn("word", stdout)
    
    def test_many_files_listing(self):
        """Test ls with many files"""
        # Create many test files
        file_count = 50
        for i in range(file_count):
            self.create_test_file(f"test_file_{i:03d}.txt", f"Content {i}")
        
        start_time = time.time()
        stdout, stderr, code = self.run_olshell_command('ls')
        end_time = time.time()
        
        self.assertNotEqual(code, -1)
        self.assertIn("test_file_000.txt", stdout)
        self.assertIn("test_file_049.txt", stdout)
        self.assertLess(end_time - start_time, 3.0, "Listing many files too slow")
    
    def test_deep_directory_navigation(self):
        """Test navigation in deep directory structures"""
        # Create nested directories
        current_path = Path(self.test_dir)
        for i in range(5):
            current_path = current_path / f"level_{i}"
            current_path.mkdir()
        
        # Test navigating deep
        cd_commands = []
        for i in range(5):
            cd_commands.append(f"cd level_{i}")
        
        cd_commands.append("pwd")
        command_string = '\n'.join(cd_commands)
        
        stdout, stderr, code = self.run_olshell_command(command_string)
        self.assertNotEqual(code, -1)
        self.assertIn("level_4", stdout)


class TestConcurrentOperations(OlshellTestBase):
    """Test concurrent shell operations"""
    
    def test_multiple_file_operations(self):
        """Test multiple file operations in sequence"""
        operations = [
            'echo "file1" > test1.txt',
            'echo "file2" > test2.txt', 
            'echo "file3" > test3.txt',
            'cat test1.txt test2.txt test3.txt > combined.txt',
            'cat combined.txt',
            'rm test1.txt test2.txt test3.txt combined.txt'
        ]
        
        command_string = '\n'.join(operations)
        
        start_time = time.time()
        stdout, stderr, code = self.run_olshell_command(command_string)
        end_time = time.time()
        
        self.assertNotEqual(code, -1)
        self.assertIn("file1", stdout)
        self.assertIn("file2", stdout)
        self.assertIn("file3", stdout)
        self.assertLess(end_time - start_time, 5.0, "Multiple operations too slow")
    
    def test_rapid_command_execution(self):
        """Test rapid succession of commands"""
        rapid_commands = []
        for i in range(30):
            rapid_commands.append(f'echo "Rapid {i}"')
        
        command_string = '\n'.join(rapid_commands)
        
        start_time = time.time()
        stdout, stderr, code = self.run_olshell_command(command_string)
        end_time = time.time()
        
        self.assertNotEqual(code, -1)
        self.assertIn("Rapid 0", stdout)
        self.assertIn("Rapid 29", stdout)
        self.assertLess(end_time - start_time, 8.0, "Rapid commands too slow")


class TestStressScenarios(OlshellTestBase):
    """Test shell under stress conditions"""
    
    def test_complex_pipeline_chains(self):
        """Test complex pipeline operations"""
        # Create test data
        self.create_test_file("data1.txt", "data line 1\ndata line 2")
        self.create_test_file("data2.txt", "data line 3\ndata line 4")
        
        # Complex pipeline
        pipeline_cmd = 'cat data1.txt data2.txt | cat | cat > pipeline_result.txt'
        
        stdout, stderr, code = self.run_olshell_command(pipeline_cmd)
        self.assertNotEqual(code, -1)
        
        # Verify result
        if self.file_exists("pipeline_result.txt"):
            content = self.read_file_content("pipeline_result.txt")
            self.assertIn("data line 1", content)
            self.assertIn("data line 4", content)
    
    def test_mixed_operation_stress(self):
        """Test mixed operations under stress"""
        stress_operations = [
            # File creation
            'echo "stress test content" > stress1.txt',
            'echo "more content" > stress2.txt',
            
            # Directory operations
            'ls',
            'pwd',
            
            # File manipulation
            'cat stress1.txt',
            'cat stress1.txt stress2.txt > combined_stress.txt',
            
            # Pipeline operations
            'echo "pipeline stress" | cat',
            'cat combined_stress.txt | cat > final_stress.txt',
            
            # Cleanup
            'ls',
            'rm stress1.txt stress2.txt combined_stress.txt final_stress.txt'
        ]
        
        command_string = '\n'.join(stress_operations)
        
        start_time = time.time()
        stdout, stderr, code = self.run_olshell_command(command_string)
        end_time = time.time()
        
        self.assertNotEqual(code, -1, "Stress test failed")
        self.assertLess(end_time - start_time, 10.0, "Stress test too slow")
        self.assertIn("stress test content", stdout)
    
    def test_error_recovery_stress(self):
        """Test error recovery under stress"""
        mixed_commands = [
            'echo "good command 1"',
            'nonexistent_command_1',  # Should fail gracefully
            'echo "good command 2"',
            'cat nonexistent_file.txt',  # Should fail gracefully
            'echo "good command 3"',
            'cd /nonexistent/path',  # Should fail gracefully
            'echo "good command 4"',
            'pwd'  # Should still work
        ]
        
        command_string = '\n'.join(mixed_commands)
        
        stdout, stderr, code = self.run_olshell_command(command_string)
        
        # Shell should not crash despite errors
        self.assertNotEqual(code, -1)
        self.assertIn("good command 1", stdout)
        self.assertIn("good command 4", stdout)


class TestResourceManagement(OlshellTestBase):
    """Test resource management and cleanup"""
    
    def test_file_handle_management(self):
        """Test that file handles are managed properly"""
        # Create and access many files
        for i in range(20):
            self.create_test_file(f"handle_test_{i}.txt", f"Content {i}")
        
        # Access all files multiple times
        for i in range(20):
            stdout, stderr, code = self.run_olshell_command(f'cat handle_test_{i}.txt')
            self.assertNotEqual(code, -1)
            self.assertIn(f"Content {i}", stdout)
    
    def test_directory_handle_cleanup(self):
        """Test directory handle cleanup"""
        # Create nested directory structure
        base_path = Path(self.test_dir)
        for i in range(10):
            dir_path = base_path / f"dir_{i}"
            dir_path.mkdir()
            (dir_path / f"file_{i}.txt").write_text(f"File in dir {i}")
        
        # Navigate through directories
        for i in range(10):
            stdout, stderr, code = self.run_olshell_command(f'cd dir_{i}\nls\ncd ..')
            self.assertNotEqual(code, -1)
    
    def test_command_cleanup(self):
        """Test that commands clean up properly"""
        # Run many commands that create temporary state
        cleanup_commands = []
        for i in range(15):
            cleanup_commands.extend([
                f'echo "temp {i}" > temp_{i}.txt',
                f'cat temp_{i}.txt',
                f'rm temp_{i}.txt'
            ])
        
        command_string = '\n'.join(cleanup_commands)
        
        stdout, stderr, code = self.run_olshell_command(command_string)
        self.assertNotEqual(code, -1)
        
        # Verify cleanup worked
        for i in range(15):
            self.assertFalse(self.file_exists(f"temp_{i}.txt"))


if __name__ == '__main__':
    print("=== OlShell Performance and Stress Tests ===")
    print("Testing performance characteristics and stress scenarios...")
    print()
    
    unittest.main(verbosity=2)
