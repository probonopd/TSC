# main.rb -- TSC MRuby manifest & toplevel file
#
# This file gets loaded by TSC everytime a new MRuby interpreter
# needs to be initialized. Anything placed here is automatically
# globally available in *all* levels (except local variables,
# of course). If you want, you can add your own code here,
# but please don't remove anything you don't know what it does.

# Alias Armadillo to Army so that both names work.
Army = Armadillo

# A Point represents a X/Y position in a level.
Point = Struct.new(:x, :y)

# A Rect instance represents a rectangle.
Rect = Struct.new(:x, :y, :width, :height)
