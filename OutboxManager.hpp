// OutboxManager.hpp
#ifndef OUTBOX_MANAGER_HPP
#define OUTBOX_MANAGER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <thread>
#include <algorithm>
#include "DataStruc.hpp"

using namespace std;

outboxEmail_Queue outbox_emailQueue;

// Class to manage the outbox
class OutboxManager {
public:
    void displayOutbox(const string& userEmail) {
        bool OutboxMenu = true;
        while (OutboxMenu) {
            cout << "\033[36m**************************************************\n";
            cout << "*               Outbox Management                *\n";
            cout << "**************************************************\033[0m\n";

            loadEmails(userEmail);

            // Get the front email in queue
            Email* front = outbox_emailQueue.getFront();

            // Set the email count
            int emailCount = 0;

            for (Email* current = front; current != nullptr; current = current->next) {
                if (!current->senderDeleted) { // Only display non-deleted emails for the current user
                    emailCount++;
                    cout << "---------------------------------------------\n";
                    
                    // Change the email to yellow color if the email is stared
                    if (current->senderStared) {
                        cout << "\033[33m <Stared>" << "\n";
                    }

                    cout << "Email " << emailCount << ":\n";
                    cout << "Subject: " << current->subject << "\n";
                    cout << "Receiver: " << current->receiver << "\n";
                    cout << "Date: " << formatDate(current->date) << " Time: " << formatTime(current->time) << "\n";
                    cout << "Content: " << current->content << "\033[0m\n";
                }
            }

            // If no non-spam emails were found, or if the list is empty, inform the user
            if (outbox_emailQueue.empty()) {
                cout << "\033[31mNo emails found for " << userEmail << ".\033[0m\n";
                char choice;
                do {
                    cout << "Enter 'm' to go back to the main menu: ";
                    cin >> choice;
                    if (choice == 'm' || choice == 'M') {
                        cout << "Back to the menu...\n";
                        this_thread::sleep_for(chrono::seconds(1));
                        clearscreen();
                        OutboxMenu = false; // Exit the loop to return to the main menu
                        return;
                    }
                    else {
                        cout << "\033[31mInvalid choice. Please try again.\033[0m\n";
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
            cout << "p - Star email\n";
            cout << "m - Return to the main menu\n";
            cout << "Enter your choice: ";
            cin >> choice;

            if (choice == 'd' || choice == 'D') {
                deleteEmail();
            }
            else if (choice == 'p' || choice == 'P') {
                starEmail();
            }
            else if (choice == 'm' || choice == 'M') {

                clearscreen();
                cout << "Back to the menu...\n";
                this_thread::sleep_for(chrono::seconds(1));
                clearscreen();
                OutboxMenu = false; // Exit the loop to return to the main menu
            }
            else {
                clearscreen();
                cout << "\033[31mInvalid choice. Please try again.\033[0m\n";
                this_thread::sleep_for(chrono::seconds(1));
                clearscreen();
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
    void deleteEmail() {
        if (outbox_emailQueue.empty()) {
            cout << "\033[33mNo emails available to delete.\033[0m\n";
            return;
        }

        // Display the emails with index numbers to the user
        cout << "\nEnter the number of the email you want to delete: ";
        int index;
        cin >> index;

        // Dequeue the selected email
        outbox_emailQueue.dequeue(index);
    }


    // Function to star an email
    void starEmail() {
        if (outbox_emailQueue.empty()) {
            cout << "\033[33mNo emails available to star.\033[0m\n";
            return;
        }

        // Display the emails with index numbers to the user
        cout << "\nEnter the number of the email you want to star: ";
        int index;
        cin >> index;

        // Star the selected email
        Email* selectedEmail = outbox_emailQueue.getEmail(index);

        if (selectedEmail != nullptr) {
            // Mark the email as deleted for the receiver
            selectedEmail->senderStared = true;
            clearscreen();
            cout << "\033[32mEmail starred successfully.\033[0m\n";
            this_thread::sleep_for(chrono::seconds(1));
            clearscreen();
            return;
        }
        else {
            // If the index is out of bounds or invalid
            clearscreen();
            cout << "\033[31mInvalid choice. No email was starred.\033[0m\n";
            this_thread::sleep_for(chrono::seconds(1));
            clearscreen();
            return;
        }
    }


    // Function to save emails back to the file
    void saveEmails(Email* front, const string& userEmail) {
        ifstream emailFile("email.txt");
        if (!emailFile.is_open()) {
            cerr << "\033[31mFailed to open email.txt for reading.\033[0m\n";
            return;
        }

        ofstream outFile("temp_email.txt"); // Use a temporary file for writing
        if (!outFile.is_open()) {
            cerr << "\033[31mFailed to open temp_email.txt for writing.\033[0m\n";
            emailFile.close();
            return;
        }

        string line;
        Email* current = front;
        while (getline(emailFile, line)) {
            istringstream iss(line);
            string receiverDeletedStr, senderDeletedStr, senderStaredStr, receiverStaredStr, subject, sender, receiver, date, time, content, isSpamStr, markSpamStr;
            getline(iss, receiverDeletedStr, ',');
            getline(iss, senderDeletedStr, ',');
            getline(iss, senderStaredStr, ',');
            getline(iss, receiverStaredStr, ',');
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
                // Update the `senderDeleted` and `senderStared` flag if the email is found in the queue
                if (current != nullptr && current->subject == subject && current->sender == sender &&
                    current->receiver == receiver && current->date == date && current->time == time &&
                    current->content == content) {
                    outFile << receiverDeletedStr << ","<< (current->senderDeleted ? "1" : "0") << "," 
                        << (current->senderStared ? "1" : "0") << "," << receiverStaredStr << ","
                        << subject << "," << sender << ","
                        << receiver << "," << date << "," << time << ","
                        << content << "," << isSpamStr << "," << markSpamStr << "\n";
                    current = current->next;
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
