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

            if (head == nullptr) {
                cout << "No non-spam emails found for " << userEmail << ".\n";
                break; // Exit the loop if there are no non-spam emails
            }
            else {
                cout << "Inbox for " << userEmail << ":\n";
                int emailCount = 0;
                for (current = head; current != nullptr; current = current->next) {
                    if (!current->isSpam) { // Only display non-spam emails for the current user
                        emailCount++;
                        cout << "---------------------------------------------\n";
                        cout << "Email " << emailCount << ":\n";
                        cout << "Subject: " << current->subject << "\n";
                        cout << "Sender: " << current->sender << "\n";
                        cout << "Date: " << formatDate(current->date) << " Time: " << formatTime(current->time) << "\n";
                        cout << "Content: " << current->content << "\n";
                    }
                }
                cout << "---------------------------------------------\n";
            }

            // Offer options to the user
            char choice;
            cout << "\nOptions:\n";
            cout << "d - Delete an email\n";
            cout << "m - Return to the main menu\n";
            cout << "Enter your choice: ";
            cin >> choice;

            if (choice == 'd' || choice == 'D') {
                head = dequeue(head);
            }
            else if (choice == 'm' || choice == 'M') {
                inInboxMenu = false; // Exit the loop to return to the main menu
            }
            else {
                cout << "Invalid choice. Please try again.\n";
            }

            // Save the updated emails (including spam status) and free the linked list memory
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

    // Function to load emails into a linked list, sorted by priority (latest date and time first)
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
            string isSpamStr;
            getline(iss, newEmail->subject, ',');
            getline(iss, newEmail->sender, ',');
            getline(iss, newEmail->receiver, ',');
            getline(iss, newEmail->date, ',');
            getline(iss, newEmail->time, ',');
            getline(iss, newEmail->content, ',');
            getline(iss, isSpamStr);
            newEmail->isSpam = (isSpamStr == "1");
            newEmail->next = nullptr;

            // Only add emails that are for the current user
            if (newEmail->receiver == userEmail) {
                head = enqueue(head, newEmail);
            }
            else {
                delete newEmail; // Discard emails that don't belong to the user
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

    // Function to dequeue (remove) an email
    Email* dequeue(Email* head) {
        if (head == nullptr) {
            cout << "No emails available to delete.\n";
            return head;
        }

        // Display the emails with index numbers to the user
        cout << "\nEnter the number of the email you want to delete: ";
        int index;
        cin >> index;

        Email* current = head;
        Email* prev = nullptr;
        int currentIndex = 1;

        // Traverse the linked list to find the email at the specified index
        while (current != nullptr) {
            if (!current->isSpam) { // Only consider non-spam emails for the index
                if (currentIndex == index) {
                    // Dequeue (delete) the selected email
                    if (prev == nullptr) {
                        head = current->next; // Remove the head email
                    }
                    else {
                        prev->next = current->next; // Remove the current email
                    }
                    delete current;
                    cout << "Email deleted successfully.\n";
                    return head; // Return the modified head
                }
                currentIndex++; // Increment the index only for non-spam emails
            }
            prev = current;
            current = current->next;
        }

        // If the index is out of bounds or invalid
        cout << "Invalid choice. No email was deleted.\n";
        return head;
    }

    // Function to compare two emails by date and time
    // Returns positive if email1 is more recent, negative if email2 is more recent, 0 if equal
    int compareDateTime(Email* email1, Email* email2) {
        if (email1->date > email2->date) {
            return 1;
        }
        else if (email1->date < email2->date) {
            return -1;
        }
        else {
            // Dates are equal, compare times
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
            detectSpam(*current); // Mark the email as spam if necessary
            current = current->next;
        }
    }

    // Function to detect spam
    void detectSpam(Email& email) {
        ifstream spamFile("spam.txt");
        if (!spamFile.is_open()) {
            cerr << "Failed to open spam.txt\n";
            return;
        }

        string spamWord;
        while (getline(spamFile, spamWord)) {
            // Convert both the email content and spam word to lowercase for case-insensitive comparison
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

    // Function to save emails back to the file
    void saveEmails(Email* head, const string& userEmail) {
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
        Email* current = head;
        while (getline(emailFile, line)) {
            istringstream iss(line);
            Email email;
            string isSpamStr;
            getline(iss, email.subject, ',');
            getline(iss, email.sender, ',');
            getline(iss, email.receiver, ',');
            getline(iss, email.date, ',');
            getline(iss, email.time, ',');
            getline(iss, email.content, ',');
            getline(iss, isSpamStr);

            // Check if this email belongs to the current user and if it needs to be saved
            if (email.receiver == userEmail) {
                // Only save the email if it is still in the linked list
                if (current != nullptr) {
                    outFile << current->subject << "," << current->sender << ","
                        << current->receiver << "," << current->date << "," << current->time << ","
                        << current->content << "," << (current->isSpam ? "1" : "0") << "\n";
                    current = current->next; // Move to the next email in the linked list
                }
            }
            else {
                // Otherwise, write the original line unchanged
                outFile << line << "\n";
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

#endif // INBOX_MANAGER_HPP
