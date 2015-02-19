IO.print("".count)   // expect: 0
IO.print("a string") // expect: a string

// Non-ASCII.
IO.print("A~¶Þॐஃ") // expect: A~¶Þॐஃ

// Multiline/Raw
IO.print([[ This is a raw \string ]]) // expect:  This is a raw \string 
IO.print([[
This shouldn't [[ start with a newline ]]
]]) // expect: This shouldn't [[ start with a newline ]]
    // expect: 

