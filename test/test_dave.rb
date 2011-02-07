require 'rubygems'
require 'eventmachine'
require 'lib/dave'
require 'test/unit'

class DaveTest < Test::Unit::TestCase
	def setup
		@dave = Dave.new('127.0.0.1', 102)
		@dave.connect
	end

	def test_fetch
		assert_equal 16, @dave.fetch(1,2).length
	end

	def test_send
		assert_equal true, @dave.send(1,'a')
		assert_equal true, @dave.send(2, "\0\0")
	end

	def test_em
		EM.run do
			assert_equal 80, @dave.fetch(10,10).length 
			EM.stop
		end
	end
end
