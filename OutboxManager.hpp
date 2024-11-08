// OutboxManager.hpp
#ifndef OUTBOX_MANAGER_HPP
#define OUTBOX_MANAGER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include "DataStruc.hpp"

using namespace std;

Email_Queue outbox_emailQueue;

// Class to manage the outbox
class OutboxManager {
public:
    void displayOutbox(const string& userEmail) {
        bool inOutboxMenu = true;
        while (inOutboxMenu) {
            loadEmails(userEmail);

            // Get the front email in queue
            Email* front = outbox_emailQueue.getFront();

            // Set the email count
            int emailCount = 0;

            for (Email* current = outbox_emailQueue.getFront(); current != nullptr; current = current->next) {
                if (!current->senderDeleted) { // Only display non-spam emails for the current user
                    emailCount++;
                    cout << "---------------------------------------------\n";
                    cout << "Email " << emailCount << ":\n";
                    cout << "Subject: " << current->subject << "\n";
                    cout << "Receiver: " << current->receiver << "\n";
                    cout << "Date: " << formatDate(current->date) << " Time: " << formatTime(current->time) << "\n";
                    cout << "Content: " << current->content << "\n";
                }
            }

            // If no non-spam emails were found, or if the list is empty, inform the user
            if (outbox_emailQueue.empty()) {
                cout << "No emails found for " << userEmail << ".\n";
                char choice;
                do {
                    cout << "Enter 'm' to go back to the main menu: ";
                    cin >> choice;
                    if (choice == 'm' || choice == 'M') {
                        inOutboxMenu = false; // Exit the loop
                        return;
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
            outbox_emailQueue.clearQueue();
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
    
    // Function to load emails into a linked list, filtering only those for the current user
    void loadEmails(const string& userEmail) {
        ifstream emailFile("email.txt");
        if (!emailFile.is_open()) {
            cerr << "Failed to open email.txt\n";
            return;
        }

        outbox_emailQueue.clearQueue();

        string line;
        while (getline(emailFile, line)) {
            outbox_emailQueue.enqueue(line, userEmail);
        }

        emailFile.close();
    }

    // Function to delete an email
    void deleteEmail(const string& userEmail) {
        if (outbox_emailQueue.empty()) {
            cout << "No emails available to delete.\n";
            return;
        }

        // Display the emails with index numbers to the user
        cout << "\nEnter the number of the email you want to delete: ";
        int index;
        cin >> index;

        // Dequeue the selected email
        outbox_emailQueue.dequeue(index);
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
            string receiverDeletedStr, senderDeletedStr, subject, sender, receiver, date, time, content, isSpamStr, markSpamStr;
            getline(iss, receiverDeletedStr, ',');
            getline(iss, senderDeletedStr, ',');
            getline(iss, subject, ',');
            getline(iss, sender, ',');
            getline(iss, receiver, ',');
            getline(iss, date, ',');
            getline(iss, time, ',');
            getline(iss, content, ',');
            getline(iss, isSpamStr, ',');
            getline(iss, markSpamStr);

            // Check if this email belongs to the current user and if it needs to be saved
            if (sender == userEmail) {
                // Update the `receiverDeleted` flag if the email is found in the linked list
                if (current != nullptr && current->subject == subject && current->sender == sender &&
                    current->receiver == receiver && current->date == date && current->time == time &&
                    current->content == content) {
                    outFile << receiverDeletedStr << ","<< (current->senderDeleted ? "1" : "0") << "," 
                        << subject << "," << sender << ","
                        << receiver << "," << date << "," << time << ","
                        << content << "," << isSpamStr << "," << markSpamStr << "\n";
                    current = current->next;
                }
                else {
                    outFile << receiverDeletedStr << "," << senderDeletedStr << ","
                        << subject << "," << sender << "," << receiver << ","
                        << date << "," << time << "," << content << "," << isSpamStr << "," << markSpamStr << "\n";
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
