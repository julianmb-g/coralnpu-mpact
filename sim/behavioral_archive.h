#ifndef SIM_BEHAVIORAL_ARCHIVE_H_
#define SIM_BEHAVIORAL_ARCHIVE_H_

#include <cstdint>
#include <functional>
#include <random>
#include <string>
#include <vector>

#include "sim/fuzzer_types.h"
#include "sim/isg/isg_engine.h"
#include "absl/container/flat_hash_map.h"

namespace coralnpu {
namespace fuzzer {

struct BehavioralDescriptor {
  std::string detector_name;

  static constexpr char kTrapHandlerCorrupted[] = "Trap Handler Corrupted";

  bool operator==(const BehavioralDescriptor& other) const {
    return detector_name == other.detector_name;
  }

  template <typename H>
  friend H AbslHashValue(H h, const BehavioralDescriptor& c) {
    return H::combine(std::move(h), c.detector_name);
  }
};

class BehavioralArchive {
 public:
  BehavioralArchive() = default;

  // Extracts the behavioral descriptor from a coverage summary
  BehavioralDescriptor ExtractDescriptor(const CoverageSummary& summary) const;

  // Attempts to add a sequence to the archive. Returns true if it was added
  // (either because the bin was empty or the sequence is strictly better).
  bool AddIfNovel(const BehavioralDescriptor& descriptor, TestSequence seq,
                  const CoverageSummary& summary,
                  std::function<void(TestSequence&)> populate_fn = nullptr);

  // Returns a random sequence from the archive using the provided engine's PRNG
  TestSequence GetRandomParent(std::mt19937_64& prng) const;

  // Returns the size of the archive
  size_t Size() const { return archive_.size(); }

  const absl::flat_hash_map<BehavioralDescriptor, TestSequence>& GetGrid()
      const {
    return archive_;
  }

 private:
  absl::flat_hash_map<BehavioralDescriptor, TestSequence> archive_;
  absl::flat_hash_map<BehavioralDescriptor, uint64_t> fitness_map_;
};

}  // namespace fuzzer
}  // namespace coralnpu

#endif  // SIM_BEHAVIORAL_ARCHIVE_H_