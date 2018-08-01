import "io" for Stdin, Stdout

// Implements the classic "Animals" guessing game. The user thinks of an animal.
// The program asks a series of yes/no questions to try to guess the animal
// they are thinking of. If the program fails, it asks the user for a new
// question and adds the animal to its knowledge base.
//
// Internally, the program's brain is stored as a binary tree. Leaf nodes are
// animals. Internal nodes are yes/no questions that choose which branch to
// explore.

class Node {
  // Reads a "yes" or "no" (or something approximating) those and returns true
  // if yes was entered.
  promptYesNo(prompt) {
    while (true) {
      var line = promptString(prompt)

      if (line.startsWith("y") || line.startsWith("Y")) return true
      if (line.startsWith("n") || line.startsWith("N")) return false

      // Quit.
      if (line.startsWith("q") || line.startsWith("Q")) Fiber.yield()
    }
  }

  // Writes a prompt and reads a string of input.
  promptString(prompt) {
    System.write("%(prompt) ")
    Stdout.flush()
    return Stdin.readLine()
  }
}

class Animal is Node {
  construct new(name) {
    _name = name
  }

  ask() {
    // Hit a leaf, so see if we guessed it.
    if (promptYesNo("Is it a %(_name)?")) {
      System.print("I won! Let's play again!")
      return null
    }

    // Nope, so add a new animal and turn this node into a branch.
    var name = promptString("I lost! What was your animal?")
    var question = promptString(
        "What question would distinguish a %(_name) from a %(name)?")
    var isYes = promptYesNo(
        "Is the answer to the question 'yes' for a %(name)?")
    System.print("I'll remember that. Let's play again!")

    var animal = Animal.new(name)
    return Question.new(question, isYes ? animal : this, isYes ? this : animal)
  }
}

class Question is Node {
  construct new(question, ifYes, ifNo) {
    _question = question
    _ifYes = ifYes
    _ifNo = ifNo
  }

  ask() {
    // Recurse into the branches.
    if (promptYesNo(_question)) {
      var result = _ifYes.ask()
      if (result != null) _ifYes = result
    } else {
      var result = _ifNo.ask()
      if (result != null) _ifNo = result
    }

    return null
  }
}

var root = Question.new("Does it live in the water?",
    Animal.new("frog"), Animal.new("goat"))

// Play games until the user quits.
Fiber.new {
  while (true) root.ask()
}.call()
