.PHONY: docs docs-clean docs-clean-only

# Clean all including documentation
clean-all: docs-clean-only

# doc-only assets: generated for the usage page, not needed for `tests`
doc_assets = tests/transient/test1_schematic.png tests/transient/test1_animate.gif
doc_assets += tests/docs/test2_schematic.png tests/docs/test2_animate.gif

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
tests/transient/test1_schematic.png: tests/transient/test1_schematic.py tests
	@$<

tests/transient/test1_animate.gif: tests/transient/test1_animate.py tests
	@$<

# test2 is a docs-only example (not part of `tests`), so it's run here instead
tests/docs/test2/vis.pvd: $(program) tests/docs/test2.pro
	@$(MKDIR_P) $(dir $@)
	@$^ > tests/docs/test2/run.log

tests/docs/test2_schematic.png: tests/docs/test2_schematic.py tests/docs/test2/vis.pvd
	@$<

tests/docs/test2_animate.gif: tests/docs/test2_animate.py tests/docs/test2/vis.pvd
	@$<
