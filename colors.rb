#
#                Copyright Noel Cower 2014.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#


module Colors
  class <<self
    attr_accessor :colored
  end

  self.colored = true

  GREY     = '[30;m'
  RED      = '[31;m'
  GREEN    = '[32;m'
  YELLOW   = '[33;m'
  BLUE     = '[34;m'
  PURPLE   = '[35;m'
  CYAN     = '[36;m'
  WHITE    = '[37;m'
  NO_COLOR = '[0;m'

  def white(*bits)  ; "#{self.colored  ? WHITE   : '' }#{ bits.join }#{ self.colored ? NO_COLOR : '' }" ; end
  def grey(*bits)   ; "#{self.colored  ? GREY    : '' }#{ bits.join }#{ self.colored ? NO_COLOR : '' }" ; end
  def red(*bits)    ; "#{self.colored  ? RED     : '' }#{ bits.join }#{ self.colored ? NO_COLOR : '' }" ; end
  def green(*bits)  ; "#{self.colored  ? GREEN   : '' }#{ bits.join }#{ self.colored ? NO_COLOR : '' }" ; end
  def yellow(*bits) ; "#{self.colored  ? YELLOW  : '' }#{ bits.join }#{ self.colored ? NO_COLOR : '' }" ; end
  def blue(*bits)   ; "#{self.colored  ? BLUE    : '' }#{ bits.join }#{ self.colored ? NO_COLOR : '' }" ; end
  def purple(*bits) ; "#{self.colored  ? PURPLE  : '' }#{ bits.join }#{ self.colored ? NO_COLOR : '' }" ; end
  def cyan(*bits)   ; "#{self.colored  ? CYAN    : '' }#{ bits.join }#{ self.colored ? NO_COLOR : '' }" ; end

  extend self
end
