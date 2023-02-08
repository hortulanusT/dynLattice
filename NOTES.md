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
1. Modules
    - Explicit Module
      - Clean up Explicit Module and children
      - Extract them to seperate Repo
      - set the mass matrix when computing the Gyro Force
    - Tangent Output Module
      - Get Matrix Condenstation to work
    - ParaView Module
      - write PVD in every step
      - nicer numbering for the output files
1. Environment
    - make all plot outputs A4
    - cleanup `.pro` file composition
1. Tests
    - include check with reference data
 
# :arrows_counterclockwise: Workflow for eqv-design
1. `jive ./scripts/running/eqv-design.py lin_init prep_runs`
1. `jive ./scripts/running/eqv-design.py lin_change lin_comp_plot show_designs print_designs`
1. make sure repo on cluster(s) is updated
1. `./scripts/cluster/masterDesignBlue.sh`
1. `./scripts/cluster/syncBack.sh`
1. make sure all runs went by nicely
1. `jive ./scripts/running/eqv-design.py work_density_plot`
1. ...

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
