WORKSPACE = ~/workspace/qmk

USER = users/elpekenin

# build some C and link it with my custom zig library for testing
native-testing:
	cd $(WORKSPACE)/$(USER) \
	&& zig build -Dmcu=native \
	&& gcc native_test.c -lelpekenin -Lzig-out/lib -o foo \
	&& ./foo

.PHONY: cp docs native-testing
