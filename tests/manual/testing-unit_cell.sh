#!/usr/bin/env -S apptainer -s exec -e /home/tgartner/jive.sif bash

make

YIELD="abs(dx/(0.05 *  280/450*  820-h_dx))^2.68 + abs(dy/(0.05 *  280/450*  820-h_dy))^2.68 + abs(dz/(0.05 *  280/450* 1870-h_dz))^1.75 + abs(rx/(0.05 *  280/450* 0.94-h_rx))^1.93 + abs(ry/(0.05 *  280/450* 0.94-h_ry))^1.93 + abs(rz/(0.05 *  280/450* 0.75-h_rz))^1.70 - 1";

rm tests/manual/testing-unit_cell.pdf -f
rm studies/output/tno-design/ARH/evo/*.csv -f
rm studies/output/tno-design/ARH/evo/*.log -f

#  -p "model.model.lattice.child.material.verbosity=2"

./bin/nonlinRod studies/output/tno-design/ARH/evo/uniaxial-1x1.pro \
  -p "log.pattern='*'" -p "control.runWhile='ymax.disp.dy/SIZE.Y>-0.005'" \
  -p "Output.tangent.sampleWhen='(i-1)%100<1'" -p "Output.sampling.sampling.sampleWhen='(i-1)%100<1'"  \
  -p "model.model.lattice.child.material.yieldCond='$YIELD'" > /dev/null &
# ./bin/nonlinRod studies/output/tno-design/ARH/evo/uniaxial-1x1_contact.pro \
#   -p "log.pattern='*'" -p "control.runWhile='ymax.disp.dy/SIZE.Y>-0.005'" \
#   -p "Output.tangent.sampleWhen='(i-1)%100<1'" -p "Output.sampling.sampling.sampleWhen='(i-1)%100<1'"> /dev/null &
./bin/nonlinRod studies/output/tno-design/ARH/evo/uniaxial-1x1_plastic.pro \
  -p "log.pattern='*'" -p "control.runWhile='ymax.disp.dy/SIZE.Y>-0.005'" \
  -p "Output.tangent.sampleWhen='(i-1)%100<1'" -p "Output.sampling.sampling.sampleWhen='(i-1)%100<1'"  \
  -p "model.model.lattice.child.material.yieldCond='$YIELD'" > /dev/null &
# ./bin/nonlinRod studies/output/tno-design/ARH/evo/uniaxial-1x1_plastic_contact.pro \
#   -p "log.pattern='*'" -p "control.runWhile='ymax.disp.dy/SIZE.Y>-0.005'" \
#   -p "Output.tangent.sampleWhen='(i-1)%100<1'" -p "Output.sampling.sampling.sampleWhen='(i-1)%100<1'" > /dev/null & 
wait

./tests/manual/testing-unit_cell.py
