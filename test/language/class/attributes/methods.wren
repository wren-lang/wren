
class Methods {

  #!getter
  method {}

  method() {}

  #!regular = 2
  #!group(key, other=value, string="hello")
  method(arg0, arg1) {}

  #!is_static = true
  static method() {}

}

var methodAttr = Methods.attributes.methods
var getter = methodAttr["method"]
var none = methodAttr["method()"]
var regular = methodAttr["method(_,_)"]
var aStatic = methodAttr["static method()"]

// (Be wary of relying on map order)

System.print(getter)                        // expect: {null: {getter: [null]}}
System.print(none)                          // expect: null
System.print(regular[null])                 // expect: {regular: [2]}
System.print(regular["group"]["key"])       // expect: [null]
System.print(regular["group"]["other"])     // expect: [value]
System.print(regular["group"]["string"])    // expect: [hello]
System.print(aStatic[null])                 // expect: {is_static: [true]}
