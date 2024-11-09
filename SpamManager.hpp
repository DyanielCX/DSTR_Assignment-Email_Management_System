#ifndef SPAM_MANAGER_HPP
#define SPAM_MANAGER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stack>
#include "Utils.hpp"
#include "DataStruc.hpp"

using namespace std;

class SpamManager {
public:
    void displaySpamEmails(const string& userEmail) {
        bool inSpamMenu = true;

        while (inSpamMenu) {
            stack<Email*> spamStack = loadSpamEmailsToStack(userEmail);
            if (spamStack.empty()) {
                cout << "No spam emails found.\n";
                char choice;
                do {
                    cout << "Enter 'm' to go back to the main menu: ";
                    cin >> choice;
                    if (choice == 'm' || choice == 'M') {
                        inSpamMenu = false; // Exit the loop to return to the main menu
                    }
                    else {
                        cout << "Invalid choice. Please try again.\n";
                    }
                } while (inSpamMenu); // Only exit if user enters 'm'
                return;
            }

            stack<Email*> tempStack = spamStack; // Copy main stack for display
            int emailNumber = 1; // Start numbering from 1

            cout << "Spam Emails:\n";
            while (!tempStack.empty()) {
                Email* current = tempStack.top();
                tempStack.pop();
                cout << "---------------------------------------------\n";
                cout << "Email " << emailNumber++ << ":\n"; // Increment emailNumber for each email
                cout << "Subject: " << current->subject << "\n";
                cout << "Sender: " << current->sender << "\n";
                cout << "Receiver: " << current->receiver << "\n";
                cout << "Date: " << formatDate(current->date) << " Time: " << formatTime(current->time) << "\n";
                cout << "Content: " << current->content << "\n";
            }
            cout << "---------------------------------------------\n";

            char choice;
            cout << "\nOptions:\n";
            cout << "d - Delete a spam email\n";
            cout << "n - Mark a spam email as non-spam\n";
            cout << "m - Return to the main menu\n";
            cout << "Enter your choice: ";
            cin >> choice;

            if (choice == 'd' || choice == 'D') {
                deleteSelectedSpamEmail(spamStack, userEmail);
            }
            else if (choice == 'n' || choice == 'N') {
                markSpamAsNonSpam(spamStack, userEmail);
            }
            else if (choice == 'm' || choice == 'M') {
                inSpamMenu = false; 
            }
            else {
                cout << "Invalid choice. Please try again.\n";
            }

            // Save the updated stack back to email.txt
            saveEmails(spamStack, userEmail);

            clearscreen();
        }
    }

private:
    stack<Email*> loadSpamEmailsToStack(const string& userEmail) {
        ifstream emailFile("email.txt");
        if (!emailFile.is_open()) {
            cerr << "Failed to open email.txt\n";
            return stack<Email*>();
        }

        set<string> spamUsers;
        ifstream spamUserFile("spamuser.txt");
        string spamUserEmail;
        while (getline(spamUserFile, spamUserEmail)) {
            spamUsers.insert(spamUserEmail);
        }
        spamUserFile.close();

        Email* sortedListHead = nullptr;
        string line;

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

            bool isUserMarkedSpam = !newEmail->isSpam && newEmail->markSpam; // User-marked spam
            bool isDetectedSpam = newEmail->isSpam && newEmail->markSpam;    // Detected as spam
            bool isSenderInSpamList = spamUsers.find(newEmail->sender) != spamUsers.end(); // Sender marked as spam

            if ((isUserMarkedSpam || isDetectedSpam || isSenderInSpamList) &&
                newEmail->receiver == userEmail && !newEmail->receiverDeleted) {
                sortedListHead = insertInOrder(sortedListHead, newEmail); 
            }
            else {
                delete newEmail;
            }
        }

        emailFile.close();

        // Reverse the sorted linked list to push emails onto the stack in reverse order
        stack<Email*> emailStack;
        Email* current = sortedListHead;
        Email* prev = nullptr;

        // Reverse the linked list
        while (current != nullptr) {
            Email* next = current->next;
            current->next = prev;
            prev = current;
            current = next;
        }

        current = prev;  
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

    int compareDateTime(Email* email1, Email* email2) {
        if (email1->date != email2->date) return email1->date < email2->date ? -1 : 1;
        return email1->time < email2->time ? -1 : 1;
    }

    string formatDate(const string& date) {
        if (date.length() != 8) return date;
        return date.substr(0, 4) + "-" + date.substr(4, 2) + "-" + date.substr(6, 2);
    }

    string formatTime(const string& time) {
        if (time.length() != 6) return time;
        return time.substr(0, 2) + ":" + time.substr(2, 2) + ":" + time.substr(4, 2);
    }

    template <typename Func>
    void modifyEmailInStack(stack<Email*>& emailStack, Func modifyFunc) {
        // Transfer emails from stack to a temporary linked list in display order
        Email* emailListHead = nullptr;
        Email* tail = nullptr;

        while (!emailStack.empty()) {
            Email* currentEmail = emailStack.top();
            emailStack.pop();

            // Insert each email at the end of the linked list (to maintain display order)
            if (!emailListHead) {
                emailListHead = currentEmail;
                tail = currentEmail;
            }
            else {
                tail->next = currentEmail;
                tail = currentEmail;
            }
            currentEmail->next = nullptr;  // Ensure end of list is clear
        }

        //Traverse the linked list to find the selected email
        int index;
        cout << "\nEnter the number of the spam email you want to modify: ";
        cin >> index;

        int currentIndex = 1;
        bool modified = false;
        Email* current = emailListHead;

        while (current != nullptr) {
            if (currentIndex == index) {
                // Apply modification function to the selected email
                modifyFunc(current);
                cout << "Spam email " << index << " modified successfully.\n";
                modified = true;
                break;
            }
            current = current->next;
            currentIndex++;
        }

        if (!modified) {
            cout << "Invalid choice. No spam email was modified.\n";
        }

        //Rebuild the stack in the original order from the linked list
        current = emailListHead;
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



    void deleteSelectedSpamEmail(stack<Email*>& spamStack, const string& userEmail) {
        modifyEmailInStack(spamStack, [](Email* email) {
            email->receiverDeleted = true;
            });
    }

    void markSpamAsNonSpam(stack<Email*>& spamStack, const string& userEmail) {
        string senderEmail;

        modifyEmailInStack(spamStack, [&senderEmail](Email* email) {
            email->markSpam = false;  
            senderEmail = email->sender;  
            });

        if (!senderEmail.empty()) {
            removeSenderFromSpamUserFile(senderEmail);
        }

        stack<Email*> tempStack;
        while (!spamStack.empty()) {
            Email* email = spamStack.top();
            spamStack.pop();

            if (email->sender == senderEmail) {
                email->markSpam = false;
            }
            tempStack.push(email);
        }

        while (!tempStack.empty()) {
            spamStack.push(tempStack.top());
            tempStack.pop();
        }
        cout << "The selected email has been unmarked as spam.\n";
        cout << "All other current emails from \"" << senderEmail << "\" have also been unmarked as spam.\n";
        cout << "Future emails from \"" << senderEmail << "\" will no longer be marked as spam.\n";
        cout << "Press Enter to continue...";
        cin.ignore();
        cin.get();
    }

    void removeSenderFromSpamUserFile(const string& senderEmail) {
        ifstream spamUserFile("spamuser.txt");
        ofstream tempFile("temp_spamuser.txt");
        string line;

        if (!spamUserFile.is_open()) {
            cerr << "Failed to open spamuser.txt for reading.\n";
            return;
        }

        if (!tempFile.is_open()) {
            cerr << "Failed to open temp_spamuser.txt for writing.\n";
            spamUserFile.close();
            return;
        }

        while (getline(spamUserFile, line)) {
            if (line != senderEmail) {
                tempFile << line << "\n";
            }
        }

        spamUserFile.close();
        tempFile.close();

        remove("spamuser.txt");
        rename("temp_spamuser.txt", "spamuser.txt");
    }

    void saveEmails(stack<Email*> emailStack, const string& userEmail) {
        ifstream emailFile("email.txt");
        if (!emailFile.is_open()) {
            cerr << "Failed to open email.txt for reading.\n";
            return;
        }

        ofstream outFile("temp_email.txt");
        if (!outFile.is_open()) {
            cerr << "Failed to open temp_email.txt for writing.\n";
            emailFile.close();
            return;
        }

        string line;
        while (getline(emailFile, line)) {
            istringstream iss(line);
            Email fileEmail;
            string receiverDeletedStr, senderDeletedStr, senderStaredStr, receiverStaredStr, isSpamStr, markSpamStr;
            getline(iss, receiverDeletedStr, ',');
            getline(iss, senderDeletedStr, ',');
            getline(iss, senderStaredStr, ',');
            getline(iss, receiverStaredStr, ',');
            getline(iss, fileEmail.subject, ',');
            getline(iss, fileEmail.sender, ',');
            getline(iss, fileEmail.receiver, ',');
            getline(iss, fileEmail.date, ',');
            getline(iss, fileEmail.time, ',');
            getline(iss, fileEmail.content, ',');
            getline(iss, isSpamStr, ',');
            getline(iss, markSpamStr);

            fileEmail.receiverDeleted = (receiverDeletedStr == "1");
            fileEmail.senderDeleted = (senderDeletedStr == "1");
            fileEmail.senderStared = (senderStaredStr == "1");
            fileEmail.receiverStared = (receiverStaredStr == "1");
            fileEmail.isSpam = (isSpamStr == "1");
            fileEmail.markSpam = (markSpamStr == "1");

            bool emailToDelete = fileEmail.receiverDeleted && fileEmail.senderDeleted;
            bool modified = false;
            stack<Email*> tempStack = emailStack;

            while (!tempStack.empty()) {
                Email* modifiedEmail = tempStack.top();
                tempStack.pop();

                if (fileEmail.subject == modifiedEmail->subject &&
                    fileEmail.sender == modifiedEmail->sender &&
                    fileEmail.receiver == modifiedEmail->receiver &&
                    fileEmail.date == modifiedEmail->date &&
                    fileEmail.time == modifiedEmail->time &&
                    fileEmail.content == modifiedEmail->content) {

                    // Apply recent modifications from the stack
                    fileEmail.receiverDeleted = modifiedEmail->receiverDeleted;
                    fileEmail.senderDeleted = modifiedEmail->senderDeleted;
                    fileEmail.senderStared = modifiedEmail->senderStared;
                    fileEmail.receiverStared = modifiedEmail->receiverStared;
                    fileEmail.isSpam = modifiedEmail->isSpam;
                    fileEmail.markSpam = modifiedEmail->markSpam;

                    emailToDelete = fileEmail.receiverDeleted && fileEmail.senderDeleted;

                    modified = true;
                    break;
                }
            }

            // Only write the email back if it is not marked for deletion
            if (!emailToDelete) {
                outFile << (fileEmail.receiverDeleted ? "1" : "0") << ","
                    << (fileEmail.senderDeleted ? "1" : "0") << ","
                    << fileEmail.senderStared << ","
                    << fileEmail.receiverStared << ","
                    << fileEmail.subject << "," << fileEmail.sender << ","
                    << fileEmail.receiver << "," << fileEmail.date << ","
                    << fileEmail.time << "," << fileEmail.content << ","
                    << (fileEmail.isSpam ? "1" : "0") << ","
                    << (fileEmail.markSpam ? "1" : "0") << "\n";
            }
        }

        emailFile.close();
        outFile.close();

        remove("email.txt");
        rename("temp_email.txt", "email.txt");
    }
};

#endif // SPAM_MANAGER_HPP
