/**
* @file Parser.h
 * @brief Reads and parses the CSV input file.
 *
 * HOW PARSING WORKS:
 * ==================
 * The input file has 4 sections, each starting with a # marker:
 *   #Submissions  → list of paper submissions
 *   #Reviewers    → list of reviewers
 *   #Parameters   → problem constraints (e.g., min reviews per submission)
 *   #Control      → what the tool should do (e.g., generate assignments, risk analysis)
 *
 * Lines starting with # are either section headers or section-ending markers.
 * Within each section, data is comma-separated.
 *
 * The parser reads line by line, detects which section it's in,
 * and fills in the appropriate data structures.
 */

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include "DataModels.h"

/**
 * @brief Parse result: contains all data read from the input CSV file.
 */
struct ParseResult {
    std::vector<Submission> submissions;  ///< All submissions
    std::vector<Reviewer> reviewers;      ///< All reviewers
    Parameters parameters;                ///< Problem parameters
    Control control;                      ///< Control settings
    bool success = false;                 ///< Whether parsing succeeded
};

/**
 * @brief Parse the input CSV file and return all data.
 * @param filename Path to the .csv input file
 * @return ParseResult containing submissions, reviewers, parameters, control, and status
 *
 * Time complexity: O(n) where n is the number of lines in the file
 */
ParseResult parseInputFile(const std::string& filename);

#endif // PARSER_H
