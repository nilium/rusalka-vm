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

  def self.white(*bits)  ; "#{self.colored  ? WHITE   : '' }#{ bits.join }#{ self.colored ? NO_COLOR : '' }" ; end
  def self.grey(*bits)   ; "#{self.colored  ? GREY    : '' }#{ bits.join }#{ self.colored ? NO_COLOR : '' }" ; end
  def self.red(*bits)    ; "#{self.colored  ? RED     : '' }#{ bits.join }#{ self.colored ? NO_COLOR : '' }" ; end
  def self.green(*bits)  ; "#{self.colored  ? GREEN   : '' }#{ bits.join }#{ self.colored ? NO_COLOR : '' }" ; end
  def self.yellow(*bits) ; "#{self.colored  ? YELLOW  : '' }#{ bits.join }#{ self.colored ? NO_COLOR : '' }" ; end
  def self.blue(*bits)   ; "#{self.colored  ? BLUE    : '' }#{ bits.join }#{ self.colored ? NO_COLOR : '' }" ; end
  def self.purple(*bits) ; "#{self.colored  ? PURPLE  : '' }#{ bits.join }#{ self.colored ? NO_COLOR : '' }" ; end
  def self.cyan(*bits)   ; "#{self.colored  ? CYAN    : '' }#{ bits.join }#{ self.colored ? NO_COLOR : '' }" ; end
end
