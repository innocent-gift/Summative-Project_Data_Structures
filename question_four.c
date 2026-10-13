#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LOCATIONS 20
#define NAME_LEN 32
#define INF 999999

/* ================================================================= */
/* DATA STRUCTURE DEFINITIONS                                        */
/* ================================================================= */

typedef struct {
    char locations[MAX_LOCATIONS][NAME_LEN];
    int weights[MAX_LOCATIONS][MAX_LOCATIONS]; // Adjacency matrix for road distances
    int count;
} WeightedGraph;

/* ================================================================= */
/* CORE FUNCTIONS                                                    */
/* ================================================================= */

// 1. Initialize the weighted graph
WeightedGraph* init_weighted_graph() {
    WeightedGraph* g = (WeightedGraph*)malloc(sizeof(WeightedGraph));
    if (!g) {
        perror("Memory allocation failed for graph");
        exit(EXIT_FAILURE);
    }
    g->count = 0;
    for (int i = 0; i < MAX_LOCATIONS; i++) {
        for (int j = 0; j < MAX_LOCATIONS; j++) {
            g->weights[i][j] = (i == j) ? 0 : INF;
        }
    }
    return g;
}

// Find location index by name (-1 if not found)
int find_location_index(WeightedGraph* g, const char* name) {
    for (int i = 0; i < g->count; i++) {
        if (strcmp(g->locations[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

// Add location if it doesn't exist
int get_or_add_location(WeightedGraph* g, const char* name) {
    int idx = find_location_index(g, name);
    if (idx != -1) return idx;

    if (g->count >= MAX_LOCATIONS) {
        printf("[Graph Error] Maximum location capacity (%d) reached.\n", MAX_LOCATIONS);
        return -1;
    }
    strncpy(g->locations[g->count], name, NAME_LEN - 1);
    g->locations[g->count][NAME_LEN - 1] = '\0';
    return g->count++;
}

// Add a weighted road between two locations
void add_road(WeightedGraph* g, const char* loc1, const char* loc2, int dist) {
    if (dist < 0) {
        printf("[Error] Road distance cannot be negative.\n");
        return;
    }
    int u = get_or_add_location(g, loc1);
    int v = get_or_add_location(g, loc2);

    if (u != -1 && v != -1) {
        g->weights[u][v] = dist;
        g->weights[v][u] = dist; // Undirected road network
    }
}

// Display all registered locations
void display_locations(WeightedGraph* g) {
    printf("\n=== Registered Medical Facilities & Warehouses ===\n");
    if (g->count == 0) {
        printf("No locations registered.\n");
        return;
    }
    for (int i = 0; i < g->count; i++) {
        printf(" %d. %s\n", i + 1, g->locations[i]);
    }
    printf("--------------------------------------------------\n");
}

// 2, 3, 4. Dijkstra's Algorithm for Shortest Route
void find_shortest_route(WeightedGraph* g, const char* start, const char* dest) {
    int src = find_location_index(g, start);
    int target = find_location_index(g, dest);

    if (src == -1 || target == -1) {
        printf("[Routing Error] Start or destination location does not exist in the network.\n");
        return;
    }
    if (src == target) {
        printf("[Routing Error] Start and destination locations are the same (%s).\n", start);
        return;
    }

    int dist[MAX_LOCATIONS];
    int prev[MAX_LOCATIONS];
    int visited[MAX_LOCATIONS];

    for (int i = 0; i < g->count; i++) {
        dist[i] = INF;
        prev[i] = -1;
        visited[i] = 0;
    }
    dist[src] = 0;

    // Dijkstra's core loop
    for (int count = 0; count < g->count - 1; count++) {
        int min = INF, u = -1;
        for (int i = 0; i < g->count; i++) {
            if (!visited[i] && dist[i] <= min) {
                min = dist[i];
                u = i;
            }
        }

        if (u == -1 || min == INF) break;
        visited[u] = 1;

        for (int v = 0; v < g->count; v++) {
            if (!visited[v] && g->weights[u][v] != INF && dist[u] != INF && dist[u] + g->weights[u][v] < dist[v]) {
                dist[v] = dist[u] + g->weights[u][v];
                prev[v] = u;
            }
        }
    }

    // Check if destination is reachable
    if (dist[target] == INF) {
        printf("[Routing] Destination '%s' cannot be reached from '%s'.\n", dest, start);
        return;
    }

    // Reconstruct path
    int path[MAX_LOCATIONS];
    int path_count = 0;
    int curr = target;
    while (curr != -1) {
        path[path_count++] = curr;
        curr = prev[curr];
    }

    // Output results
    printf("\n================ SHORTEST ROUTE RESULT ================\n");
    printf("Shortest Route:\n");
    for (int i = path_count - 1; i >= 0; i--) {
        printf("%s%s", g->locations[path[i]], (i > 0) ? " → " : "\n");
    }
    printf("Total Distance: %d km\n", dist[target]);
    printf("=======================================================\n");
}

/* ================================================================= */
/* MAIN INTERFACE                                                    */
/* ================================================================= */

void print_menu() {
    printf("\n=== EMERGENCY MEDICAL SUPPLY ROUTING ===\n");
    printf(" 1. Display locations\n");
    printf(" 2. Find shortest route\n");
    printf(" 3. Display route distance matrix\n");
    printf(" 4. Exit\n");
    printf("Enter choice: ");
}

int main() {
    WeightedGraph* g = init_weighted_graph();

    // Initialize the road network specified in the problem
    add_road(g, "Central Warehouse", "Distribution Center", 8);
    add_road(g, "Central Warehouse", "District Hospital", 14);
    add_road(g, "Distribution Center", "Health Center A", 5);
    add_road(g, "Distribution Center", "Health Center B", 11);
    add_road(g, "Health Center A", "District Hospital", 7);
    add_road(g, "Health Center B", "Health Center C", 3);
    add_road(g, "Health Center C", "District Hospital", 6);
    add_road(g, "Health Center B", "District Hospital", 9);
    add_road(g, "Health Center A", "Health Center C", 4);

    int choice;
    do {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            printf("[Error] Invalid input. Try again.\n");
            while (getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1:
                display_locations(g);
                break;
            case 2: {
                char start[NAME_LEN], dest[NAME_LEN];
                display_locations(g);
                printf("Enter Starting Location: ");
                scanf(" %[^\n]s", start); // Read string with spaces if any
                printf("Enter Destination Location: ");
                scanf(" %[^\n]s", dest);
                find_shortest_route(g, start, dest);
                break;
            }
            case 3: {
                printf("\n--- Road Network Distance Matrix (km) ---\n    ");
                for (int i = 0; i < g->count; i++) printf("%-18.16s ", g->locations[i]);
                printf("\n");
                for (int i = 0; i < g->count; i++) {
                    printf("%-4.4s ", g->locations[i]);
                    for (int j = 0; j < g->count; j++) {
                        if (g->weights[i][j] == INF) printf("%-18s ", "INF");
                        else printf("%-18d ", g->weights[i][j]);
                    }
                    printf("\n");
                }
                printf("------------------------------------------\n");
                break;
            }
            case 4:
                printf("[Exit] Freeing graph memory and exiting...\n");
                free(g);
                printf("[Exit] Program terminated safely.\n");
                return 0;
            default:
                printf("[Error] Invalid choice. Please select between 1 and 4.\n");
        }
    } while (choice != 4);

    return 0;
}
