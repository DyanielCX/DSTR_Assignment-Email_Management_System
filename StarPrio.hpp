// StarPrio.hpp
#ifndef STAR_PRIO_HPP
#define STAR_PRIO_HPP

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

Star_priorityQueue star_priorityQueue;

// Class to manage the starred email
class StarPrio {
public:
    void displayStarEmail(const string& userEmail) {
        bool StarPrioMenu = true;
        while (StarPrioMenu) {
            
            // Clear queue and load email into queue
            star_priorityQueue.clearQueue();
            loadEmails(userEmail);

            // Get the highfront email in queue
            Email* front = star_priorityQueue.getHighfront();

            // Set the email count
            int emailCount = 0;

            for (Email* current = front; current != nullptr; current = current->next) {
                if ((userEmail == current->sender && !current->senderDeleted) || (userEmail == current->receiver && !current->receiverDeleted)) {
                    emailCount++;
                    cout << "---------------------------------------------\n";

                    // Change the email to yellow color if the email is stared
                    if (current->senderStared || current->receiverStared) {
                        cout << "\033[33m <Stared>" << "\n";
                    }

                    cout << "Email " << emailCount << ":\n";
                    cout << "Subject: " << current->subject << "\n";
                    cout << "Sender: " << current->sender << "\n";
                    cout << "Receiver: " << current->receiver << "\n";
                    cout << "Date: " << formatDate(current->date) << " Time: " << formatTime(current->time) << "\n";
                    cout << "Content: " << current->content << "\033[0m\n";
                }
            }

            // If no non-spam emails were found, or if the list is empty, inform the user
            if (star_priorityQueue.highEmpty()) {
                cout << "\033[31mNo emails found for \033[0m" << userEmail << ".\n";
                char choice;
                do {
                    cout << "Enter 'm' to go back to the main menu: ";
                    cin >> choice;
                    if (choice == 'm' || choice == 'M') {
                        cout << "Back to the menu...\n";
                        this_thread::sleep_for(chrono::seconds(1));
                        clearscreen();
                        StarPrioMenu = false; // Exit the loop
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
            cout << "x - Unstar email\n";
            cout << "m - Return to the main menu\n";
            cout << "Enter your choice: ";
            cin >> choice;

            if (choice == 'x' || choice == 'X') {
                unstarEmail(userEmail);
            }
            else if (choice == 'm' || choice == 'M') {
                star_priorityQueue.clearQueue();

                clearscreen();
                cout << "Back to the menu...\n";
                this_thread::sleep_for(chrono::seconds(1));
                clearscreen();
                StarPrioMenu = false; // Exit the loop to return to the main menu
            }
            else {
                clearscreen();
                cout << "\033[31mInvalid choice. Please try again.\033[0m\n";
                this_thread::sleep_for(chrono::seconds(1));
                clearscreen();
            }

            // Save the updated emails (including spam status) and free the linked list memory
            saveEmails(front, userEmail);
            star_priorityQueue.clearQueue();
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
            cerr << "\033[31mFailed to open email.txt\033[0m\n";
            return;
        }

        star_priorityQueue.clearQueue();

        string line;
        while (getline(emailFile, line)) {
            star_priorityQueue.enqueue(line, userEmail);
        }

        emailFile.close();
    }

    // Function to unstar an email
    void unstarEmail(const string& userEmail) {
        if (star_priorityQueue.empty()) {
            cout << "\033[33mNo emails available to star.\033[0m\n";
            return;
        }

        // Display the emails with index numbers to the user
        cout << "\nEnter the number of the email you want to unstar: ";
        int index;
        cin >> index;
        
        // Get the selected email
        Email* selectedEmail = star_priorityQueue.getEmail(index, userEmail);

        /* Unstar the user selected email */
        if (selectedEmail != nullptr) {

            // Outbox email
            if (userEmail == selectedEmail->sender) {
                selectedEmail->senderStared = false;
            }
            else if (userEmail == selectedEmail->receiver) {
                selectedEmail->receiverStared = false;
            }

            // Unstar success message
            clearscreen();
            cout << "\033[32mEmail unstar successfully.\033[0m\n";
            this_thread::sleep_for(chrono::seconds(1));
            clearscreen();
            return;
        }
        else {
            // If the index is out of bounds or invalid
            clearscreen();
            cout << "\033[31mInvalid choice. No email was deleted.\033[0m\n";
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
            if (sender == userEmail || receiver == userEmail) {
                // Update the `senderStared` and `receiverStared` flag if the email is found in the queue
                if (current != nullptr && current->subject == subject && current->sender == sender &&
                    current->receiver == receiver && current->date == date && current->time == time &&
                    current->content == content) {
                    outFile << receiverDeletedStr << "," << senderDeletedStr << ","
                        << (current->senderStared ? "1" : "0") << "," << (current->receiverStared ? "1" : "0") << ","
                        << subject << "," << sender << ","
                        << receiver << "," << date << "," << time << ","
                        << content << "," << isSpamStr << "," << markSpamStr << "\n";
                    current = current->next;
                }
                else {
                    outFile << receiverDeletedStr << "," << senderDeletedStr << ","
                        << senderStaredStr << "," << receiverStaredStr << ","
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

#endif // STAR_PRIO_HPP
