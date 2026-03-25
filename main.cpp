/**
 * @file main.cpp
 * @brief Entry point for the Conference Assignment Tool.
 *
 * Supports two modes:
 * - **Interactive mode**: a text menu where the user can load a file,
 *   view data, run the assignment solver, and do risk analysis step by step.
 * - **Batch mode**: pass `-b input.csv [output.csv]` on the command line
 *   to run everything automatically and write the result to a file.
 */

#include <iostream>
#include <fstream>
#include <string>
#include "parser.h"
#include "DataModels.h"
#include "RiskAnalysis.h"
#include "AssignmentSolver.h"

using namespace std;

/**
 * @brief Write assignment results (and optionally risk analysis) to any output stream.
 *
 * This helper is used both for printing to the console and writing to a file,
 * so we don't duplicate the output formatting code.
 *
 * @param out         The output stream (e.g. cout or an ofstream).
 * @param res         The assignment result (pairings + missing reviews).
 * @param riskResult  The risk analysis result (risky reviewer IDs).
 * @param doRisk      If true, also write the risk analysis section.
 */
static void writeResults(ostream& out,
                         const AssignmentResult& res,
                         const RiskAnalysisResult& riskResult,
                         bool doRisk)
{
    // always write assignments (even if partial)
    out << "#SubmissionId,ReviewerId,Match" << endl;
    for (const auto& a : res.submissionToReviewer)
        out << a.submissionId << ", " << a.reviewerId << ", " << a.match << endl;

    out << "#ReviewerId,SubmissionId,Match" << endl;
    for (const auto& a : res.reviewerToSubmission)
        out << a.reviewerId << ", " << a.submissionId << ", " << a.match << endl;

    out << "#Total: " << res.submissionToReviewer.size() << endl;

    // if some submissions didn't get enough reviews
    if (!res.missing.empty()) {
        out << "#SubmissionId,Domain,MissingReviews" << endl;
        for (const auto& m : res.missing)
            out << m.submissionId << ", " << m.domain << ", " << m.missingReviews << endl;
    }

    // risk analysis results
    if (doRisk) {
        out << "#Risk Analysis: " << riskResult.riskLevel << endl;
        if (!riskResult.riskyReviewerIds.empty()) {
            for (size_t i = 0; i < riskResult.riskyReviewerIds.size(); i++) {
                if (i > 0) out << ", ";
                out << riskResult.riskyReviewerIds[i];
            }
            out << endl;
        }
    }
}

/**
 * @brief Interactive menu mode.
 *
 * Presents a text-based menu where the user can:
 * 1. Load an input CSV file
 * 2. List submissions
 * 3. List reviewers
 * 4. Show parameters
 * 5. Run the assignment solver
 * 6. Run risk analysis
 */
static void interactiveMode() {
    cout << "============================================" << endl;
    cout << "  Scientific Conference Assignment Tool" << endl;
    cout << "============================================" << endl;

    ParseResult data;
    bool loaded = false;

    while (true) {
        cout << "\n--- Menu ---" << endl;
        cout << "1. Load input file" << endl;
        cout << "2. List submissions" << endl;
        cout << "3. List reviewers" << endl;
        cout << "4. Show parameters" << endl;
        cout << "5. Run assignment" << endl;
        cout << "6. Run risk analysis" << endl;
        cout << "0. Exit" << endl;
        cout << "Choice: ";

        int choice;
        cin >> choice;
        if (cin.fail()) { cin.clear(); cin.ignore(10000, '\n'); cout << "Invalid input." << endl; continue; }

        if (choice == 0) { cout << "Goodbye!" << endl; break; }

        else if (choice == 1) {
            cout << "Enter filename (with .csv): ";
            string filename; cin >> filename;
            ParseResult tmp = parseInputFile(filename);
            if (tmp.success) {
                data = tmp; loaded = true;
                cout << "[OK] Loaded: " << data.submissions.size() << " submissions, " << data.reviewers.size() << " reviewers." << endl;
            } else { cout << "[ERROR] Could not load file." << endl; }
        }

        else if (!loaded) { cout << "[ERROR] Load a file first (option 1)." << endl; }

        else if (choice == 2) {
            cout << "\n--- Submissions (" << data.submissions.size() << ") ---" << endl;
            for (const auto& s : data.submissions) {
                cout << "  [" << s.id << "] " << s.title << " | Topic: " << s.primaryTopic;
                if (s.secondaryTopic != -1) cout << "/" << s.secondaryTopic;
                cout << " | " << s.authors << endl;
            }
        }
        else if (choice == 3) {
            cout << "\n--- Reviewers (" << data.reviewers.size() << ") ---" << endl;
            for (const auto& r : data.reviewers) {
                cout << "  [" << r.id << "] " << r.name << " | Expertise: " << r.primaryExpertise;
                if (r.secondaryExpertise != -1) cout << "/" << r.secondaryExpertise;
                cout << " | " << r.email << endl;
            }
        }
        else if (choice == 4) {
            cout << "\n  MinReviewsPerSubmission: " << data.parameters.minReviewsPerSubmission << endl;
            cout << "  MaxReviewsPerReviewer: " << data.parameters.maxReviewsPerReviewer << endl;
            cout << "  GenerateAssignments: " << data.control.generateAssignments << endl;
            cout << "  RiskAnalysis: " << data.control.riskAnalysis << endl;
            cout << "  OutputFileName: " << data.control.outputFileName << endl;
        }
        else if (choice == 5) {
            int gm = data.control.generateAssignments; if (gm == 0) gm = 1;
            auto res = solveAssignment(data.submissions, data.reviewers, data.parameters, gm);
            cout << "\nMax flow: " << res.maxFlow << " / " << res.requiredFlow << (res.success ? " [OK]" : " [PARTIAL]") << endl;
            RiskAnalysisResult empty;
            writeResults(cout, res, empty, false);
            // save to file
            string outFile = data.control.outputFileName.empty() ? "output.csv" : data.control.outputFileName;
            ofstream out(outFile);
            if (out.is_open()) { writeResults(out, res, empty, false); out.close(); cout << "[OK] Saved to " << outFile << endl; }
        }
        else if (choice == 6) {
            if (data.control.riskAnalysis < 1) { cout << "[INFO] RiskAnalysis = 0, skipping." << endl; continue; }
            int gm = data.control.generateAssignments; if (gm == 0) gm = 1;
            auto res = solveAssignment(data.submissions, data.reviewers, data.parameters, gm);
            auto risk = runRiskAnalysis1(data.submissions, data.reviewers, data.parameters, gm);
            writeResults(cout, res, risk, true);
            string outFile = data.control.outputFileName.empty() ? "output.csv" : data.control.outputFileName;
            ofstream out(outFile);
            if (out.is_open()) { writeResults(out, res, risk, true); out.close(); cout << "[OK] Saved to " << outFile << endl; }
        }
        else { cout << "Unknown option." << endl; }
    }
}

/**
 * @brief Batch mode: parse input, solve, and write output — no user interaction.
 *
 * @param inputFile  Path to the CSV input file.
 * @param outputFile Path to the output file (default: "output.csv").
 * @return 0 on success, 1 on error.
 */
static int batchMode(const string& inputFile, const string& outputFile) {
    ParseResult data = parseInputFile(inputFile);
    if (!data.success) { cerr << "[ERROR] Failed to parse: " << inputFile << endl; return 1; }

    ofstream out(outputFile);
    if (!out.is_open()) { cerr << "[ERROR] Cannot write: " << outputFile << endl; return 1; }

    int gm = data.control.generateAssignments; if (gm == 0) gm = 1;
    auto res = solveAssignment(data.submissions, data.reviewers, data.parameters, gm);

    bool doRisk = (data.control.riskAnalysis >= 1);
    RiskAnalysisResult riskResult;
    if (doRisk) riskResult = runRiskAnalysis1(data.submissions, data.reviewers, data.parameters, gm);

    writeResults(out, res, riskResult, doRisk);
    out.close();
    cout << "[OK] Output: " << outputFile << endl;
    return 0;
}

/**
 * @brief Program entry point.
 *
 * Usage:
 * - `./DA_Project_1`                      → interactive mode
 * - `./DA_Project_1 -b input.csv [out.csv]` → batch mode
 */
int main(int argc, char* argv[]) {
    if (argc >= 3 && string(argv[1]) == "-b") {
        string in = argv[2];
        string out = (argc >= 4) ? argv[3] : "output.csv";
        return batchMode(in, out);
    }
    interactiveMode();
    return 0;
}
