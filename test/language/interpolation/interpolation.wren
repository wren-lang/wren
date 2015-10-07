// Full string.
System.print("%(1 + 2)") // expect: 3

// Multiple in one string.
System.print("str%(1 + 2)(%(3 + 4)\%%(5 + 6)") // expect: str3(7%11

// Nested.
System.print("[%("{%("in" + "ner")}")]") // expect: [{inner}]

// Ignore newlines in template.
System.print("[%(

"template"

)]") // expect: [template]
