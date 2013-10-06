=begin
Copyright (c) 2013 Noel Raymond Cower.

This file is part of Rusalka VM.

Rusalka VM is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rusalka VM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rusalka VM.  If not, see <http://www.gnu.org/licenses/>.
=end


class Lexer

#
# A position into a lexer's source string.
#
class Position

  # The line in the source string. Starts at 1.
  attr_accessor :line
  # The column in the source string. Starts at 1.
  attr_accessor :column

  class << self ; alias_method :[], :new ; end

  def initialize(line = 0, column = 0)
    @line   = line
    @column = column
  end

  def to_s
    "[#{@line}:#{@column}]"
  end

end

#
# A token produced by Lexer.
#
class Token

  # Literal keyword token symbols to their descriptors.
  LITERAL_KEYWORDS = {
    :true_kw           => "true",
    :false_kw          => "false",
    :null_kw           => "null"
  }.freeze

  # Keyword token symbols to their descriptors.
  KEYWORDS = {}.merge!(LITERAL_KEYWORDS).freeze

  # Literal token symbols to their descriptors.
  LITERALS = {
    :integer_lit       => "integer",
    :float_lit         => "float",
    :integer_exp_lit   => "integer exp",
    :float_exp_lit     => "float exp",
    :hex_lit           => "hexnum lit",
    :bin_lit           => "binary lit",
    :single_string_lit => "'...' string",
    :double_string_lit => "\"...\" string"
  }.merge(LITERAL_KEYWORDS).freeze

  # Punctuation token symbols to their descriptors.
  PUNCTUATION = {
    :dot               => ".",
    :double_dot        => "..",
    :triple_dot        => "...",
    :bang              => "!",
    :not_equal         => "!=",
    :question          => "?",
    :hash              => "#",
    :at                => "@",
    :dollar            => "$",
    :percent           => "%",
    :paren_open        => "(",
    :paren_close       => ")",
    :bracket_open      => "[",
    :bracket_close     => "]",
    :curl_open         => "{",
    :curl_close        => "}",
    :caret             => "^",
    :tilde             => "~",
    :grave             => "`",
    :backslash         => "\\",
    :slash             => "/",
    :comma             => ",",
    :semicolon         => ";",
    :greater_than      => ">",
    :shift_right       => ">>",
    :greater_equal     => ">=",
    :less_than         => "<",
    :shift_left        => "<<",
    :lesser_equal      => "<=",
    :equals            => "=",
    :equality          => "==",
    :pipe              => "|",
    :or                => "||",
    :ampersand         => "&",
    :and               => "&&",
    :colon             => ":",
    :double_colon      => "::",
    :minus             => "-",
    :double_minus      => "--",
    :arrow             => "->",
    :plus              => "+",
    :double_plus       => "++",
    :asterisk          => "*",
    :double_asterisk   => "**"
  }.freeze

  #
  # All token symbols to their descriptors.
  #
  DESCRIPTORS = {
    :invalid           => "invalid",
    :newline           => "\\n",
    :id                => "identifier",
    :line_comment      => "// comment",
    :block_comment     => "/* comment */"
  }.merge!(PUNCTUATION).merge!(LITERALS).freeze

  # The token's kind, a symbol.
  attr_accessor :kind
  # The token's posiiton in its source string in lines/columns.
  attr_accessor :position
  # The start of the token in its source string.
  attr_accessor :from
  # The end of the token in its source string.
  attr_accessor :to
  # The token's string value.
  attr_accessor :value

  def initialize(kind = nil, position = nil, from = nil, to = nil, value = nil)
    @kind     = kind  || :invalid
    @position = if position
      position.dup
    else
      Position[-1, -1]
    end
    @from     = from  || -1
    @to       = to    || -1
    @value    = String.new(value || '').freeze
  end

  #
  # Returns a string describing the token.
  #
  def descriptor
    DESCRIPTORS[@kind].dup
  end

  #
  # Returns whether this token is an identifier.
  #
  def id?
    @kind == :id
  end

  #
  # Returns whether this token is a true or false boolean literal.
  #
  def boolean?
    @kind == :true_kw || @kind == :false_kw
  end

  #
  # Returns whether this token is a null keyword.
  #
  def null?
    @kind == :null_kw
  end

  #
  # Returns whether this token is a literal.
  #
  def literal?
    LITERALS.include? @kind
  end

  #
  # Returns whether the token is an integer literal. hex_lit and bin_lit tokens
  # are considered integer literals by this method.
  #
  def integer?
    @kind == :integer_lit || @kind == :integer_exp_lit || @kind == :hex_lit || @kind == :bin_lit
  end

  #
  # Returns whether the token is a float literal. hex_lit and bin_lit tokens are
  # not considered float literals by this method.
  #
  def float?
    @kind == :float_lit || @kind == :float_exp_lit
  end

  #
  # Returns whether the token is a string literal.
  #
  def string?
    @kind == :double_string_lit || @kind == :single_string_lit
  end

  #
  # Returns whether the token is a comment.
  #
  def comment?
    @kind == :line_comment || @kind == :block_comment
  end

  #
  # Returns whether the token is punctuation.
  #
  def punctuation?
    PUNCTUATION.include? @kind
  end

  #
  # Returns whether the token is a newline.
  #
  def newline?
    @kind == :newline
  end

  #
  # Attempts to conver the token to an integer.
  #
  def to_i
    case @kind
    when :integer_lit, :integer_exp_lit, :single_string_lit, :double_string_lit
      @value.to_i
    when :float_lit, :float_exp_lit
      @value.to_f.to_i
    when :hex_lit
      @value.to_i(16)
    when :bin_lit
      @value.to_i(2)
    else
      raise TypeError, "Cannot convert this token to an integer"
    end
  end

  #
  # Attempts to convert the token to a Float.
  #
  def to_f
    case @kind
    when :float_lit, :float_exp_lit,
         :integer_lit, :integer_exp_lit,
         :single_string_lit, :double_string_lit
      @value.to_f
    else
      self.to_i.to_f
    end
  end

  #
  # Returns the token's value.
  #
  def to_s
    @value
  end

  #
  # Returns a Hash of the token's attributes.
  #
  def to_hash
    {
      :kind   => @kind,
      :value  => @value,
      :from   => @from,
      :to     => :to,
      :line   => @position.line,
      :column => @position.column
    }
  end

  alias_method :to_h, :to_hash

end # class Token

end # class Lexer

#
# A class for reading tokens produced by Lexer. Essentially a utility for
# very simply parsing of tokens.
#
class TokenReader

  #
  # Whether to skip whitespace on read by default. This can be overridden in
  # read_token, though no other reading functions allow this. Defaults to
  # true.
  #
  attr_accessor :skip_whitespace_on_read

  #
  # Initializes a new TokenReader with a container of tokens or an enumerator.
  # tokens must respond to #each and return an enumerator if it is not already
  # an enumerator.
  #
  def initialize(tokens)
    @token_enum = tokens.kind_of?(Enumerator) ? tokens : tokens.each
    @current    = nil
    @skip_whitespace_on_read = true
  end

  #
  # Reads a token, optionally only reading the token if conditions are met. If
  # the read succeeds, the token is returned, otherwise either nil is returned
  # or an exception is thrown, depending on the +fail+ argument.
  #
  # If there are no more tokens to be read, a RuntimeError is thrown.
  #
  # === Arguments
  #
  # +kind+ and +kinds+::
  #   Single and plural token kinds to be matched -- if the next token in the
  #   reader does not match either kind or one of kinds (or both, if both are
  #   provided -- recommended you only provide either kind or kinds), the read
  #   fails.
  #
  # +hash+::
  #   The hash of a token's value to match. If the token's value.hash doesn't
  #   match, the read fails.
  #
  # +value+::
  #   If provided, compares the token's value and the provided value. If the
  #   two are not equal, the read fails.
  #
  # +skip_whitespace+::
  #   Overrides the value of skip_whitespace_on_read.
  #
  # +fail+::
  #   If non-nil and non-false, this is the argument or arguments to provide
  #   to Kernel#raise. By default, it simply raises a RuntimeError with a
  #   message saying the read failed.
  #
  def read_token(kind = nil,
                 kinds: nil,
                 hash: nil,
                 value: nil,
                 skip_whitespace: nil,
                 fail: [RuntimeError, "Failed to read token."],
                 &block)

    skip_whitespace = self.skip_whitespace_on_read if skip_whitespace.nil?
    self.skip_whitespace_tokens() if skip_whitespace

    @current = begin
      peeked = @token_enum.peek
      tok = case
            when kind && peeked.kind != kind then            nil
            when kinds && ! kinds.include?(peeked.kind) then nil
            when hash && peeked.value.hash != hash then      nil
            when value && peeked.value != value then         nil
            else @token_enum.next
            end
      if fail && tok.nil?
        case fail
        when Array then raise(*fail)
        else raise "Read error <#{fail}> on token #{peeked.inspect}"
        end
      end
      tok
    rescue StopIteration
      raise "Attempt to read past end of tokens"
    end

    if ! @current.nil? && block_given?
      yield @current
    else
      @current
    end
  end

  #
  # Reads a float literal token and returns the float value of the token read.
  #
  def read_float(hash: nil, fail: ["Expected float literal"])
    tok = read_token(kinds: %i[float_lit float_exp_lit integer_lit integer_exp_lit hex_lit bin_lit], hash: hash, fail: fail)
    tok && tok.to_f
  end

  #
  # Reads an integer literal token and returns the integer value of the token
  # read.
  #
  def read_integer(hash: nil, fail: ["Expected integer literal"])
    tok = read_token(kinds: %i[integer_lit integer_exp_lit hex_lit bin_lit], hash: hash, fail: fail)
    tok && tok.to_i
  end

  #
  # Reads a boolean literal token and returns the boolean value of the token
  # read.
  #
  def read_boolean(hash: nil, fail: ["Expected boolean literal"])
    tok = read_token(kinds: %i[true_kw false_kw], hash: hash, fail: fail)
    tok && tok.kind == :true_kw
  end

  #
  # Reads a string literal token and returns the string value of the token
  # read.
  #
  def read_string(hash: nil, fail: ["Expected string literal"])
    tok = read_token(kinds: %i[single_string_lit double_string_lit], hash: hash, fail: fail)
    tok && tok.value
  end

  #
  # call-seq:
  #   current => Token or nil
  #
  # Returns the current token -- this is the token last returned by
  # read_token.
  #
  def current
    @current
  end

  #
  # call-seq:
  #   peek => Token or nil
  #
  # Peeks the next token from the TokenReader and returns it. If there are no
  # more tokens, returns nil.
  #
  def peek
    begin
      @token_enum.peek
    rescue StopIteration
      nil
    end
  end

  #
  # call-seq:
  #   peek_kind => sym or nil
  #
  # Peeks the next token's kind from the TokenReader and returns it. If there
  # are no more tokens, returns nil.
  #
  def peek_kind
    begin
      @token_enum.peek.kind
    rescue StopIteration
      nil
    end
  end

  #
  # call-seq:
  #   next_is(*kinds, value: nil) => true or false or nil
  #
  # Returns whether the next token in the TokenReader is one of the given
  # kinds and optionally whether it has a specific value. Returns true or
  # false depending on whether the next token matches the criteria. Otherwise,
  # if there are no more tokens, it returns nil.
  #
  def next_is?(*kinds, value: nil)
    tok = peek()
    tok && kinds.include?(tok.kind) && (value.nil? || value == tok.value)
  end

  #
  # call-seq:
  #   skip_token => self
  #
  # Skips a token. Same as read_token with no options, except it returns self.
  #
  # If attempting to skip a token when there are no more tokens, a
  # RuntimeError will be thrown.
  #
  def skip_token
    @current = begin
      @token_enum.next
    rescue StopIteration
      raise "Attempt to skip token with no more tokens"
    end
    self
  end

  #
  # Skips a certain number of tokens or specific kinds of tokens, or a number
  # of a specific kind of tokens. In all cases, the method will end early if
  # there are no more tokens to read.
  #
  # === Arguments
  #
  # +count+::
  #   Skips +count+ number of tokens.
  #
  # +kinds+::
  #   Skips tokens until it encounters a token whose kind is not included in
  #   +kinds+.
  #
  # +count+ and +kinds+::
  #   Skips +count+ number of tokens until it encounters a token whose kind is
  #   not included in +kinds+.
  #
  def skip_tokens(count: nil, kinds: nil)
    if ! (count || kinds)
      raise ArgumentError, "Must provide at least count or kinds to skip_tokens"
    elsif kinds
      if count
        skip_token() until eof? || ! kinds.include?(peek_kind()) || (count -= 1) <= 0
      else
        skip_token() until eof? || ! kinds.include?(peek_kind())
      end
    elsif count
      skip_token() until eof? || (count -= 1) < 0
    end
    self
  end

  #
  # call-seq:
  #   skip_to_token(*kinds, through: false) => self
  #
  # Skips as many tokens as necessary until it finds a token of a kind
  # provided. If +through+ is true, it will skip the matching token as well.
  #
  def skip_to_token(*kinds, through: false)
    raise ArgumentError, "No token kinds provided" if kinds.empty?
    skip_token() until (tok = peek()).nil? || kinds.include?(tok.kind)
    skip_token() if through && ! eof?
    self
  end

  #
  # call-seq:
  #   skip_whitespace_tokens => self
  #
  # Skips newline tokens as well as comments. Comments aren't technically
  # whitespace, but they are basically empty space, so they're skipped as
  # well.
  #
  def skip_whitespace_tokens
    skip_tokens kinds: [:newline, :line_comment, :block_comment]
    self
  end

  #
  # call-seq:
  #   eof? => true or false
  #
  # Returns whether the TokenReader has run out of tokens.
  #
  def eof?
    peek() == nil
  end
end

#
# A simple string lexer.
#
class Lexer

  #
  # Punctuation string hash. Based on Token::PUNCTUATION.
  #
  PUNCTUATION_STRINGS = Token::PUNCTUATION.invert.freeze

  #
  # Marker class -- stores the token index, source index, last character read,
  # and source position (see: Lexer::Position).
  #
  @@Marker = Struct.new(:token_index, :source_index, :character, :position)

  #
  # The array of tokens produced by the lexer.
  #
  attr_reader :tokens

  #
  # The last error encountered by the lexer -- a Hash with keys +:code+,
  # +:description+, and +:position+.
  #
  attr_reader :error

  #
  # Returns whether char is a valid hex-digit.
  #
  def self.isxdigit(char)
    ('0' <= char && char <= '9') ||
    ('a' <= char && char <= 'f') ||
    ('A' <= char && char <= 'F')
  end

  def initialize
    @reader        = nil
    @skip_comments = false
    @skip_newlines = false
    @at_end        = false
    reset
  end

  #
  # Resets the Lexer's state and clears any accumulated tokens.
  #
  def reset
    @error  = nil
    @marker = @@Marker[
      0,    # token_index
      -1,   # source_index
      "\0",  # character
      Position[1, 0] # position
    ]
    @tokens = []
    self
  end

  #
  # Lexes the +source+ string until a token with a kind given by +until_token+
  # is encountered. If +token_count+ is provided, only that many tokens will be
  # read from the source.
  #
  def run(source, until_token = :invalid, token_count = nil)
    @at_end = false
    @source = source
    @reader = source.each_char

    read_next

    while token_count == nil || token_count > 0
      skip_whitespace
      current = @marker.character
      break unless current

      token          = Token.new
      token.kind     = :invalid
      token.from     = @marker.source_index
      token.position = @marker.position.dup

      case current
      when '"', '\''
        read_string(token)

      when '0'
        case peek_next
        when 'x', 'X', 'b', 'B' then read_base_number(token)
        else                     read_number(token)
        end

      when '1', '2', '3', '4', '5', '6', '7', '8', '9'
        read_number(token)

      # dot, double dot, triple dot, and floats beginning with a dot
      when '.'
        token.kind = :dot
        case peek_next
        when '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' then read_number(token)
        when '.'
          read_next
          token.kind = :double_dot

          if peek_next == '.'
            read_next
            token.kind = :triple_dot
          end

          token.value = Token::DESCRIPTORS[token.kind]
        else
          token.value = Token::DESCRIPTORS[token.kind]
        end

      when '_', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
        'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
        'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
        read_word(token)

      when "\n"
        token.value = current
        token.kind = :newline

      when '?', '#', '@', '$', '%', '(', ')', '[', ']', '{', '}', '^', '~', '`', ',', ';'
        token.value = current
        token.kind = PUNCTUATION_STRINGS[current]

      when '=', '|', '&', ':', '+', '*'
        current << read_next if peek_next == current
        token.value = current
        token.kind = PUNCTUATION_STRINGS[current]

      when '!'
        current << read_next if peek_next == '='
        token.value = current
        token.kind = PUNCTUATION_STRINGS[current]

      when '>', '<'
        case peek_next
        when '=', current then current << read_next
        end
        token.value = current
        token.kind = PUNCTUATION_STRINGS[current]

      when '-'
        case peek_next
        when '>', current then current << read_next
        # when '.', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
          # read_number(token)
          # token.value = '-' + token.value
        end
        # if token.kind == :invalid
          token.value = current
          token.kind = PUNCTUATION_STRINGS[current]
        # end

      when '/'
        case peek_next
        when '/' then read_line_comment(token)
        when '*' then read_block_comment(token)
        else
          token.value = Token::DESCRIPTORS[token.kind = :slash]
          read_next
        end

      end # case current

      token.to = @marker.source_index
      last_kind = token.kind
      unless (@skip_comments && token.comment?) || (@skip_newlines && token.newline?)
        if last_kind != :invalid
          @tokens << token
          yield token if block_given?
        else
          raise RuntimeError, "#{token.position} Invalid token: #{token.inspect}"
        end
      end

      break if until_token == last_kind

      read_next
      token_count -= 1 unless token_count.nil?
    end # while current && token_count > 0

    @source = nil
    @reader = nil

    self
  end

  #
  # Whether the lexer skips generating tokens for comments.
  #
  def skip_comments?
    @skip_comments
  end

  #
  # Sets whether the lexer ignores comments.
  #
  def skip_comments=(bool)
    @skip_comments = bool
  end

  #
  # Whether the lexer skips generating tokens for newlines.
  #
  def skip_newlines?
    @skip_newlines
  end

  #
  # Sets whether the lexer ignores newlines.
  #
  def skip_newlines=(bool)
    @skip_newlines = bool
  end


  private

  #
  # Returns the next character in the source string or nil if there is none.
  # Does not advance the lexer's position in the source string.
  #
  def peek_next
    return nil if @at_end

    begin
      @reader.peek
    rescue StopIteration
      nil
    end
  end

  #
  # Returns the next character in the source string and advances the lexer's
  # position in the string. Returns nil if at the end of the source string.
  #
  def read_next
    return nil if @at_end

    begin
      pos = @marker.position

      if @marker.character == "\n"
        pos.line += 1
        pos.column = 0
      end

      @marker.character = @reader.next
      @marker.source_index += 1
      pos.column += 1
    rescue StopIteration
      @at_end = true
      @marker.character = nil
    end

    @marker.character
  end

  #
  # Skips whitespace characters (spaces, tabs, and carriage returns). Does not
  # skip newlines.
  #
  def skip_whitespace
    current = @marker.character
    (current = read_next) while current == ' ' || current == "\t" || current == "\r"
  end

  DOT_TOKENS = [:dot, :double_dot, :triple_top]

  #
  # Reads a base-2 or base-16 number literal. Base literals are not tied to a
  # specific type.
  #
  def read_base_number(token)
    current = read_next

    case current
    when 'b', 'B'
      token.kind = :bin_lit
      read_next while (current = peek_next) == '1' || current == '0'
    when 'x', 'X'
      token.kind = :hex_lit
      read_next while self.class.isxdigit(current = peek_next)
    end

    token.value = @source[(token.from .. @marker.source_index)]
  end

  #
  # Reads a number literal, either integer or floating point, with an optional
  # exponent.
  #
  def read_number(token)
    current = @marker.character
    is_float = current == '.'
    is_exponent = false
    token.kind = is_float ? :float_lit : :integer_lit

    while (current = peek_next)
      case current
      # Float lit
      when '.'
        break if is_float == true
        is_float = true
        token.kind = :float_lit
        read_next

      # Digit
      when '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
        read_next

      # Exponent
      when 'e', 'E'
        if is_exponent
          token.kind = :invalid
          raise_error(:duplicate_exponent,
            "Malformed number literal: exponent already provided")
        end

        is_exponent = true
        token.kind = is_float ? :float_exp_lit : :integer_exp_lit

        read_next
        current = read_next
        current = read_next if current == '-' || current == '+'

        if current < '0' || current > '9'
          raise_error(:malformed_exponent, "Malformed number literal: exponent expected but not provided")
        end

      else break
      end
    end

    token.value = @source[(token.from .. @marker.source_index)]
  end

  #
  # Reads a word token. May be either a keyword or identifier.
  #
  def read_word(token)
    while (current = peek_next)
      break unless current == '_' ||
        ('0' <= current && current <= '9') ||
        ('a' <= current && current <= 'z') ||
        ('A' <= current && current <= 'Z')
      read_next
    end

    token.value = @source[token.from .. @marker.source_index]

    token.kind = case token.value
                 when 'true'  then :true_kw
                 when 'false' then :false_kw
                 when 'null'  then :null_kw
                 else              :id
                 end
  end

  def read_string_hex_escape(length)
    code = 0
    loop do
      code = code << 4 |
       (case peeked
        when 'A', 'a' then 0xA
        when 'B', 'b' then 0xB
        when 'C', 'c' then 0xC
        when 'D', 'd' then 0xD
        when 'E', 'e' then 0xE
        when 'F', 'f' then 0xF
        when '0' then 0x0
        when '1' then 0x1
        when '2' then 0x2
        when '3' then 0x3
        when '4' then 0x4
        when '5' then 0x5
        when '6' then 0x6
        when '7' then 0x7
        when '8' then 0x8
        when '9' then 0x9
        end)

      read_next
      peeked = peek_next
      length -= 1

      break unless self.class.isxdigit(peeked) && length > 0
    end
    code.chr(Encoding::UTF_8)
  end

  def read_string_escape(char)
    case char
    when 'x', 'X'
      # unicode hex escape
      peeked = peek_next
      unless self.class.isxdigit(peeked)
        raise_error(:malformed_unicode_escape,
          "Malformed unicode literal in string - no hex code provided.")
      end

      hexnum_length = char == 'x' ? 4 : 8
      read_string_hex_escape(hexnum_length)

    when 'r' then "\r"
    when 'n' then "\n"
    when 't' then "\t"
    when '0' then "\0"
    when 'b' then "\b"
    when 'a' then "\a"
    when 'f' then "\f"
    when 'v' then "\v"
    else          char
    end
  end

  #
  # Reads a string literal. String literals may be delimited by any single
  # character, though the lexer by default only recognizes strings between
  # single and double quotes.
  #
  # C escape codes are recognized, as well as arbitrary byte sequences (which
  # get decoded as UTF8) specified by either +\x+ or +\X+. A lowercase +x+ is
  # limited to two bytes in hexadecimal (a short) while an uppercase +X+ is
  # limited to four bytes in hexadecimal.
  #
  # Any other escaped character produces the escaped letter.
  #
  def read_string(token)
    opening_char = @marker.character
    token.kind = case opening_char
                 when '\'' then :single_string_lit
                 when '"' then :double_string_lit
                 end

    escape = false
    chars = []

    while (current = read_next)
      if escape
        current = read_string_escape(current)
        escape = false
      else
        break if current == opening_char

        if current == "\\'"
          escape = true
          next
        end
      end

      chars << current
    end

    raise_error(:unterminated_string, "Unterminated string") unless current

    token.value = chars.join('')
  end

  #
  # Reads a single line comment. Terminates after the first newline character
  # encountered.
  #
  def read_line_comment(token)
    token.kind = :line_comment
    read_next while (current = peek_next) && current != "\n"

    @skip_comments || token.value = @source[token.from .. @marker.source_index]
  end

  #
  # Reads a C-style block comment. Terminates after +*/+ is encountered.
  #
  # Block comments may not be nested in any form.
  #
  def read_block_comment(token)
    token.kind = :block_comment

    read_next
    while (current = read_next)
      if current == '*' && peek_next == '/'
        current = read_next
        break
      end
    end

    unless current
      raise_error(:unterminated_block_comment, "Unterminated block comment")
    end

    unless @skip_comments
      token.value = @source[token.from .. @marker.source_index]
    end
  end

  #
  # Raises an error with the given code, error (message), and position in the
  # source string (line:column). Also assigns the error to the lexer's error
  # property.
  #
  def raise_error(code, error, position = nil)
    position ||= @marker.position
    @error = {
      :code => code,
      :description => error,
      :position => position.dup
    }
    raise "#{position} (#{code}) #{error}"
  end

end
