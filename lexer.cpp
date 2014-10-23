/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#include "lexer.h"


namespace vm
{

static const size_t MIN_BUFFER_RESERVE = 256;


/**
 * Converts a hex character to its equivalent hex value. Returns 0xFF if `x` is
 * not a hex character. Not constexpr because of switch. Can be made constexpr
 * in C++14.
 */
static unsigned xtoi(char const x)
{
  switch (x) {
  case 'a': case 'A': return 0xAu;
  case 'b': case 'B': return 0xBu;
  case 'c': case 'C': return 0xCu;
  case 'd': case 'D': return 0xDu;
  case 'e': case 'E': return 0xEu;
  case 'f': case 'F': return 0xFu;
  case '0': return 0x0u;
  case '1': return 0x1u;
  case '2': return 0x2u;
  case '3': return 0x3u;
  case '4': return 0x4u;
  case '5': return 0x5u;
  case '6': return 0x6u;
  case '7': return 0x7u;
  case '8': return 0x8u;
  case '9': return 0x9u;
  default: return 0xFF;
  }
}



static constexpr bool is_id_char(char const c, bool first = false)
{
  return
    (c == '_') ||
    ('a' <= c && c <= 'z') ||
    ('A' <= c && c <= 'Z') ||
    (first == false && '0' <= c && c <= '9');
}



static constexpr bool is_symstr_escape(char const c)
{
  return c == '\\';
}



static constexpr bool is_whitespace(char const c)
{
  return c == ' ' || c == '\t' || c == '\r';
}



lexer::lexer(feed_fn &&feed, digest_fn &&digest)
: _feed(feed)
, _digest(digest)
{
  _buffer.reserve(MIN_BUFFER_RESERVE);
}



lexer::lexer(feed_fn const &feed, digest_fn const &digest)
: _feed(feed)
, _digest(digest)
{
  _buffer.reserve(MIN_BUFFER_RESERVE);
}



lexer::lexer(lexer &&other)
: _feed(std::move(other._feed))
, _digest(std::move(other._digest))
, _line(other._line)
, _column(other._column)
{
  other._next_char =
  other._current_char = '\0';
  other._next_char_cached = false;
  _buffer.reserve(MIN_BUFFER_RESERVE);
}



lexer::lexer(lexer const &other)
: _feed(other._feed)
, _digest(other._digest)
, _line(other._line)
, _column(other._column)
{
  _buffer.reserve(MIN_BUFFER_RESERVE);
}



lexer &lexer::operator = (lexer &&other)
{
  _feed = std::move(other._feed);
  _digest = std::move(other._digest);
  return *this;
}



lexer &lexer::operator = (lexer const &other)
{
  _feed = other._feed;
  _digest = other._digest;
  return *this;
}



auto lexer::emit(token_kind const kind) -> emit_result
{
  return _digest(token_for_kind(kind))
    ? good
    : digest_failure;
}



bool lexer::tokenize()
{
  _next_line = _line = 1;
  _next_column = _column = 1;

  _current_char = _next_char = '\0';
  _next_char_cached = false;

  for (; next() != '\0';) {
    whitespace();

    _line = _next_line;
    _column = _next_column;

    if (current() == '\0') {
      break;
    }

    emit_result const test = string();
    switch (test) {
    case no_match:
      emit(token_kind::error); // fallthrough
    case digest_failure:
      return false;
    case good:
      break;
    }

    reset_buffer();
  }

  return true;
}



auto lexer::quoted_string(token_kind const kind, char const delimiter) -> emit_result
{
  reset_buffer();

  bool escape = false;
  for (char ch = next(false); ch != '\0'; ch = next(false)) {
    if (ch == delimiter) {
      return emit(kind);
    } else if (is_symstr_escape(ch)) {
      escape = true;
      continue;
    } else if (escape) {
      escape = false;
      switch (ch) {
      case 'a': buffer_char('\a'); break;
      case 'b': buffer_char('\b'); break;
      case 'f': buffer_char('\f'); break;
      case 'n': buffer_char('\n'); break;
      case 'r': buffer_char('\r'); break;
      case 't': buffer_char('\t'); break;
      case 'v': buffer_char('\v'); break;
      case 'x': {
        unsigned const first = xtoi(next(false));
        unsigned const second = xtoi(peek());
        if (first == 0xFFu || second == 0xFFu) {
          buffer_char('x');
          if (current() != '\0') {
            buffer_char(current());
          }
          break;
        }
        next(false);
        buffer_char(static_cast<char>((first << 4) | second));
      } break;
      case 'u':
      case 'U':
        buffer_char('\\');
      default:
        buffer_char(ch);
      }
    } else {
      buffer_char(ch);
    }
  }

  return no_match;
}



auto lexer::string() -> emit_result
{
  if (current() != '"') {
    return symbol();
  }

  return quoted_string(token_kind::string, '"');
}



auto lexer::symbol() -> emit_result
{
  if (current() != '\'') {
    return numeric();
  }

  return quoted_string(token_kind::symbol, '\'');
}



auto lexer::numeric() -> emit_result
{
  size_t length = 0;
  auto test_fn = isdigit;

  switch (current()) {
  case '0':
    switch (peek()) {
      case 'x':
      case 'X':
        ++length;
        next();

        if (!isxdigit(peek())) {
          return emit(token_kind::error);
        }

        test_fn = isxdigit;
      default:
        break;
    }

  case '1': case '2': case '3': case '4': case '5':
  case '6': case '7': case '8': case '9':
    ++length;
    while (test_fn(peek())) {
      next();
      ++length;
    }
    break;

  default:
    break;
  }

  if (length > 0) {
    return emit(token_kind::numeric);
  } else {
    return identifier();
  }
}



auto lexer::identifier() -> emit_result
{
  if (is_id_char(current(), true)) {
    while (is_id_char(peek(), false)) {
      next();
    }
    size_t len = _buffer.size();
    switch (len) {
    case 2:
      if (strncmp(&_buffer[0], "if", len) == 0) {
        return emit(token_kind::if_kw);
      } break;
    case 3:
      if (strncmp(&_buffer[0], "let", len) == 0) {
        return emit(token_kind::let_kw);
      } else if (strncmp(&_buffer[0], "var", len) == 0) {
        return emit(token_kind::var_kw);
      } else if (strncmp(&_buffer[0], "def", len) == 0) {
        return emit(token_kind::def_kw);
      } else if (strncmp(&_buffer[0], "nil", len) == 0) {
        return emit(token_kind::nil_kw);
      } else if (strncmp(&_buffer[0], "for", len) == 0) {
        return emit(token_kind::for_kw);
      } break;
    case 4:
      if (strncmp(&_buffer[0], "type", len) == 0) {
        return emit(token_kind::type_kw);
      } else if (strncmp(&_buffer[0], "true", len) == 0) {
        return emit(token_kind::true_kw);
      } else if (strncmp(&_buffer[0], "else", len) == 0) {
        return emit(token_kind::else_kw);
      } else if (strncmp(&_buffer[0], "case", len) == 0) {
        return emit(token_kind::case_kw);
      } else if (strncmp(&_buffer[0], "with", len) == 0) {
        return emit(token_kind::with_kw);
      } break;
    case 5:
      if (strncmp(&_buffer[0], "break", len) == 0) {
        return emit(token_kind::break_kw);
      } else if (strncmp(&_buffer[0], "defer", len) == 0) {
        return emit(token_kind::defer_kw);
      } else if (strncmp(&_buffer[0], "yield", len) == 0) {
        return emit(token_kind::yield_kw);
      } else if (strncmp(&_buffer[0], "false", len) == 0) {
        return emit(token_kind::false_kw);
      } else if (strncmp(&_buffer[0], "match", len) == 0) {
        return emit(token_kind::match_kw);
      } break;
    case 8:
      if (strncmp(&_buffer[0], "continue", len) == 0) {
        return emit(token_kind::continue_kw);
      } break;
    default: break;
    }

    return emit(token_kind::id);
  }

  return punctuation();
}



auto lexer::punctuation() -> emit_result
{
  switch (current())
  {
  case '\n': return emit(token_kind::newline);
  case '{': return emit(token_kind::curl_open);
  case '}': return emit(token_kind::curl_close);
  case '(': return emit(token_kind::paren_open);
  case ')': return emit(token_kind::paren_close);
  case '[': return emit(token_kind::square_open);
  case ']': return emit(token_kind::square_close);
  case '+':
    switch (peek()) {
    case '+': next(); return emit(token_kind::double_plus);
    case '=': next(); return emit(token_kind::plus_equals);
    default: return emit(token_kind::plus);
    }
  case '-':
    switch (peek()) {
    case '>': next(); return emit(token_kind::single_arrow);
    case '-': next(); return emit(token_kind::double_minus);
    case '=': next(); return emit(token_kind::minus_equals);
    default: return emit(token_kind::minus);
    }
  case '/':
    switch (peek()) {
    case '=': next(); return emit(token_kind::forward_slash_equals);
    default: return emit(token_kind::forward_slash);
    }
  case '&':
    switch (peek()) {
    case '&': next(); return emit(token_kind::double_ampersand);
    case '=': next(); return emit(token_kind::ampersand_equals);
    default: return emit(token_kind::ampersand);
    }
  case '|':
    switch (peek()) {
    case '|': next(); return emit(token_kind::double_pipe);
    case '=': next(); return emit(token_kind::pipe_equals);
    default: return emit(token_kind::pipe);
    }
  case '^':
    switch (peek()) {
    case '=': next(); return emit(token_kind::caret_equals);
    default: return emit(token_kind::caret);
    }
  case '*':
    switch (peek()) {
    case '=': next(); return emit(token_kind::asterisk_equals);
    default: return emit(token_kind::asterisk);
    }
  case '!':
    switch (peek()) {
    case '=': next(); return emit(token_kind::bang_equals);
    default: return emit(token_kind::bang);
    }
  case '#': return emit(token_kind::hash);
  case '@': return emit(token_kind::at);
  case '$': return emit(token_kind::dollar);
  case '%':
    switch (peek()) {
    case '=': next(); return emit(token_kind::percent_equals);
    default: return emit(token_kind::percent);
    }
  case '?': return emit(token_kind::question);
  case '.':
    if (peek() == '.') {
      next();

      if (peek() == '.') {
        next();
        return emit(token_kind::triple_dot);
      } else {
        return emit(token_kind::double_dot);
      }
    } else {
      return emit(token_kind::dot);
    }
  case ',': return emit(token_kind::comma);
  case ':':
    switch (peek()) {
    case ':': next(); return emit(token_kind::double_colon);
    case '=': next(); return emit(token_kind::colon_equals);
    default: return emit(token_kind::colon);
    }
  case ';': return emit(token_kind::semicolon);
  case '~':
    switch (peek()) {
    case '>': next(); return emit(token_kind::tilde_arrow);
    default: return emit(token_kind::tilde);
    }
  case '`': return emit(token_kind::backtick);
  case '\\': return emit(token_kind::back_slash);
  case '<':
    switch (peek()) {
    case '=': next(); return emit(token_kind::less_equal);
    case '<': next(); return emit(token_kind::shift_left);
    default: return emit(token_kind::less);
    }
  case '>':
    switch (peek()) {
    case '=': next(); return emit(token_kind::greater_equal);
    case '>': next(); return emit(token_kind::shift_right);
    default: return emit(token_kind::greater);
    }
  case '=':
    switch (peek()) {
    case '>': next(); return emit(token_kind::double_arrow);
    case '=': next(); return emit(token_kind::double_equals);
    default: return emit(token_kind::equals);
    }
  default:
    return no_match;
  }
}



void lexer::whitespace()
{
  for (; is_whitespace(_buffer.back());) {
    _buffer.pop_back();
  }
  for (; is_whitespace(current()); next(!is_whitespace(peek()))) {
    ;
  }
}



char lexer::current() const
{
  return _current_char;
}



char lexer::next(bool buffer_next)
{
  char const current = _current_char;
  char result;

  if (current == '\n') {
    ++_next_line;
    _next_column = 1;
  } else if (current != '\0') {
    ++_next_column;
  }

  if (_next_char_cached) {
    _current_char = result = _next_char;
  } else {
    _current_char = _next_char = result = _feed();
  }
  _next_char_cached = result == '\0';

  if (result != '\0' && buffer_next) {
    buffer_char(result);
  }

  return result;
}



char lexer::peek()
{
  if (_next_char_cached) {
    return _next_char;
  } else {
    _next_char = _feed();
    _next_char_cached = true;
    return _next_char;
  }
}



token lexer::token_for_kind(token_kind const kind) const
{
  size_t const size = _buffer.size();
  char const *const begin = &_buffer[0];
  return token {
    kind,
    _line,
    _column,
    size,
    begin,
    begin + size
  };
}



void lexer::buffer_char(char const input)
{
  _buffer.push_back(input);
}



void lexer::reset_buffer()
{
  _buffer.clear();
}



// stored_token

void stored_token::init_data(token const &tok)
{
  super::operator=(tok);
  if (tok.data_begin) {
    _data.reset(new char[length]);
    memcpy(_data.get(), tok.data_begin, length);
    data_begin = data();
    data_end = data() + length;
  } else {
    data_end = nullptr;
  }
}



stored_token::stored_token(token const &tok)
{
  init_data(tok);
}



stored_token::stored_token(stored_token const &tok)
{
  init_data(tok);
}



stored_token::stored_token(stored_token &&tok)
: super(tok)
, _data{std::move(tok._data)}
{
  tok.data_begin = nullptr;
  tok.data_end = nullptr;
}



stored_token &stored_token::operator = (const token &tok)
{
  init_data(tok);
  return *this;
}



stored_token &stored_token::operator = (stored_token &&tok)
{
  super::operator=(tok);
  _data = std::move(tok._data);
  data_begin = tok.data_begin;
  data_end = tok.data_end;
  tok.data_begin = nullptr;
  tok.data_end = nullptr;
  return *this;
}



// token name

const char *token_name(vm::token_kind const kind)
{
  switch (kind) {
  case vm::token_kind::error:                 return "error";
  case vm::token_kind::id:                    return "id";
  case vm::token_kind::string:                return "string";
  case vm::token_kind::numeric:               return "numeric";
  case vm::token_kind::symbol:                return "symbol";
  case vm::token_kind::let_kw:                return "let_kw";
  case vm::token_kind::var_kw:                return "var_kw";
  case vm::token_kind::def_kw:                return "def_kw";
  case vm::token_kind::type_kw:               return "type_kw";
  case vm::token_kind::match_kw:              return "match_kw";
  case vm::token_kind::case_kw:               return "case_kw";
  case vm::token_kind::else_kw:               return "else_kw";
  case vm::token_kind::if_kw:                 return "if_kw";
  case vm::token_kind::for_kw:                return "for_kw";
  case vm::token_kind::with_kw:               return "with_kw";
  case vm::token_kind::break_kw:              return "break_kw";
  case vm::token_kind::continue_kw:           return "continue_kw";
  case vm::token_kind::defer_kw:              return "defer_kw";
  case vm::token_kind::yield_kw:              return "yield_kw";
  case vm::token_kind::true_kw:               return "true_kw";
  case vm::token_kind::false_kw:              return "false_kw";
  case vm::token_kind::nil_kw:                return "nil_kw";
  case vm::token_kind::newline:               return "newline";
  case vm::token_kind::curl_open:             return "curl_open";
  case vm::token_kind::curl_close:            return "curl_close";
  case vm::token_kind::paren_open:            return "paren_open";
  case vm::token_kind::paren_close:           return "paren_close";
  case vm::token_kind::square_open:           return "square_open";
  case vm::token_kind::square_close:          return "square_close";
  case vm::token_kind::plus:                  return "plus";
  case vm::token_kind::double_plus:           return "double_plus";
  case vm::token_kind::plus_equals:           return "plus_equals";
  case vm::token_kind::minus:                 return "minus";
  case vm::token_kind::double_minus:          return "double_minus";
  case vm::token_kind::minus_equals:          return "minus_equals";
  case vm::token_kind::forward_slash:         return "forward_slash";
  case vm::token_kind::forward_slash_equals:  return "forward_slash_equals";
  case vm::token_kind::ampersand:             return "ampersand";
  case vm::token_kind::double_ampersand:      return "double_ampersand";
  case vm::token_kind::ampersand_equals:      return "ampersand_equals";
  case vm::token_kind::pipe:                  return "pipe";
  case vm::token_kind::double_pipe:           return "double_pipe";
  case vm::token_kind::pipe_equals:           return "pipe_equals";
  case vm::token_kind::caret:                 return "caret";
  case vm::token_kind::caret_equals:          return "caret_equals";
  case vm::token_kind::asterisk:              return "asterisk";
  case vm::token_kind::asterisk_equals:       return "asterisk_equals";
  case vm::token_kind::percent:               return "percent";
  case vm::token_kind::percent_equals:        return "percent_equals";
  case vm::token_kind::bang:                  return "bang";
  case vm::token_kind::bang_equals:           return "bang_equals";
  case vm::token_kind::hash:                  return "hash";
  case vm::token_kind::at:                    return "at";
  case vm::token_kind::dollar:                return "dollar";
  case vm::token_kind::tilde_arrow:           return "tilde_arrow";
  case vm::token_kind::single_arrow:          return "single_arrow";
  case vm::token_kind::double_arrow:          return "double_arrow";
  case vm::token_kind::question:              return "question";
  case vm::token_kind::dot:                   return "dot";
  case vm::token_kind::double_dot:            return "double_dot";
  case vm::token_kind::triple_dot:            return "triple_dot";
  case vm::token_kind::comma:                 return "comma";
  case vm::token_kind::colon:                 return "colon";
  case vm::token_kind::double_colon:          return "double_colon";
  case vm::token_kind::colon_equals:          return "colon_equals";
  case vm::token_kind::semicolon:             return "semicolon";
  case vm::token_kind::tilde:                 return "tilde";
  case vm::token_kind::backtick:              return "backtick";
  case vm::token_kind::back_slash:            return "back_slash";
  case vm::token_kind::less:                  return "less";
  case vm::token_kind::shift_left:            return "shift_left";
  case vm::token_kind::less_equal:            return "less_equal";
  case vm::token_kind::greater:               return "greater";
  case vm::token_kind::shift_right:           return "shift_right";
  case vm::token_kind::greater_equal:         return "greater_equal";
  case vm::token_kind::equals:                return "equals";
  case vm::token_kind::double_equals:         return "double_equals";
  default: return "UNKNOWN TOKEN KIND"; // should be impossible.
  }
}


} // namespace vm
