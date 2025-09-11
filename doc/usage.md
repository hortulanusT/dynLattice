\page usage Usage

As an example use-case, the transient \ref transient1 is laid out and explained.

# Setup
As a first step, we need to compile the program using `jive make`. This will create the executable `bin/dynLattice` we can use to run the simulation.

The next step is to create the needed files, starting with the geometry file, in this case `tests/transient/test1.geo`. The `GMSH` syntax can be found on their [documentation](https://gmsh.info/doc/texinfo/gmsh.html#Gmsh-scripting-language).

Now, we need to create our property file with model inputs. For our case, it can be found in `tests/transient/test1.pro` and will be explained line-by-line in the following. Note that the file contains include statements for three other input files: `input.pro`, `model.pro` and `output.pro`. Some of the contents of these included files are overwritten in `test1.pro` after the `include` statements.

| Command | Explanation |
| ------- | ----------- |
| **Program Control** | |
| `control.runWhile = "t <= 30";` | lets the control module (cf. main.cpp) terminate the simulation once the variable `t` (defaults to the current simulation time in seconds) has become greater than `30`. |
| **Solver Configuration** | |
| `Solver.modules = [ "integrator" ];` | defines the modules, that make up the solver. In our case it is only one, called `integrator`. |
| `Solver.integrator.type = "MilneDevice";` | defines the type of the `integrator` module, in our case it's a MilneDeviceModule (this type needs to agree with the name it is declared under in the [ModuleFactory](https://jive-manual.dynaflow.com/classjive_1_1app_1_1ModuleFactory.html)) |
| `Solver.integrator.deltaTime = 5e-5;` | set the starting time step size for the `integrator` to \f$5 \times 10^{-5}\f$ seconds |
| **Material Properties** | |
| `params.rod_details.material.type = "ElasticRod";` | define the type of the material to be ElasticRodMaterial |
| `params.rod_details.material.cross_section = "square";` | define the rod to be of square cross-section |
| `params.rod_details.material.side_length = "sqrt(12/2e3)";` | define the sides to be \f$\sqrt{12/2e3}\approx0.0775\f$ (meter) long |
| `params.rod_details.material.young = "5.6e10/12";` | define the rods Young's modulus to be \f$5.6e10/12\approx4.67e9\f$ (Pascal) |
| `params.rod_details.material.shear_modulus = 2e9;` | define the rods shear modulus to be \f$2e9\f$ (Pascal) |
| `params.rod_details.material.density = 200.;` | define the rod to have a density of \f$200\f$ (kilogram/meter³) |
| **File Includes** | |
| `include "input.pro";` | include the \ref input file defining input modules and geometry reading |
| `include "model.pro";` | include the \ref model file defining the physical model |
| `include "output.pro";` | include the \ref output file defining output modules and data collection |
| **Additional Settings** | |
| `Input.input.order = 2;` | set the interpolation order for the input elements to 2 (quadratic elements) |
| **Force Model Override** | |
| `model.model.force.type = "None";` | override the force model from model.pro to have no applied forces |
| **Displacement Boundary Conditions** | |
| `model.model.disp.type = "LoadScale";` | set the displacement boundary condition to use load scaling ([LoadScaleModel](https://jive-manual.dynaflow.com/classjive_1_1model_1_1LoadScaleModel.html)) (in dynamic simulations displacement prescribtions are prescribing accelerations) |
| `model.model.disp.scaleFunc = "if (t<15, 6/15 * (1 - cos(2*PI/15 * t)), 0)";` | define a time-dependent scaling function: a single sinusoidal half-wave during the first 15s |
| `model.model.disp.model.type = "Dirichlet";` | specify that the displacement model uses Dirichlet boundary conditions (DirichletModel) (they will apply to the acceleration in the dynamic solves) |
| `model.model.disp.model.nodeGroups = [ "fixed" ];` | apply the displacement (acceleration) boundary condition to the "fixed" node group |
| `model.model.disp.model.factors = [ 1. ];` | set the scaling factor for the displacement (acceleration) to 1.0 |
| `model.model.disp.model.dofs = [ "rz" ];` | apply the displacement (acceleration) boundary condition to the rz (rotation about z-axis) degree of freedom |
| **CSV Output Configuration** | |
| `Output.disp.type = "Sample";` | override the output type from output.pro to use sampling output ([SampleModule](https://jive-manual.dynaflow.com/classjive_1_1app_1_1SampleModule.html)) |
| `Output.disp.file = "$(CASE_NAME)/disp.gz";` | specify the output file path using the case name variable, compressed with gzip |
| `Output.disp.dataSets = [ "free.disp.dx", "free.disp.dy", "free.disp.dz", "free.disp.rx", "free.disp.ry", "free.disp.rz" ];` | define the datasets to output: all displacement and rotation components for the "free" node group |
| `Output.disp.dataSets += "fixed.disp.rz";` | append the rz displacement of the "fixed" node group to the output datasets |
| `Output.disp.dataSets += "t";` | append the time variable to the output datasets |
| `Output.disp.separator = ",";` | set the CSV separator character to comma |
| **ParaView Visualization Output** | |
| `Output.modules += "paraview";` | add a ParaView output module to the existing output modules |
| `Output.paraview.type = "ParaView";` | specify the module type as ParaViewModule visualization output |
| `Output.paraview.output_format = "$(CASE_NAME)/visual/step%i";` | set the output format with case name and step numbering |
| `Output.paraview.groups = [ "beams" ];` | define the groups to include in ParaView output (beams) |
| `Output.paraview.beams.shape = "Line3";` | specify that beams should be visualized as 3D line elements |
| `Output.paraview.beams.disps = model.model.rodMesh.child.dofNamesTrans;` | include translational displacement fields in the ParaView output |
| `Output.paraview.beams.otherDofs = model.model.rodMesh.child.dofNamesRot;` | include rotational degree of freedom fields in the ParaView output |
| `Output.paraview.beams.node_data = ["fint", "fext", "fres"];` | specify node data to output: internal forces, external forces, and residual forces |
| `Output.paraview.beams.el_data = ["strain", "stress", "mat_stress", "mat_strain"];` | specify element data to output: strain, stress, material stress, and material strain |
| `Output.paraview.sampleWhen = "t % 0.1 < deltaTime";` | set the sampling condition: output when time modulo 0.1 is less than the time step size (every 0.1 seconds) |


\subsubsection input input.pro
The `input.pro` file defines the input modules responsible for reading geometry and defining node groups. Here are the line-by-line explanations:

| Command | Explanation |
| ------- | ----------- |
| **Input Module Configuration** | |
| `Input.modules = [ "input", "groupInput" ];` | define the input modules: "input" for geometry reading and "groupInput" for node group creation |
| **Geometry Input Settings** | |
| `Input.input.type = "GMSHInput";` | specify that the input module is a GMSHInputModule |
| `Input.input.file = "$(CASE_NAME).geo";` | set the geometry file path using the case name variable with .geo extension |
| **Node Group Definition** | |
| `Input.groupInput.type = "GroupInput";` | specify that the groupInput module is a GroupInputModule |
| `Input.groupInput.nodeGroups = [ "fixed", "free" ];` | define two node groups: "fixed" and "free" |
| **Fixed Group Criteria** | |
| `Input.groupInput.fixed.xtype = "min";` | nodes in "fixed" group have minimum x-coordinate values |
| `Input.groupInput.fixed.ytype = "min";` | nodes in "fixed" group have minimum y-coordinate values |
| `Input.groupInput.fixed.ztype = "min";` | nodes in "fixed" group have minimum z-coordinate values |
| **Free Group Criteria** | |
| `Input.groupInput.free.xtype = "max";` | nodes in "free" group have maximum x-coordinate values |
| `Input.groupInput.free.ytype = "max";` | nodes in "free" group have maximum y-coordinate values |
| `Input.groupInput.free.ztype = "max";` | nodes in "free" group have maximum z-coordinate values |

\subsubsection model model.pro
The `model.pro` file defines the physical model and boundary conditions. Here are the line-by-line explanations:

| Command | Explanation |
| ------- | ----------- |
| **Model Structure** | |
| `model.type = "Matrix";` | define the top-level model type as [MatrixModel](https://jive-manual.dynaflow.com/classjive_1_1model_1_1MatrixModel.html) |
| `model.model.type = "Multi";` | specify that the model contains multiple sub-models ([MultiModel](https://jive-manual.dynaflow.com/classjive_1_1model_1_1MultiModel.html)) |
| `model.model.models = [ "rodMesh", "fixed", "force", "disp" ];` | define the four sub-models: rod mesh, fixed boundaries, forces, and displacements |
| **Rod Mesh Configuration** | |
| `model.model.rodMesh.type = "Lattice";` | specify the rod mesh as a LatticeModel (for beam/rod networks) |
| `model.model.rodMesh.prefix = "beam_";` | set the prefix for beam element names to "beam_" |
| `model.model.rodMesh.child.type = "specialCosseratRod";` | define individual rod elements as SpecialCosseratRodModel |
| `model.model.rodMesh.child.dofNamesTrans = ["dx", "dy", "dz"];` | define translational degrees of freedom: x, y, z displacements |
| `model.model.rodMesh.child.dofNamesRot = ["rx", "ry", "rz"];` | define rotational degrees of freedom: rotations about x, y, z axes |
| `model.model.rodMesh.child += params.rod_details;` | inherit additional rod properties from the params.rod_details defined in main file |
| **Fixed Boundary Conditions** | |
| `model.model.fixed.type = "Dirichlet";` | specify the fixed boundary conditions to use the DirichletModel |
| `model.model.fixed.maxDisp = 0.;` | set maximum displacement limit to 0 (fully constrained) |
| `model.model.fixed.dispIncr = 0.;` | set displacement increment to 0 (no movement allowed) |
| `model.model.fixed.nodeGroups = [ "fixed", "fixed", "fixed" ];` | apply constraints to "fixed" node group for all three translational DOFs |
| `model.model.fixed.dofs = model.model.rodMesh.child.dofNamesTrans;` | constrain all translational degrees of freedom (dx, dy, dz) |
| `model.model.fixed.factors = [ 0., 0., 0. ];` | set constraint factors to 0 for all translational DOFs (no movement) |
| `model.model.fixed.nodeGroups += [ "fixed", "fixed", "fixed" ];` | extend constraints to rotational DOFs for the same node group |
| `model.model.fixed.dofs += model.model.rodMesh.child.dofNamesRot;` | add rotational degrees of freedom (rx, ry, rz) to constraints |
| `model.model.fixed.factors += [ 0., 0., 0. ];` | set constraint factors to 0 for all rotational DOFs (no rotation) |
| **Force Boundary Conditions** | |
| `model.model.force.type = "Neumann";` | specify the force boundary conditions to use the NeumannModel |
| `model.model.force.initLoad = 0.;` | set initial load value to 0 |
| `model.model.force.loadIncr = 0.;` | set load increment to 0 (no force applied by default) |
| `model.model.force.nodeGroups = [ "fixed" ];` | apply force boundary condition to "fixed" node group |
| `model.model.force.factors = [ 0. ];` | set force scaling factor to 0 (no force applied) |
| `model.model.force.dofs = [ "dx" ];` | specify that force is applied in x-direction |
| **Displacement Boundary Conditions** | |
| `model.model.disp.type = "Dirichlet";` | specify the prescribed displacements to use the DirichletModel |
| `model.model.disp.initDisp = 0.;` | set initial displacement to 0 |
| `model.model.disp.dispIncr = 0.;` | set displacement increment to 0 (no prescribed displacement by default) |
| `model.model.disp.nodeGroups = [ "fixed" ];` | apply displacement boundary condition to "fixed" node group |
| `model.model.disp.factors = [ 0. ];` | set displacement scaling factor to 0 (no prescribed displacement) |
| `model.model.disp.dofs = [ "dx" ];` | specify that displacement is prescribed in x-direction |

\subsubsection output output.pro
The `output.pro` file defines the output modules for data collection and logging. Here are the line-by-line explanations:

| Command | Explanation |
| ------- | ----------- |
| **Output Module Configuration** | |
| `Output.modules = [ "loadextent", "disp" ];` | define two output modules: "loadextent" for force tracking and "disp" for displacement data |
| **Load Extent Output** | |
| `Output.loadextent.type = "GroupOutput";` | use the GroupOutputModule to extract data from the output groups |
| `Output.loadextent.nodeGroups = [ "fixed", "free" ];` | track data for both "fixed" and "free" node groups |
| `Output.loadextent.dofs = model.model.rodMesh.child.dofNamesTrans;` | include translational degrees of freedom (dx, dy, dz) in output |
| `Output.loadextent.dofs += model.model.rodMesh.child.dofNamesRot;` | add rotational degrees of freedom (rx, ry, rz) to output |
| `Output.loadextent.dimensions = model.model.rodMesh.child.dofNamesTrans;` | specify spatial dimensions for output based on translational DOFs |
| **Displacement CSV Output** | |
| `Output.disp.type = "CSVOutput";` | use the CSVOutputModule to output the collected data |
| `Output.disp.file = "$(CASE_NAME)/disp.gz";` | set output file path with case name and gzip compression |
| `Output.disp.vectors = [ "state = disp" ];` | output displacement state vectors |
| **Logging Configuration** | |
| `log.pattern = "*";` | set logging pattern to capture all log messages (wildcard pattern) |
| `log.file = "-";` | direct log output to standard output (console) using "-" |
