.PHONY: tests element-tests beam-tests clean-tests

tests: element-tests beam-tests

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

# ELEMENT TEST RESULTS
.PRECIOUS: tests/element/runs/%-load.csv tests/element/runs/%-disp.csv tests/element/runs/%-resp.csv

element-tests : $(element_results)

tests/element/result_%.txt : scripts/testing/element.py $(load_results) $(disp_results) $(resp_results)
	@$< $*

tests/element/runs/%-load.csv tests/element/runs/%-disp.csv tests/element/runs/%-resp.csv : $(program) tests/element/test.pro
	@$(MKDIR_P) $(dir $@)
	$(eval options := $(call getoptions, $*))
	$(eval order := $(patsubst lin, 1, $(patsubst quad, 2, $(patsubst cub, 3, $(word 1,$(options))))))
	$(eval nodes := $(patsubst lin, 2, $(patsubst quad, 3, $(patsubst cub, 4, $(word 1,$(options))))))
	$(eval elemD := $(word 2, $(options)))
	$(eval loadT := $(word 3, $(options)))
	$(eval loadD := $(word 4, $(options)))
	@$^ -p Input.input.order=$(order)\
			-p Input.input.onelab.$(elemD)=1.\
			-p model.model.model.rodMesh.child.shape.numPoints=$(nodes)\
			-p model.model.model.$(loadT).nodeGroups=[\"free\"]\
			-p model.model.model.$(loadT).dofs=[\"$(loadD)\"]\
			-p model.model.model.$(loadT).factors=[1.]\
			-p Output.disp.file=\"tests/element/runs/$*-disp.csv\"\
			-p Output.resp.file=\"tests/element/runs/$*-resp.csv\"\
			-p Output.load.file=\"tests/element/runs/$*-load.csv\"\
			> tests/element/runs/$*.log

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
	@$(RM_R) $(dir $@)
	@$(MKDIR) $(dir $@)
	@$^ > tests/beam/test$*/run.log

# CLEAN UP THE TESTS
clean-tests :
	@$(RM_R) tests/element/runs
	@$(RM) tests/element/result_*.txt
	@$(RM_R) tests/beam/test*/
