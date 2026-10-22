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

