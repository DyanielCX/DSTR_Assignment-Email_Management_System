#ifndef INBOX_MANAGER_H
#define INBOX_MANAGER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stack>
#include <set>
#include <algorithm>
#include "Utils.hpp"
#include "DataStruc.hpp"

using namespace std;

class InboxManager {
public:
    void displayInbox(const string& userEmail) {
        bool inInboxMenu = true;

        // Load spam users from spamuser.txt into a set
        set<string> spamUsers = loadSpamUsers();

        // Load emails into a stack, with the nearest date at the bottom
        stack<Email*> emailStack = loadEmailsToStack(userEmail);

        while (inInboxMenu) {
            stack<Email*> tempStack = emailStack; // Copy main stack to display emails without modifying it

            // Check if there are any non-spam emails
            bool hasNonSpamEmail = false;
            int emailCount = tempStack.size();

            while (!tempStack.empty()) {
                Email* current = tempStack.top();
                tempStack.pop();

                if (spamUsers.find(current->sender) != spamUsers.end()) {
                    continue; // Skip emails from spam users
                }

                if (!current->isSpam) {
                    hasNonSpamEmail = true;
                    cout << "---------------------------------------------\n";
                    cout << "Email " << emailCount-- << ":\n";
                    cout << "Subject: " << current->subject << "\n";
                    cout << "Sender: " << current->sender << "\n";
                    cout << "Date: " << formatDate(current->date) << " Time: " << formatTime(current->time) << "\n";
                    cout << "Content: " << current->content << "\n";
                }
            }

            if (!hasNonSpamEmail) {
                cout << "No emails found for " << userEmail << ".\n";
                char choice;
                do {
                    cout << "Enter 'm' to go back to the main menu: ";
                    cin >> choice;
                    if (choice == 'm' || choice == 'M') {
                        inInboxMenu = false;
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
            cout << "s - Mark an email as spam\n";
            cout << "m - Return to the main menu\n";
            cout << "Enter your choice: ";
            cin >> choice;

            if (choice == 'd' || choice == 'D') {
                modifyEmailInStack(emailStack, &InboxManager::markEmailAsDeleted);
            }
            else if (choice == 's' || choice == 'S') {
                modifyEmailInStack(emailStack, [this, &spamUsers](Email* email) { markEmailAsSpam(email, spamUsers); });
            }
            else if (choice == 'm' || choice == 'M') {
                inInboxMenu = false;
            }
            else {
                cout << "Invalid choice. Please try again.\n";
            }

            // Save the updated emails
            saveEmails(emailStack, userEmail);

            // Refresh screen
            clearscreen();
        }
    }

private:
    // Load spam users from spamuser.txt into a set
    set<string> loadSpamUsers() {
        set<string> spamUsers;
        ifstream spamUserFile("spamuser.txt");
        string email;

        while (getline(spamUserFile, email)) {
            spamUsers.insert(email);
        }

        spamUserFile.close();
        return spamUsers;
    }

    // Load emails into a sorted linked list and then push into a stack
    stack<Email*> loadEmailsToStack(const string& userEmail) {
        ifstream emailFile("email.txt");
        if (!emailFile.is_open()) {
            cerr << "Failed to open email.txt\n";
            return stack<Email*>();
        }

        Email* sortedListHead = nullptr;
        string line;
        set<string> spamUsers = loadSpamUsers();  // Load spam users here

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

            // Check if the email meets display requirements
            if (newEmail->receiver == userEmail &&
                !newEmail->receiverDeleted &&
                !newEmail->isSpam &&
                spamUsers.find(newEmail->sender) == spamUsers.end()) {
                // Only add to the sorted list if it meets the criteria
                sortedListHead = insertInOrder(sortedListHead, newEmail);
            }
            else {
                delete newEmail;  // Free memory if email does not meet requirements
            }
        }
        emailFile.close();

        // Push sorted emails into the stack
        stack<Email*> emailStack;
        Email* current = sortedListHead;
        while (current != nullptr) {
            emailStack.push(current);
            current = current->next;
        }

        return emailStack;
    }


    // Insert emails into a linked list in sorted order by date and time
    Email* insertInOrder(Email* head, Email* newEmail) {
        if (!head || compareDateTime(newEmail, head) < 0) {
            newEmail->next = head;
            return newEmail;
        }

        Email* current = head;
        while (current->next && compareDateTime(newEmail, current->next) >= 0) {
            current = current->next;
        }
        newEmail->next = current->next;
        current->next = newEmail;
        return head;
    }

    // Modify an email in the stack with a flexible modification function
    template <typename Func>
    void modifyEmailInStack(stack<Email*>& emailStack, Func modifyFunc) {
        stack<Email*> tempStack;
        int index;
        cout << "\nEnter the number of the email you want to modify: ";
        cin >> index;

        int currentIndex = 1;
        while (!emailStack.empty() && currentIndex < index) {
            tempStack.push(emailStack.top());
            emailStack.pop();
            currentIndex++;
        }

        if (!emailStack.empty()) {
            // Apply the modification function to the target email
            modifyFunc(emailStack.top());
            cout << "Email modified successfully.\n";
        }
        else {
            cout << "Invalid choice. No email was modified.\n";
        }

        // Restore the stack to its original order
        while (!tempStack.empty()) {
            emailStack.push(tempStack.top());
            tempStack.pop();
        }
    }


    static void markEmailAsDeleted(Email* email) {
        email->receiverDeleted = true;
    }

    void markEmailAsSpam(Email* email, set<string>& spamUsers) {
        email->isSpam = true;
        if (spamUsers.find(email->sender) == spamUsers.end()) {
            addSenderToSpamUserFile(email->sender);
            spamUsers.insert(email->sender);
        }
    }

    void addSenderToSpamUserFile(const string& senderEmail) {
        ofstream spamUserFile("spamuser.txt", ios::app);
        if (spamUserFile.is_open()) {
            spamUserFile << senderEmail << "\n";
            spamUserFile.close();
        }
        else {
            cerr << "Failed to open spamuser.txt\n";
        }
    }

    void saveEmails(stack<Email*> emailStack, const string& userEmail) {
        ofstream outFile("temp_email.txt");
        if (!outFile.is_open()) {
            cerr << "Failed to open temp_email.txt for writing.\n";
            return;
        }

        while (!emailStack.empty()) {
            Email* email = emailStack.top();
            outFile << (email->receiverDeleted ? "1" : "0") << ","
                << (email->senderDeleted ? "1" : "0") << ","
                << email->subject << "," << email->sender << ","
                << email->receiver << "," << email->date << ","
                << email->time << "," << email->content << ","
                << (email->isSpam ? "1" : "0") << "\n";
            emailStack.pop();
        }

        outFile.close();
        remove("email.txt");
        rename("temp_email.txt", "email.txt");
    }

    int compareDateTime(Email* email1, Email* email2) {
        if (email1->date != email2->date) return email1->date < email2->date ? -1 : 1;
        return email1->time < email2->time ? -1 : email1->time > email2->time;
    }

    string formatDate(const string& date) {
        if (date.length() != 8) return date;
        return date.substr(0, 4) + "-" + date.substr(4, 2) + "-" + date.substr(6, 2);
    }

    string formatTime(const string& time) {
        if (time.length() != 6) return time;
        return time.substr(0, 2) + ":" + time.substr(2, 2) + ":" + time.substr(4, 2);
    }
};

#endif // INBOX_MANAGER_HPP
