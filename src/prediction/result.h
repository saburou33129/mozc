// Copyright 2010-2021, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef MOZC_PREDICTION_RESULT_H_
#define MOZC_PREDICTION_RESULT_H_

#include <string>
#include <utility>
#include <vector>

#include "dictionary/dictionary_token.h"
#include "prediction/zero_query_dict.h"

namespace mozc {
namespace prediction {

enum PredictionType {
  // don't need to show any suggestions.
  NO_PREDICTION = 0,
  // suggests from current key user is now typing
  UNIGRAM = 1,
  // suggests from the previous history key user typed before.
  BIGRAM = 2,
  // suggests from immutable_converter
  REALTIME = 4,
  // add suffixes like "さん", "が" which matches to the pevious context.
  SUFFIX = 8,
  // add English words.
  ENGLISH = 16,
  // add prediction to type corrected keys
  TYPING_CORRECTION = 32,
  // prefix candidates
  // "今日", "教" for the input, "きょうは"
  PREFIX = 64,

  NUMBER = 128,

  // Suggests from |converter_|. The difference from REALTIME is that it uses
  // the full converter with rewriter, history, etc.
  // TODO(noriyukit): This label should be integrated with REALTIME. This is
  // why 65536 is used to indicate that it is a temporary assignment.
  REALTIME_TOP = 65536,
};
// Bitfield to store a set of PredictionType.
using PredictionTypes = int32_t;
using ZeroQueryResult = std::pair<std::string, ZeroQueryType>;

struct Result {
  Result()
      : types(NO_PREDICTION),
        wcost(0),
        cost(0),
        lid(0),
        rid(0),
        candidate_attributes(0),
        source_info(0),
        consumed_key_size(0),
        removed(false) {}

  void InitializeByTokenAndTypes(const dictionary::Token &token,
                                 PredictionTypes types);
  void SetTypesAndTokenAttributes(
      PredictionTypes prediction_types,
      dictionary::Token::AttributesBitfield token_attr);
  void SetSourceInfoForZeroQuery(ZeroQueryType zero_query_type);
  bool IsUserDictionaryResult() const;

  std::string key;
  std::string value;
  // Indicating which PredictionType creates this instance.
  // UNIGRAM, BIGRAM, REALTIME, SUFFIX, ENGLISH or TYPING_CORRECTION
  // is set exclusively.
  PredictionTypes types;
  // Context "insensitive" candidate cost.
  int wcost;
  // Context "sensitive" candidate cost.
  int cost;
  int lid;
  int rid;
  uint32_t candidate_attributes;
  // Boundary information for realtime conversion.
  // This will be set only for realtime conversion result candidates.
  // This contains inner segment size for key and value.
  // If the candidate key and value are
  // "わたしの|なまえは|なかのです", " 私の|名前は|中野です",
  // |inner_segment_boundary| have [(4,2), (4, 3), (5, 4)].
  std::vector<uint32_t> inner_segment_boundary;
  // Segment::Candidate::SourceInfo.
  // Will be used for usage stats.
  uint32_t source_info;
  size_t consumed_key_size;
  // If removed is true, this result is not used for a candidate.
  bool removed;
#ifndef NDEBUG
  std::string log;
#endif  // NDEBUG
};

// Comparator for sorting prediction candidates.
// If we have words A and AB, for example "六本木" and "六本木ヒルズ",
// assume that cost(A) < cost(AB).
class ResultWCostLess {
 public:
  bool operator()(const Result &lhs, const Result &rhs) const {
    if (lhs.wcost != rhs.wcost) {
      return lhs.wcost < rhs.wcost;
    }
    return lhs.value < rhs.value;
  }
};

class ResultCostLess {
 public:
  bool operator()(const Result &lhs, const Result &rhs) const {
    if (lhs.cost != rhs.cost) {
      return lhs.cost > rhs.cost;
    }
    return lhs.value < rhs.value;
  }
};

}  // namespace prediction
}  // namespace mozc

#endif  // MOZC_PREDICTION_RESULT_H_