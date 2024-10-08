# TODO(elpekenin): copy outputs back in here?

WORKSPACE = ~/workspace/qmk
WEB_HOST = elraspberrin

KB = keyboards/elpekenin/access
USER = users/elpekenin

SRC = $(KB) $(USER)

# copy files into workspace
cp:
	rm -rf $(WORKSPACE)/$(USER)/docs
	cp -r $(KB)/* $(WORKSPACE)/$(KB)
	cp -r $(USER)/* $(WORKSPACE)/$(USER)

# run linters
lint:
	mkdir -p lint
	clang-tidy $(USER_SRC) > lint/clang-tidy 2>&1 || exit 0
	flawfinder $(USER_SRC) > lint/flawfinder 2>&1 || exit 0

# build documentation
docs:
	cd $(WORKSPACE)/$(USER)/docs && \
	rm -rf _build && \
	mkdir _build && \
	make html

# copy files + generate docs + copy them on raspberry
deploy-docs: cp docs
	scp -r $(WORKSPACE)/$(USER)/docs/_build/html/* $(WEB_HOST):~/qmk_docs

preview-docs: cp docs
	cd $(WORKSPACE)/$(USER)/docs/_build/html && python -m http.server

# build some C and link it with my custom zig library for testing
native-testing:
	cd $(WORKSPACE)/$(USER) && \
	zig build -Dmcu=native && \
	gcc native_test.c -lelpekenin -Lzig-out/lib -o foo && \
	./foo

.PHONY: cp lint docs native-testing
