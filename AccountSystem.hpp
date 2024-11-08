#ifndef ACCOUNTSYSTEM_H
#define ACCOUNTSYSTEM_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>
#include "Utils.hpp" 
#include "MainMenu.hpp" 

using namespace std;

// Color definitions
const string color_reset = "\033[0m";
const string color_red = "\033[31m";
const string color_green = "\033[32m";
const string color_yellow = "\033[33m";
const string color_cyan = "\033[36m";
const string color_blue = "\033[34m";

// Account and Node structure for the hash table
struct Account {
    string email;
    string password;
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

    int hash(const string& email) const {
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

    Node* find(const string& email) const {
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
    void loadAccountsFromFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << color_red << "Error: Could not open file " << filename << color_reset << "\n";
            return;
        }

        string line;
        while (getline(file, line)) {
            istringstream iss(line);
            string email, password;

            if (getline(iss, email, ',') && getline(iss, password)) {
                email.erase(0, email.find_first_not_of(" \t"));
                email.erase(email.find_last_not_of(" \t") + 1);
                password.erase(0, password.find_first_not_of(" \t"));
                password.erase(password.find_last_not_of(" \t") + 1);

                accountstable.insert({ email, password });
            }
        }
        file.close();
    }

    bool authenticate(const string& email, const string& password) {
        Node* node = accountstable.find(email);
        return node != nullptr && node->account.password == password;
    }

    bool signUp(const string& filename) {
        string email, password, confirmPassword;

        while (true) {
            clearscreen();

            cout << color_cyan;
            cout << "**************************************************\n";
            cout << "*                   Sign Up                      *\n";
            cout << "**************************************************\n" << color_reset;
            cout << "Please enter your details below to create an account.\n";
            cout << "Type '" << color_yellow << "back" << color_reset << "' in the email field to return to the main menu.\n\n";

            cout << color_blue << "Email: " << color_reset;
            cin >> email;

            if (email == "back") {
                cout << color_green << "Returning to the main menu..." << color_reset << "\n";
                this_thread::sleep_for(chrono::seconds(1));
                return false;
            }

            if (email.find('@') == string::npos || email.find('.') == string::npos) {
                cerr << color_red << "Invalid email format. Please include '@' and a domain." << color_reset << "\n";
                this_thread::sleep_for(chrono::seconds(2));
                continue;
            }
            else if (accountstable.find(email) != nullptr) {
                cerr << color_red << "Error: Account with this email already exists. Please enter a different email." << color_reset << "\n";
                this_thread::sleep_for(chrono::seconds(2));
                continue;
            }

            cout << color_blue << "Password (min 6 characters): " << color_reset;
            cin >> password;

            if (password.length() < 6) {
                cerr << color_red << "Password must be at least 6 characters long." << color_reset << "\n";
                this_thread::sleep_for(chrono::seconds(2));
                continue;
            }

            cout << color_blue << "Confirm Password: " << color_reset;
            cin >> confirmPassword;

            if (password != confirmPassword) {
                cerr << color_red << "Passwords do not match. Please try again." << color_reset << "\n";
                this_thread::sleep_for(chrono::seconds(2));
                continue;
            }

            accountstable.insert({ email, password });

            ofstream file(filename, ios::app);
            if (!file.is_open()) {
                cerr << color_red << "Error: Could not open file " << filename << " to save the new account." << color_reset << "\n";
                return false;
            }
            file << email << ", " << password << "\n";
            file.close();

            cout << color_green << "\nAccount created successfully! You can now log in with your new credentials." << color_reset << "\n";
            this_thread::sleep_for(chrono::seconds(2));

            clearscreen();
            return true;
        }
    }

    void loginPage() {
        while (true) {
            clearscreen();
            cout << color_yellow;
            cout << "**************************************************\n";
            cout << "*                 Email System                   *\n";
            cout << "**************************************************\n" << color_reset;

            cout << "\n" << color_blue << "1. Sign Up\n";
            cout << "2. Log In\n";
            cout << "3. Exit\n" << color_reset;
            cout << "Choose an option: ";

            string choiceStr;
            int choice;
            cin >> choiceStr;

            try {
                choice = stoi(choiceStr);
            }
            catch (...) {
                cout << color_red << "Invalid input. Please enter a valid number." << color_reset << "\n";
                this_thread::sleep_for(chrono::seconds(1));
                continue;
            }

            if (choice == 1) {
                signUp("user.txt");
            }
            else if (choice == 2) {
                clearscreen();
                const int maxAttempts = 3;
                int loginAttempts = 0;
                string email, password;

                while (loginAttempts < maxAttempts) {
                    clearscreen();
                    cout << color_cyan;
                    cout << "**************************************************\n";
                    cout << "*                   Log In                       *\n";
                    cout << "**************************************************\n" << color_reset;
                    cout << "Please enter your details below to log in.\n";
                    cout << "Type '" << color_yellow << "back" << color_reset << "' in the email field to return to the main menu.\n";

                    if (loginAttempts > 0) {
                        cout << "Remaining chances: " << color_cyan << (maxAttempts - loginAttempts) << color_reset << "\n\n";
                    }

                    cout << "Email: ";
                    cin >> email;

                    if (email == "back") {
                        break; // Go back to the main menu if "back" is entered
                    }

                    // Validate email format
                    if (email.find('@') == string::npos || email.find('.') == string::npos) {
                        cerr << color_red << "Invalid email format. Please include '@' and a domain." << color_reset << "\n";
                        this_thread::sleep_for(chrono::seconds(2));
                        continue; // Restart the login prompt
                    }

                    cout << "Password: ";
                    cin >> password;

                    // Validate password length
                    if (password.length() < 6) {
                        cerr << color_red << "Password must be at least 6 characters long." << color_reset << "\n";
                        this_thread::sleep_for(chrono::seconds(2));
                        continue; // Restart the login prompt
                    }

                    // Check if the credentials are correct
                    if (authenticate(email, password)) {
                        clearscreen();
                        cout << "\n" << color_reset; // Reset to default for white border
                        cout << "**************************************************\n";
                        cout << "*                \033[1;33mLogin Successful!\033[0m               *\n";
                        cout << "**************************************************\n";
                        cout << "*                                                *\n";
                        cout << "              \033[1;33mWelcome,\033[0m " << "\033[1;33m" << email << "\033[0m\n";
                        cout << "*                                                *\n";
                        cout << "**************************************************\n" << color_reset;
                        this_thread::sleep_for(chrono::seconds(2));


                     // Call the main menu after successful login
                        displayMainMenu(email);                     
                        break;
                    }
                    else {
                        loginAttempts++;
                        cerr << color_red << "\nInvalid credentials." << color_reset << "\n";
                        if (loginAttempts < maxAttempts) {
                            cout << "Remaining chances: " << color_cyan << (maxAttempts - loginAttempts) << color_reset << "\n";
                            this_thread::sleep_for(chrono::seconds(2));
                        }
                    }
                }

                // If maximum attempts are reached, deny access and go back to the main menu
                if (loginAttempts >= maxAttempts) {
                    cerr << color_red << "Too many failed attempts. Access denied." << color_reset << "\n";
                    this_thread::sleep_for(chrono::seconds(2));
                }
            }
            else if (choice == 3) {
                clearscreen();
                cout << color_reset;
                cout << "**************************************************\n";
                cout << "*                                                *\n";
                cout << "*         \033[1;33mExiting the System... Goodbye!\033[0m         *\n";
                cout << "*                                                *\n";
                cout << "**************************************************\n" << color_reset;
                this_thread::sleep_for(chrono::seconds(2));
                break;
            }

            else {
                cout << color_red << "Invalid option. Please try again." << color_reset << "\n";
                this_thread::sleep_for(chrono::seconds(1));
            }
        }
    }
};

#endif // ACCOUNTSYSTEM_H
