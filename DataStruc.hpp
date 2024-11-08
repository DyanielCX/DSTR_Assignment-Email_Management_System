#ifndef DATASTRUC_HPP
#define DATASTRUC_HPP
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <thread>
#include "Utils.hpp"
using namespace std;

struct Email {
    bool receiverDeleted = false;
    bool senderDeleted = false;
	bool senderStared = false;
	bool receiverStared = false;
    string subject;
    string sender;
    string receiver;
    string date;
    string time;
    string content;
    bool isSpam = false;
	bool markSpam = false;
    Email* next = nullptr; // Pointer to the next Email in the linked list
};


class outboxEmail_Queue{
private:
	Email* front;
	Email* rear;
	int count;	// number of elements in queue

public:
	outboxEmail_Queue() {
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
		// Get the select email
		Email* selectedEmail = getEmail(index);

		if (selectedEmail != nullptr) {
			// Mark the email as deleted for the receiver
			selectedEmail->senderDeleted = true;

			clearscreen();
			cout << "\033[31mEmail deleted successfully.\033[0m\n";
			this_thread::sleep_for(chrono::seconds(1));
			clearscreen();
			return;
		}
		else {
			// If the index is out of bounds or invalid
			clearscreen();
			cout << "Invalid choice. No email was deleted.\n";
			this_thread::sleep_for(chrono::seconds(1));
			clearscreen();
			return;
		}
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

	Email* getEmail(int index) {
		Email* current = front;
		int currentIndex = 1;

		// Traverse the linked list to find the email at the specified index
		while (current != nullptr) {
			if (current->senderDeleted != 1) { // Only consider non-deleted emails for index
				if (currentIndex == index) {
					return current;
				}
				currentIndex++;
			}
			current = current->next;
		}

		return nullptr;
	}
};
#endif