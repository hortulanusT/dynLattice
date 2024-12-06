# :arrows_counterclockwise: Workflows
## *testing/standard studies*
1. `jive make clean-all`
1. `jive make tests`
1. `jive make studies`
## *eqv-design* study
> :warning: make sure repo on cluster is updated :warning:
1. `jive scripts/running/eqv-design.py lin_init prep_runs lin_change_prep`
1. `./scripts/cluster/masterDesign.sh eqv`
1. `./scripts/cluster/syncBack.sh`
1. `jive scripts/running/eqv-design.py lin_change_plot get_failed`
1. `./scripts/cluster/masterFailed.sh eqv`
1. `./scripts/cluster/syncBack.sh`
1. `jive scripts/running/eqv-design.py work_density_plot stress_plot comp_speeds_locale_plot`
1. `jive scripts/running/eqv-design.py work_density_export prep_latex_design`
1. `rsync -vax --exclude '*.pdf' --delete studies/results/eqv-design/ ../Latex/Paper0/results/`
## *plast-size* study
1. `jive scripts/running/plast-size.py do_plast_studies_anew prep_latex_plast`
1. `rsync -vax --exclude '*.pdf' --delete studies/results/plast-size/ ../Latex/Paper1/results/`
## *tno-design* study
> :warning: make sure repo on cluster is updated :warning:
1. `jive scripts/running/tno-design.py samples_init prep_runs_nonlin lin_change_prep_nonlin`
1. `./scripts/cluster/masterDesign.sh tno`
1. `./scripts/cluster/syncBack.sh`
1. `jive scripts/running/tno-design.py lin_change_plot_nonlin get_failed`
1. `./scripts/cluster/masterFailed.sh tno`
1. `./scripts/cluster/syncBack.sh`
1. `jive scripts/running/tno-design.py work_density_plot stress_plot comp_speeds_locale_plot`
1. `jive scripts/running/tno-design.py work_density_export prep_latex_design_nonlin`
1. `rsync -vax --exclude '*.pdf' studies/results/tno-design/ ../Latex/PaperTNO/results/` 
## *nonlin-design* study
> :warning: make sure repo on cluster is updated :warning:
1. `jive scripts/running/nonlin-design.py lin_init prep_runs_nonlin`
1. `./scripts/cluster/masterDesign.sh nonlin`
</br></br></br></br>

# :hammer_and_pick: TO DO 
1. Models
    - Energy Output (various models)
      - get proper split into
        1. Kinetic
        1. Potential
        1. Dissipated
        1. Contact
      - get spatial resolution
1. Modules
    - Explicit Solver
      - keep $\bm{M}$ constant, only update $\Theta$
      - implement 'lumped' algorithm with direct 3x3 inverts
      - Old $\bm{M}$ matrix as pre-conditioner?
    - Tangent Output Module
      - Fix plasticity issues
      - Get Matrix Condensation to work
1. Application
    - get multiple input files to work
1. Environment
    - profile the code to find areas which could be improved (e.g. valgrind --callgrind or so)
    - make all plot outputs A4/A3
</br></br></br></br>

# :heavy_check_mark: Information
## cluster syncing
``` bash
rsync -vax $TARGET $DESTINATION
```
## debugging
- `(gdb) call dprint(&...)` to get any matrix/vector/... printed live
- when compiling OPT, it sets the flag `NDEBUG`
- `./jive xyz.pro |c++filt` for nicer stack traces
## .pro files
- `$(CASE_NAME)` also works with environment variables (and other settings)
- `include "xyz.pro";` works to include other .pro files
## others
- shape function node numbering in `$JIVEDIR/packages/geom/doc/images`
- `StdShape` for local coordinates and `Shape` for global coordinates
- `JEM_PRECHECK` gets always executed, `JEM_ASSERT` only in non-optimized mode
- `for f in $(git grep --cached -Il ''); do tail -c1 $f | read -r _ || echo >> $f; done` ensures all git files end with a newline
