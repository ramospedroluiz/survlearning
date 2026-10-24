# survlearning 0.3.4

* Distinguish an accuracy-verified line-search termination from optimizer
  success. Keep the same stationarity and ranking gap tolerances, and retain
  the original optimizer diagnostics. Iteration-limit failures remain rejected.
* Center scores inside the ranking pair evaluator to preserve offset invariance
  in its moment calculations, without changing the loss or regularization.
* Add independent hybrid and positive-epsilon IPCW quadratic-program checks,
  large-offset regression tests and explicit training/evaluation tie checks.
* Clarify censoring support and the distinction between strictly ordered
  training pairs and Harrell's event/censoring time-tie convention.

* Add explicit native ranking, squared-loss hybrid and restricted-response
  IPCW SVM methods, with linear and exact additive kernels. The original
  censored absolute-loss `svm` remains unchanged.
* Evaluate every comparable ranking pair through a native Fenwick calculation;
  retain all additive knots with an exact whitened penalty representation.
* Extend fold-local cached tuning, parallel execution and resumable searches
  to the new methods without adding dependencies.
* Document risk-only prediction contracts, IPCW support requirements and
  independent objective/gradient verification.

# survlearning 0.3.2

* Corrected the censored-SVR objective and its independent primal/dual checks.
  Both kernels now use an exact sparse primal; the legacy conditioned Hessian
  and its Newton/quadprog fitting paths are not used for new models.
* Additive prediction retains all training coordinates and training ranges.
* Added a documented training-fixed prediction resolution for numerical ties,
  deterministic intercept selection, and separate kernel tuning examples.
* Expanded mathematical, solver, cache, serialization and prediction tests.
  Previously fitted SVM models require refitting.

# survlearning 0.3.1

* Penalized Cox fits use a descending warm-start path ending at the exact
  requested lambda. This avoids unstable single-lambda cold starts in highly
  censored cohorts without changing the objective or the selected penalty.

# survlearning 0.3.0

* Integrated RF-SRC survival forests, with the original numerical core and
  without graphical dependencies.
* Integrated Royston-Parmar estimation on the hazard, odds and normal scales.
  Repeated event-quantile knots are diagnosed rather than silently fitting a
  degenerate spline basis.
* Compact Gehan ADMM storage retaining all informative pairs, with objective,
  dual-bound and convergence diagnostics. The requested iteration limit is
  now honored.
* Existing method identifiers and prediction contracts are unchanged.
* External randomForestSRC, flexsurv and penAFT packages are needed only for
  independent reference tests, not to use any supported method.

# survlearning 0.2.6

Initial public version.

* A common formula interface for survival model fitting, tuning, and prediction.
* Censoring-aware evaluation, calibration, and reusable validation plans.
* Native neural Cox estimation and compact linear and additive censored SVM.
* Predictive interpretation and conditional reliability summaries.
* All supported modeling engines and solvers installed as required dependencies.
* Parallel tuning and resumable searches.
