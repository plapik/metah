#ifndef SRC_METAH_SOLUTION_HPP_
#define SRC_METAH_SOLUTION_HPP_

namespace metah {
template <typename R, typename S>
struct Solution final {
  using ReprType = R;
  using ScoreType = S;
  ReprType repr;
  ScoreType score;
};
}  // namespace metah

#endif  // SRC_METAH_SOLUTION_HPP_
