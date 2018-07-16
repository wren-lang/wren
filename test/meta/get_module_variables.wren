import "meta" for Meta

var variables = Meta.getModuleVariables("./test/meta/get_module_variables")

// Includes implicitly imported core stuff.
System.print(variables.contains("Object")) // expect: true
System.print(variables.contains("Bool")) // expect: true

// Includes top level variables.
System.print(variables.contains("variables")) // expect: true

// Even ones declared later.
System.print(variables.contains("later")) // expect: true

var later = "values"

System.print(variables.contains("unknown")) // expect: false
