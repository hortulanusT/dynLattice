# Test Documentation {#tests}

## Overview

This page provides comprehensive documentation for the scientific validation test cases in the dynLattice project. These tests validate the physical correctness and numerical accuracy of the rod-based lattice simulation implementation.

## Test Categories

The test suite is organized into scientific categories, each targeting specific physical phenomena:

- [Element Tests](element/ElementTests.md) - Single element validation against analytical solutions
- [Beam Tests](beam/BeamTests.md) - Static beam network validation  
- [Transient Tests](transient/TransientTests.md) - Dynamic response validation
- [Plastic Tests](plastic/PlasticTests.md) - Inelastic material validation
- [Contact Tests](contact/ContactTests.md) - Contact mechanics validation

## Test Execution

Execute tests using the Makefile system:

```bash
make tests              # Run all tests
make element-tests      # Single element validation
make beam-tests         # Static beam networks
make transient-tests    # Dynamic simulations
make plastic-tests      # Inelastic materials  
make contact-tests      # Contact mechanics
```

## Test Structure

Each test category follows a consistent structure:
- **Configuration files** (`.pro`) - Test parameters and solver settings
- **Geometry files** (`.geo`) - GMSH mesh definitions
- **Processing scripts** (`.py`) - Post-processing and validation
- **Reference data** - Analytical or benchmark solutions for comparison
