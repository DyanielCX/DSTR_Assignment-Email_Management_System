// InboxManager.h
#ifndef INBOX_MANAGER_H
#define INBOX_MANAGER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

struct Email {
    int id;
    std::string subject;
    std::string sender;
    std::string receiver;
    std::string date;
    std::string time;
    std::string content;
};

class InboxManager {
public:
    void displayInbox(const std::string& userEmail) {
        bool inInboxMenu = true;
        while (inInboxMenu) {
            std::vector<Email> emails = loadEmails(userEmail);

            if (emails.empty()) {
                std::cout << "No emails found for " << userEmail << ".\n";
                break; // Exit the loop if there are no emails
            }
            else {
                std::cout << "Inbox for " << userEmail << ":\n";
                std::cout << "You have " << emails.size() << " email(s).\n";
                for (size_t i = 0; i < emails.size(); ++i) {
                    std::cout << "---------------------------------------------\n";
                    std::cout << "Email " << (i + 1) << ":\n";  // Display email index
                    std::cout << "Subject: " << emails[i].subject << "\n";
                    std::cout << "Sender: " << emails[i].sender << "\n";
                    std::cout << "Date: " << emails[i].date << " Time: " << emails[i].time << "\n";
                    std::cout << "Content: " << emails[i].content << "\n";
                }
                std::cout << "---------------------------------------------\n";
            }

            // Offer options to the user
            char choice;
            std::cout << "\nOptions:\n";
            std::cout << "d - Delete an email\n";
            std::cout << "m - Return to the main menu\n";
            std::cout << "Enter your choice: ";
            std::cin >> choice;

            if (choice == 'd' || choice == 'D') {
                deleteEmail(emails, userEmail);
            }
            else if (choice == 'm' || choice == 'M') {
                inInboxMenu = false; // Exit the loop to return to the main menu
            }
            else {
                std::cout << "Invalid choice. Please try again.\n";
            }
        }
    }

private:
    std::vector<Email> loadEmails(const std::string& userEmail) {
        std::ifstream emailFile("email.txt");
        std::vector<Email> emails;
        if (!emailFile.is_open()) {
            std::cerr << "Failed to open email.txt\n";
            return emails;
        }

        std::string line;
        while (std::getline(emailFile, line)) {
            std::istringstream iss(line);
            Email email;

            // Parse each field correctly
            std::string idStr;
            std::getline(iss, idStr, ',');
            email.id = std::stoi(idStr);
            std::getline(iss, email.subject, ',');
            std::getline(iss, email.sender, ',');
            std::getline(iss, email.receiver, ',');
            std::getline(iss, email.date, ',');
            std::getline(iss, email.time, ',');
            std::getline(iss, email.content);

            // Check if the email's receiver matches the current userEmail
            if (email.receiver == userEmail) {
                emails.push_back(email);
            }
        }

        emailFile.close();
        return emails;
    }

    void deleteEmail(std::vector<Email>& emails, const std::string& userEmail) {
        if (emails.empty()) {
            std::cout << "No emails available to delete.\n";
            return;
        }

        // Display the emails with index numbers
        std::cout << "\nEnter the number of the email you want to delete: ";
        size_t index;
        std::cin >> index;

        // Check if the input is valid
        if (index < 1 || index > emails.size()) {
            std::cout << "Invalid choice. No email was deleted.\n";
            return;
        }

        // Delete the selected email
        emails.erase(emails.begin() + (index - 1));
        saveEmails(emails, userEmail);  // Save the updated email list
        std::cout << "Email deleted successfully.\n";
    }

    void saveEmails(const std::vector<Email>& emails, const std::string& userEmail) {
        std::ifstream emailFile("email.txt");
        if (!emailFile.is_open()) {
            std::cerr << "Failed to open email.txt for reading.\n";
            return;
        }

        std::vector<std::string> allLines;
        std::string line;

        // Read all lines from the file
        while (std::getline(emailFile, line)) {
            allLines.push_back(line);
        }
        emailFile.close();

        // Open the file for writing
        std::ofstream outFile("email.txt");
        if (!outFile.is_open()) {
            std::cerr << "Failed to open email.txt for writing.\n";
            return;
        }

        // Rewrite all lines, only updating the user's emails
        size_t emailIndex = 0;
        for (const auto& originalLine : allLines) {
            std::istringstream iss(originalLine);
            Email email;
            std::string idStr;
            std::getline(iss, idStr, ',');
            email.id = std::stoi(idStr);
            std::getline(iss, email.subject, ',');
            std::getline(iss, email.sender, ',');
            std::getline(iss, email.receiver, ',');
            std::getline(iss, email.date, ',');
            std::getline(iss, email.time, ',');
            std::getline(iss, email.content);

            // Check if this email belongs to the user
            if (email.receiver == userEmail) {
                if (emailIndex < emails.size()) {
                    // Write the updated email
                    outFile << emails[emailIndex].id << "," << emails[emailIndex].subject << ","
                        << emails[emailIndex].sender << "," << emails[emailIndex].receiver << ","
                        << emails[emailIndex].date << "," << emails[emailIndex].time << ","
                        << emails[emailIndex].content << "\n";
                    ++emailIndex;
                }
            }
            else {
                // Write the original email for other users
                outFile << originalLine << "\n";
            }
        }

        outFile.close();
    }
};

#endif // INBOX_MANAGER_H
