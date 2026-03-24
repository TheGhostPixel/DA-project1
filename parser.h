/**
* @file Parser.h
 * @brief Reads and parses the CSV input file.
 *
 * HOW PARSING WORKS:
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

/// @brief Everything we read from the input file.
struct ParseResult {
    std::vector<Submission> submissions;
    std::vector<Reviewer> reviewers;
    Parameters parameters;
    Control control;
    bool success = false;
};

/// @brief Parse the CSV file. Returns a ParseResult with all data. O(n) where n = lines
ParseResult parseInputFile(const std::string& filename);

#endif // PARSER_H
