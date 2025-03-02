# TODO(elpekenin): copy outputs back in here?

WORKSPACE = ~/workspace/qmk
WEB_HOST = elraspberrin

KB = keyboards/elpekenin/access
USER = users/elpekenin

DOCS_ORIG = docs
DOCS_TARGET = elpekenin_docs

SRC = $(KB) $(USER)

# copy files into workspace
cp:
	rm -rf $(WORKSPACE)/$(DOCS_TARGET)
	mkdir -p $(WORKSPACE)/$(DOCS_TARGET)
	cp -r $(DOCS_ORIG)/* $(WORKSPACE)/$(DOCS_TARGET)
	cp -r $(KB)/* $(WORKSPACE)/$(KB)
	cp -r $(USER)/* $(WORKSPACE)/$(USER)

# build documentation
docs: cp
	cd $(WORKSPACE)/$(DOCS_TARGET) \
	&& rm -rf _build \
	&& mkdir _build \
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
