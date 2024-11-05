#ifndef DATASTRUC_HPP
#define DATASTRUC_HPP
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

struct Email {
    int id;
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
		string idStr, isSpamStr;
		getline(iss, idStr, ',');
		newEmail->id = stoi(idStr);
		getline(iss, newEmail->subject, ',');
		getline(iss, newEmail->sender, ',');
		getline(iss, newEmail->receiver, ',');
		getline(iss, newEmail->date, ',');
		getline(iss, newEmail->time, ',');
		getline(iss, newEmail->content, ',');
		getline(iss, isSpamStr);
		newEmail->isSpam = (isSpamStr == "1");

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
		if (empty()) {
			cout << "The inbox is empty" << endl;
			return;
		}
		else {
			Email* current = front;
			Email* prev = nullptr;
			int currentIndex = 1;

			// Traverse the linked list to find the email at the specified index
			while (current != nullptr) {
				if (!current->isSpam) { // Only consider non-spam emails for the index
					if (currentIndex == index) {
						// Delete the selected email
						if (prev == nullptr) {
							front = current->next; // Remove the head email
						}
						else {
							prev->next = current->next; // Remove the current email
						}
						if (current == rear) {	// If we deleted the last element, update rear
							rear = prev;
						}
						delete current;
						count--;
						cout << "Email deleted successfully.\n";
						return;
					}
					currentIndex++; // Increment the index only for non-spam emails
				}
				prev = current;
				current = current->next;
			}

			// If the index is out of bounds or invalid
			cout << "Invalid choice. No email was deleted.\n";
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
			cout << "The outbox is empty" << endl;
			return 0;
		}
		else {
			return front;
		}
	}
};
#endif
