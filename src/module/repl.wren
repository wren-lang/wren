import "meta" for Meta
import "io" for Stdin, Stdout
import "os" for Platform

/// Abstract base class for the REPL. Manages the input line and history, but
/// does not render.
class Repl {
  construct new() {
    _cursor = 0
    _line = ""

    _history = []
    _historyIndex = 0
  }

  cursor { _cursor }
  cursor=(value) { _cursor = value }
  line { _line }
  line=(value) { _line = value }

  run() {
    Stdin.isRaw = true
    refreshLine(false)

    while (true) {
      var byte = Stdin.readByte()
      if (handleChar(byte)) break
      refreshLine(true)
    }
  }

  handleChar(byte) {
    if (byte == Chars.ctrlC) {
      System.print()
      return true
    } else if (byte == Chars.ctrlD) {
      // If the line is empty, Ctrl_D exits.
      if (_line.isEmpty) {
        System.print()
        return true
      }

      // Otherwise, it deletes the character after the cursor.
      deleteRight()
    } else if (byte == Chars.tab) {
      var completion = getCompletion()
      if (completion != null) {
        _line = _line + completion
        _cursor = _line.count
      }
    } else if (byte == Chars.ctrlU) {
      // Clear the line.
      _line = ""
      _cursor = 0
    } else if (byte == Chars.ctrlN) {
      nextHistory()
    } else if (byte == Chars.ctrlP) {
      previousHistory()
    } else if (byte == Chars.escape) {
      var escapeType = Stdin.readByte()
      var value = Stdin.readByte()
      if (escapeType == Chars.leftBracket) {
        // ESC [ sequence.
        handleEscapeBracket(value)
      } else {
        // TODO: Handle ESC 0 sequences.
      }
    } else if (byte == Chars.carriageReturn) {
      executeInput()
    } else if (byte == Chars.delete) {
      deleteLeft()
    } else if (byte >= Chars.space && byte <= Chars.tilde) {
      insertChar(byte)
    } else {
      // TODO: Other shortcuts?
      System.print("Unhandled key-code [dec]: %(byte)")
    }

    return false
  }

  /// Inserts the character with [byte] value at the current cursor position.
  insertChar(byte) {
    var char = String.fromCodePoint(byte)
    _line = _line[0..._cursor] + char + _line[_cursor..-1]
    _cursor = _cursor + 1
  }

  /// Deletes the character before the cursor, if any.
  deleteLeft() {
    if (_cursor == 0) return

    // Delete the character before the cursor.
    _line = _line[0...(_cursor - 1)] + _line[_cursor..-1]
    _cursor = _cursor - 1
  }

  /// Deletes the character after the cursor, if any.
  deleteRight() {
    if (_cursor == _line.count) return

    // Delete the character after the cursor.
    _line = _line[0..._cursor] + _line[(_cursor + 1)..-1]
  }

  handleEscapeBracket(byte) {
    if (byte == EscapeBracket.up) {
      previousHistory()
    } else if (byte == EscapeBracket.down) {
      nextHistory()
    }
  }

  previousHistory() {
    if (_historyIndex == 0) return

    _historyIndex = _historyIndex - 1
    _line = _history[_historyIndex]
    _cursor = _line.count
  }

  nextHistory() {
    if (_historyIndex >= _history.count) return

    _historyIndex = _historyIndex + 1
    if (_historyIndex < _history.count) {
      _line = _history[_historyIndex]
      _cursor = _line.count
    } else {
      _line = ""
      _cursor = 0
    }
  }

  executeInput() {
    // Remove the completion hint.
    refreshLine(false)

    // Add it to the history (if the line is interesting).
    if (_line != "" && (_history.isEmpty || _history[-1] != _line)) {
      _history.add(_line)
      _historyIndex = _history.count
    }

    // Reset the current line.
    var input = _line
    _line = ""
    _cursor = 0

    System.print()

    // Guess if it looks like a statement or expression. If it looks like an
    // expression, we try to print the result.
    var token = lexFirst(input)

    // No code, so do nothing.
    if (token == null) return

    var isStatement =
        token.type == Token.breakKeyword ||
        token.type == Token.classKeyword ||
        token.type == Token.forKeyword ||
        token.type == Token.foreignKeyword ||
        token.type == Token.ifKeyword ||
        token.type == Token.importKeyword ||
        token.type == Token.returnKeyword ||
        token.type == Token.varKeyword ||
        token.type == Token.whileKeyword

    var closure
    if (isStatement) {
      closure = Meta.compile(input)
    } else {
      closure = Meta.compileExpression(input)
    }

    // Stop if there was a compile error.
    if (closure == null) return

    var fiber = Fiber.new(closure)

    var result = fiber.try()
    if (fiber.error != null) {
      // TODO: Include callstack.
      showRuntimeError("Runtime error: %(fiber.error)")
      return
    }

    if (!isStatement) {
      showResult(result)
    }
  }

  lex(line, includeWhitespace) {
    var lexer = Lexer.new(line)
    var tokens = []
    while (true) {
      var token = lexer.readToken()
      if (token.type == Token.eof) break

      if (includeWhitespace ||
          (token.type != Token.comment && token.type != Token.whitespace)) {
        tokens.add(token)
      }
    }

    return tokens
  }

  lexFirst(line) {
    var lexer = Lexer.new(line)
    while (true) {
      var token = lexer.readToken()
      if (token.type == Token.eof) return null

      if (token.type != Token.comment && token.type != Token.whitespace) {
        return token
      }
    }
  }

  /// Gets the best possible auto-completion for the current line, or null if
  /// there is none. The completion is the remaining string to append to the
  /// line, not the entire completed line.
  getCompletion() {
    if (_line.isEmpty) return null

    // Only complete if the cursor is at the end.
    if (_cursor != _line.count) return null

    for (name in Meta.getModuleVariables("repl")) {
      // TODO: Also allow completion if the line ends with an identifier but
      // has other stuff before it.
      if (name.startsWith(_line)) {
        return name[_line.count..-1]
      }
    }
  }
}

/// A reduced functionality REPL that doesn't use ANSI escape sequences.
class SimpleRepl is Repl {
  construct new() {
    super()
    _erase = ""
  }

  refreshLine(showCompletion) {
    // A carriage return just moves the cursor to the beginning of the line.
    // We have to erase it manually. Since we can't use ANSI escapes, and we
    // don't know how wide the terminal is, erase the longest line we've seen
    // so far.
    if (line.count > _erase.count) _erase = " " * line.count
    System.write("\r  %(_erase)")

    // Show the prompt at the beginning of the line.
    System.write("\r> ")

    // Write the line.
    System.write(line)
    Stdout.flush()
  }

  showResult(value) {
    // TODO: Syntax color based on type? It might be nice to distinguish
    // between string results versus stringified results. Otherwise, the
    // user can't tell the difference between `true` and "true".
    System.print(value)
  }

  showRuntimeError(message) {
    System.print(message)
  }
}

class AnsiRepl is Repl {
  construct new() {
    super()
  }

  handleChar(byte) {
    if (byte == Chars.ctrlA) {
      cursor = 0
    } else if (byte == Chars.ctrlB) {
      cursorLeft()
    } else if (byte == Chars.ctrlE) {
      cursor = line.count
    } else if (byte == Chars.ctrlF) {
      cursorRight()
    } else if (byte == Chars.ctrlK) {
      // Delete everything after the cursor.
      line = line[0...cursor]
    } else if (byte == Chars.ctrlL) {
      // Clear the screen.
      System.write("\x1b[2J")
      // Move cursor to top left.
      System.write("\x1b[H")
    } else {
      // TODO: Ctrl-T to swap chars.
      // TODO: ESC H and F to move to beginning and end of line. (Both ESC
      // [ and ESC 0 sequences?)
      // TODO: Ctrl-W delete previous word.
      return super.handleChar(byte)
    }

    return false
  }

  handleEscapeBracket(byte) {
    if (byte == EscapeBracket.left) {
      cursorLeft()
    } else if (byte == EscapeBracket.right) {
      cursorRight()
    }

    super.handleEscapeBracket(byte)
  }

  /// Move the cursor left one character.
  cursorLeft() {
    if (cursor > 0) cursor = cursor - 1
  }

  /// Move the cursor right one character.
  cursorRight() {
    // TODO: Take into account multi-byte characters?
    if (cursor < line.count) cursor = cursor + 1
  }

  refreshLine(showCompletion) {
    // Erase the whole line.
    System.write("\x1b[2K")

    // Show the prompt at the beginning of the line.
    System.write(Color.gray)
    System.write("\r> ")
    System.write(Color.none)

    // Syntax highlight the line.
    for (token in lex(line, true)) {
      if (token.type == Token.eof) break

      System.write(TOKEN_COLORS[token.type])
      System.write(token.text)
      System.write(Color.none)
    }

    if (showCompletion) {
      var completion = getCompletion()
      if (completion != null) {
        System.write("%(Color.gray)%(completion)%(Color.none)")
      }
    }

    // Position the cursor.
    System.write("\r\x1b[%(2 + cursor)C")
    Stdout.flush()
  }

  showResult(value) {
    // TODO: Syntax color based on type? It might be nice to distinguish
    // between string results versus stringified results. Otherwise, the
    // user can't tell the difference between `true` and "true".
    System.print("%(Color.brightWhite)%(value)%(Color.none)")
  }

  showRuntimeError(message) {
    System.print("%(Color.red)%(message)%(Color.none)")
    // TODO: Print entire stack.
  }
}

/// ANSI color escape sequences.
class Color {
  static none { "\x1b[0m" }
  static black { "\x1b[30m" }
  static red { "\x1b[31m" }
  static green { "\x1b[32m" }
  static yellow { "\x1b[33m" }
  static blue { "\x1b[34m" }
  static magenta { "\x1b[35m" }
  static cyan { "\x1b[36m" }
  static white { "\x1b[37m" }

  static gray { "\x1b[30;1m" }
  static pink { "\x1b[31;1m" }
  static brightWhite { "\x1b[37;1m" }
}

/// Utilities for working with characters.
class Chars {
  static ctrlA { 0x01 }
  static ctrlB { 0x02 }
  static ctrlC { 0x03 }
  static ctrlD { 0x04 }
  static ctrlE { 0x05 }
  static ctrlF { 0x06 }
  static tab { 0x09 }
  static lineFeed { 0x0a }
  static ctrlK { 0x0b }
  static ctrlL { 0x0c }
  static carriageReturn { 0x0d }
  static ctrlN { 0x0e }
  static ctrlP { 0x10 }
  static ctrlU { 0x15 }
  static escape { 0x1b }
  static space { 0x20 }
  static bang { 0x21 }
  static quote { 0x22 }
  static percent { 0x25 }
  static amp { 0x26 }
  static leftParen { 0x28 }
  static rightParen { 0x29 }
  static star { 0x2a }
  static plus { 0x2b }
  static comma { 0x2c }
  static minus { 0x2d }
  static dot { 0x2e }
  static slash { 0x2f }

  static zero { 0x30 }
  static nine { 0x39 }

  static colon { 0x3a }
  static less { 0x3c }
  static equal { 0x3d }
  static greater { 0x3e }
  static question { 0x3f }

  static upperA { 0x41 }
  static upperF { 0x46 }
  static upperZ { 0x5a }

  static leftBracket { 0x5b }
  static backslash { 0x5c }
  static rightBracket { 0x5d }
  static caret { 0x5e }
  static underscore { 0x5f }

  static lowerA { 0x61 }
  static lowerF { 0x66 }
  static lowerX { 0x78 }
  static lowerZ { 0x7a }

  static leftBrace { 0x7b }
  static pipe { 0x7c }
  static rightBrace { 0x7d }
  static tilde { 0x7e }
  static delete { 0x7f }

  static isAlpha(c) {
    return c >= lowerA && c <= lowerZ ||
           c >= upperA && c <= upperZ ||
           c == underscore
  }

  static isDigit(c) { c >= zero && c <= nine }

  static isAlphaNumeric(c) { isAlpha(c) || isDigit(c) }

  static isHexDigit(c) {
    return c >= zero && c <= nine ||
           c >= lowerA && c <= lowerF ||
           c >= upperA && c <= upperF
  }

  static isLowerAlpha(c) { c >= lowerA && c <= lowerZ }

  static isWhitespace(c) { c == space || c == tab || c == carriageReturn }
}

class EscapeBracket {
  static up { 0x41 }
  static down { 0x42 }
  static right { 0x43 }
  static left { 0x44 }
}

class Token {
  // Punctuators.
  static leftParen { "leftParen" }
  static rightParen { "rightParen" }
  static leftBracket { "leftBracket" }
  static rightBracket { "rightBracket" }
  static leftBrace { "leftBrace" }
  static rightBrace { "rightBrace" }
  static colon { "colon" }
  static dot { "dot" }
  static dotDot { "dotDot" }
  static dotDotDot { "dotDotDot" }
  static comma { "comma" }
  static star { "star" }
  static slash { "slash" }
  static percent { "percent" }
  static plus { "plus" }
  static minus { "minus" }
  static pipe { "pipe" }
  static pipePipe { "pipePipe" }
  static caret { "caret" }
  static amp { "amp" }
  static ampAmp { "ampAmp" }
  static question { "question" }
  static bang { "bang" }
  static tilde { "tilde" }
  static equal { "equal" }
  static less { "less" }
  static lessEqual { "lessEqual" }
  static lessLess { "lessLess" }
  static greater { "greater" }
  static greaterEqual { "greaterEqual" }
  static greaterGreater { "greaterGreater" }
  static equalEqual { "equalEqual" }
  static bangEqual { "bangEqual" }

  // Keywords.
  static breakKeyword { "break" }
  static classKeyword { "class" }
  static constructKeyword { "construct" }
  static elseKeyword { "else" }
  static falseKeyword { "false" }
  static forKeyword { "for" }
  static foreignKeyword { "foreign" }
  static ifKeyword { "if" }
  static importKeyword { "import" }
  static inKeyword { "in" }
  static isKeyword { "is" }
  static nullKeyword { "null" }
  static returnKeyword { "return" }
  static staticKeyword { "static" }
  static superKeyword { "super" }
  static thisKeyword { "this" }
  static trueKeyword { "true" }
  static varKeyword { "var" }
  static whileKeyword { "while" }

  static field { "field" }
  static name { "name" }
  static number { "number" }
  static string { "string" }
  static interpolation { "interpolation" }
  static comment { "comment" }
  static whitespace { "whitespace" }
  static line { "line" }
  static error { "error" }
  static eof { "eof" }

  construct new(source, type, start, length) {
    _source = source
    _type = type
    _start = start
    _length = length
  }

  type { _type }
  text { _source[_start...(_start + _length)] }

  start { _start }
  length { _length }

  toString { text }
}

var KEYWORDS = {
  "break": Token.breakKeyword,
  "class": Token.classKeyword,
  "construct": Token.constructKeyword,
  "else": Token.elseKeyword,
  "false": Token.falseKeyword,
  "for": Token.forKeyword,
  "foreign": Token.foreignKeyword,
  "if": Token.ifKeyword,
  "import": Token.importKeyword,
  "in": Token.inKeyword,
  "is": Token.isKeyword,
  "null": Token.nullKeyword,
  "return": Token.returnKeyword,
  "static": Token.staticKeyword,
  "super": Token.superKeyword,
  "this": Token.thisKeyword,
  "true": Token.trueKeyword,
  "var": Token.varKeyword,
  "while": Token.whileKeyword
}

var TOKEN_COLORS = {
  Token.leftParen: Color.gray,
  Token.rightParen: Color.gray,
  Token.leftBracket: Color.gray,
  Token.rightBracket: Color.gray,
  Token.leftBrace: Color.gray,
  Token.rightBrace: Color.gray,
  Token.colon: Color.gray,
  Token.dot: Color.gray,
  Token.dotDot: Color.none,
  Token.dotDotDot: Color.none,
  Token.comma: Color.gray,
  Token.star: Color.none,
  Token.slash: Color.none,
  Token.percent: Color.none,
  Token.plus: Color.none,
  Token.minus: Color.none,
  Token.pipe: Color.none,
  Token.pipePipe: Color.none,
  Token.caret: Color.none,
  Token.amp: Color.none,
  Token.ampAmp: Color.none,
  Token.question: Color.none,
  Token.bang: Color.none,
  Token.tilde: Color.none,
  Token.equal: Color.none,
  Token.less: Color.none,
  Token.lessEqual: Color.none,
  Token.lessLess: Color.none,
  Token.greater: Color.none,
  Token.greaterEqual: Color.none,
  Token.greaterGreater: Color.none,
  Token.equalEqual: Color.none,
  Token.bangEqual: Color.none,

  // Keywords.
  Token.breakKeyword: Color.cyan,
  Token.classKeyword: Color.cyan,
  Token.constructKeyword: Color.cyan,
  Token.elseKeyword: Color.cyan,
  Token.falseKeyword: Color.cyan,
  Token.forKeyword: Color.cyan,
  Token.foreignKeyword: Color.cyan,
  Token.ifKeyword: Color.cyan,
  Token.importKeyword: Color.cyan,
  Token.inKeyword: Color.cyan,
  Token.isKeyword: Color.cyan,
  Token.nullKeyword: Color.cyan,
  Token.returnKeyword: Color.cyan,
  Token.staticKeyword: Color.cyan,
  Token.superKeyword: Color.cyan,
  Token.thisKeyword: Color.cyan,
  Token.trueKeyword: Color.cyan,
  Token.varKeyword: Color.cyan,
  Token.whileKeyword: Color.cyan,

  Token.field: Color.none,
  Token.name: Color.none,
  Token.number: Color.magenta,
  Token.string: Color.yellow,
  Token.interpolation: Color.yellow,
  Token.comment: Color.gray,
  Token.whitespace: Color.none,
  Token.line: Color.none,
  Token.error: Color.red,
  Token.eof: Color.none,
}

// Data table for tokens that are tokenized using maximal munch.
//
// The key is the character that starts the token or tokens. After that is a
// list of token types and characters. As long as the next character is matched,
// the type will update to the type after that character.
var PUNCTUATORS = {
  Chars.leftParen: [Token.leftParen],
  Chars.rightParen: [Token.rightParen],
  Chars.leftBracket: [Token.leftBracket],
  Chars.rightBracket: [Token.rightBracket],
  Chars.leftBrace: [Token.leftBrace],
  Chars.rightBrace: [Token.rightBrace],
  Chars.colon: [Token.colon],
  Chars.comma: [Token.comma],
  Chars.star: [Token.star],
  Chars.percent: [Token.percent],
  Chars.plus: [Token.plus],
  Chars.minus: [Token.minus],
  Chars.tilde: [Token.tilde],
  Chars.caret: [Token.caret],
  Chars.question: [Token.question],
  Chars.lineFeed: [Token.line],

  Chars.pipe: [Token.pipe, Chars.pipe, Token.pipePipe],
  Chars.amp: [Token.amp, Chars.amp, Token.ampAmp],
  Chars.bang: [Token.bang, Chars.equal, Token.bangEqual],
  Chars.equal: [Token.equal, Chars.equal, Token.equalEqual],

  Chars.dot: [Token.dot, Chars.dot, Token.dotDot, Chars.dot, Token.dotDotDot]
}

/// Tokenizes a string of input. This lexer differs from most in that it
/// silently ignores errors from incomplete input, like a string literal with
/// no closing quote. That's because this is intended to be run on a line of
/// input while the user is still typing it.
class Lexer {
  construct new(source) {
    _source = source

    // Due to the magic of UTF-8, we can safely treat Wren source as a series
    // of bytes, since the only code points that are meaningful to Wren fit in
    // ASCII. The only place where non-ASCII code points can occur is inside
    // string literals and comments and the lexer safely treats those as opaque
    // bytes.
    _bytes = source.bytes

    _start = 0
    _current = 0

    // The stack of ongoing interpolated strings. Each element in the list is
    // a single level of interpolation nesting. The value of the element is the
    // number of unbalanced "(" still remaining to be closed.
    _interpolations = []
  }

  readToken() {
    if (_current >= _bytes.count) return makeToken(Token.eof)

    _start = _current
    var c = _bytes[_current]
    advance()

    if (!_interpolations.isEmpty) {
      if (c == Chars.leftParen) {
        _interpolations[-1] = _interpolations[-1] + 1
      } else if (c == Chars.rightParen) {
        _interpolations[-1] = _interpolations[-1] - 1

        // The last ")" in an interpolated expression ends the expression and
        // resumes the string.
        if (_interpolations[-1] == 0) {
          // This is the final ")", so the interpolation expression has ended.
          // This ")" now begins the next section of the template string.
          _interpolations.removeAt(-1)
          return readString()
        }
      }
    }

    if (PUNCTUATORS.containsKey(c)) {
      var punctuator = PUNCTUATORS[c]
      var type = punctuator[0]
      var i = 1
      while (i < punctuator.count) {
        if (!match(punctuator[i])) break
        type = punctuator[i + 1]
        i = i + 2
      }

      return makeToken(type)
    }

    // Handle "<", "<<", and "<=".
    if (c == Chars.less) {
      if (match(Chars.less)) return makeToken(Token.lessLess)
      if (match(Chars.equal)) return makeToken(Token.lessEqual)
      return makeToken(Token.less)
    }

    // Handle ">", ">>", and ">=".
    if (c == Chars.greater) {
      if (match(Chars.greater)) return makeToken(Token.greaterGreater)
      if (match(Chars.equal)) return makeToken(Token.greaterEqual)
      return makeToken(Token.greater)
    }

    // Handle "/", "//", and "/*".
    if (c == Chars.slash) {
      if (match(Chars.slash)) return readLineComment()
      if (match(Chars.star)) return readBlockComment()
      return makeToken(Token.slash)
    }

    if (c == Chars.underscore) return readField()
    if (c == Chars.quote) return readString()

    if (c == Chars.zero && peek() == Chars.lowerX) return readHexNumber()
    if (Chars.isWhitespace(c)) return readWhitespace()
    if (Chars.isDigit(c)) return readNumber()
    if (Chars.isAlpha(c)) return readName()

    return makeToken(Token.error)
  }

  // Reads a line comment until the end of the line is reached.
  readLineComment() {
    // A line comment stops at the newline since newlines are significant.
    while (peek() != Chars.lineFeed && !isAtEnd) {
      advance()
    }

    return makeToken(Token.comment)
  }

  readBlockComment() {
    // Block comments can nest.
    var nesting = 1
    while (nesting > 0) {
      // TODO: Report error.
      if (isAtEnd) break

      if (peek() == Chars.slash && peek(1) == Chars.star) {
        advance()
        advance()
        nesting = nesting + 1
      } else if (peek() == Chars.star && peek(1) == Chars.slash) {
        advance()
        advance()
        nesting = nesting - 1
        if (nesting == 0) break
      } else {
        advance()
      }
    }

    return makeToken(Token.comment)
  }

  // Reads a static or instance field.
  readField() {
    var type = Token.field

    // Read the rest of the name.
    while (match {|c| Chars.isAlphaNumeric(c) }) {}

    return makeToken(type)
  }

  // Reads a string literal.
  readString() {
    var type = Token.string

    while (!isAtEnd) {
      var c = _bytes[_current]
      advance()

      if (c == Chars.backslash) {
        // TODO: Process specific escapes and validate them.
        advance()
      } else if (c == Chars.percent) {
        // Consume the '('.
        if (!isAtEnd) advance()
        // TODO: Handle missing '('.
        _interpolations.add(1)
        type = Token.interpolation
        break
      } else if (c == Chars.quote) {
        break
      }
    }

    return makeToken(type)
  }

  // Reads a number literal.
  readHexNumber() {
    // Skip past the `x`.
    advance()

    // Read the rest of the number.
    while (match {|c| Chars.isHexDigit(c) }) {}
    return makeToken(Token.number)
  }

  // Reads a series of whitespace characters.
  readWhitespace() {
    // Read the rest of the whitespace.
    while (match {|c| Chars.isWhitespace(c) }) {}

    return makeToken(Token.whitespace)
  }

  // Reads a number literal.
  readNumber() {
    // Read the rest of the number.
    while (match {|c| Chars.isDigit(c) }) {}

    // TODO: Floating point, scientific.
    return makeToken(Token.number)
  }

  // Reads an identifier or keyword token.
  readName() {
    // Read the rest of the name.
    while (match {|c| Chars.isAlphaNumeric(c) }) {}

    var text = _source[_start..._current]
    var type = Token.name
    if (KEYWORDS.containsKey(text)) {
      type = KEYWORDS[text]
    }

    return Token.new(_source, type, _start, _current - _start)
  }

  // Returns `true` if we have scanned all characters.
  isAtEnd { _current >= _bytes.count }

  // Advances past the current character.
  advance() {
    _current = _current + 1
  }

  // Returns the byte value of the current character.
  peek() { peek(0) }

  // Returns the byte value of the character [n] bytes past the current
  // character.
  peek(n) {
    if (_current + n >= _bytes.count) return -1
    return _bytes[_current + n]
  }

  // Consumes the current character if it matches [condition], which can be a
  // numeric code point value or a function that takes a code point and returns
  // `true` if the code point matches.
  match(condition) {
    if (isAtEnd) return false

    var c = _bytes[_current]
    if (condition is Fn) {
      if (!condition.call(c)) return false
    } else if (c != condition) {
      return false
    }

    advance()
    return true
  }

  // Creates a token of [type] from the current character range.
  makeToken(type) { Token.new(_source, type, _start, _current - _start) }
}

// Fire up the REPL. We use ANSI when talking to a POSIX TTY.
if (Platform.isPosix && Stdin.isTerminal) {
  AnsiRepl.new().run()
} else {
  // ANSI escape sequences probably aren't supported, so degrade.
  SimpleRepl.new().run()
}
