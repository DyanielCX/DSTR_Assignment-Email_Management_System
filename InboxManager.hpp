#ifndef INBOX_MANAGER_HPP
#define INBOX_MANAGER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iomanip>  // For formatting the date and time
#include "DataStruc.hpp"

using namespace std;

// Class to manage the inbox with a priority queue based on date and time
class InboxManager {
public:
    void displayInbox(const string& userEmail) {
        bool inInboxMenu = true;
        while (inInboxMenu) {
            Email* head = loadEmails(userEmail);
            Email* current = head;

            // Check for spam status but do not delete spam emails
            detectAndMarkSpam(head);

            // Check if there are any non-spam emails
            bool hasNonSpamEmail = false;
            int emailCount = 0;

            for (Email* current = head; current != nullptr; current = current->next) {
                if (!current->isSpam) {
                    hasNonSpamEmail = true;
                    emailCount++;
                    cout << "---------------------------------------------\n";
                    cout << "Email " << emailCount << ":\n";
                    cout << "Subject: " << current->subject << "\n";
                    cout << "Sender: " << current->sender << "\n";
                    cout << "Date: " << formatDate(current->date) << " Time: " << formatTime(current->time) << "\n";
                    cout << "Content: " << current->content << "\n";
                }
            }

            // If no non-spam emails were found, or if the list is empty, inform the user
            if (!hasNonSpamEmail) {
                cout << "No emails found for " << userEmail << ".\n";
                char choice;
                do {
                    cout << "Enter 'm' to go back to the main menu: ";
                    cin >> choice;
                    if (choice == 'm' || choice == 'M') {
                        inInboxMenu = false; // Set the flag to false to exit the loop
                        return; // Return to ensure the function exits and does not continue
                    }
                    else {
                        cout << "Invalid choice. Please try again.\n";
                    }
                } while (true);
            }
            else {
                cout << "---------------------------------------------\n";
            }



            // Offer options to the user
            char choice;
            cout << "\nOptions:\n";
            cout << "d - Delete an email \n";
            cout << "m - Return to the main menu\n";
            cout << "Enter your choice: ";
            cin >> choice;

            if (choice == 'd' || choice == 'D') {
                markEmailAsDeleted(head);
            }
            else if (choice == 'm' || choice == 'M') {
                inInboxMenu = false; // Exit the loop to return to the main menu
            }
            else {
                cout << "Invalid choice. Please try again.\n";
            }

            // Save the updated emails (including spam status and deleted flags) and free the linked list memory
            saveEmails(head, userEmail);
            freeEmailList(head);
        }
    }

private:
    // Function to format date from YYYYMMDD to YYYY-MM-DD
    string formatDate(const string& date) {
        if (date.length() != 8) {
            return date; // Return as-is if not in expected format
        }
        return date.substr(0, 4) + "-" + date.substr(4, 2) + "-" + date.substr(6, 2);
    }

    // Function to format time from HHMMSS to HH:MM:SS
    string formatTime(const string& time) {
        if (time.length() != 6) {
            return time; // Return as-is if not in expected format
        }
        return time.substr(0, 2) + ":" + time.substr(2, 2) + ":" + time.substr(4, 2);
    }

    // Function to load emails into a linked list, excluding those marked as deleted for the receiver
    Email* loadEmails(const string& userEmail) {
        ifstream emailFile("email.txt");
        if (!emailFile.is_open()) {
            cerr << "Failed to open email.txt\n";
            return nullptr;
        }

        Email* head = nullptr;
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

            // Only add emails that are for the current user and not marked as deleted for the receiver
            if (newEmail->receiver == userEmail && !newEmail->receiverDeleted) {
                head = enqueue(head, newEmail);
            }
            else {
                delete newEmail; // Discard emails that don't belong to the user or are marked as deleted
            }
        }

        emailFile.close();
        return head;
    }

    // Function to enqueue (add) an email into the linked list by priority (latest date and time first)
    Email* enqueue(Email* head, Email* newEmail) {
        if (head == nullptr || compareDateTime(newEmail, head) > 0) {
            // Insert at the head if list is empty or newEmail is more recent
            newEmail->next = head;
            head = newEmail;
        }
        else {
            // Traverse to find the correct position for insertion
            Email* current = head;
            while (current->next != nullptr && compareDateTime(newEmail, current->next) <= 0) {
                current = current->next;
            }
            newEmail->next = current->next;
            current->next = newEmail;
        }
        return head;
    }

    // Function to mark an email as deleted for the receiver
    void markEmailAsDeleted(Email* head) {
        if (head == nullptr) {
            cout << "No emails available to delete.\n";
            return;
        }

        // Display the emails with index numbers to the user
        cout << "\nEnter the number of the email you want to deleted: ";
        int index;
        cin >> index;

        Email* current = head;
        int currentIndex = 1;

        // Traverse the linked list to find the email at the specified index
        while (current != nullptr) {
            if (!current->isSpam) { // Only consider non-spam emails for the index
                if (currentIndex == index) {
                    // Mark the email as deleted for the receiver
                    current->receiverDeleted = true;
                    cout << "Email deleted successfully.\n";
                    return;
                }
                currentIndex++; // Increment the index only for non-spam emails
            }
            current = current->next;
        }

        // If the index is out of bounds or invalid
        cout << "Invalid choice. No email was deleted.\n";
    }

    // Function to compare two emails by date and time
    int compareDateTime(Email* email1, Email* email2) {
        if (email1->date > email2->date) {
            return 1;
        }
        else if (email1->date < email2->date) {
            return -1;
        }
        else {
            if (email1->time > email2->time) {
                return 1;
            }
            else if (email1->time < email2->time) {
                return -1;
            }
            else {
                return 0;
            }
        }
    }

    // Function to detect spam and mark emails as spam
    void detectAndMarkSpam(Email* head) {
        Email* current = head;
        while (current != nullptr) {
            detectSpam(*current);
            current = current->next;
        }
    }

    void detectSpam(Email& email) {
        ifstream spamFile("spam.txt");
        if (!spamFile.is_open()) {
            cerr << "Failed to open spam.txt\n";
            return;
        }

        string spamWord;
        while (getline(spamFile, spamWord)) {
            string contentLower = email.content;
            string spamWordLower = spamWord;
            transform(contentLower.begin(), contentLower.end(), contentLower.begin(), ::tolower);
            transform(spamWordLower.begin(), spamWordLower.end(), spamWordLower.begin(), ::tolower);

            if (contentLower.find(spamWordLower) != string::npos) {
                email.isSpam = true;
                break;
            }
        }

        spamFile.close();
    }

    // Function to save emails back to the file with updated flags
    void saveEmails(Email* head, const string& userEmail) {
        ifstream emailFile("email.txt");
        if (!emailFile.is_open()) {
            cerr << "Failed to open email.txt for reading.\n";
            return;
        }

        ofstream outFile("temp_email.txt");
        if (!outFile.is_open()) {
            cerr << "Failed to open temp_email.txt for writing.\n";
            emailFile.close();
            return;
        }

        string line;
        Email* current = head;
        while (getline(emailFile, line)) {
            istringstream iss(line);
            string receiverDeletedStr, senderDeletedStr, subject, sender, receiver, date, time, content, isSpamStr;
            getline(iss, receiverDeletedStr, ',');
            getline(iss, senderDeletedStr, ',');
            getline(iss, subject, ',');
            getline(iss, sender, ',');
            getline(iss, receiver, ',');
            getline(iss, date, ',');
            getline(iss, time, ',');
            getline(iss, content, ',');
            getline(iss, isSpamStr);

            // Check if this email belongs to the current user and needs updating
            if (receiver == userEmail) {
                // Update the `receiverDeleted` flag if the email is found in the linked list
                if (current != nullptr && current->subject == subject && current->sender == sender &&
                    current->receiver == receiver && current->date == date && current->time == time &&
                    current->content == content) {
                    outFile << (current->receiverDeleted ? "1" : "0") << ","
                        << senderDeletedStr << "," << subject << "," << sender << ","
                        << receiver << "," << date << "," << time << ","
                        << content << "," << isSpamStr << "\n";
                    current = current->next;
                }
                else {
                    outFile << receiverDeletedStr << "," << senderDeletedStr << ","
                        << subject << "," << sender << "," << receiver << ","
                        << date << "," << time << "," << content << "," << isSpamStr << "\n";
                }
            }
            else {
                // Write other emails unchanged
                outFile << line << "\n";
            }
        }

        emailFile.close();
        outFile.close();

        remove("email.txt");
        rename("temp_email.txt", "email.txt");
    }

    void freeEmailList(Email* head) {
        while (head != nullptr) {
            Email* temp = head;
            head = head->next;
            delete temp;
        }
    }
};

#endif // INBOX_MANAGER_HPP
