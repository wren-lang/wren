// Should execute the module:
// expect: ran module
import "module" for DoesNotExist // expect runtime error: Could not find a variable named 'DoesNotExist' in module 'module'.
