var Module1 = "module.wren".import_("Module1")
var Module2 = "module.wren".import_("Module2")
var Module3 = "module.wren".import_("Module3")
var Module4 = "module.wren".import_("Module4")
var Module5 = "module.wren".import_("Module5")

// Only execute module body once:
// expect: ran module

IO.print(Module1) // expect: from module one
IO.print(Module2) // expect: from module two
IO.print(Module3) // expect: from module three
IO.print(Module4) // expect: from module four
IO.print(Module5) // expect: from module five
