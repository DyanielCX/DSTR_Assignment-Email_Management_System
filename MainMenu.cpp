#include "EditProfile.h"
#include "Utils.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <thread>

using namespace std;

void displayMainMenu(const string& userEmail) {
    bool isRunning = true;
    ProfileEditor profileEditor;

    while (isRunning) {
        clearscreen();

        // Display the Main Menu
        cout << "\033[36m";
        cout << "**************************************************\n";
        cout << "*                   Email System                 *\n";
        cout << "**************************************************\n";
        cout << "\033[0m";
        cout << "\033[33mLogged in as: " << userEmail << "\033[0m\n";

        // Display current time using localtime_s to avoid C4996 warning
        auto now = chrono::system_clock::now();
        time_t currentTime = chrono::system_clock::to_time_t(now);
        tm localTime;

#ifdef _WIN32
        localtime_s(&localTime, &currentTime); // Safe version for Windows
#else
        localtime_r(&currentTime, &localTime); // POSIX-compliant for Linux/Unix
#endif

        cout << "\033[33mCurrent Time: " << put_time(&localTime, "%Y-%m-%d %H:%M:%S") << "\033[0m\n";
        cout << "--------------------------------------------------\n";

        // Display menu options
        cout << "1. Inbox Management\n";
        cout << "2. Outbox Management\n";
        cout << "3. Search and Retrieval\n";
        cout << "4. Spam Messages\n";
        cout << "5. Edit Profile\n";
        cout << "6. Log Out\n";
        cout << "Choose an option: ";

        int choice;
        cin >> choice;

        clearscreen();

        // Handle user choice
        switch (choice) {
        case 1:
            cout << "Inbox Management selected.\n";
            break;
        case 2:
            cout << "Outbox Management selected.\n";
            break;
        case 3:
            cout << "Search and Retrieval selected.\n";
            break;
        case 4:
            cout << "Spam Messages selected.\n";
            break;
        case 5:
            profileEditor.editProfileMenu(userEmail); // Open Edit Profile menu
            break;
        case 6:
            cout << "Logging out...\n";
            this_thread::sleep_for(chrono::seconds(1));
            isRunning = false;
            break;
        default:
            cout << "\033[31mInvalid choice. Please try again.\033[0m\n";
            this_thread::sleep_for(chrono::seconds(1));
            break;
        }

        
    }
}
