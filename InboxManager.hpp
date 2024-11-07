#ifndef INBOX_MANAGER_HPP
#define INBOX_MANAGER_HPP

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
        checkSpamWords(emailStack);  // Check for spam words before displaying

        while (inInboxMenu) {
            stack<Email*> tempStack = emailStack; // Copy main stack to display emails without modifying it

            // Check if there are any emails that should be displayed
            bool hasDisplayableEmail = false;
            int emailNumber = 1;  // Start email numbering from 1

            while (!tempStack.empty()) {
                Email* current = tempStack.top();
                tempStack.pop();

                // Apply display logic based on isSpam and markSpam
                if ((current->isSpam && !current->markSpam) ||  // Rule 1
                    (!current->isSpam && !current->markSpam)) {  // Rule 3

                    hasDisplayableEmail = true;
                    cout << "---------------------------------------------\n";
                    cout << "Email " << emailNumber++ << ":\n";
                    cout << "Subject: " << current->subject << "\n";
                    cout << "Sender: " << current->sender << "\n";
                    cout << "Date: " << formatDate(current->date) << " Time: " << formatTime(current->time) << "\n";
                    cout << "Content: " << current->content << "\n";
                }
                // Skip emails where isSpam is false and markSpam is true (Rule 2)
            }

            if (!hasDisplayableEmail) {
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
    set<string> loadSpamWords() {
        set<string> spamWords;
        ifstream spamWordFile("spamword.txt");
        string word;

        if (!spamWordFile.is_open()) {
            cerr << "Failed to open spamword.txt\n";
            return spamWords;
        }

        while (spamWordFile >> word) {
            spamWords.insert(word);
        }
        spamWordFile.close();

        return spamWords;
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
        set<string> spamWords = loadSpamWords();  // Load spam words from spamword.txt

        while (getline(emailFile, line)) {
            istringstream iss(line);
            Email* newEmail = new Email();

            // Parse each field correctly
            string receiverDeletedStr, senderDeletedStr, isSpamStr, markSpamStr;
            getline(iss, receiverDeletedStr, ',');
            getline(iss, senderDeletedStr, ',');
            getline(iss, newEmail->subject, ',');
            getline(iss, newEmail->sender, ',');
            getline(iss, newEmail->receiver, ',');
            getline(iss, newEmail->date, ',');
            getline(iss, newEmail->time, ',');
            getline(iss, newEmail->content, ',');
            getline(iss, isSpamStr, ',');
            getline(iss, markSpamStr);

            newEmail->receiverDeleted = (receiverDeletedStr == "1");
            newEmail->senderDeleted = (senderDeletedStr == "1");
            newEmail->isSpam = (isSpamStr == "1");
            newEmail->markSpam = (markSpamStr == "1");
            newEmail->next = nullptr;

            // Check if the email meets display requirements and is not marked as spam
            if (newEmail->receiver == userEmail &&
                !newEmail->receiverDeleted &&
                !newEmail->isSpam &&
                spamUsers.find(newEmail->sender) == spamUsers.end()) {

                // Check for spam words in email content
                istringstream contentStream(newEmail->content);
                string emailWord;
                bool containsSpamWord = false;

                while (contentStream >> emailWord) {
                    emailWord.erase(remove_if(emailWord.begin(), emailWord.end(), ::ispunct), emailWord.end());
                    if (spamWords.find(emailWord) != spamWords.end()) {
                        newEmail->isSpam = true;
                        containsSpamWord = true;
                        break;
                    }
                }

                // Only add to the sorted list if it is not marked as spam
                if (!containsSpamWord) {
                    sortedListHead = insertInOrder(sortedListHead, newEmail);
                }
                else {
                    delete newEmail;  // Free memory if email is marked as spam
                }
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
        // Transfer stack to a linked list to maintain the intended order
        Email* head = nullptr;
        Email* tail = nullptr;

        while (!emailStack.empty()) {
            Email* temp = emailStack.top();
            emailStack.pop();

            if (!head) {
                head = temp;
                tail = temp;
            }
            else {
                tail->next = temp;
                tail = temp;
            }
        }

        // Show the emails and let the user select one to modify
        int index;
        cout << "\nEnter the number of the email you want to modify: ";
        cin >> index;

        Email* current = head;
        int currentIndex = 1;
        bool modified = false;

        while (current != nullptr) {
            if (currentIndex == index) {
                modifyFunc(current); // Apply modification to selected email
                cout << "Email " << index << " modified successfully.\n";
                modified = true;
                break;
            }
            current = current->next;
            currentIndex++;
        }

        if (!modified) {
            cout << "Invalid choice. No email was modified.\n";
        }

        // Push back the emails to the stack in reverse to retain original stack order
        current = head;
        stack<Email*> tempStack;

        while (current != nullptr) {
            tempStack.push(current);
            current = current->next;
        }

        while (!tempStack.empty()) {
            emailStack.push(tempStack.top());
            tempStack.pop();
        }
    }



    static void markEmailAsDeleted(Email* email) {
        email->receiverDeleted = true;
    }

    void checkSpamWords(stack<Email*>& emailStack) {
        // Load spam words from spamword.txt into a set
        set<string> spamWords = loadSpamWords();
        if (spamWords.empty()) {
            cerr << "Failed to load spam words or no spam words defined.\n";
            return;
        }

        // Temporary stack to hold emails after processing
        stack<Email*> tempStack;

        // Process each email in the stack to check for spam words
        while (!emailStack.empty()) {
            Email* currentEmail = emailStack.top();
            emailStack.pop();

            // Assume it's not spam until checked
            currentEmail->isSpam = false;
            currentEmail->markSpam = false;

            // Check if any spam word is in the email content
            istringstream contentStream(currentEmail->content);
            string emailWord;
            while (contentStream >> emailWord) {
                // Remove punctuation for cleaner comparison
                emailWord.erase(remove_if(emailWord.begin(), emailWord.end(), ::ispunct), emailWord.end());

                // Check if the cleaned word is in the set of spam words
                if (spamWords.find(emailWord) != spamWords.end()) {
                    currentEmail->isSpam = true;
                    currentEmail->markSpam = true; // Set both flags to true for detected spam
                    break; // Exit once a spam word is found
                }
            }

            // Push the email to the temp stack after processing
            tempStack.push(currentEmail);
        }

        // Restore the emails back to the original stack
        while (!tempStack.empty()) {
            emailStack.push(tempStack.top());
            tempStack.pop();
        }

        // Save the modified emails back to email.txt to ensure the spam status is updated
        saveEmails(emailStack, "");  // Adjust the userEmail parameter if needed
    }



    void markEmailAsSpam(Email* email, set<string>& spamUsers) {
        email->markSpam = true; // Set markSpam to true

        // Ask user if they want to add the sender to spamuser.txt
        char choice;
        cout << "Would you like to mark the sender \"" << email->sender << "\" as a spam user? (y/n): ";
        cin >> choice;

        if ((choice == 'y' || choice == 'Y') && spamUsers.find(email->sender) == spamUsers.end()) {
            addSenderToSpamUserFile(email->sender);
            spamUsers.insert(email->sender);
            cout << "Sender added to spam users.\n";
        }
        else {
            cout << "Sender was not added to spam users.\n";
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
            string receiverDeletedStr, senderDeletedStr, isSpamStr, markSpamStr;
            getline(iss, receiverDeletedStr, ',');
            getline(iss, senderDeletedStr, ',');
            getline(iss, email->subject, ',');
            getline(iss, email->sender, ',');
            getline(iss, email->receiver, ',');
            getline(iss, email->date, ',');
            getline(iss, email->time, ',');
            getline(iss, email->content, ',');
            getline(iss, isSpamStr, ',');
            getline(iss, markSpamStr);

            email->receiverDeleted = (receiverDeletedStr == "1");
            email->senderDeleted = (senderDeletedStr == "1");
            email->isSpam = (isSpamStr == "1");
            email->markSpam = (markSpamStr == "1");

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

        // Step 2: Apply modifications directly from emailStack to the linked list
        while (!emailStack.empty()) {
            Email* modifiedEmail = emailStack.top();
            emailStack.pop();

            // Find and update the corresponding email in the linked list
            Email* current = emailListHead;
            while (current != nullptr) {
                if (current->subject == modifiedEmail->subject &&
                    current->sender == modifiedEmail->sender &&
                    current->receiver == modifiedEmail->receiver &&
                    current->date == modifiedEmail->date &&
                    current->time == modifiedEmail->time) {

                    // Update fields based on modifications
                    current->receiverDeleted = modifiedEmail->receiverDeleted;
                    current->senderDeleted = modifiedEmail->senderDeleted;
                    current->isSpam = modifiedEmail->isSpam;
                    current->markSpam = modifiedEmail->markSpam;
                    break;
                }
                current = current->next;
            }
        }

        // Step 3: Write the entire linked list back to email.txt, including updated isSpam and markSpam statuses
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
                    << (current->isSpam ? "1" : "0") << ","
                    << (current->markSpam ? "1" : "0") << "\n";
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
