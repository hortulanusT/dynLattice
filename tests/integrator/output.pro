// OUTPUTS
Output.modules = [ "loadextent", "sampling" ];

Output.loadextent.type = "GroupOutput";
Output.loadextent.nodeGroups = [ "fixed", "free" ];
Output.loadextent.dofs = model.model.rodMesh.child.dofNamesTrans;
Output.loadextent.dofs += model.model.rodMesh.child.dofNamesRot;
Output.loadextent.dimensions = model.model.rodMesh.child.dofNamesTrans;

Output.sampling.type = "Sample";
Output.sampling.separator = ",";
Output.sampling.precision = 15;   // all digits: the differences of the convergence tests go down to 1e-12
Output.sampling.file = "$(CASE_NAME)/data.csv";
Output.sampling.header = "t,dx,dy,dz,rx,ry,rz,E_pot,E_kin";
Output.sampling.dataSets = [ "t", "free.disp.dx", "free.disp.dy", "free.disp.dz", "free.disp.rx", "free.disp.ry", "free.disp.rz", "potentialEnergy", "kineticEnergy" ];

// LOGGING
log.pattern = "*.warn";
log.file = "-";

// ParaView frames, off unless make is called with PARAVIEW=1 (tests/testing.mk)
Output.modules += [ "paraview" ];
Output.paraview.type = "None";
