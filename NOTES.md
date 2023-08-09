# :hammer_and_pick: TO DO 
1. Models
    - Explicit Solver
      - keep $M$ constant, only update $\Theta$
      - implement 'lumped' algorithm with direct 3x3 inverts
1. Modules
    - Tangent Output Module
      - Get Matrix Condenstation to work
1. Environment
    - profile the code to find areas which could be improved (e.g. valgrind --callgrind or so)
    - make all plot outputs A4/A3
1. Tests
    - include more reference data
 
# :arrows_counterclockwise: Workflow for eqv-design
:bangbang: make sure repo on cluster is updated :bangbang:
1. `jive scripts/running/eqv-design.py lin_init prep_runs prep_higher_runs lin_change_master_prep`
1. `./scripts/cluster/masterDesign.sh`
1. `./scripts/cluster/syncBack.sh`
1. `jive scripts/running/eqv-design.py lin_change_master_plot`
1. `./scripts/cluster/masterFailed.sh`
1. `jive scripts/running/eqv-design.py work_density_plot work_density_higher_plot stress_plot comp_speeds_locale_plot`
1. `jive scripts/running/eqv-design.py work_density_export prep_latex_data`

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

# For later....
- Nonlinear Eigenmode Decomposition
  - start with lin eigenmodes
  - increase force to get nonlin deformation
  - map boundary forces to deformation using ML
