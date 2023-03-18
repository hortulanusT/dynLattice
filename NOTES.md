# :hammer_and_pick: TO DO 
## Investigations
- Include Contact
- Include Plasticity
- Elastic, Dynamic Homogenization
## Code
1. Models
    - Rod Model
      - Extract material for the stress -> strain relationship
      - Create MaterialFactory
      - Why are dynamics only working with linear elements?
    - Explicit Solver
      - keep M constant
      - implement 'lumped' algorithm with direct 3x3 inverts
1. Modules
    - Tangent Output Module
      - Get Matrix Condenstation to work
1. Environment
    - profile the code to find areas which could be improved (e.g. valgrind --callgrind or so)
    - make all plot outputs A4
    - cleanup `.pro` file composition
1. Tests
    - include more reference data
 
# :arrows_counterclockwise: Workflow for eqv-design
1. `jive scripts/running/eqv-design.py lin_init`
2. make sure repo on cluster(s) is updated
## RUNS
3. `jive scripts/running/eqv-design.py prep_runs`
4. `./scripts/cluster/masterDesign.sh`
5. make sure all runs went by nicely
6. `./scripts/cluster/syncBack.sh`
7. `jive scripts/running/eqv-design.py work_density_plot work_density_export`
## LIN CHANGE
3. `jive scripts/running/eqv-design.py lin_change_master_prep`
4. `./scripts/cluster/masterChange.sh`
5. make sure all runs went by nicely
6. `./scripts/cluster/syncBack.sh`
7. `jive scripts/running/eqv-design.py lin_change_master_plot prep_latex_data`

# :heavy_check_mark: Information
## cluster syncing
``` bash
rsync -vax $TARGET $DESTINATION
```
## debugging
- call (dbg) call dprint(&...) to get any matrix/vector/... printed live
- when compiling OPT, it sets the flag `NDEBUG`
## .pro files
- `$(CASE_NAME)` also works with environment variables (and other settings)
- `include "xyz.pro";` works to include other .pro files
## others
- shape function node numbering in `$JIVEDIR/packages/geom/doc/images`
- `StdShape` for local coordinates and `Shape` for global coordinates
- `./jive xyz.pro |c++filt` for nicer stack traces
- `JEM_PRECHECK` gets executed always, `JEM_ASSERT` only in non-optimized mode

# :hourglass_flowing_sand: Ideas for the future
- Modal Reduction for Homogenization
  - dynamic homogenization for micromorphic continuum
- Nonlocal Homogenization for dynamics
