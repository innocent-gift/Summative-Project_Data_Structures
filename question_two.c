#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_COMMANDS 50
#define MAX_CMD_LEN 64

/* ================================================================= */
/* DATA STRUCTURE DEFINITIONS                                        */
/* ================================================================= */

typedef struct BSTNode {
    char command[MAX_CMD_LEN];
    struct BSTNode* left;
    struct BSTNode* right;
} BSTNode;

typedef struct {
    BSTNode* root;
    int count;
} CommandRegistry;

/* ================================================================= */
/* CORE FUNCTIONS                                                    */
/* ================================================================= */

// 1. Initialize the registry
CommandRegistry* init_registry() {
    CommandRegistry* reg = (CommandRegistry*)malloc(sizeof(CommandRegistry));
    if (!reg) {
        perror("Memory allocation failed for registry");
        exit(EXIT_FAILURE);
    }
    reg->root = NULL;
    reg->count = 0;
    return reg;
}

// Create a new BST node
BSTNode* create_node(const char* cmd) {
    BSTNode* node = (BSTNode*)malloc(sizeof(BSTNode));
    if (!node) {
        perror("Memory allocation failed for BST node");
        exit(EXIT_FAILURE);
    }
    strncpy(node->command, cmd, MAX_CMD_LEN - 1);
    node->command[MAX_CMD_LEN - 1] = '\0';
    node->left = NULL;
    node->right = NULL;
    return node;
}

// Insert command into BST (maintains lexicographical order & avoids duplicates)
BSTNode* insert_bst(BSTNode* root, const char* cmd, int* count, int* added) {
    if (!root) {
        if (*count >= MAX_COMMANDS) {
            printf("[Registry] Warning: Maximum command limit (%d) reached.\n", MAX_COMMANDS);
            *added = 0;
            return NULL;
        }
        (*count)++;
        *added = 1;
        return create_node(cmd);
    }

    int cmp = strcmp(cmd, root->command);
    if (cmp < 0) {
        root->left = insert_bst(root->left, cmd, count, added);
    } else if (cmp > 0) {
        root->right = insert_bst(root->right, cmd, count, added);
    } else {
        // Duplicate command
        *added = 0;
    }
    return root;
}

// Load commands from text file
void load_commands_from_file(CommandRegistry* reg, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("[File Error] Could not open '%s'. Initializing with default commands...\n", filename);
        // Fallback default commands if file doesn't exist yet
        const char* defaults[] = {
            "ADD_ROUTE", "DELETE_ROUTE", "SHOW_STATUS", 
            "RESET_INTERFACE", "SET_IP", "SET_GATEWAY", 
            "ENABLE_FIREWALL", "DISABLE_FIREWALL"
        };
        for (int i = 0; i < 8; i++) {
            int added = 0;
            reg->root = insert_bst(reg->root, defaults[i], &reg->count, &added);
        }
        return;
    }

    char buffer[MAX_CMD_LEN];
    while (fgets(buffer, sizeof(buffer), file)) {
        // Remove trailing newline or carriage return
        buffer[strcspn(buffer, "\r\n")] = '\0';
        if (strlen(buffer) == 0) continue; // Skip empty lines

        int added = 0;
        reg->root = insert_bst(reg->root, buffer, &reg->count, &added);
    }
    fclose(file);
    printf("[File] Successfully loaded commands from '%s'. Total: %d\n", filename, reg->count);
}

// 3. BST-based Search (Exact Match)
int search_bst(BSTNode* root, const char* cmd) {
    if (!root) return 0;
    int cmp = strcmp(cmd, root->command);
    if (cmp == 0) return 1;
    if (cmp < 0) return search_bst(root->left, cmd);
    return search_bst(root->right, cmd);
}

// Simple Levenshtein distance calculation for similarity checking
int calculate_levenshtein(const char* s1, const char* s2) {
    int len1 = strlen(s1), len2 = strlen(s2);
    int matrix[50][50];

    for (int i = 0; i <= len1; i++) matrix[i][0] = i;
    for (int j = 0; j <= len2; j++) matrix[0][j] = j;

    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            int deletion = matrix[i - 1][j] + 1;
            int insertion = matrix[i][j - 1] + 1;
            int substitution = matrix[i - 1][j - 1] + cost;

            int min = deletion < insertion ? deletion : insertion;
            matrix[i][j] = min < substitution ? min : substitution;
        }
    }
    return matrix[len1][len2];
}

// Helper structure for finding the closest match suggestion
typedef struct {
    char best_match[MAX_CMD_LEN];
    int min_distance;
} SuggestionContext;

void find_closest_match(BSTNode* root, const char* target, SuggestionContext* ctx) {
    if (!root) return;

    int dist = calculate_levenshtein(root->command, target);
    if (dist < ctx->min_distance) {
        ctx->min_distance = dist;
        strncpy(ctx->best_match, root->command, MAX_CMD_LEN - 1);
    }

    find_closest_match(root->left, target, ctx);
    find_closest_match(root->right, target, ctx);
}

// 4. Audit Logging for Unknown Commands
void log_audit(const char* cmd) {
    FILE* f = fopen("audit.log", "a");
    if (!f) {
        printf("[Error] Could not write to audit.log\n");
        return;
    }
    time_t now = time(NULL);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(f, "%s | UNKNOWN | %s\n", time_str, cmd);
    fclose(f);
    printf("[Audit Log] Recorded unknown command '%s' to audit.log\n", cmd);
}

// 3. Alphabetical In-Order Traversal
void in_order_traversal(BSTNode* root) {
    if (!root) return;
    in_order_traversal(root->left);
    printf(" - %s\n", root->command);
    in_order_traversal(root->right);
}

// 5. Clear Memory (Free BST)
void clear_bst(BSTNode* root) {
    if (!root) return;
    clear_bst(root->left);
    clear_bst(root->right);
    free(root);
}

/* ================================================================= */
/* MAIN INTERFACE                                                    */
/* ================================================================= */

void print_menu() {
    printf("\n=== NETWORK DEVICE COMMAND REGISTRY ===\n");
    printf(" v : Verify a device command\n");
    printf(" p : Print all approved commands (Alphabetical)\n");
    printf(" d : Display number of stored commands\n");
    printf(" x : Clear the command registry\n");
    printf(" q : Save session and quit\n");
    printf("Enter command: ");
}

int main() {
    CommandRegistry* reg = init_registry();
    
    // Load commands (tries 'commands.txt' first, defaults if missing)
    load_commands_from_file(reg, "commands.txt");

    char choice;
    while (1) {
        print_menu();
        if (scanf(" %c", &choice) != 1) {
            printf("[Error] Invalid input. Try again.\n");
            while (getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 'v':
            case 'V': {
                char input_cmd[MAX_CMD_LEN];
                printf("Enter device command to verify: ");
                scanf("%63s", input_cmd);

                if (search_bst(reg->root, input_cmd)) {
                    printf("RESULT: COMMAND APPROVED\n");
                } else {
                    printf("RESULT: UNKNOWN COMMAND\n");
                    log_audit(input_cmd);

                    // Check for suggestions if close enough
                    if (reg->root) {
                        SuggestionContext ctx;
                        ctx.min_distance = 999;
                        ctx.best_match[0] = '\0';
                        find_closest_match(reg->root, input_cmd, &ctx);

                        // If distance is reasonably small (e.g., <= 3 edits)
                        if (ctx.min_distance <= 3 && strlen(ctx.best_match) > 0) {
                            printf("Did you mean: %s?\n", ctx.best_match);
                        }
                    }
                }
                break;
            }
            case 'p':
            case 'P':
                printf("\n--- Approved Commands (Alphabetical Order) ---\n");
                if (reg->root == NULL) {
                    printf("Registry is empty.\n");
                } else {
                    in_order_traversal(reg->root);
                }
                printf("-----------------------------------------------\n");
                break;
            case 'd':
            case 'D':
                printf("\nTotal stored approved commands: %d / %d\n", reg->count, MAX_COMMANDS);
                break;
            case 'x':
            case 'X':
                clear_bst(reg->root);
                reg->root = NULL;
                reg->count = 0;
                printf("[Registry] Command registry cleared.\n");
                break;
            case 'q':
            case 'Q':
                printf("[Exit] Freeing BST memory and quitting...\n");
                clear_bst(reg->root);
                free(reg);
                printf("[Exit] Program terminated safely.\n");
                return 0;
            default:
                printf("[Error] Unknown option '%c'. Please use v, p, d, x, or q.\n", choice);
        }
    }

    return 0;
}
