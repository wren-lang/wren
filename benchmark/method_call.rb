#!/usr/bin/ruby
# -*- mode: ruby -*-
# $Id: methcall.ruby,v 1.1 2004-05-19 18:10:41 bfulgham Exp $
# http://www.bagley.org/~doug/shootout/
# with help from Aristarkh Zagorodnikov

class Toggle
    def initialize(start_state)
        @bool = start_state
    end

    def value
        @bool
    end

    def activate
        @bool = !@bool
        self
    end
end

class NthToggle < Toggle
    def initialize(start_state, max_counter)
        super start_state
        @count_max = max_counter
        @counter = 0
    end

    def activate
        @counter += 1
        if @counter >= @count_max
            super
            @counter = 0
        end
        self
    end
end

def main()
    start = Time.now

    n = 100000

    val = 1
    toggle = Toggle.new(val)
    n.times do
        val = toggle.activate().value()
        val = toggle.activate().value()
        val = toggle.activate().value()
        val = toggle.activate().value()
        val = toggle.activate().value()
        val = toggle.activate().value()
        val = toggle.activate().value()
        val = toggle.activate().value()
        val = toggle.activate().value()
        val = toggle.activate().value()
    end
    if val then puts "true" else puts "false" end

    val = 1
    ntoggle = NthToggle.new(val, 3)
    n.times do
        val = ntoggle.activate().value()
        val = ntoggle.activate().value()
        val = ntoggle.activate().value()
        val = ntoggle.activate().value()
        val = ntoggle.activate().value()
        val = ntoggle.activate().value()
        val = ntoggle.activate().value()
        val = ntoggle.activate().value()
        val = ntoggle.activate().value()
        val = ntoggle.activate().value()
    end
    if val then puts "true" else puts "false" end

    puts "elapsed: " + (Time.now - start).to_s
end

main()
