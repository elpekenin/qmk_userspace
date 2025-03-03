# TODO(elpekenin): copy outputs back in here?

WORKSPACE = ~/workspace/qmk
WEB_HOST = elraspberrin

KB = keyboards/elpekenin/access
USER = users/elpekenin
MODULES = modules/elpekenin

DOCS_ORIG = docs
DOCS_TARGET = elpekenin_docs

SRC = $(KB) $(USER)

# copy files into workspace
# note: do not rm because we need files that are there after compilation (users/elpekenin/generated)
cp:
	mkdir -p $(WORKSPACE)/$(DOCS_TARGET)
	cp -r $(DOCS_ORIG)/* $(WORKSPACE)/$(DOCS_TARGET)

	mkdir -p $(WORKSPACE)/$(KB)
	cp -r $(KB)/* $(WORKSPACE)/$(KB)

	mkdir -p $(WORKSPACE)/$(USER)
	cp -r $(USER)/* $(WORKSPACE)/$(USER)

	mkdir -p $(WORKSPACE)/$(MODULES)
	cp -r $(MODULES)/* $(WORKSPACE)/$(MODULES)

# build documentation
docs: cp
	cd $(WORKSPACE)/$(DOCS_TARGET) \
	&& rm -rf _build \
	&& mkdir _build \
	&& export USERSPACE_COMMIT=$(shell git rev-parse HEAD) \
	&& export MODULE_COMMIT=$(shell cd modules/elpekenin && git rev-parse HEAD) \
	&& make html

preview-docs: docs
	cd $(WORKSPACE)/$(DOCS_TARGET)/_build/html && python -m http.server

# generate docs + and put them on raspberry to serve new version
deploy-docs: docs
	scp -r $(WORKSPACE)/$(DOCS_TARGET)/_build/html/* $(WEB_HOST):~/qmk_docs

# build some C and link it with my custom zig library for testing
native-testing:
	cd $(WORKSPACE)/$(USER) \
	&& zig build -Dmcu=native \
	&& gcc native_test.c -lelpekenin -Lzig-out/lib -o foo \
	&& ./foo

.PHONY: cp docs native-testing
