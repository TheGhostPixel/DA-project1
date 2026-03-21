#include "parser.h"
#include "DataModels.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <set>
#include <algorithm>

using namespace std;

// ============================================================
// HELPER: Trim whitespace from both ends of a string
// ============================================================
static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// ============================================================
// HELPER: Remove surrounding quotes from a string
// e.g., "\"hello\"" → "hello"
// ============================================================
static std::string removeQuotes(const std::string& s) {
    std::string t = trim(s);
    if (t.size() >= 2 && t.front() == '"' && t.back() == '"')
        return t.substr(1, t.size() - 2);
    return t;
}

// ============================================================
// HELPER: Split a line by commas, respecting quoted strings
// e.g., '31, "Hello, World", 3' → ["31", "Hello, World", "3"]
// ============================================================
static std::vector<std::string> split(const std::string& line) {
    vector<string> fields;
    string current;
    bool inQuotes = false;

    for (char c : line) {
        if (c == '"') {
            inQuotes = !inQuotes;
            current += c;
        } else if (c == ',' && !inQuotes) {
            fields.push_back(trim(current));
            current.clear();
        } else {
            current += c;
        }
    }
    fields.push_back(trim(current));
    return fields;
}

// ============================================================
// HELPER: Remove everything after '#' unless inside quotes
// Needed for inline comments like: "MinReviews, 5 # comment"
// ============================================================
static std::string stripComment(const std::string& line) {
    bool inQuote = false;
    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '"') inQuote = !inQuote;
        if (line[i] == '#' && !inQuote) return line.substr(0, i);
    }
    return line;
}


/**
 * @brief Parse the input CSV file and return all data.
 * @param filename Path to the .csv input file
 * @return ParseResult with all parsed data and a success flag
 * @complexity O(n) where n = number of lines in the file
 */
ParseResult parseInputFile(const std::string& filename) {
    ParseResult result;

    // Open File //
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "[ERROR] opening CSV file '" + filename +"'" << endl;
        return result;
    }
    string section = "";
    string line;
    set<int> subIds;  // to detect duplicate submission IDs
    set<int> revIds;  // to detect duplicate reviewer IDs
    bool errors = false;

    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }

        // Lines starting with '#' → check if it's a section header or skip
        if (line[0] == '#') {
            // Convert to lowercase to do case-insensitive comparison
            std::string lower = line;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

            // Check if it matches a known section header
            if (lower.find("#submissions") == 0)      section = "submissions";
            else if (lower.find("#reviewers") == 0)    section = "reviewers";
            else if (lower.find("#parameters") == 0)   section = "parameters";
            else if (lower.find("#control") == 0)      section = "control";
            continue;
        }

        // Strip inline comments before splitting
        std::string stripped = trim(stripComment(line));
        if (stripped.empty()) continue;

        vector<string> f = split(stripped);



        if (section == "submissions" && f.size() >= 5) {
            // Format: Id, Title, Authors, E-mail, Primary, Secondary
            Submission sub;
            try {
                sub.id = std::stoi(f[0]);
                sub.title = removeQuotes(f[1]);
                sub.authors = removeQuotes(f[2]);
                sub.email = trim(f[3]);
                sub.primaryTopic = std::stoi(f[4]);
                sub.secondaryTopic = (f.size() > 5 && !f[5].empty()) ? std::stoi(f[5]) : -1;
            } catch (...) {
                cerr << "[ERROR] Bad data in submissions: " << line << endl;
                errors = true;
                continue;
            }

            // Check for duplicate submission IDs
            if (subIds.count(sub.id)) {
                cerr << "[ERROR] Duplicate submission ID: " << sub.id << endl;
                errors = true;
                continue;
            }
            subIds.insert(sub.id);
            result.submissions.push_back(sub);
        }
        else if (section == "reviewers" && f.size() >= 4) {
            // Format: Id, Name, E-mail, Primary, Secondary
            Reviewer rev;
            try {
                rev.id = std::stoi(f[0]);
                rev.name = trim(f[1]);
                rev.email = trim(f[2]);
                rev.primaryExpertise = std::stoi(f[3]);
                // FIX: use -1 for "no secondary" (not 0, since 0 is a valid topic ID)
                rev.secondaryExpertise = (f.size() > 4 && !f[4].empty()) ? std::stoi(f[4]) : -1;
            } catch (...) {
                cerr << "[ERROR] Bad data in reviewers: " << line << endl;
                errors = true;
                continue;
            }

            // Check for duplicate reviewer IDs
            if (revIds.count(rev.id)) {
                cerr << "[ERROR] Duplicate reviewer ID: " << rev.id << endl;
                errors = true;
                continue;
            }
            revIds.insert(rev.id);
            result.reviewers.push_back(rev);
        }
        else if (section == "parameters" && f.size() >= 2) {
            // Format: ParameterName, Value
            std::string key = f[0];
            try {
                int val = std::stoi(f[1]);
                if (key == "MinReviewsPerSubmission")       result.parameters.minReviewsPerSubmission = val;
                else if (key == "MaxReviewsPerReviewer")    result.parameters.maxReviewsPerReviewer = val;
                else if (key == "PrimaryReviewerExpertise") result.parameters.primaryReviewerExpertise = val;
                else if (key == "SecondaryReviewerExpertise") result.parameters.secondaryReviewerExpertise = val;
                else if (key == "PrimarySubmissionDomain")  result.parameters.primarySubmissionDomain = val;
                else if (key == "SecondarySubmissionDomain") result.parameters.secondarySubmissionDomain = val;
            } catch (...) {
                cerr << "[ERROR] Bad parameter value: " << line << endl;
                errors = true;
            }
        }
        else if (section == "control" && f.size() >= 2) {
            // Format: ControlName, Value
            std::string key = f[0];
            std::string val = f[1];
            try {
                if (key == "GenerateAssignments")      result.control.generateAssignments = std::stoi(val);
                else if (key == "RiskAnalysis")        result.control.riskAnalysis = std::stoi(val);
                else if (key == "OutputFileName")      result.control.outputFileName = removeQuotes(val);
            } catch (...) {
                cerr << "[ERROR] Bad control value: " << line << endl;
                errors = true;
            }
        }
    }

    // Basic validation
    if (result.submissions.empty()) {
        cerr << "[ERROR] No submissions found in file." << endl;
        errors = true;
    }
    if (result.reviewers.empty()) {
        cerr << "[ERROR] No reviewers found in file." << endl;
        errors = true;
    }

    // FIX: set success flag (was always false before)
    result.success = !errors;
    return result;
}