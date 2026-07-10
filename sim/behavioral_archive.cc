#include "sim/behavioral_archive.h"

#include <algorithm>
#include <string>
#include <vector>

namespace coralnpu {
namespace fuzzer {

BehavioralDescriptor BehavioralArchive::ExtractDescriptor(
    const CoverageSummary& summary) const {
  std::vector<std::string> active_detectors;
  for (const auto& [key, count] : summary) {
    if (count > 0) {
      active_detectors.push_back(key);
    }
  }
  std::sort(active_detectors.begin(), active_detectors.end());

  // Strategy: Pick the first active detector that corresponds to an empty bin.
  for (const auto& detector : active_detectors) {
    BehavioralDescriptor candidate = {detector};
    if (archive_.find(candidate) == archive_.end()) {
      return candidate;
    }
  }

  // Fallback: If all bins are full, pick the one with the highest count.
  BehavioralDescriptor bd;
  uint64_t max_count = 0;
  for (const auto& key : active_detectors) {
    uint64_t count = summary.at(key);
    if (count > max_count) {
      max_count = count;
      bd.detector_name = key;
    }
  }

  // If no active detectors or tie, default to first lexicographical
  if (bd.detector_name.empty() && !active_detectors.empty()) {
    bd.detector_name = active_detectors[0];
  }

  return bd;
}

bool BehavioralArchive::AddIfNovel(
    const BehavioralDescriptor& descriptor, TestSequence seq,
    const CoverageSummary& summary,
    std::function<void(TestSequence&)> populate_fn) {
  uint64_t current_fitness = summary.size();  // Fitness = unique states hit

  auto store_seq = [&](TestSequence& s) {
    if (populate_fn) {
      populate_fn(s);
    }
    archive_[descriptor] = std::move(s);
    fitness_map_[descriptor] = current_fitness;
  };

  if (archive_.find(descriptor) == archive_.end()) {
    store_seq(seq);
    return true;
  }

  // Local Elitism: If we already have a program in this bin, only replace it
  // if the new program has higher fitness.
  if (current_fitness > fitness_map_[descriptor]) {
    store_seq(seq);
    return true;
  }

  return false;
}

TestSequence BehavioralArchive::GetRandomParent(std::mt19937_64& prng) const {
  if (archive_.empty()) {
    return TestSequence();
  }
  // Randomly select a bin
  int idx = prng() % archive_.size();
  auto it = archive_.begin();
  std::advance(it, idx);
  return it->second;
}

}  // namespace fuzzer
}  // namespace coralnpu