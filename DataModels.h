/**
 * @file DataModels.h
 * @brief Simple data structures for submissions, reviewers, parameters, and control settings.
 *
 * These are plain structs that hold the data parsed from the CSV input file.
 * No complex logic here — just containers for the information.
 */

#ifndef DATA_MODELS_H
#define DATA_MODELS_H

#include <string>
#include <vector>

/**
 * @brief Represents a paper submission to the conference.
 *
 * Each submission has a unique ID, metadata (title, authors, email),
 * and one or two topic identifiers indicating the paper's domain.
 */
struct Submission {
    int id;                  ///< Unique submission identifier
    std::string title;       ///< Paper title
    std::string authors;     ///< Author name(s)
    std::string email;       ///< Contact email
    int primaryTopic;        ///< Primary topic/domain ID (mandatory)
    int secondaryTopic;      ///< Secondary topic/domain ID (0 if not set)
};

/**
 * @brief Represents a reviewer (expert) who reviews submissions.
 *
 * Each reviewer has a unique ID, contact info, and one or two
 * areas of expertise identified by topic IDs.
 */
struct Reviewer {
    int id;                     ///< Unique reviewer identifier
    std::string name;           ///< Reviewer's name
    std::string email;          ///< Reviewer's email
    int primaryExpertise;       ///< Primary expertise topic ID (mandatory)
    int secondaryExpertise;     ///< Secondary expertise topic ID (0 if not set)
};

/**
 * @brief Problem parameters controlling the assignment constraints.
 *
 * These define how many reviews each submission needs and how many
 * each reviewer can handle.
 */
struct Parameters {
    int minReviewsPerSubmission = 1;  ///< Min number of reviewers per submission
    int maxReviewsPerReviewer = 1;    ///< Max number of submissions a reviewer can handle
    int primaryReviewerExpertise = 1; ///< Weight/flag for primary expertise matching
    int secondaryReviewerExpertise = 0; ///< Weight/flag for secondary expertise matching
    int primarySubmissionDomain = 1;  ///< Weight/flag for primary submission domain
    int secondarySubmissionDomain = 1; ///< Weight/flag for secondary submission domain
};

/**
 * @brief Control settings that determine what the tool does.
 *
 * These settings control which type of assignment to generate,
 * whether to perform risk analysis, and where to write output.
 */
struct Control {
    int generateAssignments = 1;         ///< 0=no report, 1=primary only, 2=primary+sec submission, 3=all
    int riskAnalysis = 0;                ///< 0=none, 1=single reviewer, K>1=K reviewers
    std::string outputFileName = "output.csv"; ///< Output file name
};

/**
 * @brief A single assignment pairing: one submission matched to one reviewer.
 *
 * Also records which topic domain was used to make the match
 * (useful for the output format).
 */
struct AssignmentResult {
    int submissionId;   ///< Which submission
    int reviewerId;     ///< Which reviewer
    int matchedDomain;  ///< The topic domain that matched
};

#endif // DATA_MODELS_H
