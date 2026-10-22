.svm_duplicate_groups <- function(x, y) {
  keys <- as.data.frame(cbind(x, time = y[, 1L], event = y[, 2L]))
  n <- nrow(keys)
  o <- do.call(order, c(unname(keys), list(method = "radix")))
  same <- rep(TRUE, n - 1L)
  for (column in keys) same <- same & column[o[-1L]] == column[o[-n]]
  starts <- c(TRUE, !same)
  id <- integer(n)
  id[o] <- cumsum(starts)
  representatives <- o[starts]
  if (length(representatives) == n) return(NULL)
  list(id = id, representatives = representatives,
    count = tabulate(id, nbins = length(representatives)))
}

.svm_additive_columns <- function(x) {
  lapply(seq_len(ncol(x)), function(j) {
    o <- order(x[, j])
    value <- x[o, j]
    span <- diff(range(value))
    if (!is.finite(span) || span <= 0) {
      stop("The additive kernel cannot be applied to a constant column.", call. = FALSE)
    }
    value <- (value - min(value)) / span
    list(order = o, value = value, binary = all(value %in% c(0, 1)))
  })
}

.svm_additive_dense <- function(x) {
  if (ncol(x) != 1L) return(.svm_kernel(x, "additive"))
  span <- diff(range(x[, 1L]))
  if (!is.finite(span) || span <= 0) stop("The additive kernel cannot be applied to a constant column.", call. = FALSE)
  (span - abs(outer(x[, 1L], x[, 1L], `-`))) / span
}

.svm_additive_multiply <- function(v, columns) {
  out <- numeric(length(v))
  total <- sum(v)
  for (column in columns) {
    a <- v[column$order]
    value <- column$value
    if (column$binary) {
      upper <- sum(a[value == 1])
      contribution <- ifelse(value == 1, upper, total - upper)
    } else {
      weighted <- value * a
      contribution <- total - value * (2 * cumsum(a) - total) +
        2 * cumsum(weighted) - sum(weighted)
    }
    out[column$order] <- out[column$order] + contribution
  }
  out
}

.svm_additive_operator <- function(v, args) {
  v <- v * args$root_count
  kv <- .svm_additive_multiply(v, args$columns)
  as.vector(args$root_count * (kv + args$sign * (args$delta * kv +
    .svm_additive_multiply(args$delta * v, args$columns)) / 2))
}

.svm_additive_eigen <- function(x, columns, delta, sign, cutoff, params, root_count) {
  n <- nrow(x)
  if (sign == -1 && all(delta == 1)) {
    return(list(values = numeric(), vectors = matrix(0, n, 0),
      residual = 0, excluded_upper = 0, method = "zero_operator", nops = 0L))
  }
  k <- min(64L, n - 2L)
  args <- list(columns = columns, delta = delta, sign = sign, root_count = root_count)
  operations <- 0L
  repeat {
    # This is only the eigensolver workspace; the final QP is checked separately.
    estimate <- 8 * (n * (6 * max(1, k) + 32) + 4 * (2 * max(1, k) + 1)^2) / 1024^2
    .svm_memory_check(estimate, params)
    if (n <= 256L || k >= n / 3) {
      .svm_memory_check(8 * n^2 * 12 / 1024^2, params)
      kernel <- .svm_additive_dense(x)
      kernel <- root_count * kernel * rep(root_count, each = n)
      mixed <- (delta * kernel + t(delta * kernel)) / 2
      e <- eigen(kernel + sign * mixed, symmetric = TRUE)
      residual <- numeric(length(e$values))
      method <- "dense_block_eigen"
    } else {
      e <- RSpectra::eigs_sym(.svm_additive_operator, k = as.integer(k),
        n = n, args = args, which = "LA", opts = list(tol = 1e-11,
          maxitr = 5000L, initvec = sin(seq_len(n) * sqrt(2)) + cos(seq_len(n) * sqrt(3))))
      operations <- operations + e$nops
      if (e$nconv != k || length(e$values) != k) {
        stop("Additive SVM eigensolver did not converge; no incomplete rank is accepted.", call. = FALSE)
      }
      residual <- vapply(seq_along(e$values), function(j) {
        r <- .svm_additive_operator(e$vectors[, j], args) - e$values[j] * e$vectors[, j]
        sqrt(sum(r * r))
      }, 0)
      method <- "matrix_free_lanczos"
    }
    if (any(!is.finite(e$values)) || any(!is.finite(residual))) {
      stop("Non-finite additive SVM eigenvalues or residuals.", call. = FALSE)
    }
    uncertainty <- residual + max(1, max(abs(e$values))) * 1e-12
    if (any(abs(e$values - cutoff) <= uncertainty)) {
      stop("Additive SVM eigenvalue is numerically ambiguous at the reference cutoff; no rank approximation is accepted.", call. = FALSE)
    }
    if (max(residual) > max(1, max(abs(e$values))) * 1e-9) {
      stop("Additive SVM spectral residual exceeds tolerance.", call. = FALSE)
    }
    if (method == "dense_block_eigen" || min(e$values + uncertainty) < cutoff) break
    k <- min(n - 1L, 2L * k)
  }
  keep <- e$values > cutoff
  excluded <- e$values[!keep] + uncertainty[!keep]
  list(values = e$values[keep], vectors = e$vectors[, keep, drop = FALSE],
    residual = max(residual), excluded_upper = if (length(excluded)) max(excluded) else -Inf,
    method = method, nops = operations)
}

.svm_additive_hessian <- function(x, delta, params, groups = NULL) {
  original_n <- nrow(x)
  if (!is.null(groups)) {
    x <- x[groups$representatives, , drop = FALSE]
    delta <- delta[groups$representatives]
  }
  n <- nrow(x)
  root_count <- if (is.null(groups)) rep(1, n) else sqrt(groups$count)
  columns <- .svm_additive_columns(x)
  if (!requireNamespace("RSpectra", quietly = TRUE)) {
    stop("Compact additive SVM needs RSpectra; install.packages('RSpectra').", call. = FALSE)
  }
  # Determine the global cutoff before selecting modes in either block.
  tops <- vapply(c(1, -1), function(sign) {
    if (sign == -1 && all(delta == 1)) return(0)
    args <- list(columns = columns, delta = delta, sign = sign, root_count = root_count)
    if (n <= 4L) {
      k <- .svm_additive_dense(x)
      k <- root_count * k * rep(root_count, each = n)
      return(max(eigen(k + sign * (delta * k + t(delta * k)) / 2,
        symmetric = TRUE, only.values = TRUE)$values))
    }
    e <- RSpectra::eigs_sym(.svm_additive_operator, k = 1L, n = n,
      args = args, which = "LA", opts = list(tol = 1e-12, maxitr = 5000L,
        initvec = sin(seq_len(n) * sqrt(2)) + cos(seq_len(n) * sqrt(3))))
    if (e$nconv != 1L) stop("Additive SVM leading eigenvalue did not converge.", call. = FALSE)
    e$values[1L]
  }, 0)
  top <- max(tops)
  cutoff <- top * 1e-6
  plus <- .svm_additive_eigen(x, columns, delta, 1, cutoff, params, root_count)
  minus <- .svm_additive_eigen(x, columns, delta, -1, cutoff, params, root_count)
  rank <- length(plus$values) + length(minus$values)
  estimate <- 8 * (n * ncol(x) * 8 + n * rank * 24 + rank^2 * 4) / 1024^2
  .svm_memory_check(estimate, params)
  qp <- plus$vectors / root_count / sqrt(2)
  qm <- minus$vectors / root_count / sqrt(2)
  q <- cbind(rbind(qp, -qp), rbind(qm, qm))
  out <- .svm_project_hessian(q, c(plus$values, minus$values), top)
  out$factor <- out$factor * rep(root_count, 2L)
  out$workspace_estimate_mb <- estimate
  out$diagnostics <- list(spectral_method = c(plus$method, minus$method),
    reference_eigen_cutoff = cutoff, spectral_tolerance = 1e-11,
    maximum_eigen_residual = max(plus$residual, minus$residual),
    excluded_upper = max(plus$excluded_upper, minus$excluded_upper),
    block_ranks = c(plus = length(plus$values), minus = length(minus$values)),
    original_observations = original_n, optimization_groups = n,
    multiplicity_total = if (is.null(groups)) original_n else sum(groups$count),
    kernel_matrix_allocated = any(c(plus$method, minus$method) == "dense_block_eigen"),
    rspectra_version = as.character(utils::packageVersion("RSpectra")))
  out
}

.svm_additive_prediction <- function(sv, weights) {
  columns <- lapply(seq_len(ncol(sv)), function(j) {
    value <- sv[, j]
    knots <- sort(unique(value))
    span <- diff(range(knots))
    if (!is.finite(span) || span <= 0) {
      stop("The additive kernel cannot be applied to a constant support-vector column.", call. = FALSE)
    }
    grouped <- rowsum(matrix(weights, ncol = 1L), match(value, knots), reorder = FALSE)
    mass <- numeric(length(knots))
    mass[as.integer(rownames(grouped))] <- grouped[, 1L]
    list(knots = knots, span = span, mass = c(0, cumsum(mass)),
      weighted = c(0, cumsum(knots * mass)))
  })
  list(columns = columns, total = sum(weights))
}

.svm_additive_predict <- function(prepared, x) {
  out <- rep(length(prepared$columns) * prepared$total, nrow(x))
  for (j in seq_along(prepared$columns)) {
    column <- prepared$columns[[j]]
    i <- findInterval(x[, j], column$knots) + 1L
    distance <- x[, j] * (2 * column$mass[i] - utils::tail(column$mass, 1L)) -
      (2 * column$weighted[i] - utils::tail(column$weighted, 1L))
    out <- out - distance / column$span
  }
  as.vector(out)
}
