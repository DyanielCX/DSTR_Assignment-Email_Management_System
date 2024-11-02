#include "AccountSystem.h"
using namespace std;

int main() {
    AccountManagement accountManager;
    accountManager.loadAccountsFromFile("user.txt"); // Load accounts from file
    accountManager.loginPage(); // Start login process
    return 0;
}
