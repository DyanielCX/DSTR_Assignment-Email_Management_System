// InboxManager.hpp
#ifndef INBOX_MANAGER_H
#define INBOX_MANAGER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

using namespace std;

// Structure to represent an Email
struct Email {
    int id;
    string subject;
    string sender;
    string receiver;
    string date;
    string time;
    string content;
    bool isSpam = false;
    Email* next = nullptr; // Pointer to the next Email in the linked list
};

// Class to manage the inbox
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
                        cout << "Date: " << current->date << " Time: " << current->time << "\n";
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
                head = deleteEmail(head, userEmail);
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
    // Function to load emails into a linked list, filtering only those for the current user
    Email* loadEmails(const string& userEmail) {
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
            getline(iss, idStr, ',');
            newEmail->id = stoi(idStr);
            getline(iss, newEmail->subject, ',');
            getline(iss, newEmail->sender, ',');
            getline(iss, newEmail->receiver, ',');
            getline(iss, newEmail->date, ',');
            getline(iss, newEmail->time, ',');
            getline(iss, newEmail->content, ',');
            getline(iss, isSpamStr);
            newEmail->isSpam = (isSpamStr == "1");

            // Only add emails that are for the current user
            if (newEmail->receiver == userEmail) {
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
                delete newEmail; // Discard emails that don't belong to the user
            }
        }

        emailFile.close();
        return head;
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

    // Function to delete an email
    Email* deleteEmail(Email* head, const string& userEmail) {
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
                    // Delete the selected email
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
            string idStr, isSpamStr;
            getline(iss, idStr, ',');
            email.id = stoi(idStr);
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
                if (current != nullptr && current->id == email.id) {
                    outFile << current->id << "," << current->subject << "," << current->sender << ","
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

#endif // INBOX_MANAGER_H
