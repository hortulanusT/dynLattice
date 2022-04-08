beamruns = $(addprefix tests/beam/test, $(addsuffix /run.log, 0 0_ref 1 2 3 4 5))
beamplots = $(addprefix tests/beam/test, $(addsuffix /result.pdf, 0 1 2 3 4 5))

.PHONY: test-element beam-tests

test-element: all
	@$(RM) tests/element/*.res
	@$(RM) tests/element/*.log
	@$(RM_R) tests/element/DIFF
	@$(RM_R) tests/element/FAIL
	@$(MKDIR) tests/element/DIFF
	@$(MKDIR) tests/element/FAIL
	@./scripts/testing/element.py

beam-tests: $(beamplots)

$(beamplots): $(beamruns)
	@scripts/$(patsubst %/,%,$(dir $@)).py

$(beamruns): all
	@$(RM_R) $(dir $@)
	@$(MKDIR) $(dir $@)
	@$(program) $(patsubst %/,%,$(dir $@)).pro > /dev/null