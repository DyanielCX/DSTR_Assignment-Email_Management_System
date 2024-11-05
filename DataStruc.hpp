#ifndef DATASTRUC_HPP
#define DATASTRUC_HPP
#include <iostream>
#include <fstream>
#include <string>
#include "Utils.hpp"
using namespace std;

struct Email {
    bool receiverDeleted = false;
    bool senderDeleted = false;
    string subject;
    string sender;
    string receiver;
    string date;
    string time;
    string content;
    bool isSpam = false;
    Email* next = nullptr; // Pointer to the next Email in the linked list
};


class Email_Queue{
private:
	Email* front;
	Email* rear;
	int count;	// number of elements in queue

public:
	Email_Queue() {
		front = rear = nullptr;
		count = 0;
	}

	bool empty() {
		return (front == nullptr);
	}

	void enqueue(string line, const string& userEmail) {
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

		// Only add emails that are for the current user
		if (newEmail->sender == userEmail) {
			if (empty()) {
				front = rear = newEmail;
			}
			else {
				rear->next = newEmail;
				rear = newEmail;
				count++;
			}
		}
		else {
			delete newEmail; // Discard emails that don't belong to the user
		}
	}

	void dequeue(int index) {
		Email* current = front;
		Email* prev = nullptr;
		int currentIndex = 1;

		// Traverse the linked list to find the email at the specified index
		while (current != nullptr) {
			if (!current->isSpam) { // Only consider non-spam emails for the index
				if (currentIndex == index) {
					// Mark the email as deleted for the receiver
					current->senderDeleted = true;
					clearscreen();
					cout << "\033[31mEmail deleted successfully.\033[0m\n";
					return;
				}
				currentIndex++; // Increment the index only for non-spam emails
			}
			current = current->next;
		}

		// If the index is out of bounds or invalid
		cout << "Invalid choice. No email was deleted.\n";
		return;
	}

	void clearQueue() {
		Email* current = front;
		while (current != nullptr) {
			Email* temp = current; 
			current = current->next; 
			delete temp; 
		}
		// Reset the front, rear, and count after clearing
		front = rear = nullptr;
		count = 0;
	}

	Email* getFront() {
		if (empty()) {
			return 0;
		}
		else {
			return front;
		}
	}
};
#endif
