##
# Copyright (C) Navaneeth.K.N
#
# This is part of libvarnam. See LICENSE.txt for the license
##

require 'unicode_utils'

if not ARGV.length == 2
	puts "Start and end points are required"
	exit(1)
end

start_point = Integer(ARGV[0])
end_point = Integer(ARGV[1])

start_point.upto(end_point) do |point|
 puts UnicodeUtils::Codepoint.new(point).to_s
end

