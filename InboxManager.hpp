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
                continue;
            }
            else {
                cout << "Invalid choice. Please try again.\n";
                continue;
            }

            // Save the updated emails
            saveEmails(emailStack, userEmail);

            // Refresh the email stack to reflect the deletion or spam marking
            emailStack = loadEmailsToStack(userEmail);

            // Clear screen to display updated inbox
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

        // Push sorted emails into a temporary stack (to reverse order)
        stack<Email*> tempStack;
        Email* current = sortedListHead;
        while (current != nullptr) {
            tempStack.push(current);
            current = current->next;
        }

        // Reverse the stack by pushing from tempStack to emailStack
        stack<Email*> emailStack;
        while (!tempStack.empty()) {
            emailStack.push(tempStack.top());
            tempStack.pop();
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

    template <typename Func>
    void modifyEmailInStack(stack<Email*>& emailStack, Func modifyFunc) {
        stack<Email*> tempStack;
        int index;
        cout << "\nEnter the number of the email you want to modify: ";
        cin >> index;

        // Reverse the stack to access emails in display order (oldest at top)
        while (!emailStack.empty()) {
            tempStack.push(emailStack.top());
            emailStack.pop();
        }

        // Apply the modification function to the specified email in display order
        int currentIndex = 1;
        bool modified = false;

        while (!tempStack.empty()) {
            Email* currentEmail = tempStack.top();
            tempStack.pop();

            if (currentIndex == index) {
                // Apply the modification function to the selected email
                modifyFunc(currentEmail);
                cout << "Email " << index << " modified successfully.\n";
                modified = true;
            }

            emailStack.push(currentEmail); // Restore email to original stack
            currentIndex++;
        }

        if (!modified) {
            cout << "Invalid choice. No email was modified.\n";
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
        // Step 1: Load all emails from the original file into a linked list
        ifstream emailFile("email.txt");
        if (!emailFile.is_open()) {
            cerr << "Failed to open email.txt\n";
            return;
        }

        Email* emailListHead = nullptr;  // Linked list to store all emails
        Email* emailListTail = nullptr;
        string line;

        while (getline(emailFile, line)) {
            istringstream iss(line);
            Email* email = new Email();

            // Parse each field in the line
            string receiverDeletedStr, senderDeletedStr, isSpamStr;
            getline(iss, receiverDeletedStr, ',');
            getline(iss, senderDeletedStr, ',');
            getline(iss, email->subject, ',');
            getline(iss, email->sender, ',');
            getline(iss, email->receiver, ',');
            getline(iss, email->date, ',');
            getline(iss, email->time, ',');
            getline(iss, email->content, ',');
            getline(iss, isSpamStr);

            email->receiverDeleted = (receiverDeletedStr == "1");
            email->senderDeleted = (senderDeletedStr == "1");
            email->isSpam = (isSpamStr == "1");

            // Append email to the linked list
            if (emailListHead == nullptr) {
                emailListHead = email;
                emailListTail = email;
            }
            else {
                emailListTail->next = email;
                emailListTail = email;
            }
        }
        emailFile.close();

        // Step 2: Apply modifications from emailStack to the linked list
        stack<Email*> reversedStack;
        while (!emailStack.empty()) {
            reversedStack.push(emailStack.top());
            emailStack.pop();
        }

        while (!reversedStack.empty()) {
            Email* modifiedEmail = reversedStack.top();
            reversedStack.pop();

            // Find and update the corresponding email in the linked list
            Email* current = emailListHead;
            while (current != nullptr) {
                if (current->subject == modifiedEmail->subject &&
                    current->sender == modifiedEmail->sender &&
                    current->receiver == modifiedEmail->receiver &&
                    current->date == modifiedEmail->date &&
                    current->time == modifiedEmail->time) {

                    current->receiverDeleted = modifiedEmail->receiverDeleted;
                    current->senderDeleted = modifiedEmail->senderDeleted;
                    current->isSpam = modifiedEmail->isSpam;
                    break;
                }
                current = current->next;
            }
        }

        // Step 3: Write the entire linked list back to email.txt, excluding emails where both receiverDeleted and senderDeleted are true
        ofstream outFile("email.txt");
        if (!outFile.is_open()) {
            cerr << "Failed to open email.txt for writing.\n";
            return;
        }

        Email* current = emailListHead;
        while (current != nullptr) {
            // Only write the email back if it doesn't meet the condition for deletion
            if (!(current->receiverDeleted && current->senderDeleted)) {
                outFile << (current->receiverDeleted ? "1" : "0") << ","
                    << (current->senderDeleted ? "1" : "0") << ","
                    << current->subject << "," << current->sender << ","
                    << current->receiver << "," << current->date << ","
                    << current->time << "," << current->content << ","
                    << (current->isSpam ? "1" : "0") << "\n";
            }

            Email* toDelete = current;
            current = current->next;
            delete toDelete;  // Free memory
        }

        outFile.close();
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
