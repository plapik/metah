#include <chrono>
#include <memory>
#include <vector>
#include <fmt/ranges.h>
#include <metah/algorithms/genetic_algorithm.hpp>
#include <metah/algorithms/simulated_annealing.hpp>
#include <metah/algorithms/tabu_search.hpp>
#include <metah/observers/population_stats_logger.hpp>
#include <metah/observers/solution_score_logger.hpp>
#include <metah/operators/permut/pmx.hpp>
#include <metah/operators/permut/reverse_mut.hpp>
#include <metah/operators/permut/reverse_nbr.hpp>
#include <metah/operators/permut/reverse_perturbation.hpp>
#include <metah/operators/tournament.hpp>

#include "tsp_evaluator.hpp"

template <typename T>
void run_and_measure(T& algo, const char* name) {
  const auto start = std::chrono::high_resolution_clock::now();
  algo.run();
  auto [repr, score] = algo.getBest();
  const auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();
  fmt::println("{}: {} ms\nScore: {}\nRepresentation: {}", name, duration,
               score, repr);
}

int main() {
  // P01 instance from
  // https://people.sc.fsu.edu/~jburkardt/datasets/tsp/tsp.html
  const auto tsp = std::make_shared<Tsp>(
      15,
      std::vector<int64_t>{
           0, 29, 82, 46, 68, 52, 72, 42, 51, 55, 29, 74, 23, 72, 46,
          29,  0, 55, 46, 42, 43, 43, 23, 23, 31, 41, 51, 11, 52, 21,
          82, 55,  0, 68, 46, 55, 23, 43, 41, 29, 79, 21, 64, 31, 51,
          46, 46, 68,  0, 82, 15, 72, 31, 62, 42, 21, 51, 51, 43, 64,
          68, 42, 46, 82, 0,  74, 23, 52, 21, 46, 82, 58, 46, 65, 23,
          52, 43, 55, 15, 74,  0, 61, 23, 55, 31, 33, 37, 51, 29, 59,
          72, 43, 23, 72, 23, 61,  0, 42, 23, 31, 77, 37, 51, 46, 33,
          42, 23, 43, 31, 52, 23, 42,  0, 33, 15, 37, 33, 33, 31, 37,
          51, 23, 41, 62, 21, 55, 23, 33,  0, 29, 62, 46, 29, 51, 11,
          55, 31, 29, 42, 46, 31, 31, 15, 29,  0, 51, 21, 41, 23, 37,
          29, 41, 79, 21, 82, 33, 77, 37, 62, 51,  0, 65, 42, 59, 61,
          74, 51, 21, 51, 58, 37, 37, 33, 46, 21, 65,  0, 61, 11, 55,
          23, 11, 64, 51, 46, 51, 51, 33, 29, 41, 42, 61,  0, 62, 23,
          72, 52, 31, 43, 65, 29, 46, 31, 51, 23, 59, 11, 62,  0, 59,
          46, 21, 51, 64, 23, 59, 33, 37, 11, 37, 61, 55, 23, 59,  0});
  namespace m = metah;
  namespace mp = metah::permut;

  TspRandomGenerator tsp_random_gen;

  m::GeneticAlgorithm ga(
      tsp, 100, 300, 4, 4,
      std::make_unique<m::Tournament<Tsp::ScoreType, true>>(5),
      std::make_unique<mp::Pmx<Tsp::ReprType>>(0.9),
      std::make_unique<mp::Reverse<Tsp::ReprType>>(0.2), tsp_random_gen);
  auto ga_ps =
      m::PopulationStatsFileLogger<Tsp::ReprType, Tsp::ScoreType>("tsp_ga.csv");
  ga.addLowPrioObserver(&ga_ps);

  m::TabuSearch ts(tsp, 1000, 30, 4, 2,
                   std::make_unique<mp::ReverseNeighbour<Tsp::ReprType>>(30),
                   tsp_random_gen);
  auto ta_ss =
      m::SolutionScoreFileLogger<Tsp::ReprType, Tsp::ScoreType>("tsp_ts.csv");
  ts.addLowPrioObserver(&ta_ss);

  m::SimulatedAnnealing sa(
      tsp, 30000, 100.0,
      std::make_unique<mp::ReversePerturbation<Tsp::ReprType>>(),
      std::make_unique<m::GeometricCooling>(0.9997),
      std::make_unique<m::MetropolisAcceptance<Tsp::ScoreType, true>>(),
      tsp_random_gen);
  auto sa_ss =
      m::SolutionScoreFileLogger<Tsp::ReprType, Tsp::ScoreType>("tsp_sa.csv");
  sa.addLowPrioObserver(&sa_ss);

  run_and_measure(ga, "GA");
  run_and_measure(ts, "TS");
  run_and_measure(sa, "SA");

  return 0;
}
