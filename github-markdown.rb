#!/usr/bin/ruby

if ARGV.length != 1 then
   puts "Usage: #{$0} <file>"
   exit 1
end

require 'github/markup'
GitHub::Markup.render( ARGV[0], File.read( ARGV[0] ))

