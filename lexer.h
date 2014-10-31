/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include <functional>
#include <memory>
#include <vector>



namespace vm
{

enum class token_kind
{
  error,                 // No valid associated data

  // Compounds (string-backed tokens)
  id,                    // See is_id_char (lexer.cpp) for what is and is not
                         // considered an identifier character.
  string,                // :=  '"' (string_char | string_escape)* '"'
  numeric,               // :=  ([1-9]\d* | 0) ('.' \d+)? ([eE] [+-]? [1-9] \d*)? # Decimal
                         //     | '0' [xX] [0-9a-fA-F]+                           # Hex
  symbol,                // :=  '\'' (string_char | string_escape)* '\''

  // Keywords
  let_kw,                // :=  'let'
  var_kw,                // :=  'var'
  def_kw,                // :=  'def'
  type_kw,               // :=  'type'
  match_kw,              // :=  'match'
  case_kw,               // :=  'case'
  else_kw,               // :=  'else'
  if_kw,                 // :=  'if'
  for_kw,                // :=  'for'
  with_kw,               // :=  'with'
  break_kw,              // :=  'break'
  continue_kw,           // :=  'continue'
  defer_kw,              // :=  'defer'
  yield_kw,              // :=  'yield'
  true_kw,               // :=  'true'
  false_kw,              // :=  'false'
  nil_kw,                // :=  'nil'

  newline,               // :=  '\n'

  // Braces
  curl_open,             // :=  '{'
  curl_close,            // :=  '}'

  paren_open,            // :=  '('
  paren_close,           // :=  ')'

  square_open,           // :=  '['
  square_close,          // :=  ']'

  // Operators
  plus,                  // :=  '+'
  double_plus,           // :=  '++'
  plus_equals,           // :=  '+='
  minus,                 // :=  '-'
  double_minus,          // :=  '--'
  minus_equals,          // :=  '-='

  forward_slash,         // :=  '/'
  forward_slash_equals,  // :=  '/='

  ampersand,             // :=  '&'
  double_ampersand,      // :=  '&&'
  ampersand_equals,      // :=  '&='

  pipe,                  // :=  '|'
  double_pipe,           // :=  '||'
  pipe_equals,           // :=  '|='

  caret,                 // :=  '^'
  caret_equals,          // :=  '^='
  asterisk,              // :=  '*'
  asterisk_equals,       // :=  '*='
  percent,               // :=  '%'
  percent_equals,        // :=  '%='
  bang,                  // :=  '!'
  bang_equals,           // :=  '!='
  hash,                  // :=  '#'
  at,                    // :=  '@'
  dollar,                // :=  '$'

  // Arrows
  tilde_arrow,           // :=  '~>'
  single_arrow,          // :=  '->'
  double_arrow,          // :=  '=>'

  // Punctuation
  question,              // :=  '?'
  dot,                   // :=  '.'
  double_dot,            // :=  '..'
  triple_dot,            // :=  '...'
  comma,                 // :=  ','
  colon,                 // :=  ':'
  double_colon,          // :=  '::'
  colon_equals,          // :=  ':='
  semicolon,             // :=  ';'
  tilde,                 // :=  '~'
  backtick,              // :=  '`'
  back_slash,            // :=  '\'
  underscore,            // :=  '_'

  // Comparisons
  less,                  // :=  '<'
  shift_left,            // :=  '<<'
  less_equal,            // :=  '<='

  greater,               // :=  '>'
  shift_right,           // :=  '>>'
  greater_equal,         // :=  '>='

  equals,                // :=  '='
  double_equals,         // :=  '=='
};


/**
 * Light-weight token. Does not keep a copy of its string data. If it's
 * necessary to keep a copy of a token with its string data intact, use
 * stored_token.
 */
struct token
{
  /** The token kind. See the above enum. */
  token_kind kind;

  /** Line number the token started on. */
  size_t line;
  /** Column number the token started on. */
  size_t column;

  // The length in chars of the token. Only relevant to compound tokens but
  // set for all token kinds.
  /**
   * The length, in bytes, of the token. Only relevant to compound tokens like
   * strings, identifiers, and so on, but is set for all token kinds.
   */
  size_t length;

  /**
   * Pointer to the first character of a contiguous block of characters.
   *
   * Data pointers are set for compounds. These pointers are only valid for
   * the duration of a call to a digest function and are not guaranteed to be
   * valid nor point to valid memory locations afterward. Their value for
   * non-compound tokens is not defined.
   */
  char const *data_begin;
  /**
   * Pointer to one character after the end of the data. See data_begin for
   * more info.
   */
  char const *data_end;
};


/**
 * A heavy token -- copies data referenced by an existing token and captures it
 * for the lifetime of the stored_token. This performs heap allocation to create
 * storage for the token data.
 */
struct stored_token : public token
{
private:
  using super = token;

  std::unique_ptr<char> _data = nullptr;

  void init_data(token const &tok);

public:
  stored_token(token const &token);
  stored_token(stored_token const &token);
  stored_token(stored_token &&token);
  ~stored_token() = default;

  stored_token &operator = (const token &token);
  stored_token &operator = (stored_token &&token);

  char const *data() const { return _data.get(); }
};


/**
 * Basic token lexer for Rusalka-associated text data.
 *
 * See token_kind for the kinds of tokens read by lexer.
 */
class lexer
{
public:
  /**
   * feed_fn functions supply individual characters to the lexer. The lexer
   * assumes all characters provided by feed_fn are sequential and will tokenize
   * them as such. Returning a null character kills the lexer.
   */
  using feed_fn   = std::function<char(void)>;
  /**
   * digest_fn functions receive tokens found in the stream of characters
   * provided by the feed function. They are always received in order (left to
   * right, top to bottom for LTR text). If a digest_fn returns false, the
   * token is considered unhandled and the lexer terminates.
   */
  using digest_fn = std::function<bool(token const &)>;

  /**
   * Initializes an empty lexer with the given feed and digest functions.
   *
   * Given a feed function, the lexer reads from `feed` until it encounters
   * either a null character (may be an error) or an error occurs. For each
   * token read, it is passed to digest.
   *
   * If feed() has no more characters to yield, it should return the null
   * character ('\0'). If digest fails at any point, it should return false to
   * silently end tokenization or throw an exception.
   */
  lexer(feed_fn &&feed, digest_fn &&digest);
  lexer(feed_fn const &feed, digest_fn const &digest);

  /**
   * Destroys the lexer.
   */
  ~lexer() = default;

  /**
   * Construct a lexer by taking ownership of another lexer's resources.
   *
   * This leaves the other lexer in an almost-pristine state, though it should
   * be treated as dead and/or invalid.
   */
  lexer(lexer &&);
  /**
   * Construct a lexer by copying another lexer's resources. This does not
   * invalidate the other lexer.
   */
  lexer(lexer const &);

  /**
   * Copy another lexer's resources into this lexer. Does not invalidate the
   * other lexer.
   */
  lexer &operator = (const lexer &);
  /**
   * Move another lexer's resources into this lexer. See the move constructor
   * for more details.
   */
  lexer &operator = (lexer &&);

  /**
   * Runs the lexer.
   * @return True if the entire stream of characters could be successfully
   * tokenized and digested, otherwise false.
   */
  bool tokenize();

private:
  enum emit_result : int
  {
    // No match found for a given sequence of characters and token type.
    // Attempt to match next token type.
    no_match = 0,
    // Match found.
    good = 1,
    // Error: digest_fn returned false.
    digest_failure = 2,
  };

  // Internal character buffer for token data. In the future, may also be used
  // to walk back lexing for ambiguous token types.
  std::vector<char> _buffer {};

  feed_fn   _feed;
  digest_fn _digest;

  // Starting line for current token.
  size_t _line;
  // Starting column for current token.
  size_t _column;

  // Next line. Copied to _line upon token digest.
  size_t _next_line;
  // Next column. Copied to _column upon token digest.
  size_t _next_column;

  // Whether _next_char is set.
  mutable bool _next_char_cached = false;
  // The character currently being read.
  char _current_char = '\0';
  // The next character in the stream. Only set if peek() is called, in which
  // case the next character is stored here and copied to _current_char on
  // next().
  mutable char _next_char = '\0';

  /**
   * Emits a token to the digest function and returns a resulting status code.
   */
  emit_result emit(token_kind const kind);

  /**
   * Returns the character currently being processed. This is the character
   * returned by the last call to next().
   */
  char current() const;

  /**
   * Advances the stream by consuming the last-peeked character or calling
   * _feed.
   *
   * @param buffer_next If true, stores the next-read character in the token
   *                    data buffer.
   */
  char next(bool buffer_next = true);

  /**
   * Reads the next character from _feed() and caches it, returning it for
   * all subsequent calls to peek() until the next call to next().
   */
  char peek() const;

  /**
   * Returns a token for the kind given with the rest of its members variables
   * initialized as needed.
   */
  token token_for_kind(token_kind const kind) const;

  /**
   * Stores a character in the current buffer.
   */
  void buffer_char(char const input);

  /**
   * Resets the current token data buffer to empty.
   */
  void reset_buffer();

  /**
   * Advances past whitespace (carriage returns, spaces, and tabs). Newlines
   * are treated as tokens, not whitespace.
   */
  void whitespace();

  /**
   * Reads a quoted string-like token of the given kind and with a delimiter.
   * Ideally, the delimiter is some closing quotation character, but may be
   * any single character.
   */
  emit_result quoted_string(token_kind const kind, char const delimiter);


  // Token function chain. If the first character matches, it attempts to read
  // the token type the function is named for. If there's no match, it calls
  // into the next token reading function. The first token that's attempted is
  // a string because matching one requires only a check for a double quote.
  /**
   * Attempts to read a string literal token (double-quoted string). If there's
   * no match, proceeds to try to read a symbol literal token.
   */
  emit_result string(); // no match -> symbol
  /**
   * Attempts to read a symbol literal token (single-quoted string). If there's
   * no match, proceeds to try to read a numeric token.
   */
  emit_result symbol(); // no match -> numeric
  /**
   * Attempts to read a numeric (integer of base-10, or -16 (0x prefix) or a
   * base-10 decimal number). If there's no match, proceeds to try to read an
   * identifier or other word-based token
   */
  emit_result numeric(); // no match -> identifier
  /**
   * Attempts to read an identifier or word-based token. Emits either the id
   * token type or another _kw token type for a match. If there's no match,
   * proceeds to try to read a punctuation token.
   */
  emit_result identifier(); // no match -> punctuation
  /**
   * Attempts to read a punctuation token (operators, symbols, and other tokens
   * that are not numbers, literals, or words). If there's no match, it returns
   * failure and in turn halts the lexer.
   */
  emit_result punctuation(); // no match -> failure

};


/**
 * Get the string name of a token kind. The returned string is null-terminated
 * and may not be modified or freed.
 */
const char *token_name(vm::token_kind const kind);


} // namespace vm
