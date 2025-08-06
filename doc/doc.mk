.PHONY: docs docs-clean docs-clean-only

# Clean all including documentation
clean-all: docs-clean-only

# Generate documentation
docs: tests
	doxygen doc/doxygen.conf

# Clean and regenerate documentation
docs-clean: tests docs-clean-only
	doxygen doc/doxygen.conf

# Clean documentation only
docs-clean-only:
	@rm -rf doc/html
