# This file is taken unmodified from mruby 1.2.0. It defines the #puts
# method family in terms of a method __printstr__(), which mruby by
# default maps to fprintf() on stdout; in TSC, it is mapped to a
# special function that outputs to the game console instead.
#
# Copyright (c) 2015 mruby developers
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

##
# Kernel
#
# ISO 15.3.1
module Kernel
  ##
  # Invoke method +print+ on STDOUT and passing +*args+
  #
  # ISO 15.3.1.2.10
  def print(*args)
    i = 0
    len = args.size
    while i < len
      __printstr__ args[i].to_s
      i += 1
    end
  end

  ##
  # Invoke method +puts+ on STDOUT and passing +*args*+
  #
  # ISO 15.3.1.2.11
  def puts(*args)
    i = 0
    len = args.size
    while i < len
      s = args[i].to_s
      __printstr__ s
      __printstr__ "\n" if (s[-1] != "\n")
      i += 1
    end
    __printstr__ "\n" if len == 0
    nil
  end

  ##
  # Print human readable object description
  #
  # ISO 15.3.1.3.34
  def p(*args)
    i = 0
    len = args.size
    while i < len
      __printstr__ args[i].inspect
      __printstr__ "\n"
      i += 1
    end
    args[0]
  end

  unless Kernel.respond_to?(:sprintf)
    def printf(*args)
      raise NotImplementedError.new('printf not available')
    end
    def sprintf(*args)
      raise NotImplementedError.new('sprintf not available')
    end
  else
    def printf(*args)
      __printstr__(sprintf(*args))
      nil
    end
  end
end
