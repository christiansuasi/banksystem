#ifndef _bankserver_h
#define _bankserver_h
typedef struct account{
    char name[100];
    float balance;
    bool session;//indicating whether or not the account is currently being serviced    
}Account;


/*
 * printAccounts prints all the current
 * active accounts in the server
 */
void printAccounts(Account list[]);

/*
 * makeFiller just fills (initialize?!?) the 
 * accounts in the array and sets it them to 0
 */
Account makeFiller();

/*
 * createAccount makes a new account with
 * the name that was given and sets the 
 * balance to 0
 */
Account createAccount(char clientName[]);

/*
 * credit takes an account and adds 
 * the input amount to its balance
 */
int credit(float add, Account list[], int curr);

/*
 * debit takes an account and subtracts
 * the input amount from its balance
 */
int debit(float sub, Account list[], int curr);

/*
 * balance just prints out the 
 * current balance of the given account
 */
float balance(int curr, Account list[]);

/*
 * destroyAccount gets rid of an
 * account from the server
 */
void destroyAccount(Account *list[], char clientName[]);


#endif
