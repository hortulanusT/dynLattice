.PHONY: element-tests beam-tests clean-tests

.PRECIOUS: tests/beam/test%/disp.csv tests/beam/test%/resp.csv

element-tests: $(program)
	@$(RM) tests/element/*.res
	@$(RM) tests/element/*.log
	@$(RM_R) tests/element/DIFF
	@$(RM_R) tests/element/FAIL
	@$(MKDIR) tests/element/DIFF
	@$(MKDIR) tests/element/FAIL
	@./scripts/testing/element.py

beam-tests: $(addprefix tests/beam/test, \
						$(addsuffix /result.pdf, 0 1 2 3 4 5))

tests/beam/test0/result.pdf:  scripts/testing/beam0.py\
 														  tests/beam/test0/disp.csv\
 														  tests/beam/test0/resp.csv\
 														  tests/beam/test0_ref/disp.csv\
 														  tests/beam/test0_ref/resp.csv
	@$<

tests/beam/test%/result.pdf:  scripts/testing/beam%.py\
 														  tests/beam/test%/disp.csv\
 														  tests/beam/test%/resp.csv
	@$<

tests/beam/test%/disp.csv tests/beam/test%/resp.csv:\
														  $(program) tests/beam/test%.pro
	@$(RM_R) $(dir $@)
	@$(MKDIR) $(dir $@)
	@$^ > tests/beam/test$*/run.log

clean-tests:
	@$(RM) tests/element/*.res
	@$(RM) tests/element/*.log
	@$(RM_R) tests/element/DIFF
	@$(RM_R) tests/element/FAIL
	@$(RM_R) tests/beam/test*/
