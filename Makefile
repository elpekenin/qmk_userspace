.PHONY: lint

USER_SRC = users/elpekenin/src/* keyboards/*

LINTERS := clang-tidy flawfinder
lint:
	$(foreach LINTER,$(LINTERS),$(shell mkdir -p lint && $(LINTER) $(USER_SRC) > lint/$(LINTER) 2>&1 || exit 0))

# build some arbitrary C (foo.c) and link it with my custom zig library
native-testing:
	@cd users/elpekenin && zig build -Dmcu=native && gcc foo.c -lelpekenin -Lzig-out/lib -o foo
