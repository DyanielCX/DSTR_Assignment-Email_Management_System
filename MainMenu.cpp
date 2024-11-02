#include "MainMenu.h"
#include "Utils.h"
#include <iostream>
#include <chrono>
#include <thread>

void displayMainMenu() {
    bool isRunning = true;

    while (isRunning) {
        clearscreen();
        std::cout << "\033[36m";
        std::cout << "**************************************************\n";
        std::cout << "*                   Main Menu                    *\n";
        std::cout << "**************************************************\n";
        std::cout << "\033[0m";
        std::cout << "1. Inbox Management\n";
        std::cout << "2. Outbox Management\n";
        std::cout << "3. Search and Retrieval\n";
        std::cout << "4. Spam Messages\n";
        std::cout << "5. Edit Profile\n";
        std::cout << "6. Log Out\n";
        std::cout << "Choose an option: ";

        int choice;
        std::cin >> choice;

        switch (choice) {
        case 1:
            std::cout << "Inbox Management selected.\n";
            break;
        case 2:
            std::cout << "Outbox Management selected.\n";
            break;
        case 3:
            std::cout << "Search and Retrieval selected.\n";
            break;
        case 4:
            std::cout << "Spam Messages selected.\n";
            break;
        case 5:
            std::cout << "Edit Profile selected.\n";
            break;
        case 6:
            std::cout << "Logging out...\n";
            isRunning = false;
            break;
        default:
            std::cout << "\033[31mInvalid choice. Please try again.\033[0m\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            break;
        }

        if (isRunning) {
            std::cout << "Press Enter to return to the main menu...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
        }
    }
}
