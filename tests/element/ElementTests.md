# Element Tests {#element_tests}

## Overview

The element tests validate the fundamental beam element implementation against analytical solutions for single structural elements. These tests ensure that the geometrically exact Cosserat rod formulation correctly captures the essential physics of beam deformation under small loading conditions.

## Test Structure

Each element test consists of:

| File | Purpose |
|------|---------|
| `test.geo` | GMSH geometry definition for single element |
| `model.pro` | Material properties and boundary conditions |
| `input.pro` | Mesh import and preprocessing |
| `output.pro` | Result extraction configuration |
| `test.pro` | Main test driver with solver settings |
| `processing.py` | Post-processing and validation script |

## Test Cases

### Geometric Configurations

Tests are performed for elements oriented along each coordinate axis:
- **X-direction**: `lin_x`, `quad_x`, `cub_x`
- **Y-direction**: `lin_y`, `quad_y`, `cub_y`  
- **Z-direction**: `lin_z`, `quad_z`, `cub_z`

### Element Orders

Three interpolation orders are tested:
- **Linear** (`lin`): 2-node elements with linear shape functions
- **Quadratic** (`quad`): 3-node elements with quadratic shape functions
- **Cubic** (`cub`): 4-node elements with cubic shape functions

### Loading Conditions

Each element is tested under 12 loading scenarios:

| Load Type | DOF | Description |
|-----------|-----|-------------|
| Force | `dx`, `dy`, `dz` | Axial and transverse point loads |
| Force | `rx`, `ry`, `rz` | Applied moments and torques |
| Displacement | `dx`, `dy`, `dz` | Prescribed translations |
| Displacement | `rx`, `ry`, `rz` | Prescribed rotations |

## Analytical Solutions

For small deformations, the element behavior is compared against:

1. **Axial deformation**: \f$u = \frac{FL}{EA}\f$
2. **Bending deflection**: \f$w = \frac{FL^3}{3EI}\f$
3. **Torsional rotation**: \f$\theta = \frac{TL}{GJ}\f$
4. **Shear deformation**: \f$\gamma = \frac{V}{GA_s}\f$

## Material Properties

Standard test parameters ensure consistent validation:

| Property | Value | Units |
|----------|-------|-------|
| Young's Modulus | 1000 | Pa |
| Shear Modulus | 1000 | Pa |
| Cross-sectional Area | 1.0 | m² |
| Area Moment of Inertia | 1.0 | m⁴ |
| Polar Moment of Inertia | 1.0 | m⁴ |
| Shear Correction Factor | 1.0 | - |

## Execution

Execute element tests using:
```bash
make element-tests
```

This generates validation results in `tests/element/result_*.txt` files.

## See Also

- [Beam Tests](@ref beam_tests) for multi-element beam validation
- [Transient Tests](@ref transient_tests) for dynamic response validation
