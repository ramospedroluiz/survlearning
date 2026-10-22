// Derived from penAFT 0.3.2, Molstad and Suder, GPL >= 2; see COPYRIGHTS.
// Preserve pair order and ADMM algebra; compress the incidence lookup arrays.
#define ARMA_DONT_USE_OPENMP
#include <RcppArmadillo.h>
#include <algorithm>
#include <limits>
#include <vector>

namespace {
struct PairGraph {
    std::vector<int> left, right, incoming, outgoing_start, incoming_start;
    PairGraph(const arma::vec& delta) {
        const int n = delta.n_elem;
        std::vector<int> events;
        for (int i = 0; i < n; ++i) if (delta(i) == 1) events.push_back(i);
        const double m = events.size(), count = m * n - m * (m + 1) / 2;
        if (count > std::numeric_limits<int>::max()) Rcpp::stop("Exact Gehan pair count exceeds the supported index range.");
        left.reserve(static_cast<size_t>(count)); right.reserve(static_cast<size_t>(count));
        outgoing_start.resize(n + 1); incoming_start.assign(n + 1, 0);
        for (int i = 0; i < n; ++i) {
            if (i % 128 == 0) Rcpp::checkUserInterrupt();
            outgoing_start[i] = left.size();
            if (delta(i) == 1) {
                for (int j = i + 1; j < n; ++j) {
                    left.push_back(i); right.push_back(j); ++incoming_start[j + 1];
                }
            } else {
                for (auto it = std::upper_bound(events.begin(), events.end(), i); it != events.end(); ++it) {
                    left.push_back(i); right.push_back(*it); ++incoming_start[*it + 1];
                }
            }
        }
        outgoing_start[n] = left.size();
        for (int i = 0; i < n; ++i) incoming_start[i + 1] += incoming_start[i];
        incoming.resize(left.size());
        std::vector<int> cursor(incoming_start);
        for (size_t k = 0; k < left.size(); ++k) incoming[cursor[right[k]]++] = k;
    }
    arma::vec transpose(const arma::vec& v) const {
        const size_t n = outgoing_start.size() - 1;
        arma::vec out(n, arma::fill::zeros);
        for (size_t i = 0; i < n; ++i) {
            const int a = outgoing_start[i + 1] - outgoing_start[i];
            const int b = incoming_start[i + 1] - incoming_start[i];
            double sum = 0;
            // Preserve the original interleaved positive/negative accumulation.
            for (int j = 0; j < std::max(a, b); ++j) {
                if (j < a) sum += v(outgoing_start[i] + j);
                if (j < b) sum -= v(incoming[incoming_start[i] + j]);
            }
            out(i) = sum;
        }
        return out;
    }
    arma::vec difference(const arma::vec& v) const {
        arma::vec out(left.size());
        for (size_t i = 0; i < left.size(); ++i) out(i) = v(left[i]) - v(right[i]);
        return out;
    }
};

double gehan_value(const arma::vec& residual, const arma::vec& delta) {
    std::vector<double> sorted(residual.begin(), residual.end());
    std::sort(sorted.begin(), sorted.end());
    std::vector<long double> suffix(sorted.size() + 1, 0);
    for (size_t k = sorted.size(); k > 0; --k) suffix[k - 1] = suffix[k] + sorted[k - 1];
    long double loss = 0;
    for (size_t i = 0; i < residual.n_elem; ++i) if (delta(i) == 1) {
        const size_t k = std::upper_bound(sorted.begin(), sorted.end(), residual(i)) - sorted.begin();
        loss += suffix[k] - static_cast<long double>(sorted.size() - k) * residual(i);
    }
    return static_cast<double>(loss / residual.n_elem / residual.n_elem);
}
}

RcppExport SEXP sl_gehan_fit(SEXP x_, SEXP logtime_, SEXP event_, SEXP lambda_,
                           SEXP alpha_, SEXP eta_, SEXP maxiter_, SEXP tolerance_) {
    BEGIN_RCPP
    const arma::mat X = Rcpp::as<arma::mat>(x_);
    const arma::vec logtime = Rcpp::as<arma::vec>(logtime_), delta = Rcpp::as<arma::vec>(event_);
    const double lambda = Rcpp::as<double>(lambda_), alpha = Rcpp::as<double>(alpha_);
    const double eta = Rcpp::as<double>(eta_), tol_rel = Rcpp::as<double>(tolerance_);
    const unsigned int max_iter = Rcpp::as<unsigned int>(maxiter_);
    const double tol_abs = 1e-8;
    const int n = X.n_rows, p = X.n_cols;
    PairGraph graph(delta);
    const size_t l = graph.left.size();
    arma::vec y = graph.difference(logtime), Theta(y), Gamma = -arma::sign(y);
    arma::sp_mat Beta(p, 1);
    const arma::mat Xt = X.t();
    arma::vec tXB(l, arma::fill::zeros), t0(l), scratch(l), previous(l);
    const arma::mat lam(p, 1, arma::fill::value(lambda * alpha / eta));
    const arma::mat lam2(p, 1, arma::fill::value(lambda * (1 - alpha) / eta));
    long double squared_y = 0;
    for (size_t i = 0; i < l; ++i) squared_y += y(i) * y(i);
    const double euc_y = std::sqrt(static_cast<double>(squared_y));
    double rho = .1, update_step = 1;
    double primal = R_PosInf, dual = R_PosInf, eprim = 0, edual = 0;
    unsigned int iteration = 0;
    bool converged = false;
    for (unsigned int it = 1; it <= max_iter; ++it) {
        iteration = it;
        if (it % 10 == 1) Rcpp::checkUserInterrupt();
        t0 = y - tXB - ((1 / rho) * Gamma);
        scratch = t0 - Theta;
        Beta = ((1 / eta) * Xt * graph.transpose(scratch)) + Beta;
        arma::sp_mat signs(Beta);
        signs.for_each([](arma::sp_mat::elem_type& v) { v = (v > 0) - (v < 0); });
        Beta = arma::abs(Beta);
        Beta = Beta - (lam / rho);
        Beta.for_each([](arma::sp_mat::elem_type& v) { v = std::max(v, 0.0); });
        Beta = Beta % signs;
        Beta = Beta / (1 + lam2 / rho);
        tXB = graph.difference(arma::vec(X * Beta));
        previous = Theta;
        t0 = y - tXB - ((1 / rho) * Gamma);
        const double nrho = std::pow(n, 2) * rho;
        for (size_t k = 0; k < l; ++k) {
            const double upper = delta(graph.right[k]) / nrho, lower = delta(graph.left[k]) / nrho;
            Theta(k) = t0(k) > upper ? t0(k) - upper : (t0(k) < -lower ? t0(k) + lower : 0);
        }
        Gamma = Gamma + 1.5 * rho * (Theta - y + tXB);
        if (it % static_cast<unsigned int>(update_step) == 0) {
            scratch = Theta - previous;
            dual = rho * arma::norm(Xt * graph.transpose(scratch), 2);
            primal = arma::norm(Theta - y + tXB, 2);
            eprim = std::sqrt(l) * tol_abs + tol_rel * std::max(std::max(arma::norm(tXB, 2), arma::norm(Theta, 2)), euc_y);
            edual = std::sqrt(p) * tol_abs + tol_rel * arma::norm(Xt * graph.transpose(Gamma), 2);
            if (primal / eprim > 10 * dual / edual) rho *= 2;
            if (dual / edual > 10 * primal / eprim) rho /= 2;
            if (it > 10 && primal < eprim && dual < edual) { converged = true; break; }
            update_step = (update_step + 1) * 1.1;
        }
    }
    const arma::vec beta(Beta);
    if (!beta.is_finite()) Rcpp::stop("Gehan ADMM returned non-finite coefficients.");
    const double objective = gehan_value(logtime - X * beta, delta) +
        lambda * alpha * arma::accu(arma::abs(beta)) + lambda * .5 * (1 - alpha) * arma::dot(beta, beta);
    // Feasible dual lower bound, independently of the residual stopping rule.
    for (size_t k = 0; k < l; ++k) Gamma(k) =
        std::max(-delta(graph.right[k]) / std::pow(n, 2),
          std::min(delta(graph.left[k]) / std::pow(n, 2), Gamma(k)));
    arma::vec v = Xt * graph.transpose(Gamma);
    if (alpha == 1 && arma::max(arma::abs(v)) > lambda) {
        const double scale = lambda / arma::max(arma::abs(v));
        Gamma *= scale; v *= scale;
    }
    double conjugate = 0;
    if (alpha < 1) {
        const arma::vec excess = arma::clamp(arma::abs(v) - lambda * alpha, 0, R_PosInf);
        conjugate = arma::dot(excess, excess) / (2 * lambda * (1 - alpha));
    }
    const double lower_bound = -arma::dot(Gamma, y) - conjugate;
    return Rcpp::List::create(Rcpp::Named("beta") = beta,
        Rcpp::Named("iterations") = iteration, Rcpp::Named("converged") = converged,
        Rcpp::Named("pairs") = static_cast<double>(l), Rcpp::Named("objective") = objective,
        Rcpp::Named("dual_objective") = lower_bound, Rcpp::Named("duality_gap") = objective - lower_bound,
        Rcpp::Named("primal_residual") = primal, Rcpp::Named("dual_residual") = dual,
        Rcpp::Named("primal_tolerance") = eprim, Rcpp::Named("dual_tolerance") = edual);
    END_RCPP
}
