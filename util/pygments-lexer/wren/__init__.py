import re
from pygments import highlight
from pygments.lexers import PythonLexer
from pygments.formatters import HtmlFormatter

from pygments.lexer import RegexLexer
from pygments.token import *

class WrenLexer(RegexLexer):
    name = 'Wren'
    aliases = ['wren']
    filenames = ['*.wren']

    flags = re.MULTILINE | re.DOTALL

    tokens = {
        'root': [
            # Whitespace.
            (r'\s+', Text),
            (r'[,\\\[\]{}]', Punctuation),

            # Push a parenthesized state so that we know the corresponding ')'
            # is for a parenthesized expression and not interpolation.
            (r'\(', Punctuation, ('parenthesized', 'root')),

            # In this state, we don't know whether a closing ')' is for a
            # parenthesized expression or the end of an interpolation. So, do
            # a non-consuming match and let the parent state (either
            # 'parenthesized' or 'interpolation' decide.
            (r'(?=\))', Text, '#pop'),

            # Keywords.
            (r'(break|class|construct|else|for|foreign|if|import|in|is|'
             r'return|static|super|var|while)\b', Keyword),

            (r'(true|false|null)\b', Keyword.Constant),

            (r'this\b', Name.Builtin),

            # Comments.
            (r'/\*', Comment.Multiline, 'comment'),
            (r'//.*?$', Comment.Single),

            # Names and operators.
            (r'[~!$%^&*\-=+\\|/?<>\.:]+', Operator),
            (r'[A-Z][a-zA-Z_0-9]+', Name.Variable.Global),
            (r'__[a-zA-Z_0-9]+', Name.Variable.Class),
            (r'_[a-zA-Z_0-9]+', Name.Variable.Instance),
            (r'[a-z][a-zA-Z_0-9]+', Name),

            # Numbers.
            (r'\d+\.\d+([eE]-?\d+)?', Number.Float),
            (r'0x[0-9a-fA-F]+', Number.Hex),
            (r'\d+', Number.Integer),

            # Strings.
            (r'L?"', String, 'string'),
        ],
        'comment': [
            (r'/\*', Comment.Multiline, '#push'),
            (r'\*/', Comment.Multiline, '#pop'),
            (r'.', Comment.Multiline), # All other characters.
        ],
        'string': [
            (r'"', String, '#pop'),
            (r'\\[\\%0abfnrtv"\']', String.Escape), # Escape.
            (r'\\x[a-fA-F0-9]{2}', String.Escape), # Byte escape.
            (r'\\u[a-fA-F0-9]{4}', String.Escape), # Unicode escape.
            (r'\\U[a-fA-F0-9]{8}', String.Escape), # Long Unicode escape.

            (r'%\(', String.Interpol, ('interpolation', 'root')),
            (r'.', String), # All other characters.
        ],
        'parenthesized': [
            # We only get to this state when we're at a ')'.
            (r'\)', Punctuation, '#pop'),
        ],
        'interpolation': [
            # We only get to this state when we're at a ')'.
            (r'\)', String.Interpol, '#pop'),
        ],
    }
