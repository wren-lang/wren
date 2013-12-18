#!/usr/bin/python
# http://www.bagley.org/~doug/shootout/

import sys
import time

class Toggle(object):
    def __init__(self, start_state):
        self.bool = start_state
    def value(self):
        return(self.bool)
    def activate(self):
        self.bool = not self.bool
        return(self)

class NthToggle(Toggle):
    def __init__(self, start_state, max_counter):
        Toggle.__init__(self, start_state)
        self.count_max = max_counter
        self.counter = 0
    def activate(self):
        self.counter += 1
        if (self.counter >= self.count_max):
            super(NthToggle, self).activate()
            self.counter = 0
        return(self)


def main():
    start = time.clock()

    NUM = 1000000

    val = 1
    toggle = Toggle(val)
    for i in xrange(0,NUM):
        val = toggle.activate().value()
    if val:
        print "true"
    else:
        print "false"

    val = 1
    ntoggle = NthToggle(val, 3)
    for i in xrange(0,NUM):
        val = ntoggle.activate().value()
    if val:
        print "true"
    else:
        print "false"

    print("elapsed: " + str(time.clock() - start))


main()
