/**
* @file main.cpp
 * @brief Entry point for the Scientific Conference Assignment Tool.
 *
 * Supports two execution modes:
 *   Interactive: command-line menu (options 1-4 implemented, 5-6 in T2.1/T2.2)
 *   Batch:       ./DA_Project_1 -b input.csv output.csv
 */

#include <iostream>
#include <fstream>
#include <string>

#include "parser.h"
#include "DataModels.h"

using namespace std;

// ============================================================
// INTERACTIVE MENU  (T1.1)
// ============================================================

static void interactiveMode() {
    cout << "============================================" << endl;
    cout << "  Scientific Conference Assignment Tool" << endl;
    cout << "============================================" << endl;

    ParseResult data;
    bool loaded = false;

    while (true) {
        cout << "\n--- Menu ---" << std::endl;
        cout << "1. Load input file" << std::endl;
        cout << "2. List submissions" << std::endl;
        cout << "3. List reviewers" << std::endl;
        cout << "4. Show parameters" << std::endl;
        cout << "5. Run assignment" << std::endl;
        cout << "6. Run risk analysis" << std::endl;
        cout << "0. Exit" << std::endl;
        cout << "Choice: ";

        int choice;
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input. Please enter a number." << std::endl;
            continue;
        };

        //  0: Exit
        if (choice == 0) {
            cout << "Goodbye!" << endl;
            break;
        }

        //  1: Load input file
        else if (choice == 1) {
            cout << "Enter filename (.csv): ";
            string filename;
            cin >> filename;

            ParseResult tmp = parseInputFile(filename);
            if (tmp.success) {
                data   = tmp;
                loaded = true;
                cout << "[OK] File loaded: " << filename << endl;
                cout << "     Submissions : " << data.submissions.size() << endl;
                cout << "     Reviewers   : " << data.reviewers.size()   << endl;
            } else {
                cout << "[ERROR] Could not load file. See errors above." << endl;
            }
        }

        // All other options need a file loaded first
        else if (!loaded) {
            cout << "[ERROR] No file loaded. Please use option 1 first." << endl;
        }

        //  2: List submissions
        else if (choice == 2) {
            cout << "\n--- Submissions (" << data.submissions.size() << ") ---" << endl;
            for (const auto& s : data.submissions) {
                cout << "  [" << s.id << "] " << s.title
                     << " | Topic: " << s.primaryTopic;
                if (s.secondaryTopic != -1)
                    cout << "/" << s.secondaryTopic;
                cout << " | " << s.authors << endl;
            }
        }

        // 3: List reviewers
        else if (choice == 3) {
            cout << "\n--- Reviewers (" << data.reviewers.size() << ") ---" << endl;
            for (const auto& r : data.reviewers) {
                cout << "  [" << r.id << "] " << r.name
                     << " | Expertise: " << r.primaryExpertise;
                if (r.secondaryExpertise != -1)
                    cout << "/" << r.secondaryExpertise;
                cout << " | " << r.email << endl;
            }
        }

        //  4: Show parameters
        else if (choice == 4) {
            cout << "\n--- Parameters ---" << endl;
            cout << "  MinReviewsPerSubmission   : " << data.parameters.minReviewsPerSubmission   << endl;
            cout << "  MaxReviewsPerReviewer     : " << data.parameters.maxReviewsPerReviewer     << endl;
            cout << "  PrimaryReviewerExpertise  : " << data.parameters.primaryReviewerExpertise  << endl;
            cout << "  SecondaryReviewerExpertise: " << data.parameters.secondaryReviewerExpertise << endl;
            cout << "  PrimarySubmissionDomain   : " << data.parameters.primarySubmissionDomain   << endl;
            cout << "  SecondarySubmissionDomain : " << data.parameters.secondarySubmissionDomain  << endl;
            cout << "\n--- Control ---" << endl;
            cout << "  GenerateAssignments: " << data.control.generateAssignments << endl;
            cout << "  RiskAnalysis       : " << data.control.riskAnalysis        << endl;
            cout << "  OutputFileName     : " << data.control.outputFileName      << endl;
        }

        //  5: Run assignment (T2.1)
        else if (choice == 5) {
            // TODO: implement in T2.1 using FlowNetwork
            cout << "[TODO] Assignment will be implemented in T2.1" << endl;
        }

        //  6: Run risk analysis (T2.2) ───────────────────────────────
        else if (choice == 6) {
            // TODO: implement in T2.2 using FlowNetwork
            cout << "[TODO] Risk analysis will be implemented in T2.2" << endl;
        }

        else {
            cout << "Unknown option. Please enter 0-6." << endl;
        }
    }
};


// ============================================================
// BATCH MODE  [T1.1]
// Runs without the interactive menu.
// All output goes to the output file.
// Error messages go to stderr.
// ============================================================

static int batchMode(const string& inputFile, const string& outputFile) {
    ParseResult data = parseInputFile(inputFile);
    if (!data.success) {
        cerr << "[ERROR] Failed to parse: " << inputFile << endl;
        return 1;
    }

    ofstream out(outputFile);
    if (!out.is_open()) {
        cerr << "[ERROR] Cannot open output file: " << outputFile << endl;
        return 1;
    }

    // TODO: call FlowNetwork here in T2.1
    // For now write a summary so the output file is not empty
    out << "# Input parsed successfully" << endl;
    out << "# Submissions: " << data.submissions.size() << endl;
    out << "# Reviewers: "   << data.reviewers.size()   << endl;
    out << "# GenerateAssignments: " << data.control.generateAssignments << endl;
    out << "# RiskAnalysis: "        << data.control.riskAnalysis        << endl;
    cout << "[OK] Parsed: " << data.submissions.size() << " submissions, "
         << data.reviewers.size() << " reviewers." << endl;
    cout << "[OK] Output file ready: " << outputFile << endl;

    out.close();
    return 0;
}


    int main(int argc, char* argv[]) {

        // Check for batch mode
        if (argc >= 3 && string(argv[1]) == "-b") {
            string inputFile  = argv[2];
            string outputFile = (argc >= 4) ? argv[3] : "output.csv";
            return batchMode(inputFile, outputFile);
        }

        // Otherwise run interactive menu
        interactiveMode();
        return 0;
    }


