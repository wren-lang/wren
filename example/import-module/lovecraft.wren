import "cthulu" for Cthulu

class Lovecraft {
  say { (new Cthulu).message }
}

IO.print((new Lovecraft).say)
