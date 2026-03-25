/**
 * @file AssignmentSolver.cpp
 * @brief Builds a flow network and uses Edmonds-Karp to assign reviewers to submissions.
 *
 * The flow network has this structure:
 * ```
 * Source → Submissions → Reviewers → Sink
 * ```
 * - Source → each submission: capacity = minReviewsPerSubmission
 * - Submission → reviewer: capacity = 1 (only if topics match)
 * - Reviewer → sink: capacity = maxReviewsPerReviewer
 *
 * The max-flow value tells us how many review assignments can be made.
 * If max-flow == required flow, all submissions get enough reviews.
 *
 * @note Time complexity: O(V × E²) per solve (dominated by Edmonds-Karp).
 */

#include "AssignmentSolver.h"
#include "MaxFlow.h"
#include <algorithm>
#include <map>

using namespace std;

/**
 * @brief Solve using only primary topic matching (mode 1).
 *
 * A reviewer can review a submission only if their primary expertise
 * matches the submission's primary topic.
 */
AssignmentResult solvePrimaryAssignment(
    const vector<Submission>& submissions,
    const vector<Reviewer>& reviewers,
    const Parameters& parameters
) {
    AssignmentResult result;
    Graph<int> g;

    int nSub = (int)submissions.size();
    int nRev = (int)reviewers.size();
    int source = 0;
    int firstSub = 1;
    int firstRev = firstSub + nSub;
    int sink = firstRev + nRev;

    // create nodes
    for (int i = 0; i <= sink; i++) g.addVertex(i);

    // source -> submissions
    for (int i = 0; i < nSub; i++)
        g.addFlowEdge(source, firstSub + i, parameters.minReviewsPerSubmission);

    // submissions -> reviewers (if primary topic matches)
    for (int i = 0; i < nSub; i++)
        for (int j = 0; j < nRev; j++)
            if (submissions[i].primaryTopic == reviewers[j].primaryExpertise)
                g.addFlowEdge(firstSub + i, firstRev + j, 1);

    // reviewers -> sink
    for (int j = 0; j < nRev; j++)
        g.addFlowEdge(firstRev + j, sink, parameters.maxReviewsPerReviewer);

    // run max-flow
    result.requiredFlow = nSub * parameters.minReviewsPerSubmission;
    result.maxFlow = edmondsKarp(g, source, sink);
    result.success = (result.maxFlow == result.requiredFlow);

    // read assignments from the flow
    for (int i = 0; i < nSub; i++) {
        auto v = g.findVertex(firstSub + i);
        int assigned = 0;

        for (auto e : v->getAdj()) {
            int dest = e->getDest()->getInfo();
            if (dest >= firstRev && dest < sink && e->getFlow() > 0) {
                int rIdx = dest - firstRev;
                AssignmentPair a;
                a.submissionId = submissions[i].id;
                a.reviewerId = reviewers[rIdx].id;
                a.match = submissions[i].primaryTopic;
                result.submissionToReviewer.push_back(a);
                result.reviewerToSubmission.push_back(a);
                assigned += (int)e->getFlow();
            }
        }
        if (assigned < parameters.minReviewsPerSubmission) {
            MissingReview m;
            m.submissionId = submissions[i].id;
            m.domain = submissions[i].primaryTopic;
            m.missingReviews = parameters.minReviewsPerSubmission - assigned;
            result.missing.push_back(m);
        }
    }

    // sort everything
    sort(result.submissionToReviewer.begin(), result.submissionToReviewer.end(),
         [](auto& a, auto& b) { return a.submissionId != b.submissionId ? a.submissionId < b.submissionId : a.reviewerId < b.reviewerId; });
    sort(result.reviewerToSubmission.begin(), result.reviewerToSubmission.end(),
         [](auto& a, auto& b) { return a.reviewerId != b.reviewerId ? a.reviewerId < b.reviewerId : a.submissionId < b.submissionId; });
    sort(result.missing.begin(), result.missing.end(),
         [](auto& a, auto& b) { return a.submissionId != b.submissionId ? a.submissionId < b.submissionId : a.domain < b.domain; });

    return result;
}


/**
 * @brief Check if a reviewer's expertise matches a submission's topic.
 *
 * The matching rules depend on the generation mode (genMode):
 * - **Mode 1**: Only primary topic vs primary expertise.
 * - **Mode 2**: Primary + secondary submission topic vs primary expertise.
 * - **Mode 3**: All combinations of primary/secondary on both sides.
 *
 * @param sub         The submission to check.
 * @param rev         The reviewer to check.
 * @param genMode     Which matching mode to use (1, 2, or 3).
 * @param matchDomain Output: the topic ID that matched (if any).
 * @return true if there is a matching topic, false otherwise.
 */
static bool topicMatches(const Submission& sub, const Reviewer& rev, int genMode, int& matchDomain) {
    if (genMode <= 1) {
        if (sub.primaryTopic == rev.primaryExpertise) { matchDomain = sub.primaryTopic; return true; }
        return false;
    }
    // mode 2: primary + secondary submission domain vs primary reviewer
    if (genMode == 2) {
        if (sub.primaryTopic == rev.primaryExpertise) { matchDomain = sub.primaryTopic; return true; }
        if (sub.secondaryTopic > 0 && sub.secondaryTopic == rev.primaryExpertise) { matchDomain = sub.secondaryTopic; return true; }
        return false;
    }
    // mode 3: all combinations
    if (sub.primaryTopic == rev.primaryExpertise) { matchDomain = sub.primaryTopic; return true; }
    if (rev.secondaryExpertise > 0 && sub.primaryTopic == rev.secondaryExpertise) { matchDomain = sub.primaryTopic; return true; }
    if (sub.secondaryTopic > 0 && sub.secondaryTopic == rev.primaryExpertise) { matchDomain = sub.secondaryTopic; return true; }
    if (sub.secondaryTopic > 0 && rev.secondaryExpertise > 0 && sub.secondaryTopic == rev.secondaryExpertise) { matchDomain = sub.secondaryTopic; return true; }
    return false;
}

/**
 * @brief General assignment solver supporting all matching modes.
 *
 * Delegates to solvePrimaryAssignment for mode 1. For modes 2 and 3,
 * builds the flow network with broader topic matching.
 */
AssignmentResult solveAssignment(
    const vector<Submission>& submissions,
    const vector<Reviewer>& reviewers,
    const Parameters& parameters,
    int genMode
) {
    if (genMode <= 1) return solvePrimaryAssignment(submissions, reviewers, parameters);

    AssignmentResult result;
    Graph<int> g;
    int nSub = (int)submissions.size();
    int nRev = (int)reviewers.size();
    int source = 0, firstSub = 1, firstRev = firstSub + nSub, sink = firstRev + nRev;

    for (int i = 0; i <= sink; i++) g.addVertex(i);
    for (int i = 0; i < nSub; i++) g.addFlowEdge(source, firstSub + i, parameters.minReviewsPerSubmission);

    // track which domain matched for each edge
    map<pair<int,int>, int> matchedDomains;
    for (int i = 0; i < nSub; i++)
        for (int j = 0; j < nRev; j++) {
            int md = 0;
            if (topicMatches(submissions[i], reviewers[j], genMode, md)) {
                g.addFlowEdge(firstSub + i, firstRev + j, 1);
                matchedDomains[{i, j}] = md;
            }
        }

    for (int j = 0; j < nRev; j++) g.addFlowEdge(firstRev + j, sink, parameters.maxReviewsPerReviewer);

    result.requiredFlow = nSub * parameters.minReviewsPerSubmission;
    result.maxFlow = edmondsKarp(g, source, sink);
    result.success = (result.maxFlow == result.requiredFlow);

    for (int i = 0; i < nSub; i++) {
        auto v = g.findVertex(firstSub + i);
        int assigned = 0;
        for (auto e : v->getAdj()) {
            int dest = e->getDest()->getInfo();
            if (dest >= firstRev && dest < sink && e->getFlow() > 0) {
                int rIdx = dest - firstRev;
                AssignmentPair a;
                a.submissionId = submissions[i].id;
                a.reviewerId = reviewers[rIdx].id;
                a.match = matchedDomains[{i, rIdx}];
                result.submissionToReviewer.push_back(a);
                result.reviewerToSubmission.push_back(a);
                assigned += (int)e->getFlow();
            }
        }
        if (assigned < parameters.minReviewsPerSubmission) {
            MissingReview m; m.submissionId = submissions[i].id; m.domain = submissions[i].primaryTopic;
            m.missingReviews = parameters.minReviewsPerSubmission - assigned;
            result.missing.push_back(m);
        }
    }

    sort(result.submissionToReviewer.begin(), result.submissionToReviewer.end(),
         [](auto& a, auto& b) { return a.submissionId != b.submissionId ? a.submissionId < b.submissionId : a.reviewerId < b.reviewerId; });
    sort(result.reviewerToSubmission.begin(), result.reviewerToSubmission.end(),
         [](auto& a, auto& b) { return a.reviewerId != b.reviewerId ? a.reviewerId < b.reviewerId : a.submissionId < b.submissionId; });
    sort(result.missing.begin(), result.missing.end(),
         [](auto& a, auto& b) { return a.submissionId != b.submissionId ? a.submissionId < b.submissionId : a.domain < b.domain; });

    return result;
}
