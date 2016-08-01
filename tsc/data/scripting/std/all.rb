# _std.rb -- standard extension manifest & loading file.
# This file reads in all files from the standard scripting
# extension for TSC.

# Namespace for non-core (read non-C++) stuff for TSC
# scripting.
module Std
end

TSC.require "std/enable"
TSC.require "std/exit_door"
TSC.require "std/immediate_sprite"
TSC.require "std/switch"
TSC.require "std/climbing_box"
TSC.require "std/giant_jewel_box"
