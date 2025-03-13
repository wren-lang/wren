TOP=../../..

restore: restore.c $(TOP)/lib/libwren_d.a
	gcc -Wall -I$(TOP)/src/include -o $@.o -c $<
	gcc -o $@ $@.o $(TOP)/lib/libwren_d.a -lm

button-led.wrenb: button-led.wren
	env WREN_SNAPSHOT=ys WREN_SNAPSHOT_TO=$@ $(TOP)/bin/wren_test_d $<

run:
	env WREN_SNAPSHOT=R ./restore button-led.wrenb
