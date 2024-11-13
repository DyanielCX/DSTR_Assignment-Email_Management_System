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
    Email* next = nullptr; 
};

class EmailStack {
private:
	struct StackNode {
		Email* email;
		StackNode* next;
		StackNode(Email* e) : email(e), next(nullptr) {}
	};

	StackNode* topNode;

public:
	EmailStack() : topNode(nullptr) {}

	// Push an email onto the stack
	void push(Email* email) {
		StackNode* newNode = new StackNode(email);
		newNode->next = topNode;
		topNode = newNode;
	}

	// Check if the stack is empty
	bool empty() const {
		return topNode == nullptr;
	}

	// Peek at the top item without removing it
	Email* top() const {
		return topNode ? topNode->email : nullptr;
	}

	// Pop the top item from the stack
	void pop() {
		if (!empty()) {
			StackNode* temp = topNode;
			topNode = topNode->next;
			delete temp;
		}
	}

	// Clear the stack
	void clear() {
		while (!empty()) {
			pop();
		}
	}
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
			if (selectedEmail->receiverDeleted == true) {
				// Delete the selected email
				Email* trav = front;

				while (trav != nullptr) {
					if (trav->next == selectedEmail) {
						trav->next = trav->next->next;
						
						delete selectedEmail;
						clearscreen();
						cout << "\033[32mEmail deleted successfully.\033[0m\n";
						this_thread::sleep_for(chrono::seconds(1));
						clearscreen();
						return;
					}
					trav = trav->next;
				}
			}
			else {
				// Mark the email as deleted for the receiver
				selectedEmail->senderDeleted = true;

				clearscreen();
				cout << "\033[32mEmail deleted successfully.\033[0m\n";
				this_thread::sleep_for(chrono::seconds(1));
				clearscreen();
				return;
			}
		}
		else {
			// If the index is out of bounds or invalid
			clearscreen();
			cout << "\033[31mInvalid choice. No email was deleted.\033[0m\n";
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


class Star_priorityQueue {
private:
	Email* highFront;
	Email* highRear;
	Email* lowFront;
	Email* lowRear;

public:
	Star_priorityQueue() {
		highFront = highRear = lowFront = lowRear = nullptr;
	}

	bool empty() {
		return (highFront == nullptr && lowFront == nullptr);
	}

	bool highEmpty() {
		return (highFront == nullptr);
	}

	bool lowEmpty() {
		return (lowFront == nullptr);
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
		if (newEmail->sender == userEmail || newEmail->receiver == userEmail) {

			/* Enqueue the email */
			// High priority (Starred)
			if ((userEmail == newEmail->sender && newEmail->senderStared == true) || (userEmail == newEmail->receiver && newEmail->receiverStared == true)) {
				if (highEmpty()) {
					highFront = highRear = newEmail;
				}
				else {
					highRear->next = newEmail;
					highRear = newEmail;
				}
			}

			// Low priority
			else {
				if (lowEmpty()) {
					lowFront = lowRear = newEmail;
				}
				else {
					lowRear->next = newEmail;
					lowRear = newEmail;
				}
			}
		}
		else {
			delete newEmail; // Discard emails that don't belong to the user
		}
	}

	bool dequeue() {
		
		// Dequeue high priority (Starred)
		if (!highEmpty()) {
			Email* temp = highFront;
			highFront = highFront->next;

			if (highEmpty()) {
				highRear = nullptr;
			}

			delete temp;
			return true;
		}

		// Dequeue low priority
		else if (!lowEmpty()) {
			Email* temp = lowFront;
			lowFront = lowFront->next;

			if (lowEmpty()) {
				lowRear = nullptr;
			}

			delete temp;
			return true;
		}
		else {	// Queue is empty
			return false;
		}
	}

	void clearQueue() {
		while (true) {
			// Check the queue is empty or not (contain email = true, empty = false)
			bool checkQueue = dequeue();

			// Queue is clear
			if (checkQueue == false) {
				break;
			}
		}
	}

	Email* getHighfront() {
		if (empty()) {
			return 0;
		}
		else {
			return highFront;
		}
	}

	Email* getLowfront() {
		if (empty()) {
			return 0;
		}
		else {
			return lowFront;
		}
	}

	Email* getEmail(int index, const string& userEmail) {
		Email* current = highFront;
		int currentIndex = 1;

		// Traverse the linked list to find the email at the specified index
		while (current != nullptr) {
			// Only consider non-deleted emails for index
			if ((userEmail == current->sender && !current->senderDeleted) || (userEmail == current->receiver && !current->receiverDeleted)) { 
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