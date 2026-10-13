#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SERVERS 20
#define NAME_LEN 32

/* ================================================================= */
/* DATA STRUCTURE DEFINITIONS                                        */
/* ================================================================= */

typedef struct {
    char servers[MAX_SERVERS][NAME_LEN];
    int matrix[MAX_SERVERS][MAX_SERVERS]; // matrix[u][v] = 1 if u depends on/points to v
    int server_count;
} ServerGraph;

/* ================================================================= */
/* CORE FUNCTIONS                                                    */
/* ================================================================= */

// 1. Initialize the directed graph
ServerGraph* init_server_graph() {
    ServerGraph* g = (ServerGraph*)malloc(sizeof(ServerGraph));
    if (!g) {
        perror("Memory allocation failed for graph");
        exit(EXIT_FAILURE);
    }
    g->server_count = 0;
    memset(g->matrix, 0, sizeof(g->matrix));
    return g;
}

// Find server index by name (-1 if not found)
int find_server_index(ServerGraph* g, const char* name) {
    for (int i = 0; i < g->server_count; i++) {
        if (strcmp(g->servers[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

// 3. Add a new server
void add_server(ServerGraph* g, const char* name) {
    if (g->server_count >= MAX_SERVERS) {
        printf("[Graph Error] Maximum server capacity (%d) reached.\n", MAX_SERVERS);
        return;
    }
    if (find_server_index(g, name) != -1) {
        printf("[Graph Error] Server '%s' already exists.\n", name);
        return;
    }
    strncpy(g->servers[g->server_count], name, NAME_LEN - 1);
    g->servers[g->server_count][NAME_LEN - 1] = '\0';
    g->server_count++;
    printf("[Success] Server '%s' added successfully.\n", name);
}

// 3. Remove an existing server and all its connections
void remove_server(ServerGraph* g, const char* name) {
    int idx = find_server_index(g, name);
    if (idx == -1) {
        printf("[Graph Error] Server '%s' does not exist.\n", name);
        return;
    }

    // Shift servers down
    for (int i = idx; i < g->server_count - 1; i++) {
        strcpy(g->servers[i], g->servers[i + 1]);
    }

    // Shift adjacency matrix rows
    for (int i = idx; i < g->server_count - 1; i++) {
        for (int j = 0; j < g->server_count; j++) {
            g->matrix[i][j] = g->matrix[i + 1][j];
        }
    }

    // Shift adjacency matrix columns
    for (int i = 0; i < g->server_count; i++) {
        for (int j = idx; j < g->server_count - 1; j++) {
            g->matrix[i][j] = g->matrix[i][j + 1];
        }
    }

    // Clear the last row and column
    g->server_count--;
    for (int i = 0; i <= g->server_count; i++) {
        g->matrix[g->server_count][i] = 0;
        g->matrix[i][g->server_count] = 0;
    }

    printf("[Success] Server '%s' and its connections removed.\n", name);
}

// 3. Create a directed dependency (from -> to)
void create_dependency(ServerGraph* g, const char* from, const char* to) {
    int u = find_server_index(g, from);
    int v = find_server_index(g, to);

    if (u == -1 || v == -1) {
        printf("[Graph Error] One or both servers ('%s', '%s') do not exist.\n", from, to);
        return;
    }
    if (g->matrix[u][v] == 1) {
        printf("[Graph Error] Dependency already exists: %s -> %s\n", from, to);
        return;
    }

    g->matrix[u][v] = 1;
    printf("[Success] Dependency created: %s -> %s\n", from, to);
}

// 3. Delete an existing dependency
void delete_dependency(ServerGraph* g, const char* from, const char* to) {
    int u = find_server_index(g, from);
    int v = find_server_index(g, to);

    if (u == -1 || v == -1) {
        printf("[Graph Error] One or both servers ('%s', '%s') do not exist.\n", from, to);
        return;
    }
    if (g->matrix[u][v] == 0) {
        printf("[Graph Error] No existing dependency from %s to %s.\n", from, to);
        return;
    }

    g->matrix[u][v] = 0;
    printf("[Success] Dependency removed: %s -> %s\n", from, to);
}

// 2. Query server dependencies (Direct dependencies & incoming dependent services)
void query_server(ServerGraph* g, const char* name) {
    int idx = find_server_index(g, name);
    if (idx == -1) {
        printf("[Graph Error] Server '%s' does not exist.\n", name);
        return;
    }

    printf("\n=== Dependency Query: %s ===\n", name);
    
    printf("Direct dependencies (servers %s depends on):\n", name);
    int dep_count = 0;
    for (int j = 0; j < g->server_count; j++) {
        if (g->matrix[idx][j] == 1) {
            printf(" - %s\n", g->servers[j]);
            dep_count++;
        }
    }
    if (dep_count == 0) printf(" (None)\n");

    printf("Services depending on %s:\n", name);
    int dependent_count = 0;
    for (int i = 0; i < g->server_count; i++) {
        if (g->matrix[i][idx] == 1) {
            printf(" - %s\n", g->servers[i]);
            dependent_count++;
        }
    }
    if (dependent_count == 0) printf(" (None)\n");
    printf("----------------------------------------\n");
}

// 4. Generate and display adjacency matrix
void display_adjacency_matrix(ServerGraph* g) {
    if (g->server_count == 0) {
        printf("[Graph] Network is empty.\n");
        return;
    }

    printf("\n======================= SERVER ADJACENCY MATRIX =======================\n    ");
    for (int i = 0; i < g->server_count; i++) {
        printf("%-16.14s ", g->servers[i]);
    }
    printf("\n");

    for (int i = 0; i < g->server_count; i++) {
        printf("%-4.4s ", g->servers[i]);
        for (int j = 0; j < g->server_count; j++) {
            printf("%-16d ", g->matrix[i][j]);
        }
        printf("\n");
    }
    printf("=======================================================================\n");
}

/* ================================================================= */
/* MAIN INTERFACE                                                    */
/* ================================================================= */

void print_menu() {
    printf("\n=== SERVER DEPENDENCY NETWORK ANALYZER ===\n");
    printf(" a : Add server\n");
    printf(" r : Remove server\n");
    printf(" c : Create dependency\n");
    printf(" d : Delete dependency\n");
    printf(" q : Query server dependencies\n");
    printf(" m : Display adjacency matrix\n");
    printf(" x : Exit program\n");
    printf("Enter command: ");
}

int main() {
    ServerGraph* g = init_server_graph();

    // Initialize the default network topology specified in the problem
    add_server(g, "Web Server");
    add_server(g, "API Server");
    add_server(g, "Authentication Server");
    add_server(g, "Database Server");
    add_server(g, "Cache Server");
    add_server(g, "Backup Server");

    create_dependency(g, "Web Server", "API Server");
    create_dependency(g, "Web Server", "Authentication Server");
    create_dependency(g, "API Server", "Database Server");
    create_dependency(g, "API Server", "Cache Server");
    create_dependency(g, "Authentication Server", "Database Server");
    create_dependency(g, "Cache Server", "Database Server");
    create_dependency(g, "Database Server", "Backup Server");

    char choice;
    while (1) {
        print_menu();
        if (scanf(" %c", &choice) != 1) {
            printf("[Error] Invalid input. Try again.\n");
            while (getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 'a':
            case 'A': {
                char name[NAME_LEN];
                printf("Enter new server name: ");
                scanf("%31s", name);
                add_server(g, name);
                break;
            }
            case 'r':
            case 'R': {
                char name[NAME_LEN];
                printf("Enter server name to remove: ");
                scanf("%31s", name);
                remove_server(g, name);
                break;
            }
            case 'c':
            case 'C': {
                char from[NAME_LEN], to[NAME_LEN];
                printf("Enter source server (dependency from): ");
                scanf("%31s", from);
                printf("Enter target server (dependency to): ");
                scanf("%31s", to);
                create_dependency(g, from, to);
                break;
            }
            case 'd':
            case 'D': {
                char from[NAME_LEN], to[NAME_LEN];
                printf("Enter source server: ");
                scanf("%31s", from);
                printf("Enter target server: ");
                scanf("%31s", to);
                delete_dependency(g, from, to);
                break;
            }
            case 'q':
            case 'Q': {
                char name[NAME_LEN];
                printf("Enter server name to query: ");
                scanf("%31s", name);
                query_server(g, name);
                break;
            }
            case 'm':
            case 'M':
                display_adjacency_matrix(g);
                break;
            case 'x':
            case 'X':
                printf("[Exit] Freeing graph memory and exiting...\n");
                free(g);
                printf("[Exit] Program terminated safely.\n");
                return 0;
            default:
                printf("[Error] Unknown command '%c'. Please use a, r, c, d, q, m, or x.\n", choice);
        }
    }

    return 0;
}
