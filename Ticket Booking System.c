// Batch : I1 + I2
// Group : 6
// Topic : Ticket booking system
// Members :  
// Avantika Gurav : UIT2023810
// Manushri Deshpande : UIT2023822
// Nandini Raut : UIT2023858
// Prajakta Gaikwad : UIT2023824


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_EVENTS 5
#define MAX_SEATS 10
#define MAX_USERS 10
#define MAX_WAITLIST 10
#define HASH_SIZE 10

// Structures
typedef struct Booking {
    char eventName[50];
    int seatNumber;
    struct Booking* next;
} Booking;

typedef struct {
    char username[50];
    char password[50];
    bool isVIP;
    Booking* bookingHistory;
} User;

typedef struct {
    char name[50];
    int availableSeats;
    int seats[MAX_SEATS];
    int waitlist[MAX_WAITLIST];
    int waitlistCount;
} Event;

typedef struct Stack {
    int eventID;
    int seatNumber;
    struct Stack* next;
} Stack;

// Structure to hold application state
typedef struct {
    Event events[MAX_EVENTS];
    User users[MAX_USERS];
    User* userHashTable[HASH_SIZE];
    Stack* undoStack;
    int numUsers;
    User* loggedInUser;  // Keep track of the logged-in user
} AppState;

// Function Prototypes
int hashFunction(const char* username);
void addUserToHashTable(AppState* appState, User* user);
User* searchUserInHashTable(AppState* appState, const char* username);
void pushUndo(AppState* appState, int eventID, int seatNumber);
Stack* popUndo(AppState* appState);
void displayEvents(AppState* appState);
void bookTicket(AppState* appState, Event* event, int seatChoice);
void undoLastBooking(AppState* appState);
void registerUser(AppState* appState);
void loginUser(AppState* appState);
void initializeAppState(AppState* appState);

// Main Function
int main() {
    AppState appState;
    initializeAppState(&appState);

    int choice;

    while (1) {
        printf("\n--- Ticket Booking System ---\n");
        printf("1. Register\n");
        printf("2. Login\n");
        printf("3. View Events\n");
        printf("4. Book Ticket\n");
        printf("5. Undo Booking\n");
        printf("6. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                registerUser(&appState);
                break;
            case 2:
                loginUser(&appState);
                break;
            case 3:
                displayEvents(&appState);  // View events without login
                break;
            case 4:
                if (appState.loggedInUser == NULL) {
                    printf("Login required to book a ticket.\n");
                    break;
                }
                displayEvents(&appState);
                int eventChoice;
                printf("Select event (1-%d): ", MAX_EVENTS);
                scanf("%d", &eventChoice);
                if (eventChoice >= 1 && eventChoice <= MAX_EVENTS) {
                    // Ask user to select a seat
                    printf("Enter seat number to book (1-%d): ", MAX_SEATS);
                    int seatChoice;
                    scanf("%d", &seatChoice);
                    if (seatChoice >= 1 && seatChoice <= MAX_SEATS) {
                        bookTicket(&appState, &appState.events[eventChoice - 1], seatChoice - 1);
                    } else {
                        printf("Invalid seat choice.\n");
                    }
                } else {
                    printf("Invalid event choice.\n");
                }
                break;
            case 5:
                if (appState.loggedInUser == NULL) {
                    printf("Login required to undo booking.\n");
                    break;
                }
                undoLastBooking(&appState);
                break;
            case 6:
                printf("Goodbye!\n");
                exit(0);
            default:
                printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}

// Function Definitions
void initializeAppState(AppState* appState) {
    const char* eventNames[] = {"Movie A", "Concert B", "Play C", "Comedy D", "Festival E"};
    for (int i = 0; i < MAX_EVENTS; i++) {
        strcpy(appState->events[i].name, eventNames[i]);
        appState->events[i].availableSeats = MAX_SEATS;
        memset(appState->events[i].seats, 0, sizeof(appState->events[i].seats));
        appState->events[i].waitlistCount = 0;
    }
    appState->undoStack = NULL;
    appState->numUsers = 0;
    appState->loggedInUser = NULL;  // No one is logged in initially
    memset(appState->userHashTable, 0, sizeof(appState->userHashTable));
}

int hashFunction(const char* username) {
    int hashValue = 0;
    for (int i = 0; i < strlen(username); i++) {
        hashValue = (hashValue + username[i]) % HASH_SIZE;
    }
    return hashValue;
}

void addUserToHashTable(AppState* appState, User* user) {
    int hashIndex = hashFunction(user->username);
    appState->userHashTable[hashIndex] = user;
}

User* searchUserInHashTable(AppState* appState, const char* username) {
    int hashIndex = hashFunction(username);
    User* user = appState->userHashTable[hashIndex];
    return (user && strcmp(user->username, username) == 0) ? user : NULL;
}

void pushUndo(AppState* appState, int eventID, int seatNumber) {
    Stack* newNode = (Stack*)malloc(sizeof(Stack));
    newNode->eventID = eventID;
    newNode->seatNumber = seatNumber;
    newNode->next = appState->undoStack;
    appState->undoStack = newNode;
}

Stack* popUndo(AppState* appState) {
    if (appState->undoStack == NULL) return NULL;
    Stack* temp = appState->undoStack;
    appState->undoStack = appState->undoStack->next;
    return temp;
}

void displayEvents(AppState* appState) {
    printf("\n=============================================\n");
    printf("         Available Events            \n");
    printf("\n=============================================\n");
    printf("| %-3s | %-20s | %-12s |\n", "ID", "Event Name", "Seats Left");
    printf("\n=============================================\n");
    for (int i = 0; i < MAX_EVENTS; i++) {
        printf("| %-3d | %-20s | %-12d |\n",
               i + 1,
               appState->events[i].name,
               appState->events[i].availableSeats);
    }
    printf("\n=============================================\n");
}

void bookTicket(AppState* appState, Event* event, int seatChoice) {
    if (event->availableSeats == 0) {
        printf("No available seats for %s. Adding to waitlist...\n", event->name);
        if (event->waitlistCount < MAX_WAITLIST) {
            event->waitlist[event->waitlistCount++] = appState->loggedInUser - appState->users;
            printf("Added to waitlist.\n");
        } else {
            printf("Waitlist is full.\n");
        }
    } else {
        if (event->seats[seatChoice] == 0) {
            event->seats[seatChoice] = appState->loggedInUser - appState->users + 1;
            event->availableSeats--;
            printf("Booked seat %d for %s.\n", seatChoice + 1, appState->loggedInUser->username);
            pushUndo(appState, event - appState->events, seatChoice);
        } else {
            printf("Seat %d is already booked.\n", seatChoice + 1);
        }
    }
}

void undoLastBooking(AppState* appState) {
    Stack* undoAction = popUndo(appState);
    if (undoAction) {
        Event* event = &appState->events[undoAction->eventID];
        event->seats[undoAction->seatNumber] = 0;
        event->availableSeats++;
        printf("Undid booking for event: %s, seat: %d\n", event->name, undoAction->seatNumber + 1);
        free(undoAction);
    } else {
        printf("No actions to undo.\n");
    }
}

void registerUser(AppState* appState) {
    User newUser;
    printf("Enter username: ");
    scanf("%s", newUser.username);
    printf("Enter password: ");
    scanf("%s", newUser.password);
    newUser.isVIP = false; // For simplicity, setting VIP status to false.
    newUser.bookingHistory = NULL;
    appState->users[appState->numUsers] = newUser;
    addUserToHashTable(appState, &appState->users[appState->numUsers]);
    appState->numUsers++;
    printf("User registered successfully.\n");
}

void loginUser(AppState* appState) {
    char username[50], password[50];
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    User* user = searchUserInHashTable(appState, username);
    if (user && strcmp(user->password, password) == 0) {
        appState->loggedInUser = user;
        printf("Login successful.\n");
    } else {
        printf("Invalid credentials.\n");
    }
}

/*
--- Ticket Booking System ---
1. Register
2. Login
3. View Events
4. Book Ticket
5. Undo Booking
6. Exit
Enter choice: 3

=============================================
         Available Events            

=============================================
| ID  | Event Name           | Seats Left   |

=============================================
| 1   | Movie A              | 10           |
| 2   | Concert B            | 10           |
| 3   | Play C               | 10           |
| 4   | Comedy D             | 10           |
| 5   | Festival E           | 10           |

=============================================

--- Ticket Booking System ---
1. Register
2. Login
3. View Events
4. Book Ticket
5. Undo Booking
6. Exit
Enter choice: 1
Enter username: manushri
Enter password: abc123
User registered successfully.

--- Ticket Booking System ---
1. Register
2. Login
3. View Events
4. Book Ticket
5. Undo Booking
6. Exit
Enter choice: 2
Enter username: manushri
Enter password: abc123
Login successful.

--- Ticket Booking System ---
1. Register
2. Login
3. View Events
4. Book Ticket
5. Undo Booking
6. Exit
Enter choice: 4

=============================================
         Available Events            

=============================================
| ID  | Event Name           | Seats Left   |

=============================================
| 1   | Movie A              | 10           |
| 2   | Concert B            | 10           |
| 3   | Play C               | 10           |
| 4   | Comedy D             | 10           |
| 5   | Festival E           | 10           |

=============================================
Select event (1-5): 3
Enter seat number to book (1-10): 5
Booked seat 5 for manushri.

--- Ticket Booking System ---
1. Register
2. Login
3. View Events
4. Book Ticket
5. Undo Booking
6. Exit
Enter choice: 1
Enter username: nandini
Enter password: xyz567
User registered successfully.

--- Ticket Booking System ---
1. Register
2. Login
3. View Events
4. Book Ticket
5. Undo Booking
6. Exit
Enter choice: 2
Enter username: nandini
Enter password: xyz567
Login successful.

--- Ticket Booking System ---
1. Register
2. Login
3. View Events
4. Book Ticket
5. Undo Booking
6. Exit
Enter choice: 4

=============================================
         Available Events            

=============================================
| ID  | Event Name           | Seats Left   |

=============================================
| 1   | Movie A              | 10           |
| 2   | Concert B            | 10           |
| 3   | Play C               | 9            |
| 4   | Comedy D             | 10           |
| 5   | Festival E           | 10           |

=============================================
Select event (1-5): 4
Enter seat number to book (1-10): 8
Booked seat 8 for nandini.

--- Ticket Booking System ---
1. Register
2. Login
3. View Events
4. Book Ticket
5. Undo Booking
6. Exit
Enter choice: 3

=============================================
         Available Events            

=============================================
| ID  | Event Name           | Seats Left   |

=============================================
| 1   | Movie A              | 10           |
| 2   | Concert B            | 10           |
| 3   | Play C               | 9            |
| 4   | Comedy D             | 9            |
| 5   | Festival E           | 10           |

=============================================

--- Ticket Booking System ---
1. Register
2. Login
3. View Events
4. Book Ticket
5. Undo Booking
6. Exit
Enter choice: 5
Undid booking for event: Comedy D, seat: 8

--- Ticket Booking System ---
1. Register
2. Login
3. View Events
4. Book Ticket
5. Undo Booking
6. Exit
Enter choice: 3

=============================================
         Available Events            

=============================================
| ID  | Event Name           | Seats Left   |

=============================================
| 1   | Movie A              | 10           |
| 2   | Concert B            | 10           |
| 3   | Play C               | 9            |
| 4   | Comedy D             | 10           |
| 5   | Festival E           | 10           |

=============================================

--- Ticket Booking System ---
1. Register
2. Login
3. View Events
4. Book Ticket
5. Undo Booking
6. Exit
Enter choice: 6
Goodbye!

*/