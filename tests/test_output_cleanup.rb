##
# Copyright (C) Navaneeth.K.N
#
# This is part of libvarnam. See LICENSE.txt for the license
##


require 'fileutils'

FileUtils.rm_rf("output")
Dir::mkdir("output")
File.new("output/00-suggestions.vst", 'w')
sleep 1
exit(0)
