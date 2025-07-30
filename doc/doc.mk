.PHONY: docs docs-clean docs-clean-only

# Clean all including documentation
clean-all: docs-clean-only

# Generate documentation
docs: tests
	doxygen doxygen.conf

# Clean and regenerate documentation
docs-clean: tests
	@find doc ! -name 'doc*' -exec rm -rf {} \;
	doxygen doxygen.conf

# Clean documentation only
docs-clean-only:
	@find doc ! -name 'doc*' -exec rm -rf {} \;
