# TODO(elpekenin): copy outputs back in here?

WORKSPACE = ~/workspace/qmk

KB = keyboards/elpekenin/access
USER = users/elpekenin
SRC = $(KB) $(USER)

LINTERS := clang-tidy flawfinder

# copy files into workspace
cp:
	cp -r $(KB)/* $(WORKSPACE)/$(KB)
	cp -r $(USER)/* $(WORKSPACE)/$(USER)

# run linters
lint:
	$(foreach LINTER,$(LINTERS),$(shell cd $(WORKSPACE)/$(USER) mkdir -p lint && $(LINTER) $(USER_SRC) > lint/$(LINTER) 2>&1 || exit 0))

# build documentation
docs:
	cd $(WORKSPACE)/$(USER)/docs && \
	rm -rf _build && \
	mkdir _build && \
	make html && \
	cd _build/html && \
	python -m http.server

# build some C and link it with my custom zig library for testing
native-testing:
	cd $(WORKSPACE)/$(USER) && \
	zig build -Dmcu=native && \
	gcc native_test.c -lelpekenin -Lzig-out/lib -o foo && \
	./foo

.PHONY: cp lint docs native-testing
