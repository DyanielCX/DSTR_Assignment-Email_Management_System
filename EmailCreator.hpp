// EmailCreator.hpp
#ifndef EMAIL_CREATOR_HPP
#define EMAIL_CREATOR_HPP

#include <iostream>
#include <string>
using namespace std;

class EmailCreator {
public:
    void displaySendEmail() {
        string receiver, subject, content;

        cout << "**************************************************" << endl;
        cout << "*                  Send Email                    *" << endl;
        cout << "**************************************************" << endl;

        // Prompting for inputs
        cout << "To (Receiver's Email): ";
        cin >> receiver;

        cout << "Subject: ";
        cin >> subject;

        cout << "Content: ";
        cin >> content;

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
            cout << "Sending email..." << endl;
            // Add code to send the email here
            break;
        case 2:
            cout << "Returning to edit..." << endl;
            displaySendEmail(); // Restart the input process
            break;
        case 3:
            cout << "Email sending canceled." << endl;
            break;
        default:
            cout << "Invalid option. Returning to main menu." << endl;
            break;
        }
    }
};


#endif // EMAIL_CREATOR_HPP