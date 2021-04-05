System.print("".count)   // expect: 0
System.print("a string") // expect: a string

// Non-ASCII.
System.print("A~¶Þॐஃ") // expect: A~¶Þॐஃ

// Raw strings.
System.print("""A raw string""") // expect: A raw string
System.print("""   A raw string""") // expect:    A raw string
System.print("""A raw string   """) // expect: A raw string   

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

// Raw strings ignore whitespace on the line with the """

var noNewlines = """
no newlines
"""
System.print(noNewlines) // expect: no newlines

// Spaces after the """ but before the \n
var noLeadingSpaces = """    
no leading spaces
"""
System.print(noLeadingSpaces) // expect: no leading spaces

// Spaces before the end """ after the \n
var noTrailingSpaces = """    
no trailing spaces
       """
System.print(noTrailingSpaces) // expect: no trailing spaces

var newlineBefore = """    
newline before"""
System.print(newlineBefore) // expect: newline before

var newlineAfter = """newline after
"""
System.print(newlineAfter) // expect: newline after
