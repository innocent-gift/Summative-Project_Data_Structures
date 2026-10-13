#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_EVENTS 25

/* ================================================================= */
/* DATA STRUCTURE DEFINITIONS                                        */
/* ================================================================= */

typedef struct EventNode {
    int event_id;
    char event_type[32];
    char device_id[32];
    char timestamp[32];
    int severity; // 1: Info, 2: Warning, 3: Critical
    struct EventNode* prev;
    struct EventNode* next;
} EventNode;

typedef struct {
    EventNode* head;
    EventNode* tail;
    EventNode* current;
    int count;
} EventLogger;

/* ================================================================= */
/* CORE FUNCTIONS                                                    */
/* ================================================================= */

// 1. Initialize the event logger
EventLogger* init_event_logger() {
    EventLogger* logger = (EventLogger*)malloc(sizeof(EventLogger));
    if (!logger) {
        perror("Memory allocation failed for logger");
        exit(EXIT_FAILURE);
    }
    logger->head = NULL;
    logger->tail = NULL;
    logger->current = NULL;
    logger->count = 0;
    return logger;
}

// Helper to generate current timestamp string
void get_current_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// 2. Event Insertion & Memory Limit Handling (Max 25 events)
void add_event(EventLogger* logger, int id, const char* type, const char* dev_id, int severity) {
    EventNode* new_node = (EventNode*)malloc(sizeof(EventNode));
    if (!new_node) {
        printf("[Error] Memory allocation failed for new event.\n");
        return;
    }

    new_node->event_id = id;
    strncpy(new_node->event_type, type, sizeof(new_node->event_type) - 1);
    new_node->event_type[sizeof(new_node->event_type) - 1] = '\0';
    
    strncpy(new_node->device_id, dev_id, sizeof(new_node->device_id) - 1);
    new_node->device_id[sizeof(new_node->device_id) - 1] = '\0';
    
    new_node->severity = severity;
    get_current_timestamp(new_node->timestamp, sizeof(new_node->timestamp));

    new_node->prev = NULL;
    new_node->next = NULL;

    // Handle memory limit: If list reaches 25, remove the oldest event (head)
    if (logger->count >= MAX_EVENTS) {
        EventNode* old_head = logger->head;
        logger->head = old_head->next;
        
        if (logger->head) {
            logger->head->prev = NULL;
        } else {
            logger->tail = NULL;
        }

        // Adjust current pointer if it points to the removed node
        if (logger->current == old_head) {
            logger->current = logger->head;
        }

        free(old_head);
        logger->count--;
        printf("[Memory Manager] Maximum capacity (25) reached. Oldest event removed.\n");
    }

    // Insert new event at the end (tail)
    if (logger->head == NULL) {
        logger->head = new_node;
        logger->tail = new_node;
        logger->current = new_node; // Current points to the first event inserted
    } else {
        logger->tail->next = new_node;
        new_node->prev = logger->tail;
        logger->tail = new_node;
    }
    
    logger->count++;
    printf("[Success] Event ID %d added. Total stored: %d/%d\n", id, logger->count, MAX_EVENTS);
}

// 3. Event Navigation
void move_next(EventLogger* logger) {
    if (!logger->current) {
        printf("[Navigation] The event list is empty.\n");
        return;
    }
    if (logger->current->next) {
        logger->current = logger->current->next;
        printf("[Navigation] Moved to newer event.\n");
    } else {
        printf("[Navigation] Already at the newest event.\n");
    }
}

void move_prev(EventLogger* logger) {
    if (!logger->current) {
        printf("[Navigation] The event list is empty.\n");
        return;
    }
    if (logger->current->prev) {
        logger->current = logger->current->prev;
        printf("[Navigation] Moved to older event.\n");
    } else {
        printf("[Navigation] Already at the oldest event.\n");
    }
}

// Display current event details
void display_current_event(EventLogger* logger) {
    if (!logger->current) {
        printf("\n--- Current Event ---\n");
        printf("No events available in the logger.\n");
        printf("---------------------\n");
        return;
    }
    printf("\n==================== CURRENT EVENT ====================\n");
    printf("Event ID   : %d\n", logger->current->event_id);
    printf("Event Type : %s\n", logger->current->event_type);
    printf("Device ID  : %s\n", logger->current->device_id);
    printf("Timestamp  : %s\n", logger->current->timestamp);
    printf("Severity   : %d (1: Info, 2: Warning, 3: Critical)\n", logger->current->severity);
    printf("-------------------------------------------------------\n");
}

// 5. Memory Management: Clear all events
void clear_all_events(EventLogger* logger) {
    EventNode* current = logger->head;
    int freed_count = 0;
    while (current) {
        EventNode* next_node = current->next;
        free(current);
        current = next_node;
        freed_count++;
    }
    logger->head = NULL;
    logger->tail = NULL;
    logger->current = NULL;
    logger->count = 0;
    printf("[Memory] Cleared all events. %d nodes released.\n", freed_count);
}

/* ================================================================= */
/* MAIN INTERFACE                                                    */
/* ================================================================= */

void print_menu() {
    printf("\n=== EMBEDDED DEVICE EVENT LOGGER ===\n");
    printf(" n : Move to next / newer event\n");
    printf(" p : Move to previous / older event\n");
    printf(" a : Add a new event\n");
    printf(" c : Display current event\n");
    printf(" x : Clear all events\n");
    printf(" q : Save session and terminate\n");
    printf("Enter command: ");
}

int main() {
    EventLogger* logger = init_event_logger();
    char command;
    int auto_id = 100;

    // Seed initial sample events
    add_event(logger, auto_id++, "TEMP_WARNING", "Sensor_TEMP_01", 2);
    add_event(logger, auto_id++, "LOW_BATTERY", "Power_Unit_B", 3);
    add_event(logger, auto_id++, "COMM_ERROR", "Gateway_NET", 3);

    while (1) {
        display_current_event(logger);
        print_menu();
        
        if (scanf(" %c", &command) != 1) {
            printf("[Error] Invalid input. Try again.\n");
            while (getchar() != '\n'); // clear buffer
            continue;
        }

        switch (command) {
            case 'n':
            case 'N':
                move_next(logger);
                break;
            case 'p':
            case 'P':
                move_prev(logger);
                break;
            case 'a':
            case 'A': {
                char type[32], dev_id[32];
                int severity;
                printf("Enter Event Type (e.g., TEMP_WARNING): ");
                scanf("%31s", type);
                printf("Enter Sensor/Device ID: ");
                scanf("%31s", dev_id);
                printf("Enter Severity Level (1-3): ");
                if (scanf("%d", &severity) != 1) {
                    printf("[Error] Invalid severity value.\n");
                    while (getchar() != '\n');
                    break;
                }
                add_event(logger, auto_id++, type, dev_id, severity);
                break;
            }
            case 'c':
            case 'C':
                display_current_event(logger);
                break;
            case 'x':
            case 'X':
                clear_all_events(logger);
                break;
            case 'q':
            case 'Q':
                printf("[Exit] Saving session and freeing remaining memory...\n");
                clear_all_events(logger);
                free(logger);
                printf("[Exit] Program terminated safely.\n");
                return 0;
            default:
                printf("[Error] Unknown command '%c'. Please use n, p, a, c, x, or q.\n", command);
        }
    }

    return 0;
}
