#include "EmailCreator.hpp"
#include "InboxManager.hpp"
#include "OutboxManager.hpp"
#include "StarPrio.hpp"
#include "SearchRetrieval.hpp"
#include "SpamManager.hpp"
#include "EditProfile.hpp"
#include "Utils.hpp"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <thread>


using namespace std;

void displayMainMenu(const string& userEmail) {
    bool isRunning = true;
    EmailCreator emailCreator;
    InboxManager inboxManager;
    OutboxManager outboxManager;
    StarPrio starPrio;
    SpamManager spamManager;
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
        cout << "1. Send Email\n";
        cout << "2. Inbox Management\n";
        cout << "3. Outbox Management\n";
        cout << "4. Starred Email\n";
        cout << "5. Search and Retrieval\n";
        cout << "6. Spam Messages\n";
        cout << "7. Edit Profile\n";
        cout << "8. Log Out\n";
        cout << "Choose an option: ";

        int choice;
        cin >> choice;

        clearscreen();

        // Handle user choice
        switch (choice) {
        case 1:
            cout << "Send Email selected.\n";
            this_thread::sleep_for(chrono::seconds(1));
            clearscreen();

            emailCreator.displaySendEmail(userEmail);
            break;
        case 2:
            cout << "Inbox Management selected.\n";
            this_thread::sleep_for(chrono::seconds(1));
            clearscreen();

            inboxManager.displayInbox(userEmail);
            break;
        case 3:
            cout << "Outbox Management selected.\n";
            this_thread::sleep_for(chrono::seconds(1));
            clearscreen();

            outboxManager.displayOutbox(userEmail);
            break;
        case 4: {
            cout << "Starred Email selected.\n";
            this_thread::sleep_for(chrono::seconds(1));
            clearscreen();

            starPrio.displayStarEmail(userEmail);
            break;
        }
        case 5: {
            cout << "Search and Retrieval selected.\n";
            this_thread::sleep_for(chrono::seconds(1));
            clearscreen();
            
            SearchRetrieval searchRetrieval;
            searchRetrieval.displaySearchMenu(userEmail);
            break;
        }
        case 6:
            cout << "Spam Messages selected.\n";
            this_thread::sleep_for(chrono::seconds(1));
            clearscreen();

            spamManager.displaySpamEmails(userEmail);
            break;
        case 7:
            profileEditor.editProfileMenu(userEmail); // Open Edit Profile menu
            break;
        case 8:
            cout << color_red;
            cout << "\n**************************************************\n";
            cout << "*                 Logged Out                     *\n";
            cout << "**************************************************\n" << color_reset;
            this_thread::sleep_for(chrono::seconds(2));
            return;
        default:
            cout << "\033[31mInvalid choice. Please try again.\033[0m\n";
            this_thread::sleep_for(chrono::seconds(1));
            break;
        }

        
    }
}
