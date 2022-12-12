# :hammer_and_pick: TO DO 
## Investigations
- Fix 3D dynamics
- Refactor Makefiles into Python scripts
## Code
1. Models
    - Rod Model
      - Extract material for the stress -> strain relationship
      - Create MaterialFactory
1. Environment
    - add .cpp for all headers...
 
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
- use the `*.tiny` setting of the solver module to enable convergence for smaller load steps

# :hourglass_flowing_sand: Ideas for the future
- Modal Reduction for Homogenization
  - dynamic homogenization for micromorphic continuum
- Nonlocal Homogenization for dynamics
- Custom Application
  - add shortcuts for `$(CASE_NAME)` without folder
  - enable multiple `.pro` files in command line
