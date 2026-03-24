// reads the CSV input file section by section
// The file has 4 sections: #Submissions, #Reviewers, #Parameters, #Control
// We read line by line, figure out which section we are in, and fill the structs.
// O(n) where n = number of lines

#include "parser.h"
#include "DataModels.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <set>
#include <algorithm>

using namespace std;

// trim whitespace
static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// remove quotes around a string
static string removeQuotes(const string& s) {
    string t = trim(s);
    if (t.size() >= 2 && t.front() == '"' && t.back() == '"')
        return t.substr(1, t.size() - 2);
    return t;
}

// split line by commas, respecting quoted strings
static vector<string> split(const string& line) {
    vector<string> fields;
    string current;
    bool inQuotes = false;
    for (char c : line) {
        if (c == '"') { inQuotes = !inQuotes; current += c; }
        else if (c == ',' && !inQuotes) { fields.push_back(trim(current)); current.clear(); }
        else { current += c; }
    }
    fields.push_back(trim(current));
    return fields;
}

// remove inline comments (everything after # outside quotes)
static string stripComment(const string& line) {
    bool inQuote = false;
    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '"') inQuote = !inQuote;
        if (line[i] == '#' && !inQuote) return line.substr(0, i);
    }
    return line;
}

/// @brief Parse the input CSV file. O(n)
ParseResult parseInputFile(const string& filename) {
    ParseResult result;

    ifstream file(filename);
    if (!file.is_open()) {
        cout << "[ERROR] opening CSV file '" + filename + "'" << endl;
        return result;
    }

    string section = "";
    string line;
    set<int> subIds, revIds;
    bool errors = false;

    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;

        // section headers
        if (line[0] == '#') {
            string lower = line;
            transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            if (lower.find("#submissions") == 0)      section = "submissions";
            else if (lower.find("#reviewers") == 0)    section = "reviewers";
            else if (lower.find("#parameters") == 0)   section = "parameters";
            else if (lower.find("#control") == 0)      section = "control";
            continue;
        }

        string stripped = trim(stripComment(line));
        if (stripped.empty()) continue;
        vector<string> f = split(stripped);

        if (section == "submissions" && f.size() >= 5) {
            Submission sub;
            try {
                sub.id = stoi(f[0]);
                sub.title = removeQuotes(f[1]);
                sub.authors = removeQuotes(f[2]);
                sub.email = trim(f[3]);
                sub.primaryTopic = stoi(f[4]);
                sub.secondaryTopic = (f.size() > 5 && !f[5].empty()) ? stoi(f[5]) : -1;
            } catch (...) { cerr << "[ERROR] Bad submission: " << line << endl; errors = true; continue; }
            if (subIds.count(sub.id)) { cerr << "[ERROR] Duplicate submission ID: " << sub.id << endl; errors = true; continue; }
            subIds.insert(sub.id);
            result.submissions.push_back(sub);
        }
        else if (section == "reviewers" && f.size() >= 4) {
            Reviewer rev;
            try {
                rev.id = stoi(f[0]);
                rev.name = trim(f[1]);
                rev.email = trim(f[2]);
                rev.primaryExpertise = stoi(f[3]);
                rev.secondaryExpertise = (f.size() > 4 && !f[4].empty()) ? stoi(f[4]) : -1;
            } catch (...) { cerr << "[ERROR] Bad reviewer: " << line << endl; errors = true; continue; }
            if (revIds.count(rev.id)) { cerr << "[ERROR] Duplicate reviewer ID: " << rev.id << endl; errors = true; continue; }
            revIds.insert(rev.id);
            result.reviewers.push_back(rev);
        }
        else if (section == "parameters" && f.size() >= 2) {
            string key = f[0];
            try {
                int val = stoi(f[1]);
                if (key == "MinReviewsPerSubmission")       result.parameters.minReviewsPerSubmission = val;
                else if (key == "MaxReviewsPerReviewer")    result.parameters.maxReviewsPerReviewer = val;
                else if (key == "PrimaryReviewerExpertise") result.parameters.primaryReviewerExpertise = val;
                else if (key == "SecondaryReviewerExpertise") result.parameters.secondaryReviewerExpertise = val;
                else if (key == "PrimarySubmissionDomain")  result.parameters.primarySubmissionDomain = val;
                else if (key == "SecondarySubmissionDomain") result.parameters.secondarySubmissionDomain = val;
            } catch (...) { cerr << "[ERROR] Bad parameter: " << line << endl; errors = true; }
        }
        else if (section == "control" && f.size() >= 2) {
            string key = f[0], val = f[1];
            try {
                if (key == "GenerateAssignments")  result.control.generateAssignments = stoi(val);
                else if (key == "RiskAnalysis")    result.control.riskAnalysis = stoi(val);
                else if (key == "OutputFileName")  result.control.outputFileName = removeQuotes(val);
            } catch (...) { cerr << "[ERROR] Bad control: " << line << endl; errors = true; }
        }
    }

    if (result.submissions.empty()) { cerr << "[ERROR] No submissions found." << endl; errors = true; }
    if (result.reviewers.empty()) { cerr << "[ERROR] No reviewers found." << endl; errors = true; }

    result.success = !errors;
    return result;
}
