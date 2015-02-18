import "cthulu" for Cthulu

class Lovecraft {
    say {
        return (new Cthulu).message
    }
}

IO.print((new Lovecraft).say)
