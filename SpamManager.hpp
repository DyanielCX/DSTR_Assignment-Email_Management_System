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
            // Load spam emails into a stack with oldest emails at the bottom
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

            // Display spam emails
            stack<Email*> tempStack = spamStack; // Copy main stack for display
            int emailCount = tempStack.size();
            cout << "Spam Emails:\n";
            while (!tempStack.empty()) {
                Email* current = tempStack.top();
                tempStack.pop();
                cout << "---------------------------------------------\n";
                cout << "Email " << emailCount-- << ":\n";
                cout << "Subject: " << current->subject << "\n";
                cout << "Sender: " << current->sender << "\n";
                cout << "Receiver: " << current->receiver << "\n";
                cout << "Date: " << current->date << " Time: " << current->time << "\n";
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

            // Check if the email meets the new spam criteria
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

        // Push sorted emails directly into the emailStack so latest emails are on top
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

    int compareDateTime(Email* email1, Email* email2) {
        if (email1->date != email2->date) return email1->date < email2->date ? -1 : 1;
        return email1->time < email2->time ? -1 : 1;
    }

    template <typename Func>
    void modifyEmailInStack(stack<Email*>& emailStack, Func modifyFunc) {
        // Step 1: Transfer emails from stack to a temporary linked list in display order
        Email* emailListHead = nullptr;

        while (!emailStack.empty()) {
            Email* currentEmail = emailStack.top();
            emailStack.pop();

            // Insert each email at the head of the linked list (to maintain display order)
            currentEmail->next = emailListHead;
            emailListHead = currentEmail;
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
        while (current != nullptr) {
            emailStack.push(current);
            current = current->next;
        }
    }


    void deleteSelectedSpamEmail(stack<Email*>& spamStack, const string& userEmail) {
        modifyEmailInStack(spamStack, [](Email* email) {
            email->receiverDeleted = true;
            });
    }

    void markSpamAsNonSpam(stack<Email*>& spamStack, const string& userEmail) {
        modifyEmailInStack(spamStack, [](Email* email) {
            email->isSpam = false;
            });
    }

    // Function to save the updated stack of emails back to the file
    void saveEmails(stack<Email*> emailStack, const string& userEmail) {
        // Logic to read all emails from "email.txt" and save updated emails
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
            string receiverDeletedStr, senderDeletedStr, isSpamStr;
            getline(iss, receiverDeletedStr, ',');
            getline(iss, senderDeletedStr, ',');
            getline(iss, fileEmail.subject, ',');
            getline(iss, fileEmail.sender, ',');
            getline(iss, fileEmail.receiver, ',');
            getline(iss, fileEmail.date, ',');
            getline(iss, fileEmail.time, ',');
            getline(iss, fileEmail.content, ',');
            getline(iss, isSpamStr);

            fileEmail.isSpam = (isSpamStr == "1");

            // Compare with updated stack emails
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

                    outFile << (modifiedEmail->receiverDeleted ? "1" : "0") << ","
                        << (modifiedEmail->senderDeleted ? "1" : "0") << ","
                        << modifiedEmail->subject << "," << modifiedEmail->sender << ","
                        << modifiedEmail->receiver << "," << modifiedEmail->date << ","
                        << modifiedEmail->time << "," << modifiedEmail->content << ","
                        << (modifiedEmail->isSpam ? "1" : "0") << "\n";
                    modified = true;
                    break;
                }
            }

            if (!modified) {
                outFile << line << "\n";
            }
        }

        emailFile.close();
        outFile.close();

        // Replace original file
        remove("email.txt");
        rename("temp_email.txt", "email.txt");
    }
};

#endif // SPAM_MANAGER_HPP
