// The program picks a random number from 1 to 100 and you have to guess it.
// Each time you guess, it tells you if your guess is too low, too high, or on
// the money.

import "io" for Stdin
import "random" for Random

var number = Random.new().int(100) + 1

while (true) {
  System.write("Guess a number: ")
  var guess = Num.fromString(Stdin.readLine())
  if (guess == null) {
    System.print("That's not a number!")
  } else if (guess < number) {
    System.print("Too low.")
  } else if (guess == number) {
    System.print("You win!")
    break
  } else {
    System.print("Too high.")
  }
}
