#include <stdio.h>
#include <string.h>

#include "lexer.h"

typedef struct
{
    Buffer* source;

    int start; // The beginning of the current token.
    int pos;

    Token* head;
    Token* tail;

} Lexer;

static void readName(Lexer* lexer);
static void readNumber(Lexer* lexer);
static void readString(Lexer* lexer);
static void readEmbedded(Lexer* lexer);
static void readWhitespace(Lexer* lexer);
static int isKeyword(Lexer* lexer, const char* keyword);
static int isName(char c);
static int isDigit(char c);
static char advance(Lexer* lexer);
static char peek(Lexer* lexer);
static void emitToken(Lexer* lexer, TokenType type);

Token* tokenize(Buffer* source)
{
    Lexer lexer;
    lexer.source = source;
    lexer.start = 0;
    lexer.pos = 0;
    lexer.head = NULL;
    lexer.tail = NULL;

    while (peek(&lexer) != '\0')
    {
        lexer.start = lexer.pos;

        char c = advance(&lexer);
        switch (c)
        {
            case '(': emitToken(&lexer, TOKEN_LEFT_PAREN); break;
            case ')': emitToken(&lexer, TOKEN_RIGHT_PAREN); break;
            case '[': emitToken(&lexer, TOKEN_LEFT_BRACKET); break;
            case ']': emitToken(&lexer, TOKEN_RIGHT_BRACKET); break;
            case '{': emitToken(&lexer, TOKEN_LEFT_BRACE); break;
            case '}': emitToken(&lexer, TOKEN_RIGHT_BRACE); break;
            case ':': emitToken(&lexer, TOKEN_COLON); break;
            case '.': emitToken(&lexer, TOKEN_DOT); break;
            case ',': emitToken(&lexer, TOKEN_COMMA); break;
            case '*': emitToken(&lexer, TOKEN_STAR); break;
            case '/': emitToken(&lexer, TOKEN_SLASH); break;
            case '%': emitToken(&lexer, TOKEN_PERCENT); break;
            case '+': emitToken(&lexer, TOKEN_PLUS); break;
            case '-': emitToken(&lexer, TOKEN_MINUS); break;
            case '|': emitToken(&lexer, TOKEN_PIPE); break;
            case '&': emitToken(&lexer, TOKEN_AMP); break;
            case '=':
                if (peek(&lexer) == '=')
                {
                    advance(&lexer);
                    emitToken(&lexer, TOKEN_EQEQ);
                }
                else
                {
                    emitToken(&lexer, TOKEN_EQ);
                }
                break;

            case '<':
                if (peek(&lexer) == '=')
                {
                    advance(&lexer);
                    emitToken(&lexer, TOKEN_LTEQ);
                }
                else
                {
                    emitToken(&lexer, TOKEN_LT);
                }
                break;

            case '>':
                if (peek(&lexer) == '=')
                {
                    advance(&lexer);
                    emitToken(&lexer, TOKEN_GTEQ);
                }
                else
                {
                    emitToken(&lexer, TOKEN_GT);
                }
                break;

            case '!':
                if (peek(&lexer) == '=')
                {
                    advance(&lexer);
                    emitToken(&lexer, TOKEN_BANGEQ);
                }
                else
                {
                    emitToken(&lexer, TOKEN_BANG);
                }
                break;

            case '\n': emitToken(&lexer, TOKEN_LINE); break;

            case ' ': readWhitespace(&lexer); break;
            case '"': readString(&lexer); break;
            case '`': readEmbedded(&lexer); break;

            default:
                if (isName(c))
                {
                    readName(&lexer);
                }
                else if (isDigit(c))
                {
                    readNumber(&lexer);
                }
                else
                {
                    emitToken(&lexer, TOKEN_ERROR);
                }
                break;
        }
    }

    lexer.start = lexer.pos;
    emitToken(&lexer, TOKEN_EOF);

    return lexer.head;
}

static void readName(Lexer* lexer)
{
    // TODO(bob): Handle digits and EOF.
    while (isName(peek(lexer)) || isDigit(peek(lexer))) advance(lexer);

    TokenType type = TOKEN_NAME;

    if (isKeyword(lexer, "else")) type = TOKEN_ELSE;
    else if (isKeyword(lexer, "if")) type = TOKEN_IF;
    else if (isKeyword(lexer, "var")) type = TOKEN_VAR;

    emitToken(lexer, type);
}

static int isKeyword(Lexer* lexer, const char* keyword)
{
    size_t length = lexer->pos - lexer->start;
    size_t keywordLength = strlen(keyword);
    return length == keywordLength &&
    strncmp(lexer->source->bytes + lexer->start, keyword, length) == 0;
}

static void readNumber(Lexer* lexer)
{
    // TODO(bob): Floating point, hex, scientific, etc.
    while (isDigit(peek(lexer))) advance(lexer);

    emitToken(lexer, TOKEN_NUMBER);
}

static void readString(Lexer* lexer)
{
    // TODO(bob): Escape sequences, EOL, EOF, etc.
    while (advance(lexer) != '"');

    emitToken(lexer, TOKEN_STRING);
}

static void readEmbedded(Lexer* lexer)
{
    // TODO(bob): EOF.
    while (advance(lexer) != '`');

    emitToken(lexer, TOKEN_EMBEDDED);
}

static void readWhitespace(Lexer* lexer)
{
    while (peek(lexer) == ' ') advance(lexer);
    
    emitToken(lexer, TOKEN_WHITESPACE);
}

static int isName(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int isDigit(char c)
{
    return c >= '0' && c <= '9';
}

static char advance(Lexer* lexer)
{
    char c = peek(lexer);
    lexer->pos++;
    return c;
}

static char peek(Lexer* lexer)
{
    return lexer->source->bytes[lexer->pos];
}

static void emitToken(Lexer* lexer, TokenType type)
{
    Token* token = newToken(type, lexer->start, lexer->pos);
    
    token->prev = lexer->tail;
    token->next = NULL;
    
    if (lexer->tail == NULL)
    {
        // First token.
        lexer->head = token;
    }
    else
    {
        // Not the first token, so add it to the end.
        lexer->tail->next = token;
    }
    
    lexer->tail = token;
}
