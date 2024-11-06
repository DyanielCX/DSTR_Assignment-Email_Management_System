#ifndef SPAM_MANAGER_HPP
#define SPAM_MANAGER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "DataStruc.hpp"

using namespace std;

// Class to manage spam emails
class SpamManager {
public:
    void displaySpamEmails(const string& userEmail) {
        bool inSpamMenu = true;

        while (inSpamMenu) {
            // Load spam emails
            Email* head = loadSpamEmails(userEmail);
            if (head == nullptr) {
                cout << "No spam emails found.\n";
                char choice;
                do {
                    cout << "Enter 'm' to go back to the main menu: ";
                    cin >> choice;
                    if (choice == 'm' || choice == 'M') {
                        inSpamMenu = false; // Exit the loop to return to the main menu
                    }
                    else {
                        cout << "Invalid choice. Please try again.\n";
                    }
                } while (inSpamMenu); // Only exit if user enters 'm'
                return; // Exit the function after handling no spam emails
            }

            // Display spam emails
            Email* current = head;
            int emailCount = 0;
            cout << "Spam Emails:\n";
            while (current != nullptr) {
                emailCount++;
                cout << "---------------------------------------------\n";
                cout << "Email " << emailCount << ":\n";
                cout << "Subject: " << current->subject << "\n";
                cout << "Sender: " << current->sender << "\n";
                cout << "Receiver: " << current->receiver << "\n";
                cout << "Date: " << current->date << " Time: " << current->time << "\n";
                cout << "Content: " << current->content << "\n";
                current = current->next;
            }
            cout << "---------------------------------------------\n";

            // Offer options to the user
            char choice;
            cout << "\nOptions:\n";
            cout << "d - Delete a spam email\n";
            cout << "m - Return to the main menu\n";
            cout << "Enter your choice: ";
            cin >> choice;

            if (choice == 'd' || choice == 'D') {
                // Capture the details of the email to be deleted
                deleteSelectedSpamEmail(head, userEmail);
            }
            else if (choice == 'm' || choice == 'M') {
                inSpamMenu = false; // Exit the loop to return to the main menu
            }
            else {
                cout << "Invalid choice. Please try again.\n";
            }

            // Free the linked list memory before the next loop iteration
            freeEmailList(head);
        }
    }

private:
    // Function to load spam emails into a linked list for a specific user
    Email* loadSpamEmails(const string& userEmail) {
        ifstream emailFile("email.txt");
        if (!emailFile.is_open()) {
            cerr << "Failed to open email.txt\n";
            return nullptr;
        }

        Email* head = nullptr;
        Email* tail = nullptr;
        string line;
        while (getline(emailFile, line)) {
            istringstream iss(line);
            Email* newEmail = new Email();

            // Parse each field correctly
            string receiverDeletedStr, senderDeletedStr, isSpamStr;
            getline(iss, receiverDeletedStr, ',');
            getline(iss, senderDeletedStr, ',');
            getline(iss, newEmail->subject, ',');
            getline(iss, newEmail->sender, ',');
            getline(iss, newEmail->receiver, ',');
            getline(iss, newEmail->date, ',');
            getline(iss, newEmail->time, ',');
            getline(iss, newEmail->content, ',');
            getline(iss, isSpamStr);

            newEmail->receiverDeleted = (receiverDeletedStr == "1");
            newEmail->senderDeleted = (senderDeletedStr == "1");
            newEmail->isSpam = (isSpamStr == "1");
            newEmail->next = nullptr;

            // Add email to the linked list if it is marked as spam and belongs to the specified user
            if (newEmail->isSpam && newEmail->receiver == userEmail && !newEmail->receiverDeleted) {
                if (head == nullptr) {
                    head = newEmail;
                    tail = newEmail;
                }
                else {
                    tail->next = newEmail;
                    tail = newEmail;
                }
            }
            else {
                delete newEmail; // Discard emails that don't meet the criteria
            }
        }

        emailFile.close();
        return head;
    }

    // Function to delete a spam email based on user input and save changes to the file
    void deleteSelectedSpamEmail(Email* head, const string& userEmail) {
        if (head == nullptr) {
            cout << "No spam emails available to delete.\n";
            return;
        }

        // Display the emails with index numbers
        cout << "\nEnter the number of the spam email you want to delete: ";
        int index;
        cin >> index;

        Email* current = head;
        int currentIndex = 1;

        // Traverse the linked list to find the email at the specified index
        while (current != nullptr) {
            if (currentIndex == index) {
                // Store the email details for comparison
                Email emailToDelete = *current;
                cout << "Spam email deleted successfully.\n";

                // Now save the emails back to the file, omitting the selected email
                saveEmailsAfterDeletion(emailToDelete);
                return; // Exit after deletion
            }
            currentIndex++;
            current = current->next;
        }

        // If the index is out of bounds or invalid
        cout << "Invalid choice. No spam email was deleted.\n";
    }

    // Function to save updated emails back to the file after a spam email is deleted
    void saveEmailsAfterDeletion(const Email& emailToDelete) {
        ifstream emailFile("email.txt");
        if (!emailFile.is_open()) {
            cerr << "Failed to open email.txt for reading.\n";
            return;
        }

        ofstream outFile("temp_email.txt"); // Use a temporary file for writing
        if (!outFile.is_open()) {
            cerr << "Failed to open temp_email.txt for writing.\n";
            emailFile.close();
            return;
        }

        string line;
        while (getline(emailFile, line)) {
            istringstream iss(line);
            Email fileEmail;
            string receiverDeletedStr, senderDeletedStr, isSpamStr;
            getline(iss, receiverDeletedStr, ',');
            getline(iss, senderDeletedStr, ',');
            getline(iss, fileEmail.subject, ',');
            getline(iss, fileEmail.sender, ',');
            getline(iss, fileEmail.receiver, ',');
            getline(iss, fileEmail.date, ',');
            getline(iss, fileEmail.time, ',');
            getline(iss, fileEmail.content, ',');
            getline(iss, isSpamStr);

            fileEmail.isSpam = (isSpamStr == "1");

            // Check if this is the email to delete
            if (fileEmail.subject == emailToDelete.subject &&
                fileEmail.sender == emailToDelete.sender &&
                fileEmail.receiver == emailToDelete.receiver &&
                fileEmail.date == emailToDelete.date &&
                fileEmail.time == emailToDelete.time &&
                fileEmail.content == emailToDelete.content) {
                // Skip writing this email to effectively delete it
                continue;
            }

            // Otherwise, write the email to the output file
            outFile << receiverDeletedStr << ","
                << senderDeletedStr << ","
                << fileEmail.subject << ","
                << fileEmail.sender << ","
                << fileEmail.receiver << ","
                << fileEmail.date << ","
                << fileEmail.time << ","
                << fileEmail.content << ","
                << isSpamStr << "\n";
        }

        emailFile.close();
        outFile.close();

        // Replace the original file with the updated file
        remove("email.txt");
        rename("temp_email.txt", "email.txt");
    }

    // Function to free the linked list memory
    void freeEmailList(Email* head) {
        while (head != nullptr) {
            Email* temp = head;
            head = head->next;
            delete temp;
        }
    }
};

#endif // SPAM_MANAGER_HPP
