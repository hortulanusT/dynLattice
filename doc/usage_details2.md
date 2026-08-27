\page prop2 Properties for Example 2

| Command | Explanation |
| ------- | ----------- |
| **Program Control** | |
| `control.runWhile="ymax.disp.dy/SIZE.Y>-0.5 && ymax.velo.dy <= 0.";` | terminate once the impactor ("ymax") has moved down by more than half the lattice's height (50% nominal compaction) or has stopped moving downward (i.e. it has started to rebound) |
| **Solver Configuration** | |
| `Solver.modules = [ "integrator" ];` | defines the modules that make up the solver; again only one, called `integrator` |
| `Solver.integrator.type = "MilneDevice";` | use the same adaptive MilneDeviceModule integrator as in \ref prop1 |
| `Solver.integrator.deltaTime = 2e-9;` | start with a much smaller time step \f$2\times10^{-9}\f$s than Test 1, since the impact loading and thin rods involve much higher characteristic frequencies |
| `Solver.integrator.dofs_SO3 = ["rx", "ry", "rz"];` | tell the integrator which degrees of freedom are rotations living on SO(3), so it can update/interpolate them consistently |
| `Solver.integrator.lengthScale = 3.7500000000000003e-05;` | a characteristic length (here the rod's half cross-section side length) used internally to non-dimensionalize the adaptive error estimate |
| `Solver.integrator.precision = .3e-4;` | set the target relative error used by the adaptive step-size control |
| **File Includes** | |
| `include "input.pro";` | include the \ref input2 file defining input modules, the parameterized geometry and periodic node groups |
| `include "model.pro";` | include the \ref model2 file defining the lattice, contact, impact and support models |
| `include "output.pro";` | include the \ref output2 file defining output modules and data collection |

\section input2 input.pro
The `input.pro` file defines the input modules that read the parameterized geometry and set up the periodic node groups.

| Command | Explanation |
| ------- | ----------- |
| **Input Module Configuration** | |
| `Input.modules = [ "input", "nodeInput" ];` | define the input modules: "input" for geometry reading and "nodeInput" for node group creation |
| **Geometry Input Settings** | |
| `Input.input.type = "GMSHInput";` | specify that the input module is a GMSHInputModule |
| `Input.input.verbose = false;` | suppress some outputs |
| `Input.input.store_tangents = true;` | additionally store the tangent direction of each rod segment at the nodes, needed to construct the initial (curved) rod kinematics |
| `Input.input.onelab.Rotation = "180";` | set the GMSH `onelab` parameter `Rotation` (rotates the whole structure by 180°) |
| `Input.input.onelab.BaseWidth="0.0083";` | set the honeycomb's base bar width \f$b=.3\f$mm |
| `Input.input.onelab.LengthRatio="0.5883253012048192";` | set the ratio between the two bar lengths of the re-entrant unit cell |
| `Input.input.onelab.Angle="55";` | set the re-entrant angle between the two bars to 55° |
| `Input.input.onelab.Thickness="0.00075";` | set the (unused by the .geo file directly, but referenced for context) wall thickness to 0.75mm |
| `Input.input.onelab.RepX="6.0";` | tile the unit cell 6 times in the horizontal (x) direction |
| `Input.input.onelab.RepY="4";` | tile the unit cell 4 times in the vertical (y) direction |
| `Input.input.onelab.Scale="1.0";` | apply no additional overall scaling |
| `Input.input.file="tests/docs/re-entrant.geo";` | set the geometry file path |
| **Node Group Definition** | |
| `Input.nodeInput.type = "PBCGroupInput";` | use PBCGroupInputModule instead of the plain GroupInputModule from \ref prop1, which additionally derives the boundary node groups (`xmin`, `xmax`, `ymin`, `ymax`, and the corner nodes) needed to apply periodic boundary conditions |
| `Input.nodeInput.groupSettings.restrictToGroup = "points";` | only consider the original GMSH geometry points (not intermediate/duplicated nodes) when constructing these boundary groups |
| `Input.nodeInput.ymax.xbounds=["ORIGIN.X+SIZE.X/2-SIZE.X/6","ORIGIN.X+SIZE.X/2+SIZE.X/6"];` | narrow the default "ymax" (top edge) group down to its central third, matching the finite width of the physical impactor |
| `Input.nodeInput.ymax.restrictToGroup = "beams";` | loosen the restriction for the narrowed "ymax" group to all nodes that belong to the lattice ("beams") |

\section model2 model.pro
The `model.pro` file defines the lattice, its material and contact behavior, and the impact/support boundary conditions.

| Command | Explanation |
| ------- | ----------- |
| **Model Structure** | |
| `model.type = "Matrix";` | define the top-level model type as [MatrixModel](https://jive-manual.dynaflow.com/classjive_1_1model_1_1MatrixModel.html) |
| `model.model.type = "Multi";` | specify that the model contains multiple sub-models ([MultiModel](https://jive-manual.dynaflow.com/classjive_1_1model_1_1MultiModel.html)) |
| `model.model.models = [ "lattice", "load", "fixed" ];` | define three sub-models: the lattice itself, the loading/support conditions, and additional (planar) constraints |
| **Lattice / Material Configuration** | |
| `model.model.lattice.type = "Lattice";` | as in \ref prop1, use a LatticeModel to assemble many individual rods into one model |
| `model.model.lattice.prefix = "beam_";` | prefix used for the individual rod element names |
| `model.model.lattice.child.type = 'specialCosseratRod";` | each rod in the lattice is again a SpecialCosseratRodModel |
| `model.model.lattice.child.dofNamesTrans = ["dx", "dy", "dz"];` | translational degrees of freedom |
| `model.model.lattice.child.dofNamesRot = ["rx", "ry", "rz"];` | rotational degrees of freedom |
| `model.model.lattice.child.material_ey = [ 0., 0., 1. ];` | set the rod's local cross-section "up" direction, needed to orient the (non-circular) rectangular cross-section |
| `model.model.lattice.child.material.type="ElastoPlasticRod";` | unlike Test 1's purely elastic rod, use the elasto-plastic rod material to capture plastic hinge formation |
| `model.model.lattice.child.material.cross_section = "rectangle";` | rectangular cross-section |
| `model.model.lattice.child.material.side_length=[0.00075, 0.00075];` | 0.75mm × 0.75mm cross-section |
| `model.model.lattice.child.material.young = 210e9;` | steel Young's modulus, \f$210\f$GPa |
| `model.model.lattice.child.material.poisson_ratio=0.265;` | steel Poisson's ratio |
| `model.model.lattice.child.material.density=8000.;` | steel density, \f$8000\f$ kg/m³ |
| `model.model.lattice.child.material.yieldCond = "...";` | define the (multi-axial) yield surface, fitted so its yield strength matches a macroscopic 280 MPa yield strength scaled from a microscopic J2 value of 450 MPa and the rod's cross-section |
| `model.model.lattice.child.material.kinematicTensor = [...];` | define the 6×6 kinematic-hardening modulus tensor, scaled from a reference hardening modulus of 20 GPa down to the 1.75 GPa hardening modulus fitted to the Johnson-Cook material model used experimentally |
| `model.model.lattice.child.material.edge_factor = 1.25;` | increase the rod's effective bending stiffness by 25% to account for the stiffening effect of the physical joints connecting the rods |
| **Self-Contact** | |
| `model.model.lattice.contact.type = "RodContact";` | enable rod-on-rod self-contact, since the re-entrant cells touch each other once sufficiently compacted |
| `model.model.lattice.contact.radius=0.000375;` | contact radius, matching the rod's half cross-section side length |
| `model.model.lattice.contact.penaltySTS=56250.0;` | segment-to-segment contact penalty stiffness |
| `model.model.lattice.contact.penaltyNTS=562500.0;` | node-to-segment contact penalty stiffness |
| `model.model.lattice.jointContact.type = "JointContact";` | additionally enable contact between the rod joints themselves |
| `model.model.lattice.jointContact.radius=0.000375;` | joint contact radius |
| `model.model.lattice.jointContact.penalty=5625000.0;` | joint contact penalty stiffness |
| **Loading and Support** | |
| `model.model.load.type = "Multi";` | bundle the impact and support related sub-models together |
| `model.model.load.models=["fix_dynamic","impact","init","top_bottom_mass"];` | define the four loading sub-models |
| `model.model.load.fix_dynamic.type = "Dirichlet";` | guide the top and bottom edges so they can only move vertically |
| `model.model.load.fix_dynamic.nodeGroups = [ "ymin", "ymax", "ymax" ];` | apply this to the "ymin" and "ymax" edges |
| `model.model.load.fix_dynamic.dofs = [ "rz", "dx", "rz" ];` | constrain the bottom edge's rotation and the top edge's horizontal displacement and rotation |
| `model.model.load.fix_dynamic.factors = [ 0., 0., 0. ];` | all three constraints are homogeneous (zero) |
| `model.model.load.init.type = "InitLoad";` | InitLoadModel, used to set the impactor's initial velocity |
| `model.model.load.init.veloGroups = "ymax";` | apply the initial velocity to the top ("ymax", impactor) group |
| `model.model.load.init.veloDofs = "dy";` | in the vertical direction |
| `model.model.load.init.veloVals="-1*70.0";` | initial impact velocity of \f$70\f$m/s, directed downward |
| `model.model.load.impact.type = "ImpactBC";` | ImpactModel, used to give the impactor a finite effective mass so it decelerates realistically under the lattice's resistance |
| `model.model.load.impact.nodeGroups = "ymax";` | applies to the impactor group |
| `model.model.load.impact.dofs = "dy";` | in the vertical direction |
| `model.model.load.impact.weights = "1.2 / 25 * 0.75";` | (fraction of the) impactor mass |
| `model.model.load.top_bottom_mass.type = "Mass";` | add an additional lumped mass representing the striker plate's inertia |
| `model.model.load.top_bottom_mass.nodeGroups = ["ymax"];` | attached to the impactor group |
| `model.model.load.top_bottom_mass.totalMass = ["0.1308/25*0.75"];` | total mass distributed over that group |
| `model.model.load.top_bottom_mass.dofs = ["dx", "dy", "dz"];` | mass acts equally in all three translational directions |
| **Additional Planar Constraints** | |
| `model.model.fixed.type = "Dirichlet";` | keep the whole lattice planar |
| `model.model.fixed.nodeGroups = [ "all", "all", "all" ];` | applies to every node |
| `model.model.fixed.dofs = ["dz", "rx", "ry"];` | constrain out-of-plane translation and the two in-plane bending rotations |
| `model.model.fixed.dispIncr = 0.;` | homogeneous (zero) constraint |
| **Compliant Support Plate** | |
| `model.model.models += "plate_springs";` | add a further sub-model representing the support plate |
| `model.model.plate_springs.type = 'springMass";` | SpringMassModel, used to model the compliant support as a small elastic rod rather than a rigid boundary |
| `model.model.plate_springs.boundary = "ymin";` | attach the spring to the bottom ("ymin") edge |
| `model.model.plate_springs.springs = [ "plate1"];` | define a single named spring, "plate1" |
| `model.model.plate_springs.plate1.extentVector = [ 0., -0.005, 0.];` | the spring rod extends 5mm further downward from the lattice's base |
| `model.model.plate_springs.plate1.nElem = 4;` | discretize the spring rod into 4 elements |
| `model.model.plate_springs.plate1.pElem = 1;` | use linear (order 1) elements for the spring rod |
| `model.model.plate_springs.plate1.type	= 'specialCosseratRod";` | the spring itself is again a SpecialCosseratRodModel |
| `model.model.plate_springs.plate1.dofNamesTrans	= [ "dx", "dy", "dz" ];` | translational degrees of freedom |
| `model.model.plate_springs.plate1.dofNamesRot	= [ "rx", "ry", "rz" ];` | rotational degrees of freedom |
| `model.model.plate_springs.plate1.material_ey = [0., 0., 1.];` | cross-section orientation |
| `model.model.plate_springs.plate1.material = "ElasticRod" { ... };` | a purely elastic rod representing a steel plate, \f$0.18\f$m wide and 0.75mm thick, used to give the support a realistic, finite stiffness rather than being perfectly rigid |

\section output2 output.pro
The `output.pro` file defines the output modules used to record the impact response and produce the ParaView visualization.

| Command | Explanation |
| ------- | ----------- |
| **Logging Configuration** | |
| `log.pattern = "*.info";` | only log "info"-level messages |
| `log.file="$(CASE_NAME)/run.log";` | write the log to a file next to the case's other outputs |
| **Output Module Configuration** | |
| `Output.modules = ["groupOut", 'sampling", "paraview"];` | define three output modules: group averaging, CSV sampling, and ParaView visualization |
| `Output.groupOut.type = "GroupOutput";` | GroupOutputModule, used to compute averaged group quantities used below |
| `Output.groupOut.nodeGroups = ["ymax", "ymin", "plate1_bot"];` | track the impactor, the base of the lattice, and the bottom of the support spring |
| `Output.groupOut.dofs = "dy";` | in the vertical direction |
| **CSV Output Configuration** | |
| `Output.sampling.type = 'sample";` | SampleModule, used to write a CSV time history |
| `Output.sampling.file = "$(CASE_NAME)/data.csv";` | output file path |
| `Output.sampling.header="...";` | descriptive column header for the CSV file: time, impactor velocity/displacement/force, base force, nominal strain, top/bottom pressure, potential/dissipated/kinetic energy, mass, support-plate force |
| `Output.sampling.dataSets=[...];` | the corresponding data expressions: `t`, `ymax.velo.dy`, `ymax.disp.dy`, `ymax.resp.dy`, `ymin.resp.dy`, the nominal compaction `ymax.disp.dy/SIZE.Y`, the top/bottom pressures (force divided by the lattice's cross-sectional area), `potentialEnergy`, `dissipatedEnergy`, `kineticEnergy`, `mass`, and the support-plate force `plate1_bot.resp.dy` |
| `Output.sampling.separator = ",";` | comma-separated output |
| **ParaView Visualization Output** | |
| `Output.paraview.type = "ParaView";` | ParaViewModule visualization output, as in \ref prop1 |
| `Output.paraview.groups = [ "beams","plate1" ];` | output both the lattice ("beams") and the support spring ("plate1") |
| `Output.paraview.beams.disps = ["dx", "dy", "dz"];` | translational displacement fields |
| `Output.paraview.beams.otherDofs = ["rx", "ry", "rz"];` | rotational degree of freedom fields |
| `Output.paraview.beams.el_data = [ "mat_strain", "mat_stress", "plast_strain" ];` | element data: material strain, stress, and (unlike Test 1) accumulated plastic strain |
| `Output.paraview.beams.node_data = ["fres", "F_contact", "kineticEnergy", "mass"];` | node data: residual force, contact force, kinetic energy and mass |
| `Output.paraview.beams.shape = "Line2";` | visualize the (linear) rod elements as 2-node line segments |
| `Output.paraview.plate1 = Output.paraview.beams;` | re-use the same settings for the support spring group ... |
| `Output.paraview.plate1.el_data = [ "mat_strain", "mat_stress" ];` | ... except for element data, since the spring is purely elastic (no plastic strain) |
| `Output.paraview.plate1.node_data = ["fres"];` | ... and node data, since it carries no contact or lumped mass |
| `Output.paraview.output_format = "$(CASE_NAME)/vis%i";` | output file naming pattern |
| `Output.paraview.sampleWhen = "(i-1)%100<1";` | write a ParaView frame every 100 time steps |
