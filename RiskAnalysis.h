//
// Created by ASUS on 3/22/2026.
//
//  check which reviewers are critical
// If removing one reviewer breaks the assignment, they are "risky"

#ifndef RISKANALYSIS_H
#define RISKANALYSIS_H
#include <vector>
#include "DataModels.h"

/// @brief Result: list of reviewer IDs whose absence would break the assignment.
struct RiskAnalysisResult {
    int riskLevel = 1;
    std::vector<int> riskyReviewerIds;
};

/// @brief For each reviewer, remove them and re-run. If it fails, they are risky. O(R * V * E^2)
RiskAnalysisResult runRiskAnalysis1(
    const std::vector<Submission>& submissions,
    const std::vector<Reviewer>& reviewers,
    const Parameters& parameters,
    int genMode = 1
);
#endif
