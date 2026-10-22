example_data <- function(n = 80L) {
  survlearning:::.with_seed(41L, {
    x <- stats::rnorm(n)
    z <- stats::rnorm(n)
    failure <- exp(2 - 0.7 * x + 0.3 * z + stats::rnorm(n, sd = 0.4))
    censoring <- stats::rexp(n, rate = 0.04)
    data.frame(time = pmin(failure, censoring),
      event = as.integer(failure <= censoring), x = x, z = z,
      batch = factor(rep(c("A", "B"), length.out = n)))
  })
}

small_params <- function(method) {
  switch(method,
    tree = list(min_node = 5L),
    rsf = list(trees = 30L, min_node = 5L),
    boosting = list(trees = 30L, min_node = 5L),
    aft_lasso = , aft_ridge = list(lambda = 0.01, max_iter = 5000L),
    list())
}
