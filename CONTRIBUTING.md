# Contributing to survlearning

Please open an issue to discuss a new method or a change to the statistical
interface before preparing a pull request. Bug reports should include a small
reproducible example and the output of `sessionInfo()`.

## Add an engine, not a new user interface

1. Add a declarative specification to `.reference_specs()` or the existing
   `.method_specs()`: identifier, description, optional engine, native prediction
   capabilities and named defaults. Keep existing identifiers compatible.
2. Validate every parameter, including categorical values and boundary cases.
   Accept the common formula design. Learn any scaling, knots and time grid on
   training rows only. Never discard censored rows or silently subsample.
3. Implement fitting and prediction helpers, dispatched from the existing
   reference registry. Optional engines must be loaded explicitly when restoring
   saved objects. Fit and prediction must not modify global options or RNG state.
4. Define the increasing-risk convention. If a method predicts distributions,
   declare its time origin, support and extrapolation assumptions. An absolute
   probability or median must not be invented for a risk-only engine.
5. Store training-only baselines and preprocessing state, plus readable engine
   warnings and status. Use public engine APIs and preserve licensing/attribution.
6. Test native prediction agreement, single-row inputs, reordered/repeated times,
   S(0)=1, monotonicity, finite risk, saved-object prediction in a fresh session,
   reproducible seeds, unseen factors, missing values and fold-only preprocessing.
7. Add a small example, parameter documentation, NEWS entry and benchmark case.
   Optional-engine tests should skip only when that dependency/runtime is absent,
   not when an installed engine fails. Do not mark warnings as successful convergence.

## Check before review

Run `testthat::test_local()` from the repository root, then `R CMD build .`
and `R CMD check --as-cran survlearning_0.2.5.tar.gz` from a terminal.
Optional-engine tests require their corresponding packages. Slow experiments
belong outside mandatory examples and tests.
Core functionality must work without torch, Python, downloads or every forest
engine installed. Heavy runtimes are installed only by explicit user action.

Grouped and temporal data require structurally matching inner/outer validation.
No contribution may silently reuse a final test set for tuning or early stopping.
New observation types such as competing risks or left truncation need a new,
reviewed task contract; relaxing the current right-censoring checks is insufficient.
