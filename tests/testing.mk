.PHONY: tests beam-tests transient-tests integrator-tests clean-tests

tests: beam-tests transient-tests plastic-tests contact-tests integrator-tests

clean-all: clean-tests

# SETTINGS
# the scripts write their figures to files: no display
export MPLBACKEND = Agg
# ParaView frames only on request, e.g. for a test that has run already:
#   make PARAVIEW=1 -W tests/contact/test2.pro tests/contact/test2/result.pdf
paraview = $(if $(PARAVIEW),,-p "Output.paraview.sampleWhen=false")
beam_cases = 1 2 4 5 6 7
transient_cases = 1 2 3
plastic_cases = 1 2a 2b 3 4
contact_cases = 1 2 3
integrator_cases = 1 2 3 4 5 6

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
	@$^ $(paraview) > tests/beam/test$*/run.log

tests/beam/test%.pro: tests/beam/input.pro tests/beam/output.pro tests/beam/model.pro

# TRANSIENT TEST RESULTS
.PRECIOUS: tests/transient/test%/disp.gz

transient-tests: $(addprefix tests/transient/test, $(addsuffix /result.pdf, $(transient_cases)))

tests/transient/test%/result.pdf: tests/transient/test%.py\
																	 tests/transient/test%/disp.gz
	@$<

tests/transient/test%/disp.gz: $(program) tests/transient/test%.pro
	@$(MKDIR_P) $(dir $@)
	-@$^ $(paraview) > tests/transient/test$*/run.log

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
	-@$^ $(paraview) > tests/plastic/test$*/run.log

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
	-@$^ $(paraview) > tests/contact/test$*/run.log

tests/contact/test%.pro: tests/contact/input.pro tests/contact/output.pro tests/contact/model.pro

# INTEGRATOR TEST RESULTS
# Each test runs its input several times, with other integrators, fixed time steps or settings; the scripts only
# evaluate the runs. $(call integrator_run,<test>,<run>,<integrator>,<time step>,<end time>,<further -p settings>)
# writes tests/integrator/<test>/<run>.csv. Settings with a comma go into a variable first.
# With PARAVIEW=1 the runs that are made also write ParaView frames (every 1e-3) to <test>/<run>_visual/, e.g.
#   rm tests/integrator/test1/LeapFrog_3D_2.csv; make PARAVIEW=1 tests/integrator/test1/LeapFrog_3D_2.csv
# PARAVIEW_EVERY=<time> changes the interval of the frames.
integrator-tests: $(addprefix tests/integrator/test, $(addsuffix /result.pdf, $(integrator_cases)))

tests/integrator/test%/result.pdf: tests/integrator/test%.py
	@$<

integrator_inputs = tests/integrator/input.pro tests/integrator/model.pro tests/integrator/output.pro
leapfrog = -p "Solver.integrator.type='LeapFrog'"
milne = -p "Solver.integrator.type='MilneDevice'"
ode23 = -p "Solver.integrator.type='EmbeddedRK'" -p "Solver.integrator.kind='ode23'"
ode45 = -p "Solver.integrator.type='EmbeddedRK'" -p "Solver.integrator.kind='ode45'"
integrator_paraview = -p "Output.paraview.type='ParaView'" -p "Output.paraview.output_format='$$(@:.csv=_visual)/step%i'" \
	-p "Output.paraview.sampleWhen='t % $(or $(PARAVIEW_EVERY),1e-3) < deltaTime'" -p "Output.paraview.groups=['beams']" \
	-p "Output.paraview.beams.shape='Line2'" -p "Output.paraview.beams.disps=['dx','dy','dz']" \
	-p "Output.paraview.beams.otherDofs=['rx','ry','rz']" -p "Output.paraview.beams.node_data=['fint','fext']" \
	-p "Output.paraview.beams.el_data=['strain','stress']"

# a run that fails does not stop make: the script of the test then reports it
define integrator_run
tests/integrator/$(1)/result.pdf: tests/integrator/$(1)/$(2).csv
tests/integrator/$(1)/$(2).csv: $(program) tests/integrator/$(1).pro $(integrator_inputs)
	@$(MKDIR_P) $$(@D)
	-@$(program) tests/integrator/$(1).pro $(3) -p Solver.integrator.deltaTime=$(4) -p Solver.integrator.minDTime=$(4) \
		-p Solver.integrator.maxDTime=$(4) -p "control.runWhile='t < $(5) - $(4) / 2'" -p "Output.sampling.file='$$@'" \
		$(6) $(if $(PARAVIEW),$(integrator_paraview)) > $$(@:.csv=.log)
endef

# test 1, convergence order: five steps each, halving, to t = 2 (about three periods of the first mode).
# MilneDevice only to 0.02, where its damping of the fast modes is small
test1_planar = -p "model.model.init.veloVals=[1.,0.]" -p Solver.integrator.updateWhen=false
$(eval $(call integrator_run,test1,LeapFrog_3D_0,$(leapfrog),5e-5,2.))
$(eval $(call integrator_run,test1,LeapFrog_3D_1,$(leapfrog),2.5e-5,2.))
$(eval $(call integrator_run,test1,LeapFrog_3D_2,$(leapfrog),1.25e-5,2.))
$(eval $(call integrator_run,test1,LeapFrog_3D_3,$(leapfrog),6.25e-6,2.))
$(eval $(call integrator_run,test1,LeapFrog_3D_4,$(leapfrog),3.125e-6,2.))
$(eval $(call integrator_run,test1,LeapFrog_planar_0,$(leapfrog),5e-5,2.,$(test1_planar)))
$(eval $(call integrator_run,test1,LeapFrog_planar_1,$(leapfrog),2.5e-5,2.,$(test1_planar)))
$(eval $(call integrator_run,test1,LeapFrog_planar_2,$(leapfrog),1.25e-5,2.,$(test1_planar)))
$(eval $(call integrator_run,test1,LeapFrog_planar_3,$(leapfrog),6.25e-6,2.,$(test1_planar)))
$(eval $(call integrator_run,test1,LeapFrog_planar_4,$(leapfrog),3.125e-6,2.,$(test1_planar)))
$(eval $(call integrator_run,test1,Milne_3D_0,$(milne),2e-5,0.02))
$(eval $(call integrator_run,test1,Milne_3D_1,$(milne),1e-5,0.02))
$(eval $(call integrator_run,test1,Milne_3D_2,$(milne),5e-6,0.02))
$(eval $(call integrator_run,test1,Milne_3D_3,$(milne),2.5e-6,0.02))
$(eval $(call integrator_run,test1,Milne_3D_4,$(milne),1.25e-6,0.02))
$(eval $(call integrator_run,test1,Milne_planar_0,$(milne),2e-5,0.02,$(test1_planar)))
$(eval $(call integrator_run,test1,Milne_planar_1,$(milne),1e-5,0.02,$(test1_planar)))
$(eval $(call integrator_run,test1,Milne_planar_2,$(milne),5e-6,0.02,$(test1_planar)))
$(eval $(call integrator_run,test1,Milne_planar_3,$(milne),2.5e-6,0.02,$(test1_planar)))
$(eval $(call integrator_run,test1,Milne_planar_4,$(milne),1.25e-6,0.02,$(test1_planar)))
$(eval $(call integrator_run,test1,ode23_3D_0,$(ode23),1e-4,2.))
$(eval $(call integrator_run,test1,ode23_3D_1,$(ode23),5e-5,2.))
$(eval $(call integrator_run,test1,ode23_3D_2,$(ode23),2.5e-5,2.))
$(eval $(call integrator_run,test1,ode23_3D_3,$(ode23),1.25e-5,2.))
$(eval $(call integrator_run,test1,ode23_3D_4,$(ode23),6.25e-6,2.))
$(eval $(call integrator_run,test1,ode23_planar_0,$(ode23),1e-4,2.,$(test1_planar)))
$(eval $(call integrator_run,test1,ode23_planar_1,$(ode23),5e-5,2.,$(test1_planar)))
$(eval $(call integrator_run,test1,ode23_planar_2,$(ode23),2.5e-5,2.,$(test1_planar)))
$(eval $(call integrator_run,test1,ode23_planar_3,$(ode23),1.25e-5,2.,$(test1_planar)))
$(eval $(call integrator_run,test1,ode23_planar_4,$(ode23),6.25e-6,2.,$(test1_planar)))
$(eval $(call integrator_run,test1,ode45_3D_0,$(ode45),2e-4,2.))
$(eval $(call integrator_run,test1,ode45_3D_1,$(ode45),1e-4,2.))
$(eval $(call integrator_run,test1,ode45_3D_2,$(ode45),5e-5,2.))
$(eval $(call integrator_run,test1,ode45_3D_3,$(ode45),2.5e-5,2.))
$(eval $(call integrator_run,test1,ode45_3D_4,$(ode45),1.25e-5,2.))
$(eval $(call integrator_run,test1,ode45_planar_0,$(ode45),2e-4,2.,$(test1_planar)))
$(eval $(call integrator_run,test1,ode45_planar_1,$(ode45),1e-4,2.,$(test1_planar)))
$(eval $(call integrator_run,test1,ode45_planar_2,$(ode45),5e-5,2.,$(test1_planar)))
$(eval $(call integrator_run,test1,ode45_planar_3,$(ode45),2.5e-5,2.,$(test1_planar)))
$(eval $(call integrator_run,test1,ode45_planar_4,$(ode45),1.25e-5,2.,$(test1_planar)))

# test 2, axial wave over three cycles: each integrator at about the same accuracy per cost
$(eval $(call integrator_run,test2,LeapFrog,$(leapfrog),1e-3,12.))
$(eval $(call integrator_run,test2,Milne,$(milne),2.5e-4,12.))
$(eval $(call integrator_run,test2,ode23,$(ode23),1e-3,12.))
$(eval $(call integrator_run,test2,ode45,$(ode45),2e-3,12.))

# test 3, eigenfrequencies over ten periods of the first Timoshenko mode, 2 pi 10 / w_1: each integrator on 16 elements,
# and LeapFrog on 32 and 64 elements for the convergence with the mesh
test3_slender = -p model.model.rodMesh.child.material.radius=0.02
test3_stubby = -p model.model.rodMesh.child.material.radius=0.1
test3_slender_end = 17.88651121317495
test3_stubby_end = 3.6542784244207325
test3_32 = -p Input.input.onelab.Elements=32.
test3_64 = -p Input.input.onelab.Elements=64.
$(eval $(call integrator_run,test3,slender_LeapFrog,$(leapfrog),2.5e-4,$(test3_slender_end),$(test3_slender)))
$(eval $(call integrator_run,test3,slender_Milne,$(milne),1e-4,$(test3_slender_end),$(test3_slender)))
$(eval $(call integrator_run,test3,slender_ode23,$(ode23),2.5e-4,$(test3_slender_end),$(test3_slender)))
$(eval $(call integrator_run,test3,slender_ode45,$(ode45),1.25e-4,$(test3_slender_end),$(test3_slender)))
$(eval $(call integrator_run,test3,slender_LeapFrog_32,$(leapfrog),1.25e-4,$(test3_slender_end),$(test3_slender) $(test3_32)))
$(eval $(call integrator_run,test3,slender_LeapFrog_64,$(leapfrog),6.25e-5,$(test3_slender_end),$(test3_slender) $(test3_64)))
$(eval $(call integrator_run,test3,stubby_LeapFrog,$(leapfrog),2.5e-4,$(test3_stubby_end),$(test3_stubby)))
$(eval $(call integrator_run,test3,stubby_Milne,$(milne),1e-4,$(test3_stubby_end),$(test3_stubby)))
$(eval $(call integrator_run,test3,stubby_ode23,$(ode23),2.5e-4,$(test3_stubby_end),$(test3_stubby)))
$(eval $(call integrator_run,test3,stubby_ode45,$(ode45),1.25e-4,$(test3_stubby_end),$(test3_stubby)))
$(eval $(call integrator_run,test3,stubby_LeapFrog_32,$(leapfrog),1.25e-4,$(test3_stubby_end),$(test3_stubby) $(test3_32)))
$(eval $(call integrator_run,test3,stubby_LeapFrog_64,$(leapfrog),6.25e-5,$(test3_stubby_end),$(test3_stubby) $(test3_64)))

# test 4, freely spinning body of Krysl, Endres (2005), Sect. 4.1, to t = 100: LeapFrog at five steps for the order;
# output every 0.01, i.e. every 0.01 / step steps
test4_every_10 = -p "Output.sampling.sampleWhen='i % 10 < 1'"
$(eval $(call integrator_run,test4,LeapFrog_0,$(leapfrog),1e-3,100.,$(test4_every_10)))
$(eval $(call integrator_run,test4,LeapFrog_1,$(leapfrog),5e-4,100.,-p "Output.sampling.sampleWhen='i % 20 < 1'"))
$(eval $(call integrator_run,test4,LeapFrog_2,$(leapfrog),2.5e-4,100.,-p "Output.sampling.sampleWhen='i % 40 < 1'"))
$(eval $(call integrator_run,test4,LeapFrog_3,$(leapfrog),1.25e-4,100.,-p "Output.sampling.sampleWhen='i % 80 < 1'"))
$(eval $(call integrator_run,test4,LeapFrog_4,$(leapfrog),6.25e-5,100.,-p "Output.sampling.sampleWhen='i % 160 < 1'"))
$(eval $(call integrator_run,test4,Milne,$(milne),2.5e-4,100.,-p "Output.sampling.sampleWhen='i % 40 < 1'"))
$(eval $(call integrator_run,test4,ode23,$(ode23),1e-3,100.,$(test4_every_10)))
$(eval $(call integrator_run,test4,ode45,$(ode45),1e-3,100.,$(test4_every_10)))

# test 5, pure rotation of Holzinger, Gerstmayr (2021), Sect. 7.1.2, to t = 100
$(eval $(call integrator_run,test5,LeapFrog,$(leapfrog),1e-3,100.))
$(eval $(call integrator_run,test5,Milne,$(milne),2.5e-4,100.))
$(eval $(call integrator_run,test5,ode23,$(ode23),1e-3,100.))
$(eval $(call integrator_run,test5,ode45,$(ode45),1e-3,100.))

# test 6, flying spaghetti of Kinon, Eugster, Betsch (2026), Sect. 6.1, to t = 15, with the state of all nodes for the
# momenta; LeapFrog also at five steps to t = 5 for the order, without the state
test6_state = -p "Output.state.file='$$(@:.csv=_state.gz)'"
test6_no_state = -p "Output.state.type='None'"
$(eval $(call integrator_run,test6,LeapFrog,$(leapfrog),1e-3,15.,$(test6_state)))
$(eval $(call integrator_run,test6,Milne,$(milne),2.5e-4,15.,$(test6_state)))
$(eval $(call integrator_run,test6,ode23,$(ode23),1e-3,15.,$(test6_state)))
$(eval $(call integrator_run,test6,ode45,$(ode45),1e-3,15.,$(test6_state)))
$(eval $(call integrator_run,test6,LeapFrog_0,$(leapfrog),1e-3,5.,$(test6_no_state)))
$(eval $(call integrator_run,test6,LeapFrog_1,$(leapfrog),5e-4,5.,$(test6_no_state)))
$(eval $(call integrator_run,test6,LeapFrog_2,$(leapfrog),2.5e-4,5.,$(test6_no_state)))
$(eval $(call integrator_run,test6,LeapFrog_3,$(leapfrog),1.25e-4,5.,$(test6_no_state)))
$(eval $(call integrator_run,test6,LeapFrog_4,$(leapfrog),6.25e-5,5.,$(test6_no_state)))

# CLEAN UP THE TESTS
clean-tests:
	@$(RM_R) tests/beam/test*/	
	@$(RM_R) tests/transient/test*/
	@$(RM_R) tests/plastic/test*/
	@$(RM_R) tests/contact/test*/
	@$(RM_R) tests/integrator/test*/
	@$(RM) tests/*.png
