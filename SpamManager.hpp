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

// Class to manage spam emails
class SpamManager {
public:
    void displaySpamEmails(const string& userEmail) {
        bool inSpamMenu = true;

        while (inSpamMenu) {
            // Load spam emails into a stack with the oldest emails at the top
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
                return; // Exit the function after handling no spam emails
            }

            // Display spam emails with correct numbering
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

            // Offer options to the user
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
                inSpamMenu = false; // Exit the loop to return to the main menu
            }
            else {
                cout << "Invalid choice. Please try again.\n";
            }

            // Save the updated stack back to email.txt
            saveEmails(spamStack, userEmail);

            // Clear screen to display updated inbox
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

        // Load spam users from spamuser.txt into a set
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

            // Check if the email meets the spam criteria
            bool isUserMarkedSpam = !newEmail->isSpam && newEmail->markSpam; // User-marked spam
            bool isDetectedSpam = newEmail->isSpam && newEmail->markSpam;    // Detected as spam
            bool isSenderInSpamList = spamUsers.find(newEmail->sender) != spamUsers.end(); // Sender marked as spam

            if ((isUserMarkedSpam || isDetectedSpam || isSenderInSpamList) &&
                newEmail->receiver == userEmail && !newEmail->receiverDeleted) {
                sortedListHead = insertInOrder(sortedListHead, newEmail); // Insert in order
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

        // Now push reversed linked list onto the stack
        current = prev;  // 'prev' is now the head of the reversed list
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
        // Step 1: Transfer emails from stack to a temporary linked list in display order
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

        // Step 2: Traverse the linked list to find the selected email
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

        // Step 3: Rebuild the stack in the original order from the linked list
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
        modifyEmailInStack(spamStack, [](Email* email) {
            email->markSpam = false;  
            });
    }

    // Function to save the updated stack of emails back to the file
    void saveEmails(stack<Email*> emailStack, const string& userEmail) {
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
        while (getline(emailFile, line)) {
            istringstream iss(line);
            Email fileEmail;
            string receiverDeletedStr, senderDeletedStr, isSpamStr, markSpamStr;
            getline(iss, receiverDeletedStr, ',');
            getline(iss, senderDeletedStr, ',');
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
            fileEmail.isSpam = (isSpamStr == "1");
            fileEmail.markSpam = (markSpamStr == "1");

            // Check if this email is marked for deletion by both receiver and sender
            bool emailToDelete = fileEmail.receiverDeleted && fileEmail.senderDeleted;

            // Compare with updated stack emails to apply any recent modifications
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
                    fileEmail.isSpam = modifiedEmail->isSpam;
                    fileEmail.markSpam = modifiedEmail->markSpam;

                    // Check again if this email should be deleted after modification
                    emailToDelete = fileEmail.receiverDeleted && fileEmail.senderDeleted;

                    modified = true;
                    break;
                }
            }

            // Only write the email back if it is not marked for deletion
            if (!emailToDelete) {
                outFile << (fileEmail.receiverDeleted ? "1" : "0") << ","
                    << (fileEmail.senderDeleted ? "1" : "0") << ","
                    << fileEmail.subject << "," << fileEmail.sender << ","
                    << fileEmail.receiver << "," << fileEmail.date << ","
                    << fileEmail.time << "," << fileEmail.content << ","
                    << (fileEmail.isSpam ? "1" : "0") << ","
                    << (fileEmail.markSpam ? "1" : "0") << "\n";
            }
        }

        emailFile.close();
        outFile.close();

        // Replace the original file with the updated file
        remove("email.txt");
        rename("temp_email.txt", "email.txt");
    }
};

#endif // SPAM_MANAGER_HPP
