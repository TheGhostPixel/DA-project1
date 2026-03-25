/**
 * @file RiskAnalysis.cpp
 * @brief Risk analysis: identifies critical (risky) reviewers.
 *
 * For each reviewer, temporarily removes them from the pool and re-runs
 * the assignment solver. If the assignment fails without that reviewer,
 * they are considered "risky" — i.e., the system depends on them.
 *
 * @note Time complexity: O(R × V × E²) because we run Edmonds-Karp
 *       once per reviewer.
 */

#include "RiskAnalysis.h"
#include "AssignmentSolver.h"
#include <algorithm>

using namespace std;

RiskAnalysisResult runRiskAnalysis1(
    const vector<Submission>& submissions,
    const vector<Reviewer>& reviewers,
    const Parameters& parameters,
    int genMode
) {
    RiskAnalysisResult result;
    result.riskLevel = 1;

    for (int i = 0; i < (int)reviewers.size(); i++) {
        // make a list without this reviewer
        vector<Reviewer> reduced;
        for (int j = 0; j < (int)reviewers.size(); j++)
            if (j != i) reduced.push_back(reviewers[j]);

        // run the solver without them
        auto res = solveAssignment(submissions, reduced, parameters, genMode);

        // if it fails, this reviewer is critical
        if (!res.success)
            result.riskyReviewerIds.push_back(reviewers[i].id);
    }

    sort(result.riskyReviewerIds.begin(), result.riskyReviewerIds.end());
    return result;
}
