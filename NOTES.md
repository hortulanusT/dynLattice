# :hammer_and_pick: TO DO 
## Investigations
- Find defined re-entrant unit cell
- Equivalent (linear static) unit cells in other structures
- Do finite strain comparision
- Do dynamic comparison
- Fix 3D dynamics
## Code
1. Models
    - Rod Model
      - Joint stiffening by increase of geometry
    - PBC Model
      - proper adressing of H in the boundary conditions (allow for mixed loadings)
      - get rid of load/disp factor -> detect implicitly from the loading type specified (Hxx vs Pxx)
2. Modules
    - PBC Output extent other side
      - $\frac{du}{dX} \approx \frac{u_2-u_1}{\Delta X}$
3. Environment
    - move studies to makefile targets
    - add .cpp for all headers...
 
# :heavy_check_mark: Information
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
- Custom Application
  - add shortcuts for `$(CASE_NAME)` without folder
  - enable multiple `.pro` files in command line
