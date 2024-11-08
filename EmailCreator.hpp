// EmailCreator.hpp
#ifndef EMAIL_CREATOR_HPP
#define EMAIL_CREATOR_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <thread>
#include "MainMenu.hpp"
#include "Utils.hpp"
#include "DataStruc.hpp"
using namespace std;



class EmailCreator {
public:
    void displaySendEmail(const string& userEmail) {
        string receiver, subject, content;

        // Create a new email
        Email* newEmail = new Email();

        cout << "**************************************************" << endl;
        cout << "*                  Send Email                    *" << endl;
        cout << "**************************************************" << endl;

        // Clear the input buffer before using getline
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        // Prompting for inputs
        cout << "To (Receiver's Email): ";
        getline(cin, receiver);

        cout << "Subject: ";
        getline(cin, subject);

        cout << "Content: ";
        getline(cin, content);

        // Confirm the email details before sending
        cout << "\n--------------------------------------------------" << endl;
        cout << "Review Your Email:" << endl;
        cout << "To: " << receiver << endl;
        cout << "Subject: " << subject << endl;
        cout << "Content: " << content << endl;
        cout << "--------------------------------------------------" << endl;
        cout << "1. Send Email" << endl;
        cout << "2. Edit" << endl;
        cout << "3. Cancel" << endl;
        cout << "Choose an option: ";

        int option;
        cin >> option;
        cin.ignore(); // To handle the newline character after entering the option

        // Process the option
        switch (option) {
        case 1:
            clearscreen();
            cout << "Sending email..." << endl;
            this_thread::sleep_for(chrono::seconds(1));
            clearscreen();

            // Create new email
            newEmail = create_newEmail(receiver, subject, content, userEmail);

            // Add new email to text file
            saveEmails(newEmail);

            // Email send succesful message
            cout << "**********************************************" << endl;
            cout << "*          Email Sent Successfully           *" << endl;
            cout << "**********************************************" << endl;

            this_thread::sleep_for(chrono::seconds(1));
            clearscreen();
            break;
        case 2:
            clearscreen();
            cout << "Returning to edit..." << endl;
            this_thread::sleep_for(chrono::seconds(1));
            clearscreen();

            displaySendEmail(userEmail); // Restart the input process
            break;
        case 3:
            clearscreen();
            cout << "Email sending canceled." << endl;
            this_thread::sleep_for(chrono::seconds(1));
            clearscreen();
            break;
        default:
            clearscreen();
            cout << "Invalid option. Returning to main menu." << endl;
            this_thread::sleep_for(chrono::seconds(1));
            clearscreen();
            break;
        }
    }


    Email* create_newEmail(string receiver, string subject, string content, const string& userEmail) {
        // Create a new email
        Email* newEmail = new Email();

        /* Get the current date and time */
        auto now = chrono::system_clock::now();
        time_t now_c = chrono::system_clock::to_time_t(now);

        // Convert time_t to tm for date and time breakdown using localtime_s
        tm now_tm;
        localtime_s(&now_tm, &now_c); 

        // Set current date format
        stringstream dateStream;
        dateStream << put_time(&now_tm, "%Y%m%d"); // Format: YYYYMMDD
        string date = dateStream.str();

        // Set current time format
        stringstream timeStream;
        timeStream << put_time(&now_tm, "%H%M%S"); // Format: HHMMSS
        string time = timeStream.str();

        // Insert all the user input into email
        newEmail->subject = subject;
        newEmail->sender = userEmail;
        newEmail->receiver = receiver;
        newEmail->date = date;
        newEmail->time = time;
        newEmail->content = content;

        // Return the new created email
        return newEmail;
    }

    // Function to save emails back to the file
    void saveEmails(Email* newEmail) {
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

            // Otherwise, write the original line unchanged
            outFile << line << "\n";
        }

        // Insert line for new created email
        outFile << (newEmail->receiverDeleted ? "1" : "0") << "," << (newEmail->senderDeleted ? "1" : "0") << ","
            << (newEmail->senderStared ? "1" : "0") << "," << (newEmail->receiverStared ? "1" : "0") << ","
            << newEmail->subject << "," << newEmail->sender << ","
            << newEmail->receiver << "," << newEmail->date << "," << newEmail->time << ","
            << newEmail->content << "," << (newEmail->isSpam ? "1" : "0") << ","
            << (newEmail->markSpam ? "1" : "0") << "\n";

        emailFile.close();
        outFile.close();

        // Replace the original file with the updated file
        remove("email.txt");
        rename("temp_email.txt", "email.txt");
    }
};


#endif // EMAIL_CREATOR_HPP