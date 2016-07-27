#!/usr/bin/env ruby
# -*- coding: utf-8 -*-
#############################################################################
# generate_credits.rb  -  credits.cpp generator
#
# Copyright © 2013-2016 The TSC Contributors
#############################################################################
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
##
#
# This simple script parses the docs/authors.txt and docs/specialthanks.txt
# files and outputs them to the file passed as the only commandline argument
# to this script.

THIS_DIR = File.dirname(File.expand_path(__FILE__))

unless ARGV[0]
  raise "No target file specified."
end

File.open(ARGV[0], "w:utf-8") do |f|
  # Write first part
  f.puts(%Q!
namespace TSC {

	const char* g_credits = "\\\n!)

  puts "processing authors.txt"
  File.open("#{THIS_DIR}/docs/authors.txt", "r:utf-8") do |f2|
    # Skip to real content
    loop do
      line = f2.gets
      break if line.start_with?("=-=-=-")
    end
    f2.gets

    # Write out the remaining part to the cpp file
    while (line = f2.gets)
      f.write(line.chomp.gsub(/<.*>/, "").strip) # Remove email address
      f.puts("\\n\\")
    end
  end

  f.puts("\\n\\")
  f.puts("-- Special Thanks --\\n\\")
  f.puts("\\n\\")

  puts "processing specialthanks.txt"
  File.open("#{THIS_DIR}/docs/specialthanks.txt", "r:utf-8") do |f2|
    # Skip to real content
    loop do
      line = f2.gets
      break if line.start_with?("=-=-=-")
    end
    f2.gets

    # Write out the remaining part to the cpp file
    while (line = f2.gets)
      f.write(line.chomp.gsub(/<.*>/, "").strip) # Remove email address
      f.puts("\\n\\")
    end
  end

  # Write final part
  f.write("\";\n}\n")
end
