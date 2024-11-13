// EditProfile.h
#ifndef EDITPROFILE_H
#define EDITPROFILE_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>

using namespace std;

// Color definitions for console output
const string color_reset = "\033[0m";
const string color_red = "\033[31m";
const string color_green = "\033[32m";
const string color_cyan = "\033[36m";

class ProfileEditor {
private:
    // Helper function to update user password in the file
    bool updatePasswordInFile(const string& email, const string& newPassword, const string& filename = "user.txt") {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << color_red << "Error: Could not open file " << filename << color_reset << "\n";
            return false;
        }

        stringstream buffer;
        string line;
        bool updated = false;

        // Read each line and update the password for the matching email
        while (getline(file, line)) {
            istringstream iss(line);
            string fileEmail, filePassword;

            // Parse email and password from each line
            if (getline(iss, fileEmail, ',') && getline(iss, filePassword)) {
                // Trim whitespace from parsed email and password
                fileEmail.erase(0, fileEmail.find_first_not_of(" \t"));
                fileEmail.erase(fileEmail.find_last_not_of(" \t") + 1);
                filePassword.erase(0, filePassword.find_first_not_of(" \t"));
                filePassword.erase(filePassword.find_last_not_of(" \t") + 1);

                // If email matches, update the password in the buffer
                if (fileEmail == email) {
                    buffer << fileEmail << ", " << newPassword << "\n";
                    updated = true;
                }
                else {
                    buffer << line << "\n";
                }
            }
        }
        file.close();

        if (!updated) {
            cerr << color_red << "Error: Email not found in file." << color_reset << "\n";
            return false;
        }

        // Write updated content to file, overwriting original file
        ofstream outFile(filename, ios::trunc); // Use ios::trunc to clear the file content
        if (!outFile.is_open()) {
            cerr << color_red << "Error: Could not write to file " << filename << color_reset << "\n";
            return false;
        }
        outFile << buffer.str(); // Write buffer contents to file
        outFile.close();

        return true;
    }

public:
    // Function to display Edit Profile menu with options
    void editProfileMenu(const string& userEmail) {
        while (true) {
            // Display Edit Profile menu options
            cout << color_cyan;
            cout << "**************************************************\n";
            cout << "*                  Edit Profile                  *\n";
            cout << "**************************************************\n";
            cout << color_reset;
            cout << "1. Change Password\n";
            cout << "2. Back to Main Menu\n";
            cout << "Choose an option: ";

            int choice;
            cin >> choice;

            if (choice == 1) {
                changePassword(userEmail); // Call changePassword to update password
                return; // Return to main menu after password change
            }
            else if (choice == 2) {
                return; // Go back to main menu without changing password
            }
            else {
                cerr << color_red << "Invalid option. Please try again." << color_reset << "\n";
            }
        }
    }

    // Function to allow the user to change their password
    void changePassword(const string& userEmail) {
        string newPassword, confirmPassword;

        while (true) {
            cout << "Enter new password (min 6 characters): ";
            cin >> newPassword;

            if (newPassword.length() < 6) {
                cerr << color_red << "Password must be at least 6 characters long." << color_reset << "\n";
                this_thread::sleep_for(chrono::seconds(1));
                continue;
            }

            cout << "Confirm new password: ";
            cin >> confirmPassword;

            if (newPassword != confirmPassword) {
                cerr << color_red << "Passwords do not match. Please try again." << color_reset << "\n";
                this_thread::sleep_for(chrono::seconds(1));
            }
            else {
                break;
            }
        }

        if (updatePasswordInFile(userEmail, newPassword)) {
            cout << color_green << "Password updated successfully!" << color_reset << "\n";
            this_thread::sleep_for(chrono::seconds(2));
        }
        else {
            cerr << color_red << "Error updating password. Please try again." << color_reset << "\n";
        }
    }
};

#endif // EDITPROFILE_H
