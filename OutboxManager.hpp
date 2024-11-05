// OutboxManager.hpp
#ifndef OUTBOX_MANAGER_HPP
#define OUTBOX_MANAGER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include "Utils.hpp"
#include "DataStruc.hpp"

using namespace std;

Email_Queue emailQueue;

// Class to manage the outbox
class OutboxManager {
public:
    void displayOutbox(const string& userEmail) {
        bool inOutboxMenu = true;
        while (inOutboxMenu) {
            loadEmails(userEmail);

            // Check for spam status but do not delete spam emails
            Email* front = emailQueue.getFront();
            detectAndMarkSpam(front);

            if (emailQueue.empty()) {
                cout << "No non-spam emails found for " << userEmail << ".\n";
                break; // Exit the loop if there are no non-spam emails
            }
            else {
                cout << "Inbox for " << userEmail << ":\n";
                int emailCount = 0;
                for (Email* current = emailQueue.getFront(); current != nullptr; current = current->next) {
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
                deleteEmail(userEmail);
            }
            else if (choice == 'm' || choice == 'M') {
                inOutboxMenu = false; // Exit the loop to return to the main menu
            }
            else {
                cout << "Invalid choice. Please try again.\n";
            }

            // Save the updated emails (including spam status) and free the linked list memory
            saveEmails(front, userEmail);
            emailQueue.clearQueue();

            // Refresh screen
            clearscreen();
        }
    }

private:
    // Function to load emails into a linked list, filtering only those for the current user
    void loadEmails(const string& userEmail) {
        ifstream emailFile("email.txt");
        if (!emailFile.is_open()) {
            cerr << "Failed to open email.txt\n";
            return;
        }

        emailQueue.clearQueue();

        string line;
        while (getline(emailFile, line)) {
            emailQueue.enqueue(line, userEmail);
        }

        emailFile.close();
    }

    // Function to detect spam and mark emails as spam
    void detectAndMarkSpam(Email* front) {
        Email* current = front;
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
    void deleteEmail(const string& userEmail) {
        if (emailQueue.empty()) {
            cout << "No emails available to delete.\n";
        }

        // Display the emails with index numbers to the user
        cout << "\nEnter the number of the email you want to delete: ";
        int index;
        cin >> index;

        // Dequeue the selected email
        emailQueue.dequeue(index);
    }


    // Function to save emails back to the file
    void saveEmails(Email* front, const string& userEmail) {
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
        Email* current = front;
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
            if (email.sender == userEmail) {
                // Only save the email if it is still in the linked list
                if (current != nullptr && current->id == email.id) {
                    outFile << current->id << "," << current->subject << "," << current->sender << ","
                        << current->receiver << "," << current->date << "," << current->time << ","
                        << current->content << "," << (current->isSpam ? "1" : "0") << "\n";
                    current = current->next; // Move to the next email in the queue
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
};

#endif // OUTBOX_MANAGER_HPP
