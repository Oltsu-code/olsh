"""
Test runner for all olshell tests
Provides convenient way to run all test suites
"""

import unittest
import sys
import os
from pathlib import Path

# Add the tests directory to the path
tests_dir = Path(__file__).parent
sys.path.insert(0, str(tests_dir))

# Import all test modules
try:
    from test_comprehensive import *
    from test_input_handling import *
    from test_performance import *
    from test_edge_cases import *
except ImportError as e:
    print(f"Warning: Could not import all test modules: {e}")
    print("Some tests may not be available.")


def create_test_suite():
    """Create a comprehensive test suite with all tests"""
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()
    
    # Test modules to include
    test_modules = [
        'test_comprehensive',
        'test_input_handling', 
        'test_performance',
        'test_edge_cases'
    ]
    
    for module_name in test_modules:
        try:
            if module_name in sys.modules:
                module = sys.modules[module_name]
                suite.addTests(loader.loadTestsFromModule(module))
                print(f"✓ Loaded tests from {module_name}")
            else:
                print(f"⚠ Could not load {module_name}")
        except Exception as e:
            print(f"✗ Error loading {module_name}: {e}")
    
    return suite


def run_specific_test_category(category):
    """Run tests from a specific category"""
    loader = unittest.TestLoader()
    
    if category == "comprehensive":
        import test_comprehensive
        suite = loader.loadTestsFromModule(test_comprehensive)
    elif category == "input":
        import test_input_handling
        suite = loader.loadTestsFromModule(test_input_handling)
    elif category == "performance":
        import test_performance
        suite = loader.loadTestsFromModule(test_performance)
    elif category == "edge":
        import test_edge_cases
        suite = loader.loadTestsFromModule(test_edge_cases)
    else:
        print(f"Unknown test category: {category}")
        print("Available categories: comprehensive, input, performance, edge")
        return
    
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    return result.wasSuccessful()


def main():
    """Main test runner"""
    print("=" * 60)
    print("OlShell Test Suite")
    print("=" * 60)
    print()
    
    # Check if specific category requested
    if len(sys.argv) > 1:
        category = sys.argv[1].lower()
        print(f"Running {category} tests only...")
        print()
        success = run_specific_test_category(category)
        sys.exit(0 if success else 1)
    
    # Run all tests
    print("Running all test categories...")
    print()
    
    # Create and run the full test suite
    suite = create_test_suite()
    
    if suite.countTestCases() == 0:
        print("No tests found to run!")
        sys.exit(1)
    
    print(f"Found {suite.countTestCases()} tests total")
    print()
    
    # Run the tests
    runner = unittest.TextTestRunner(
        verbosity=2,
        stream=sys.stdout,
        buffer=True  # Capture output during test execution
    )
    
    result = runner.run(suite)
    
    # Print summary
    print()
    print("=" * 60)
    print("TEST SUMMARY")
    print("=" * 60)
    print(f"Tests run: {result.testsRun}")
    print(f"Failures: {len(result.failures)}")
    print(f"Errors: {len(result.errors)}")
    print(f"Skipped: {len(result.skipped)}")
    print()
    
    if result.failures:
        print("FAILURES:")
        for test, traceback in result.failures:
            print(f"  - {test}")
        print()
    
    if result.errors:
        print("ERRORS:")
        for test, traceback in result.errors:
            print(f"  - {test}")
        print()
    
    if result.wasSuccessful():
        print("🎉 ALL TESTS PASSED!")
        exit_code = 0
    else:
        print("❌ SOME TESTS FAILED")
        exit_code = 1
    
    print()
    sys.exit(exit_code)


if __name__ == '__main__':
    main()
