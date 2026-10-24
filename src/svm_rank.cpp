#include <Rcpp.h>
#include <algorithm>
#include <numeric>
#include <vector>

namespace {
struct Moments {
  long double count = 0, sum = 0, square = 0;
  Moments operator-(const Moments& other) const {
    return {count - other.count, sum - other.sum, square - other.square};
  }
};
struct Fenwick {
  std::vector<Moments> tree;
  explicit Fenwick(int n) : tree(n + 1) {}
  void add(int k, double value) {
    for (++k; k < static_cast<int>(tree.size()); k += k & -k) {
      tree[k].count += 1;
      tree[k].sum += value;
      tree[k].square += static_cast<long double>(value) * value;
    }
  }
  Moments prefix(int k) const {
    Moments result;
    for (; k > 0; k -= k & -k) {
      result.count += tree[k].count;
      result.sum += tree[k].sum;
      result.square += tree[k].square;
    }
    return result;
  }
};
}

// Exact squared-hinge loss over every strictly time-ordered comparable pair.
// Two sweeps give both gradient contributions without storing the pair matrix.
extern "C" SEXP sl_svm_rank(SEXP score_sexp, SEXP time_sexp, SEXP event_sexp) {
  BEGIN_RCPP
  Rcpp::NumericVector input_score(score_sexp), time(time_sexp), event(event_sexp);
  const int n = input_score.size();
  if (n < 2 || time.size() != n || event.size() != n)
    Rcpp::stop("Ranking inputs must have equal lengths of at least two.");
  std::vector<int> order(n), rank(n);
  std::iota(order.begin(), order.end(), 0);
  std::vector<double> score(n);
  for (int i = 0; i < n; ++i) {
    if (!R_FINITE(input_score[i]) || !R_FINITE(time[i]) ||
        (event[i] != 0 && event[i] != 1))
      Rcpp::stop("Ranking inputs must be finite with binary events.");
    // Ranking depends only on differences; remove a common offset before moments.
    score[i] = input_score[i] - input_score[0];
    if (!R_FINITE(score[i])) Rcpp::stop("Ranking score range exceeds numerical limits.");
  }
  std::vector<double> values(score.begin(), score.end());
  std::sort(values.begin(), values.end());
  values.erase(std::unique(values.begin(), values.end()), values.end());
  const int m = values.size();
  for (int i = 0; i < n; ++i)
    rank[i] = std::lower_bound(values.begin(), values.end(), score[i]) - values.begin();
  std::stable_sort(order.begin(), order.end(), [&](int a, int b) {return time[a] < time[b];});
  Rcpp::NumericVector gradient(n);
  Fenwick later(m), earlier(m);
  long double loss = 0, pairs = 0;
  for (int end = n; end > 0;) {
    Rcpp::checkUserInterrupt();
    int begin = end - 1;
    while (begin > 0 && time[order[begin - 1]] == time[order[end - 1]]) --begin;
    const Moments total = later.prefix(m);
    for (int k = begin; k < end; ++k) {
      const int i = order[k];
      if (event[i] == 0) continue;
      pairs += total.count;
      const int cut = std::upper_bound(values.begin(), values.end(), score[i] - 1) - values.begin();
      const Moments active = total - later.prefix(cut);
      const long double a = 1 - static_cast<long double>(score[i]);
      loss += std::max(0.0L, active.count * a * a + 2 * a * active.sum + active.square);
      gradient[i] -= static_cast<double>(2 * (a * active.count + active.sum));
    }
    for (int k = begin; k < end; ++k) later.add(rank[order[k]], score[order[k]]);
    end = begin;
  }
  for (int begin = 0; begin < n;) {
    Rcpp::checkUserInterrupt();
    int end = begin + 1;
    while (end < n && time[order[end]] == time[order[begin]]) ++end;
    for (int k = begin; k < end; ++k) {
      const int j = order[k];
      const int cut = std::lower_bound(values.begin(), values.end(), score[j] + 1) - values.begin();
      const Moments active = earlier.prefix(cut);
      gradient[j] += static_cast<double>(2 * ((1 + static_cast<long double>(score[j])) * active.count - active.sum));
    }
    for (int k = begin; k < end; ++k)
      if (event[order[k]] == 1) earlier.add(rank[order[k]], score[order[k]]);
    begin = end;
  }
  if (pairs == 0) Rcpp::stop("No strictly time-ordered comparable pairs for ranking.");
  gradient = gradient / static_cast<double>(pairs);
  return Rcpp::List::create(Rcpp::Named("loss") = static_cast<double>(loss / pairs),
    Rcpp::Named("gradient") = gradient, Rcpp::Named("pairs") = static_cast<double>(pairs));
  END_RCPP
}
