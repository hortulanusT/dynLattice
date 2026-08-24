.PHONY: docs docs-clean docs-clean-only

# Clean all including documentation
clean-all: docs-clean-only

# doc-only assets: generated for the usage page, not needed for `tests`
doc_assets = tests/transient/test1_schematic.png tests/transient/test1_animate.gif

# Generate documentation
docs: tests $(doc_assets)
	@doxygen doc/doxygen.conf

# Clean and regenerate documentation
docs-clean: tests $(doc_assets) docs-clean-only
	@doxygen doc/doxygen.conf

# Clean documentation only
docs-clean-only:
	@rm -rf doc/html

# Documentation assets
tests/transient/test1_schematic.png: tests/transient/test1_schematic.py
	@$<

tests/transient/test1_animate.gif: tests/transient/test1_animate.py tests/transient/test1/disp.gz
	@$<
