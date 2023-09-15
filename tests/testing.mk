.PHONY: tests element-tests beam-tests transient-tests clean-tests

tests: element-tests beam-tests transient-tests plastic-tests

clean-all: clean-tests

# SETTINGS
getoptions = $(subst _, ,$(notdir $(basename $(1))))

ELEMENT_TYPES := lin_x lin_y lin_z quad_x quad_y quad_z cub_x cub_y cub_z
element_results := $(addprefix tests/element/result_, $(addsuffix .txt, $(ELEMENT_TYPES)))
ELEMENT_LOADS := $(foreach dir, dx dy dz rx ry rz, force_$(dir) disp_$(dir))
load_results := $(addprefix tests/element/runs/%_, $(addsuffix -load.csv, $(ELEMENT_LOADS)))
disp_results := $(addprefix tests/element/runs/%_, $(addsuffix -disp.csv, $(ELEMENT_LOADS)))
resp_results := $(addprefix tests/element/runs/%_, $(addsuffix -resp.csv, $(ELEMENT_LOADS)))

beam_cases = 0 1 2 3 4 5
transient_cases = 0 1 2 3
plastic_cases = 1 2a #2b

# general dependency of .pro files on .geo files
%.pro: %.geo

# ELEMENT TEST RESULTS
.PRECIOUS: tests/element/runs/%-load.csv tests/element/runs/%-disp.csv tests/element/runs/%-resp.csv

element-tests : $(element_results)

tests/element/result_%.txt : scripts/testing/element.py $(load_results) $(disp_results) $(resp_results)
	@$< $*

tests/element/runs/%-load.csv tests/element/runs/%-disp.csv tests/element/runs/%-resp.csv : $(program) tests/element/test.pro
	@$(MKDIR_P) $(dir $@)
	$(eval options := $(call getoptions, $*))
	$(eval order := $(patsubst lin, 1, $(patsubst quad, 2, $(patsubst cub, 3, $(word 1,$(options))))))
	$(eval elemD := $(word 2, $(options)))
	$(eval loadT := $(word 3, $(options)))
	$(eval loadD := $(word 4, $(options)))
	@$^ -p Input.input.order=$(order)\
			-p Input.input.onelab.$(elemD)=1.\
			-p model.model.model.$(loadT).nodeGroups=[\"free\"]\
			-p model.model.model.$(loadT).dofs=[\"$(loadD)\"]\
			-p model.model.model.$(loadT).factors=[1.]\
			-p Output.disp.file=\"tests/element/runs/$*-disp.csv\"\
			-p Output.resp.file=\"tests/element/runs/$*-resp.csv\"\
			-p Output.load.file=\"tests/element/runs/$*-load.csv\"\
			> tests/element/runs/$*.log

tests/element/test.pro: tests/element/input.pro tests/element/output.pro tests/element/model.pro

# BEAM TEST RESULTS
.PRECIOUS: tests/beam/test%/disp.csv tests/beam/test%/resp.csv

beam-tests : $(addprefix tests/beam/test, $(addsuffix /result.pdf, $(beam_cases)))

tests/beam/test0/result.pdf : scripts/testing/beam0.py\
															 tests/beam/test0/disp.csv\
															 tests/beam/test0/resp.csv\
															 tests/beam/test0_ref/disp.csv\
															 tests/beam/test0_ref/resp.csv
	@$<

tests/beam/test%/result.pdf : scripts/testing/beam%.py\
															 tests/beam/test%/disp.csv\
															 tests/beam/test%/resp.csv
	@$<

tests/beam/test%/disp.csv tests/beam/test%/resp.csv :\
															$(program) tests/beam/test%.pro
	@$(MKDIR_P) $(dir $@)
	@$^ > tests/beam/test$*/run.log

tests/beam/test%.pro: tests/beam/input.pro tests/beam/output.pro tests/beam/model.pro

# TRANSIENT TEST RESULTS
.PRECIOUS: tests/transient/test%/disp.gz

transient-tests : $(addprefix tests/transient/test, $(addsuffix /result.pdf, $(transient_cases)))

tests/transient/test%/result.pdf: scripts/testing/transient%.py\
																	 tests/transient/test%/disp.gz
	@$<

tests/transient/test%/disp.gz: $(program) tests/transient/test%.pro
	@$(MKDIR_P) $(dir $@)
	-@$^ > tests/transient/test$*/run.log

tests/transient/test%.pro: tests/transient/input.pro tests/transient/output.pro tests/transient/model.pro

# PLASTIC TEST RESULTS
.PRECIOUS: tests/plastic/test%/disp.csv tests/plastic/test%/resp.csv

plastic-tests : $(addprefix tests/plastic/test, $(addsuffix /result.pdf, $(plastic_cases)))

tests/plastic/test%/result.pdf : scripts/testing/plastic%.py\
															 tests/plastic/test%/disp.csv\
															 tests/plastic/test%/resp.csv
	@$<

tests/plastic/test%/disp.csv tests/plastic/test%/resp.csv :\
															$(program) tests/plastic/test%.pro
	@$(MKDIR_P) $(dir $@)
	-@$^ > tests/plastic/test$*/run.log

tests/plastic/test%.pro: tests/plastic/input.pro tests/plastic/output.pro tests/plastic/model.pro

# CLEAN UP THE TESTS
clean-tests :
	@$(RM_R) tests/element/runs
	@$(RM) tests/element/result_*.txt

	@$(RM_R) tests/beam/test*/	
	@$(RM_R) tests/transient/test*/
	@$(RM_R) tests/plastic/test*/
