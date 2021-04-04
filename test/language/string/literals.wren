System.print("".count)   // expect: 0
System.print("a string") // expect: a string

// Non-ASCII.
System.print("A~¶Þॐஃ") // expect: A~¶Þॐஃ

// Raw strings.
System.print("""A raw string""") // expect: A raw string

var long = "
  A
  multi line
  regular string
"
System.print(long) // expect: 
                   // expect:   A
                   // expect:   multi line
                   // expect:   regular string
                   // expect: 

var raw = """
  A if*(<invalid>)*
  multi line /{}()
  raw string [\]/
  "json": "value"
"""
System.print(raw) // expect:   A if*(<invalid>)*
                  // expect:   multi line /{}()
                  // expect:   raw string [\]/
                  // expect:   "json": "value"