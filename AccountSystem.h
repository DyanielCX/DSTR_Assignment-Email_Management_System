#ifndef ACCOUNTSYSTEM_H
#define ACCOUNTSYSTEM_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>
#include "Utils.h" 
#include "MainMenu.h" 

// Color definitions
const std::string color_reset = "\033[0m";
const std::string color_red = "\033[31m";
const std::string color_green = "\033[32m";
const std::string color_yellow = "\033[33m";
const std::string color_cyan = "\033[36m";
const std::string color_blue = "\033[34m";

// Account and Node structure for the hash table
struct Account {
    std::string email;
    std::string password;
};

struct Node {
    Account account;
    Node* next;

    Node(const Account& acc) : account(acc), next(nullptr) {}
};

// Custom Hash Table for storing accounts
class CustomHashTable {
private:
    static const int table_size = 100;
    Node* table[table_size] = { nullptr };

    int hash(const std::string& email) const {
        int hashvalue = 0;
        for (char ch : email) {
            hashvalue = (hashvalue * 31 + ch) % table_size;
        }
        return hashvalue;
    }

public:
    ~CustomHashTable() {
        for (int i = 0; i < table_size; ++i) {
            Node* current = table[i];
            while (current != nullptr) {
                Node* temp = current;
                current = current->next;
                delete temp;
            }
        }
    }

    bool insert(const Account& account) {
        int index = hash(account.email);
        Node* newnode = new Node(account);

        Node* current = table[index];
        while (current != nullptr) {
            if (current->account.email == account.email) {
                delete newnode;
                return false;
            }
            current = current->next;
        }

        newnode->next = table[index];
        table[index] = newnode;
        return true;
    }

    Node* find(const std::string& email) const {
        int index = hash(email);
        Node* current = table[index];
        while (current != nullptr) {
            if (current->account.email == email) {
                return current;
            }
            current = current->next;
        }
        return nullptr;
    }
};

// Account Management class for handling sign-up, login, and menu display
class AccountManagement {
private:
    CustomHashTable accountstable;

public:
    void loadAccountsFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << color_red << "Error: Could not open file " << filename << color_reset << "\n";
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string email, password;

            if (std::getline(iss, email, ',') && std::getline(iss, password)) {
                email.erase(0, email.find_first_not_of(" \t"));
                email.erase(email.find_last_not_of(" \t") + 1);
                password.erase(0, password.find_first_not_of(" \t"));
                password.erase(password.find_last_not_of(" \t") + 1);

                accountstable.insert({ email, password });
            }
        }
        file.close();
    }

    bool authenticate(const std::string& email, const std::string& password) {
        Node* node = accountstable.find(email);
        return node != nullptr && node->account.password == password;
    }

    bool signUp(const std::string& filename) {
        std::string email, password, confirmPassword;

        while (true) {
            clearscreen();

            std::cout << color_cyan;
            std::cout << "**************************************************\n";
            std::cout << "*                   Sign Up                      *\n";
            std::cout << "**************************************************\n" << color_reset;
            std::cout << "Please enter your details below to create an account.\n";
            std::cout << "Type '" << color_yellow << "back" << color_reset << "' in the email field to return to the main menu.\n\n";

            std::cout << color_blue << "Email: " << color_reset;
            std::cin >> email;

            if (email == "back") {
                std::cout << color_green << "Returning to the main menu..." << color_reset << "\n";
                std::this_thread::sleep_for(std::chrono::seconds(1));
                return false;
            }

            if (email.find('@') == std::string::npos || email.find('.') == std::string::npos) {
                std::cerr << color_red << "Invalid email format. Please include '@' and a domain." << color_reset << "\n";
                std::this_thread::sleep_for(std::chrono::seconds(2));
                continue;
            }
            else if (accountstable.find(email) != nullptr) {
                std::cerr << color_red << "Error: Account with this email already exists. Please enter a different email." << color_reset << "\n";
                std::this_thread::sleep_for(std::chrono::seconds(2));
                continue;
            }

            std::cout << color_blue << "Password (min 6 characters): " << color_reset;
            std::cin >> password;

            if (password.length() < 6) {
                std::cerr << color_red << "Password must be at least 6 characters long." << color_reset << "\n";
                std::this_thread::sleep_for(std::chrono::seconds(2));
                continue;
            }

            std::cout << color_blue << "Confirm Password: " << color_reset;
            std::cin >> confirmPassword;

            if (password != confirmPassword) {
                std::cerr << color_red << "Passwords do not match. Please try again." << color_reset << "\n";
                std::this_thread::sleep_for(std::chrono::seconds(2));
                continue;
            }

            accountstable.insert({ email, password });

            std::ofstream file(filename, std::ios::app);
            if (!file.is_open()) {
                std::cerr << color_red << "Error: Could not open file " << filename << " to save the new account." << color_reset << "\n";
                return false;
            }
            file << email << ", " << password << "\n";
            file.close();

            std::cout << color_green << "\nAccount created successfully! You can now log in with your new credentials." << color_reset << "\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));

            clearscreen();
            return true;
        }
    }

    void loginPage() {
        while (true) {
            clearscreen();
            std::cout << color_yellow;
            std::cout << "**************************************************\n";
            std::cout << "*                 Email System                   *\n";
            std::cout << "**************************************************\n" << color_reset;

            std::cout << "\n" << color_blue << "1. Sign Up\n";
            std::cout << "2. Log In\n";
            std::cout << "3. Exit\n" << color_reset;
            std::cout << "Choose an option: ";

            std::string choiceStr;
            int choice;
            std::cin >> choiceStr;

            try {
                choice = std::stoi(choiceStr);
            }
            catch (...) {
                std::cout << color_red << "Invalid input. Please enter a valid number." << color_reset << "\n";
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            if (choice == 1) {
                signUp("user.txt");
            }
            else if (choice == 2) {
                clearscreen();
                const int maxAttempts = 3;
                int loginAttempts = 0;
                std::string email, password;

                while (loginAttempts < maxAttempts) {
                    clearscreen();
                    std::cout << color_cyan;
                    std::cout << "**************************************************\n";
                    std::cout << "*                   Log In                       *\n";
                    std::cout << "**************************************************\n" << color_reset;
                    std::cout << "Please enter your details below to log in.\n";
                    std::cout << "Type '" << color_yellow << "back" << color_reset << "' in the email field to return to the main menu.\n";

                    if (loginAttempts > 0) {
                        std::cout << "Remaining chances: " << color_cyan << (maxAttempts - loginAttempts) << color_reset << "\n\n";
                    }

                    std::cout << "Email: ";
                    std::cin >> email;

                    if (email == "back") {
                        break; // Go back to the main menu if "back" is entered
                    }

                    // Validate email format
                    if (email.find('@') == std::string::npos || email.find('.') == std::string::npos) {
                        std::cerr << color_red << "Invalid email format. Please include '@' and a domain." << color_reset << "\n";
                        std::this_thread::sleep_for(std::chrono::seconds(2));
                        continue; // Restart the login prompt
                    }

                    std::cout << "Password: ";
                    std::cin >> password;

                    // Validate password length
                    if (password.length() < 6) {
                        std::cerr << color_red << "Password must be at least 6 characters long." << color_reset << "\n";
                        std::this_thread::sleep_for(std::chrono::seconds(2));
                        continue; // Restart the login prompt
                    }

                    // Check if the credentials are correct
                    if (authenticate(email, password)) {
                        clearscreen();
                        std::cout << color_green << "\n\nAccess granted!\nWelcome, " << email << "!" << color_reset << "\n";
                        std::this_thread::sleep_for(std::chrono::seconds(1));

                        // Call the main menu after successful login
                        displayMainMenu();
                        return; // Exit the loginPage function after the user logs out
                    }
                    else {
                        loginAttempts++;
                        std::cerr << color_red << "\nInvalid credentials." << color_reset << "\n";
                        if (loginAttempts < maxAttempts) {
                            std::cout << "Remaining chances: " << color_cyan << (maxAttempts - loginAttempts) << color_reset << "\n";
                            std::this_thread::sleep_for(std::chrono::seconds(2));
                        }
                    }
                }

                // If maximum attempts are reached, deny access and go back to the main menu
                if (loginAttempts >= maxAttempts) {
                    std::cerr << color_red << "Too many failed attempts. Access denied." << color_reset << "\n";
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                }
            }
            else if (choice == 3) {
                clearscreen();
                std::cout << color_green << "Exiting the system. Goodbye!" << color_reset << "\n";
                break;
            }
            else {
                std::cout << color_red << "Invalid option. Please try again." << color_reset << "\n";
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
}; // <-- This closing brace ends the AccountManagement class

#endif // ACCOUNTSYSTEM_H
