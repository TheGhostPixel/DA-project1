#include <iostream>
#include <string>

using namespace std;

// ============================================================
// INTERACTIVE MENU
// ============================================================
static void interactiveMode() {
    cout << "============================================" << endl;
    cout << "  Scientific Conference Assignment Tool" << endl;
    cout << "============================================" << endl;

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

        //Implement the next on on going
        if (choice == 0) {
            break;
        }
    }
};


int main() {
    interactiveMode();
    return 0;
}