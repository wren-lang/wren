var Module = "outer"

if (true) {
  import "./module" for Module
  // expect: ran module

  System.print(Module) // expect: from module
}

System.print(Module) // expect: outer
