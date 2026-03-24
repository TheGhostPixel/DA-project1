//
// Created by ASUS on 3/22/2026.
//
// builds Source -> Submissions -> Reviewers -> Sink
// and runs Edmonds-Karp to assign reviewers to submissions.
// O(V * E^2)

#include "AssignmentSolver.h"
#include "MaxFlow.h"
#include <algorithm>
#include <map>

using namespace std;

// Solve using only primary topics (mode 1)
// Network: source -> each submission (cap = minReviews)
//          submission -> reviewer if topics match (cap = 1)
//          reviewer -> sink (cap = maxReviews)
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


// Check if a reviewer can review a submission based on the mode
static bool topicMatches(const Submission& sub, const Reviewer& rev, int genMode, int& matchDomain) {
    // mode 1: primary only
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

// General solver - same idea as above but supports modes 2 and 3
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
