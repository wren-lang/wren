// Should execute the module:
// expect: ran module
var DoesNotExist = "module".import_("DoesNotExist") // expect runtime error: Could not find a variable named 'DoesNotExist' in module 'module'.
