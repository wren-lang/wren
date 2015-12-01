var fn = Fn.new {|arg| System.print(arg) }

fn("string") // expect: string

fn = Fn.new {|block| System.print(block()) }
fn { "block" } // expect: block

fn = Fn.new {|a, b, c| System.print("%(a) %(b) %(c())") }
fn(1, 2) { 3 } // expect: 1 2 3
