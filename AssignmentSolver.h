//
// Created by ASUS on 3/22/2026.
//

// builds the flow network and gets assignments from max-flow

#ifndef ASSIGNMENTSOLVER_H
#define ASSIGNMENTSOLVER_H

#include <vector>
#include <string>
#include "DataModels.h"
#include "Graph.h"

/// @brief One assignment: submission <-> reviewer on a matched topic.
struct AssignmentPair {
    int submissionId;
    int reviewerId;
    int match;  // the topic that matched
};

/// @brief A submission that didn't get enough reviews.
struct MissingReview {
    int submissionId;
    int domain;
    int missingReviews;
};

/// @brief Result of running the assignment solver.
struct AssignmentResult {
    bool success = false;
    double maxFlow = 0;
    int requiredFlow = 0;

    std::vector<AssignmentPair> submissionToReviewer;  // sorted by submission
    std::vector<AssignmentPair> reviewerToSubmission;  // sorted by reviewer
    std::vector<MissingReview> missing;                // what's still needed
};

/// @brief Solve using only primary domains (mode 1). O(V * E^2)
AssignmentResult solvePrimaryAssignment(
    const std::vector<Submission>& submissions,
    const std::vector<Reviewer>& reviewers,
    const Parameters& parameters
);

/// @brief General solver for all modes. O(V * E^2)
/// genMode: 1=primary only, 2=+secondary submission domain, 3=all domains
AssignmentResult solveAssignment(
    const std::vector<Submission>& submissions,
    const std::vector<Reviewer>& reviewers,
    const Parameters& parameters,
    int genMode
);



#endif //ASSIGNMENTSOLVER_H
