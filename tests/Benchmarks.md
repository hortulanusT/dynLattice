# Benchmarks {#benchmarks}

## Overview

This page provides comprehensive documentation for the benchmark cases in the dynLattice project. These tests compare the implementation with the results from literature or ensure the correctness of the implementation in other ways.

## Test Categories

The test suite is organized into scientific categories, each targeting specific physical phenomena:

- [Beam Benchmarks](BeamBenchmarks.md) - Static, elastic beam benchmarks  
- [Transient Benchmarks](TransientBenchmarks.md) - Dynamic beam benchmarks
- [Plastic Benchmarks](PlasticBenchmarks.md) - Inelastic material benchmarks
- [Contact Benchmarks](ContactBenchmarks.md) - Contact benchmarks

## Test Execution

Execute tests using the Makefile system:

```bash
make tests              # Run all tests
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
