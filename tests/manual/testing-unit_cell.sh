#!/usr/bin/env -S apptainer -s exec -e /home/tgartner/jive.sif bash

# CLEANUP
make
rm -fr tests/manual/testing-unit_cell/
rm -f  tests/manual/testing-unit_cell*.log
rm -f  tests/manual/testing-unit_cell*.csv
rm -f  tests/manual/testing-unit_cell*.pdf

echo "Simulations underway..."

# ELASTIC
bin/nonlinRod -p "params.scale=1.0" -p "params.name='elastic'"        tests/manual/testing-unit_cell.pro -p "model.model.lattice.child.material.type='ElasticRod'" > /dev/null &
bin/nonlinRod -p "params.scale=0.1" -p "params.name='elastic_scaled'" tests/manual/testing-unit_cell.pro -p "model.model.lattice.child.material.type='ElasticRod'" > /dev/null &

# IDEAL PLASTIC
bin/nonlinRod -p "params.scale=1.0" -p "params.name='noHard'"         tests/manual/testing-unit_cell.pro -p "Output.paraview.beams.el_data+='plast_strain'" -p 'model.model.lattice.child.material.yieldCond="abs(dx/700/$(params.scale)^2)^2.04+abs(dy/700/$(params.scale)^2)^2.04+abs(dz/1470/$(params.scale)^2)^1.76+abs(rx/0.62/$(params.scale)^3)^2.09+abs(ry/0.62/$(params.scale)^3)^2.09+abs(rz/0.56/$(params.scale)^3)^1.73-1"' > /dev/null &
bin/nonlinRod -p "params.scale=0.1" -p "params.name='noHard_scaled'"  tests/manual/testing-unit_cell.pro -p "Output.paraview.beams.el_data+='plast_strain'" -p 'model.model.lattice.child.material.yieldCond="abs(dx/700/$(params.scale)^2)^2.04+abs(dy/700/$(params.scale)^2)^2.04+abs(dz/1470/$(params.scale)^2)^1.76+abs(rx/0.62/$(params.scale)^3)^2.09+abs(ry/0.62/$(params.scale)^3)^2.09+abs(rz/0.56/$(params.scale)^3)^1.73-1"' > /dev/null &

# ISOTROPIC HARDENING
bin/nonlinRod -p "params.scale=1.0" -p "params.name='isoHard'"        tests/manual/testing-unit_cell.pro -p "Output.paraview.beams.el_data+='plast_strain'" -p 'model.model.lattice.child.material.yieldCond="abs(dx/700/$(params.scale)^2)^2.04+abs(dy/700/$(params.scale)^2)^2.04+abs(dz/1470/$(params.scale)^2)^1.76+abs(rx/0.62/$(params.scale)^3)^2.09+abs(ry/0.62/$(params.scale)^3)^2.09+abs(rz/0.56/$(params.scale)^3)^1.73-a"' -p 'model.model.lattice.child.material.hardeningCoefficient=10.' > /dev/null &
bin/nonlinRod -p "params.scale=0.1" -p "params.name='isoHard_scaled'" tests/manual/testing-unit_cell.pro -p "Output.paraview.beams.el_data+='plast_strain'" -p 'model.model.lattice.child.material.yieldCond="abs(dx/700/$(params.scale)^2)^2.04+abs(dy/700/$(params.scale)^2)^2.04+abs(dz/1470/$(params.scale)^2)^1.76+abs(rx/0.62/$(params.scale)^3)^2.09+abs(ry/0.62/$(params.scale)^3)^2.09+abs(rz/0.56/$(params.scale)^3)^1.73-a"' -p 'model.model.lattice.child.material.hardeningCoefficient=10.' > /dev/null &

# KINEMATIC HARDENING
bin/nonlinRod -p "params.scale=1.0" -p "params.name='kinHard'"        tests/manual/testing-unit_cell.pro -p "Output.paraview.beams.el_data+='plast_strain'" -p 'model.model.lattice.child.material.yieldCond="abs(dx/(700-b_dx)/$(params.scale)^2)^2.04+abs(dy/(700-b_dy)/$(params.scale)^2)^2.04+abs(dz/(1470-b_dz)/$(params.scale)^2)^1.76+abs(rx/(0.62-b_rx)/$(params.scale)^3)^2.09+abs(ry/(0.62-b_ry)/$(params.scale)^3)^2.09+abs(rz/(0.56-b_rz)/$(params.scale)^3)^1.73-1"' -p "model.model.lattice.child.material.hardeningTensor=[19014e+0,17547e+0,33121e+0,16069e-3,16743e-3,15552e-3,17547e+0,19014e+0,33121e+0,16743e-3,16069e-3,15556e-3,33121e+0,33121e+0,56864e+0,24578e-3,24578e-3,26757e-3,16069e-3,16743e-3,24578e-3,15015e-6,15009e-6,12715e-6,16743e-3,16069e-3,24578e-3,15009e-6,16015e-6,12715e-6,15552e-3,15556e-3,26757e-3,12715e-6,12715e-6,10434e-6]" > /dev/null &
bin/nonlinRod -p "params.scale=0.1" -p "params.name='kinHard_scaled'" tests/manual/testing-unit_cell.pro -p "Output.paraview.beams.el_data+='plast_strain'" -p 'model.model.lattice.child.material.yieldCond="abs(dx/(700-b_dx)/$(params.scale)^2)^2.04+abs(dy/(700-b_dy)/$(params.scale)^2)^2.04+abs(dz/(1470-b_dz)/$(params.scale)^2)^1.76+abs(rx/(0.62-b_rx)/$(params.scale)^3)^2.09+abs(ry/(0.62-b_ry)/$(params.scale)^3)^2.09+abs(rz/(0.56-b_rz)/$(params.scale)^3)^1.73-1"' -p "model.model.lattice.child.material.hardeningTensor=[19014e+0,17547e+0,33121e+0,16069e-3,16743e-3,15552e-3,17547e+0,19014e+0,33121e+0,16743e-3,16069e-3,15556e-3,33121e+0,33121e+0,56864e+0,24578e-3,24578e-3,26757e-3,16069e-3,16743e-3,24578e-3,15015e-6,15009e-6,12715e-6,16743e-3,16069e-3,24578e-3,15009e-6,16015e-6,12715e-6,15552e-3,15556e-3,26757e-3,12715e-6,12715e-6,10434e-6]" > /dev/null &

wait
echo "Simulations done!"

# PLOTTING
tests/manual/testing-unit_cell.py
