#ifndef SEARCHRETRIEVAL_HPP
#define SEARCHRETRIEVAL_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "DataStruc.hpp"
#include "Utils.hpp"

using namespace std;

class SearchRetrieval {
public:
    void displaySearchMenu(const string& userEmail) {
        bool inSearchMenu = true;

        while (inSearchMenu) {
            cout << "\033[36m**************************************************\n";
            cout << "*               Search and Retrieval             *\n";
            cout << "**************************************************\033[0m\n";

            cout << "Please choose a search option or go back:\n";
            cout << "1. Search by Subject\n";
            cout << "2. Search by Sender\n";
            cout << "3. Search by Receiver\n";
            cout << "4. Search by Date\n";
            cout << "5. Search by Keyword in Content\n";
            cout << "6. Back to Main Menu\n";
            cout << "Your choice: ";

            int choice;
            cin >> choice;
            cin.ignore();

            if (choice == 6) {
                inSearchMenu = false;
                continue;
            }

            clearscreen();

            switch (choice) {
            case 1: displayHeader("Search by Subject"); break;
            case 2: displayHeader("Search by Sender"); break;
            case 3: displayHeader("Search by Receiver"); break;
            case 4: displayHeader("Search by Date"); break;
            case 5: displayHeader("Search by Keyword in Content"); break;
            default: cout << "\033[31mInvalid choice.\033[0m\n"; continue;
            }

            string searchTerm;
            if (choice == 4) {
                cout << "Enter the date in this format YYYYMMDD (or type 'back' to return to the menu): ";
            }
            else {
                cout << "Enter your search term (or type 'back' to return to the menu): ";
            }
            getline(cin, searchTerm);

            if (searchTerm == "back") {
                clearscreen();
                continue;
            }

            Email* searchResults = performSearch(choice, searchTerm, userEmail);
            stack<Email*> emailStack = createStackFromLinkedList(searchResults);
            displayResults(emailStack, userEmail);

            clearscreen();
        }
    }

private:
    void displayHeader(const string& headerText) {
        cout << "\033[32m**************************************************\n";
        cout << "*               " << headerText << "               *\n";
        cout << "**************************************************\033[0m\n";
    }

    Email* performSearch(int choice, string searchTerm, const string& userEmail) {
        if (choice == 1 || choice == 5) {
            transform(searchTerm.begin(), searchTerm.end(), searchTerm.begin(), ::tolower);
        }

        Email* sortedListHead = loadUserEmails(userEmail);
        Email* resultHead = nullptr;

        Email* current = sortedListHead;
        while (current) {
            bool isMatch = false;

            if (choice == 1) {
                string subjectLower = current->subject;
                transform(subjectLower.begin(), subjectLower.end(), subjectLower.begin(), ::tolower);
                if (subjectLower.find(searchTerm) != string::npos) {
                    isMatch = true;
                }
            }
            else if (choice == 2 && current->receiver == userEmail && current->sender == searchTerm) {
                isMatch = true;
            }
            else if (choice == 3 && current->sender == userEmail && current->receiver == searchTerm) {
                isMatch = true;
            }
            else if (choice == 4 && current->date == searchTerm) {
                isMatch = true;
            }
            else if (choice == 5) {
                string contentLower = current->content;
                transform(contentLower.begin(), contentLower.end(), contentLower.begin(), ::tolower);
                if (contentLower.find(searchTerm) != string::npos) {
                    isMatch = true;
                }
            }

            if (isMatch) {
                Email* matchedEmail = new Email(*current);
                matchedEmail->next = resultHead;
                resultHead = matchedEmail;
            }
            current = current->next;
        }
        return resultHead;
    }

    Email* loadUserEmails(const string& userEmail) {
        ifstream emailFile("email.txt");
        if (!emailFile.is_open()) {
            cerr << "Failed to open email.txt\n";
            return nullptr;
        }

        Email* sortedListHead = nullptr;
        string line;

        while (getline(emailFile, line)) {
            istringstream iss(line);
            Email* email = new Email();

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

            if ((email->sender == userEmail || email->receiver == userEmail) && !email->receiverDeleted) {
                sortedListHead = insertInReverseOrder(sortedListHead, email);
            }
            else {
                delete email;
            }
        }
        emailFile.close();
        return sortedListHead;
    }

    Email* insertInReverseOrder(Email* head, Email* newEmail) {
        if (!head || compareDateTime(newEmail, head) > 0) {
            newEmail->next = head;
            return newEmail;
        }

        Email* current = head;
        while (current->next && compareDateTime(newEmail, current->next) <= 0) {
            current = current->next;
        }
        newEmail->next = current->next;
        current->next = newEmail;
        return head;
    }

    stack<Email*> createStackFromLinkedList(Email* head) {
        stack<Email*> emailStack;
        while (head) {
            emailStack.push(head);
            head = head->next;
        }
        return emailStack;
    }

    void displayResults(stack<Email*> results, const string& userEmail) {
        if (results.empty()) {
            cout << "\033[31mNo emails found matching your search criteria.\033[0m\n";
        }
        else {
            int count = 1;
            while (!results.empty()) {
                Email* email = results.top();
                results.pop();

                cout << "\033[32m---------------------------------------------\n";
                cout << "\033[33mEmail " << count++ << " ";

                string emailType;
                if (email->sender == userEmail && (email->isSpam || email->markSpam)) {
                    emailType = "(Sent - Spammed)";
                }
                else if (email->receiver == userEmail && (email->isSpam || email->markSpam)) {
                    emailType = "(Received - Spammed)";
                }
                else if (email->sender == userEmail) {
                    emailType = "(Sent)";
                }
                else if (email->receiver == userEmail) {
                    emailType = "(Received)";
                }
                cout << emailType << "\033[0m\n";

                cout << "\033[0mSubject: " << email->subject << "\n";
                cout << "Sender: " << email->sender << "\n";
                cout << "Receiver: " << email->receiver << "\n";
                cout << "Date: " << formatDate(email->date) << " Time: " << formatTime(email->time) << "\n";
                cout << "Content: " << email->content << "\n";
                cout << "\033[32m---------------------------------------------\033[0m\n";

                delete email;
            }
        }

        cout << "\nPress Enter to return to the search menu...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    int compareDateTime(Email* email1, Email* email2) {
        if (email1->date != email2->date) return email1->date > email2->date ? 1 : -1;
        return email1->time > email2->time ? 1 : -1;
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

#endif // SEARCHRETRIEVAL_HPP
