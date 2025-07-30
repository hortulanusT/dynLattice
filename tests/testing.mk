.PHONY: tests beam-tests transient-tests clean-tests

tests: beam-tests transient-tests plastic-tests contact-tests

clean-all: clean-tests

# SETTINGS
beam_cases = 1 2 4 5
transient_cases = 1 2 3
plastic_cases = 1 2a 2b 3 4
contact_cases = 0 1 2

# general dependency of .pro files on .geo files
%.pro: %.geo

# BEAM TEST RESULTS
.PRECIOUS: tests/beam/test%/disp.csv tests/beam/test%/resp.csv

beam-tests: $(addprefix tests/beam/test, $(addsuffix /result.pdf, $(beam_cases)))

tests/beam/test0/result.pdf: tests/beam/test0.py\
															 tests/beam/test0/disp.csv\
															 tests/beam/test0/resp.csv\
															 tests/beam/test0_ref/disp.csv\
															 tests/beam/test0_ref/resp.csv
	@$<

tests/beam/test%/result.pdf: tests/beam/test%.py\
															 tests/beam/test%/disp.csv\
															 tests/beam/test%/resp.csv
	@$<

tests/beam/test%/disp.csv tests/beam/test%/resp.csv:\
															$(program) tests/beam/test%.pro
	@$(MKDIR_P) $(dir $@)
	@$^ > tests/beam/test$*/run.log

tests/beam/test%.pro: tests/beam/input.pro tests/beam/output.pro tests/beam/model.pro

# TRANSIENT TEST RESULTS
.PRECIOUS: tests/transient/test%/disp.gz

transient-tests: $(addprefix tests/transient/test, $(addsuffix /result.pdf, $(transient_cases)))

tests/transient/test%/result.pdf: tests/transient/test%.py\
																	 tests/transient/test%/disp.gz
	@$<

tests/transient/test%/disp.gz: $(program) tests/transient/test%.pro
	@$(MKDIR_P) $(dir $@)
	-@$^ > tests/transient/test$*/run.log

tests/transient/test%.pro: tests/transient/input.pro tests/transient/output.pro tests/transient/model.pro

# PLASTIC TEST RESULTS
.PRECIOUS: tests/plastic/test%/disp.csv tests/plastic/test%/resp.csv

plastic-tests: $(addprefix tests/plastic/test, $(addsuffix /result.pdf, $(plastic_cases)))

tests/plastic/test%/result.pdf: tests/plastic/test%.py\
															 tests/plastic/test%/disp.csv\
															 tests/plastic/test%/resp.csv
	@$<

tests/plastic/test%/disp.csv tests/plastic/test%/resp.csv:\
															$(program) tests/plastic/test%.pro
	@$(MKDIR_P) $(dir $@)
	-@$^ > tests/plastic/test$*/run.log

tests/plastic/test%.pro: tests/plastic/input.pro tests/plastic/output.pro tests/plastic/model.pro

# CONTACT TEST RESULTS
.PRECIOUS: tests/contact/test%/disp.csv tests/contact/test%/resp.csv

contact-tests: $(addprefix tests/contact/test, $(addsuffix /result.pdf, $(contact_cases)))

tests/contact/test%/result.pdf: tests/contact/test%.py\
															 tests/contact/test%/disp.csv\
															 tests/contact/test%/resp.csv
	@$<

tests/contact/test%/disp.csv tests/contact/test%/resp.csv:\
															$(program) tests/contact/test%.pro
	@$(MKDIR_P) $(dir $@)
	-@$^ > tests/contact/test$*/run.log

tests/contact/test%.pro: tests/contact/input.pro tests/contact/output.pro tests/contact/model.pro

# CLEAN UP THE TESTS
clean-tests:
	@$(RM_R) tests/element/runs
	@$(RM) tests/element/result_*.txt

	@$(RM_R) tests/beam/test*/	
	@$(RM_R) tests/transient/test*/
	@$(RM_R) tests/plastic/test*/
	@$(RM_R) tests/contact/test*/
