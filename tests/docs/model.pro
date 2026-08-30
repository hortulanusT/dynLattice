// ACTUAL_MODEL
model.type = "Matrix";
model.model.type = "Multi";
model.model.models = [ "lattice", "load", "fixed" ];

model.model.lattice.type = "Lattice";
model.model.lattice.prefix = "beam_";
model.model.lattice.child.type = "specialCosseratRod";
model.model.lattice.child.dofNamesTrans = ["dx", "dy", "dz"];
model.model.lattice.child.dofNamesRot = ["rx", "ry", "rz"];
model.model.lattice.child.material_ey = [ 0., 0., 1. ];
model.model.lattice.child.material.type="ElastoPlasticRod";
model.model.lattice.child.material.cross_section = "rectangle";
model.model.lattice.child.material.side_length=[0.00075, 0.00075];
model.model.lattice.child.material.young = 210e9;
model.model.lattice.child.material.poisson_ratio=0.265;
model.model.lattice.child.material.density=8000.;
// Adapted to the Poisson's ratio and density of the steel used in the experiments
// Yield surface reported for microscopic J2-yield strength of 450 MPa, we need to scale it to 280 MPa (additional Factor is a fit)
// the numerical values are scaled down compared to the Herrnböck Paper, as the original model was for a side length of 2mm
model.model.lattice.child.material.yieldCond  = "  abs(dx/(280/450 *     205*0.75^2-h_dx))^2.68 ";
model.model.lattice.child.material.yieldCond += "+ abs(dy/(280/450 *     205*0.75^2-h_dy))^2.68 "; 
model.model.lattice.child.material.yieldCond += "+ abs(dz/(280/450 *   467.5*0.75^2-h_dz))^1.75 ";
model.model.lattice.child.material.yieldCond += "+ abs(rx/(280/450 *  0.1175*0.75^3-h_rx))^1.93 ";
model.model.lattice.child.material.yieldCond += "+ abs(ry/(280/450 *  0.1175*0.75^3-h_ry))^1.93 ";
model.model.lattice.child.material.yieldCond += "+ abs(rz/(280/450 * 0.09375*0.75^3-h_rz))^1.70 ";
model.model.lattice.child.material.yieldCond += "- 1";
// Hardening parameters are scaled to the hardening modulus of the Johnson Cook model (1750 MPa)
// versus assumed 20000 MPa in the original model (additional factor is a fit)
model.model.lattice.child.material.kinematicTensor  = ["1750/20000 *19014e+0*0.75^2","1750/20000 * 17547e+0*0.75^2","1750/20000 * 33121e+0*0.75^2","1750/20000 * 16069e-3*0.75^3","1750/20000 * 16743e-3*0.75^3","1750/20000 * 15552e-3*0.75^3"];
model.model.lattice.child.material.kinematicTensor += ["1750/20000 *17547e+0*0.75^2","1750/20000 * 19014e+0*0.75^2","1750/20000 * 33121e+0*0.75^2","1750/20000 * 16743e-3*0.75^3","1750/20000 * 16069e-3*0.75^3","1750/20000 * 15556e-3*0.75^3"];
model.model.lattice.child.material.kinematicTensor += ["1750/20000 *33121e+0*0.75^2","1750/20000 * 33121e+0*0.75^2","1750/20000 * 56864e+0*0.75^2","1750/20000 * 24578e-3*0.75^3","1750/20000 * 24578e-3*0.75^3","1750/20000 * 26757e-3*0.75^3"];
model.model.lattice.child.material.kinematicTensor += ["1750/20000 *16069e-3*0.75^3","1750/20000 * 16743e-3*0.75^3","1750/20000 * 24578e-3*0.75^3","1750/20000 * 15015e-6*0.75^4","1750/20000 * 15009e-6*0.75^4","1750/20000 * 12715e-6*0.75^4"];
model.model.lattice.child.material.kinematicTensor += ["1750/20000 *16743e-3*0.75^3","1750/20000 * 16069e-3*0.75^3","1750/20000 * 24578e-3*0.75^3","1750/20000 * 15009e-6*0.75^4","1750/20000 * 16015e-6*0.75^4","1750/20000 * 12715e-6*0.75^4"];
model.model.lattice.child.material.kinematicTensor += ["1750/20000 *15552e-3*0.75^3","1750/20000 * 15556e-3*0.75^3","1750/20000 * 26757e-3*0.75^3","1750/20000 * 12715e-6*0.75^4","1750/20000 * 12715e-6*0.75^4","1750/20000 * 10434e-6*0.75^4"];
// The edge factor is increased to 1.25 to account for the joint stiffening around the joints of the lattice
model.model.lattice.child.material.edge_factor = 1.25;
model.model.lattice.contact.type = "RodContact";
model.model.lattice.contact.radius=0.000375;
model.model.lattice.contact.penaltySTS=56250.0;
model.model.lattice.contact.penaltyNTS=562500.0;
model.model.lattice.jointContact.type = "JointContact";
model.model.lattice.jointContact.radius=0.0005;
model.model.lattice.jointContact.penalty=5625000.0;

model.model.load.type = "Multi";
model.model.load.models=['fix_dynamic','impact','init',"top_bottom_mass"];

model.model.load.fix_dynamic.type = "Dirichlet";
model.model.load.fix_dynamic.nodeGroups = [ "ymin", "ymax", "ymax" ];
model.model.load.fix_dynamic.dofs = [ "rz", "dx", "rz" ];
model.model.load.fix_dynamic.factors = [ 0., 0., 0. ];

model.model.load.init.type = "InitLoad";
model.model.load.init.veloGroups = "ymax";
model.model.load.init.veloDofs = "dy";
model.model.load.init.veloVals='-1*70.0';

model.model.load.impact.type = "ImpactBC";
model.model.load.impact.nodeGroups = "ymax";
model.model.load.impact.dofs = "dy";
model.model.load.impact.weights = "1.2 / 25 * 0.75";

model.model.load.top_bottom_mass.type = "Mass";
model.model.load.top_bottom_mass.nodeGroups = ["ymax"];
model.model.load.top_bottom_mass.totalMass = ["0.1308/25*0.75"];
model.model.load.top_bottom_mass.dofs = ["dx", "dy", "dz"];

model.model.fixed.type = "Dirichlet";
model.model.fixed.nodeGroups = [ "all", "all", "all" ];
model.model.fixed.dofs = ["dz", "rx", "ry"];
model.model.fixed.dispIncr = 0.;

model.model.models += "plate_springs";
model.model.plate_springs.type = "SpringMass";
model.model.plate_springs.boundary = "ymin";
model.model.plate_springs.springs = [ "plate1"];
model.model.plate_springs.plate1.extentVector = [ 0., -0.01, 0.];
model.model.plate_springs.plate1.nElem = 4;
model.model.plate_springs.plate1.pElem = 1;
model.model.plate_springs.plate1.type	= "specialCosseratRod";
model.model.plate_springs.plate1.dofNamesTrans	= [ "dx", "dy", "dz" ];
model.model.plate_springs.plate1.dofNamesRot	= [ "rx", "ry", "rz" ];
model.model.plate_springs.plate1.material_ey = [0., 0., 1.];
model.model.plate_springs.plate1.material = "ElasticRod"
  {
    young = 210e9;
    poisson_ratio = 0.265;
    density = 8000.;
    cross_section = "rectangle";
    side_length = ["0.18", "0.75*1e-3"];
  };