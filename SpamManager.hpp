#ifndef SPAM_MANAGER_HPP
#define SPAM_MANAGER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include "DataStruc.hpp"

using namespace std;

// Class to manage spam emails
class SpamManager {
public:
    void displaySpamEmails() {
        bool inSpamMenu = true;

        while (inSpamMenu) {
            // Load spam emails
            Email* head = loadSpamEmails();
            if (head == nullptr) {
                cout << "No spam emails found.\n";
                return;
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
                head = deleteSpamEmail(head);
            }
            else if (choice == 'm' || choice == 'M') {
                inSpamMenu = false; // Exit the loop to return to the main menu
            }
            else {
                cout << "Invalid choice. Please try again.\n";
            }

            // Save the updated spam emails to the file if a deletion occurred
            if (choice == 'd' || choice == 'D') {
                saveEmailsAfterSpamUpdate(head);
            }

            // Free the linked list memory
            freeEmailList(head);
        }
    }

private:
    // Function to load spam emails into a linked list
    Email* loadSpamEmails() {
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
            string idStr, isSpamStr;
            getline(iss, newEmail->subject, ',');
            getline(iss, newEmail->sender, ',');
            getline(iss, newEmail->receiver, ',');
            getline(iss, newEmail->date, ',');
            getline(iss, newEmail->time, ',');
            getline(iss, newEmail->content, ',');
            getline(iss, isSpamStr);
            newEmail->isSpam = (isSpamStr == "1");

            // Add email to the linked list if it is marked as spam
            if (newEmail->isSpam) {
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
                delete newEmail; // Discard emails that are not spam
            }
        }

        emailFile.close();
        return head;
    }

    // Function to delete a spam email
    Email* deleteSpamEmail(Email* head) {
        if (head == nullptr) {
            cout << "No spam emails available to delete.\n";
            return head;
        }

        // Display the emails with index numbers
        cout << "\nEnter the number of the spam email you want to delete: ";
        int index;
        cin >> index;

        Email* current = head;
        Email* prev = nullptr;
        int currentIndex = 1;

        // Traverse the linked list to find the email at the specified index
        while (current != nullptr) {
            if (currentIndex == index) {
                // Delete the selected email
                if (prev == nullptr) {
                    head = current->next; // Remove the head email
                }
                else {
                    prev->next = current->next; // Remove the current email
                }
                delete current;
                cout << "Spam email deleted successfully.\n";
                return head; // Return the modified head
            }
            currentIndex++;
            prev = current;
            current = current->next;
        }

        // If the index is out of bounds or invalid
        cout << "Invalid choice. No spam email was deleted.\n";
        return head;
    }

    // Function to save updated emails back to the file after a spam email is deleted
    void saveEmailsAfterSpamUpdate(Email* head) {
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
            Email email;
            string idStr, isSpamStr;
            getline(iss, email.subject, ',');
            getline(iss, email.sender, ',');
            getline(iss, email.receiver, ',');
            getline(iss, email.date, ',');
            getline(iss, email.time, ',');
            getline(iss, email.content, ',');
            getline(iss, isSpamStr);
            email.isSpam = (isSpamStr == "1");

            // Write the email to the output file if:
            // - The email is not a spam (leave it as it is)
            // - The email is in the linked list (which means it was not deleted)
            if (!email.isSpam) {
                outFile << email.subject << "," << email.sender << ","
                    << email.receiver << "," << email.date << "," << email.time << ","
                    << email.content << "," << (email.isSpam ? "1" : "0") << "\n";
            }
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
