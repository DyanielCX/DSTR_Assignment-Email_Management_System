#ifndef INBOX_MANAGER_HPP
#define INBOX_MANAGER_HPP

#include <chrono>
#include <iomanip>
#include <ctime>
#include <thread>
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

        set<string> spamUsers = loadSpamUsers();

        stack<Email*> emailStack = loadEmailsToStack(userEmail);
       
        while (inInboxMenu) {
            cout << "\033[36m**************************************************\n";
            cout << "*                Inbox Management                *\n";
            cout << "**************************************************\033[0m\n";

            stack<Email*> tempStack = emailStack;

            bool hasDisplayableEmail = false;
            int emailNumber = 1;  // Start email numbering from 1

            while (!tempStack.empty()) {
                Email* current = tempStack.top();
                tempStack.pop();

                if (!current->markSpam) {
                    hasDisplayableEmail = true;
                    cout << "---------------------------------------------\n";

                    // Change the email to yellow color if the email is stared
                    if (current->receiverStared) {
                        cout << "\033[33m <Stared>" << "\n";
                    }

                    cout << "Email " << emailNumber++ << ":\n";
                    cout << "Subject: " << current->subject << "\n";
                    cout << "Sender: " << current->sender << "\n";
                    cout << "Date: " << formatDate(current->date) << " Time: " << formatTime(current->time) << "\n";
                    cout << "Content: " << current->content << "\033[0m\n";
                }
            }

            if (!hasDisplayableEmail) {
                cout << "\033[31mNo emails found for " << userEmail << ".\033[0m\n";
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

            char choice;
            cout << "\nOptions:\n";
            cout << "d - Delete an email\n";
            cout << "s - Mark an email as spam\n";
            cout << "p - Star email\n";
            cout << "m - Return to the main menu\n";
            cout << "Enter your choice: ";
            cin >> choice;

            if (choice == 'd' || choice == 'D') {
                modifyEmailInStack(emailStack, &InboxManager::markEmailAsDeleted);
            }
            else if (choice == 's' || choice == 'S') {
                modifyEmailInStack(emailStack, [this, &spamUsers](Email* email) { markEmailAsSpam(email, spamUsers); });
            }
            else if (choice == 'p' || choice == 'P') {
                starEmail(emailStack);
            }
            else if (choice == 'm' || choice == 'M') {
                inInboxMenu = false;
                continue;
            }
            else {
                cout << "Invalid choice. Please try again.\n";
                continue;
            }

            saveEmails(emailStack, userEmail);

            // Refresh the email stack to reflect the deletion or spam marking
            emailStack = loadEmailsToStack(userEmail);

            clearscreen();
        }
    }


private:
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
        set<string> spamUsers = loadSpamUsers();  
        set<string> spamWords = loadSpamWords();  

        while (getline(emailFile, line)) {
            istringstream iss(line);
            Email* newEmail = new Email();
            string receiverDeletedStr, senderDeletedStr, senderStaredStr, receiverStaredStr, isSpamStr, markSpamStr;
            getline(iss, receiverDeletedStr, ',');
            getline(iss, senderDeletedStr, ',');
            getline(iss, senderStaredStr, ',');
            getline(iss, receiverStaredStr, ',');
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
            newEmail->senderStared = (senderStaredStr == "1");
            newEmail->receiverStared = (receiverStaredStr == "1");
            newEmail->isSpam = (isSpamStr == "1");
            newEmail->markSpam = (markSpamStr == "1");
            newEmail->next = nullptr;

            // Check if the email meets display requirements
            if (newEmail->receiver == userEmail && !newEmail->receiverDeleted &&
                spamUsers.find(newEmail->sender) == spamUsers.end() &&
                !newEmail->markSpam) {

                // Skip spam word check if isSpam is true but markSpam is false
                if (!(newEmail->isSpam && !newEmail->markSpam)) {
                    // Check for spam words in email content
                    istringstream contentStream(newEmail->content);
                    string emailWord;

                    while (contentStream >> emailWord) {
                        // Remove punctuation and convert to lowercase
                        emailWord.erase(remove_if(emailWord.begin(), emailWord.end(), ::ispunct), emailWord.end());
                        transform(emailWord.begin(), emailWord.end(), emailWord.begin(), ::tolower);

                        if (spamWords.find(emailWord) != spamWords.end()) {
                            newEmail->isSpam = true;
                            newEmail->markSpam = true;
                            break;
                        }
                    }
                }

                sortedListHead = insertInOrder(sortedListHead, newEmail);
            }
            else {
                delete newEmail;  // Free memory if email does not meet requirements
            }
        }
        emailFile.close();

        stack<Email*> tempStack;
        Email* current = sortedListHead;
        while (current != nullptr) {
            tempStack.push(current);
            current = current->next;
        }

        stack<Email*> emailStack;
        while (!tempStack.empty()) {
            emailStack.push(tempStack.top());
            tempStack.pop();
        }
        
        saveEmails(emailStack, userEmail);    
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

        int index;
        cout << "\nEnter the number of the email you want to modify: ";
        cin >> index;

        Email* current = head;
        int currentIndex = 1;
        bool modified = false;

        while (current != nullptr) {
            if (currentIndex == index) {
                modifyFunc(current); 
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

    void markEmailAsSpam(Email* email, set<string>& spamUsers) {
        email->markSpam = true; 

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


    // Function to star an email
    void starEmail(stack<Email*>& emailStack) {
        int emailIndex;
        cout << "\nEnter the number of the email you want to star: ";
        cin >> emailIndex;

        Email* current = nullptr;
        stack<Email*> tempStack;
        int currentIndex = 1;
        bool emailFound = false;

        // Unstack emails to find the targeted email and re-stack them
        while (!emailStack.empty()) {
            current = emailStack.top();
            emailStack.pop();

            if (currentIndex == emailIndex) {
                current->receiverStared = true;
                
                clearscreen();
                cout << "\033[32mEmail starred successfully.\033[0m\n";
                this_thread::sleep_for(chrono::seconds(1));
                clearscreen();
                emailFound = true;
            }
            tempStack.push(current);
            currentIndex++;
        }

        // Re-stack the emails back to emailStack
        while (!tempStack.empty()) {
            emailStack.push(tempStack.top());
            tempStack.pop();
        }

        if (!emailFound) {
            // If the index is out of bounds or invalid
            clearscreen();
            cout << "Invalid choice. No email was starred.\n";
            this_thread::sleep_for(chrono::seconds(1));
            clearscreen();
        }
    }

    void saveEmails(stack<Email*> emailStack, const string& userEmail) {
        //Load all emails from the original file into a linked list
        ifstream emailFile("email.txt");
        if (!emailFile.is_open()) {
            cerr << "Failed to open email.txt\n";
            return;
        }

        Email* emailListHead = nullptr;  
        Email* emailListTail = nullptr;  
        string line;

        while (getline(emailFile, line)) {
            istringstream iss(line);
            Email* email = new Email();

            string receiverDeletedStr, senderDeletedStr, senderStaredStr, receiverStaredStr, isSpamStr, markSpamStr;
            getline(iss, receiverDeletedStr, ',');
            getline(iss, senderDeletedStr, ',');
            getline(iss, senderStaredStr, ',');
            getline(iss, receiverStaredStr, ',');
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
            email->senderStared = (senderStaredStr == "1");
            email->receiverStared = (receiverStaredStr == "1");
            email->isSpam = (isSpamStr == "1");
            email->markSpam = (markSpamStr == "1");

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

        stack<Email*> emailStackCopy;
        stack<Email*> tempStack = emailStack;

        // Deep-copy each Email object
        while (!tempStack.empty()) {
            Email* originalEmail = tempStack.top();
            tempStack.pop();

            Email* copiedEmail = new Email(*originalEmail);
            emailStackCopy.push(copiedEmail);
        }

        while (!emailStackCopy.empty()) {
            Email* modifiedEmail = emailStackCopy.top();
            emailStackCopy.pop();

            // Find the matching email in the linked list and update it
            Email* current = emailListHead;
            while (current != nullptr) {
                if (current->subject == modifiedEmail->subject &&
                    current->sender == modifiedEmail->sender &&
                    current->receiver == modifiedEmail->receiver &&
                    current->date == modifiedEmail->date &&
                    current->time == modifiedEmail->time) {

                    // Update the spam status and deletion flags based on modifications
                    current->receiverDeleted = modifiedEmail->receiverDeleted;
                    current->senderDeleted = modifiedEmail->senderDeleted;
                    current->senderStared = modifiedEmail->senderStared;
                    current->receiverStared = modifiedEmail->receiverStared;
                    current->isSpam = modifiedEmail->isSpam;
                    current->markSpam = modifiedEmail->markSpam;
                    break;
                }
                current = current->next;
            }

            delete modifiedEmail;
        }

        //Write the updated linked list back to email.txt
        ofstream outFile("email.txt");
        if (!outFile.is_open()) {
            cerr << "Failed to open email.txt for writing.\n";
            return;
        }

        Email* current = emailListHead;
        while (current != nullptr) {
            if (!(current->receiverDeleted && current->senderDeleted)) {
                outFile << (current->receiverDeleted ? "1" : "0") << ","
                    << (current->senderDeleted ? "1" : "0") << ","
                    << current->senderStared << ","
                    << (current->receiverStared ? "1" : "0") << ","
                    << current->subject << "," << current->sender << ","
                    << current->receiver << "," << current->date << ","
                    << current->time << "," << current->content << ","
                    << (current->isSpam ? "1" : "0") << ","
                    << (current->markSpam ? "1" : "0") << "\n";
            }

            Email* toDelete = current;
            current = current->next;
            delete toDelete;
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
