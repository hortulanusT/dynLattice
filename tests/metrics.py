#!/usr/bin/python3

# Comparison metrics imported by tests/<category>/test*.py via sys.path.

import numpy as np

# NaN policy: NaN in a reference is padding (the digitised CSVs are padded to
# equal length) and is dropped; NaN or inf in a simulation result means the
# run is broken, so the metrics raise and the calling test script reports FAILED.


def _check_finite(y, name):
    if not np.all(np.isfinite(y)):
        raise ValueError(f"non-finite values in {name}")


def interp_on_reference(x_sim, y_sim, x_ref, max_overshoot=0.01):
    """Linear interpolation of (x_sim, y_sim) onto the reference abscissa.

    The simulation samples are sorted by x before calling np.interp, because
    np.interp assumes a monotone x array and the load history coming from
    the solver is not guaranteed to be sorted (especially after an arc-length
    step).

    np.interp silently clamps outside the simulated range, so a truncated run
    would still be graded.  Therefore the reference range must be covered by the
    simulation, up to max_overshoot times the simulated range (digitised
    references sometimes run slightly past the end of the simulation).
    """
    x_sim, y_sim, x_ref = (np.asarray(a, dtype=float) for a in (x_sim, y_sim, x_ref))
    _check_finite(x_sim, "simulated abscissa")
    _check_finite(y_sim, "simulated values")
    x_ref = x_ref[np.isfinite(x_ref)]
    span = np.ptp(x_sim)
    if np.min(x_ref) < np.min(x_sim) - max_overshoot * span or \
       np.max(x_ref) > np.max(x_sim) + max_overshoot * span:
        raise ValueError(
            f"simulation covers [{x_sim.min():g}, {x_sim.max():g}] but the "
            f"reference needs [{x_ref.min():g}, {x_ref.max():g}]")
    order = np.argsort(x_sim)
    return np.interp(x_ref, x_sim[order], y_sim[order])


def relative_L2(y_sim, y_ref, scale=None):
    """||y_sim - y_ref|| / ||y_ref||, with a safe fallback when y_ref is
    essentially zero (the load step at t=0 in some traces).

    If scale is given, the error is divided by ||scale|| instead of ||y_ref||.
    Use this when y_ref is small compared to the other components of the same
    group (e.g. a nearly vanishing force component), so that the relative error
    would only measure noise.
    """
    _check_finite(y_sim, "simulated values")
    y_ref = np.asarray(y_ref, dtype=float)
    ok = np.isfinite(y_ref)
    y_sim, y_ref = np.asarray(y_sim)[ok], y_ref[ok]
    denom = np.linalg.norm(y_ref if scale is None else scale)
    if denom < 1e-12:
        denom = max(np.max(np.abs(y_ref)), 1e-12)
    return float(np.linalg.norm(y_sim - y_ref) / denom)


def envelope_error(sim, ref):
    """Worst-case relative error of the max and the min of two 1-D signals.

    Plastic hysteresis loops sweep the same force range many times; a
    pointwise comparison would need a load-history alignment that we cannot
    recover from the CSVs alone.  Matching the envelope (peak tension and
    peak compression) is the next-best thing and is what one would typically
    look at on a printed figure.
    """
    _check_finite(sim, "simulated values")
    ref = ref[np.isfinite(ref)]
    peak_err = abs(np.max(sim) - np.max(ref)) / max(abs(np.max(ref)), 1e-12)
    valley_err = abs(np.min(sim) - np.min(ref)) / max(abs(np.min(ref)), 1e-12)
    return max(float(peak_err), float(valley_err))


def curve_distance_2d(sim_xy, ref_xy):
    """Symmetric mean nearest-neighbour distance between two planar curves,
    normalised by the diagonal of the reference bounding box.

    Designed for the post-buckling responses of the 45-degree bend, where the
    load-displacement curve folds back on itself and is not a function of any
    single coordinate.  Treating the curve as an unordered point cloud is
    robust to that.  The normalisation keeps the metric dimensionless so the
    same tolerance can be applied to all benchmarks.
    """
    _check_finite(sim_xy, "simulated curve")
    ref_xy = ref_xy[np.all(np.isfinite(ref_xy), axis=1)]
    # Pairwise distance matrix (small enough -- ~100 x 100 at most).
    delta = sim_xy[:, None, :] - ref_xy[None, :, :]
    dist = np.sqrt((delta * delta).sum(axis=2))
    # For every reference point, the distance to its closest simulation point.
    mean_nn = dist.min(axis=0).mean()
    bbox = ref_xy.max(axis=0) - ref_xy.min(axis=0)
    return float(mean_nn / max(np.linalg.norm(bbox), 1e-12))
