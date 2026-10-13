#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TREE_HT 256

/* ================================================================= */
/* DATA STRUCTURE DEFINITIONS                                        */
/* ================================================================= */

// Huffman Tree Node
typedef struct HuffmanNode {
    unsigned char data;
    unsigned frequency;
    struct HuffmanNode *left, *right;
} HuffmanNode;

// Min Heap for Huffman Tree construction
typedef struct MinHeap {
    unsigned size;
    unsigned capacity;
    HuffmanNode** array;
} MinHeap;

/* ================================================================= */
/* CORE HUFFMAN FUNCTIONS                                            */
/* ================================================================= */

// Create a new Huffman node
HuffmanNode* create_node(unsigned char data, unsigned frequency) {
    HuffmanNode* node = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    node->data = data;
    node->frequency = frequency;
    node->left = node->right = NULL;
    return node;
}

// Create a Min Heap
MinHeap* create_min_heap(unsigned capacity) {
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    heap->size = 0;
    heap->capacity = capacity;
    heap->array = (HuffmanNode**)malloc(capacity * sizeof(HuffmanNode*));
    return heap;
}

// Swap two heap nodes
void swap_nodes(HuffmanNode** a, HuffmanNode** b) {
    HuffmanNode* t = *a;
    *a = *b;
    *b = t;
}

// Min Heapify
void min_heapify(MinHeap* heap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < heap->size && heap->array[left]->frequency < heap->array[smallest]->frequency)
        smallest = left;

    if (right < heap->size && heap->array[right]->frequency < heap->array[smallest]->frequency)
        smallest = right;

    if (smallest != idx) {
        swap_nodes(&heap->array[smallest], &heap->array[idx]);
        min_heapify(heap, smallest);
    }
}

// Extract minimum value node from heap
HuffmanNode* extract_min(MinHeap* heap) {
    HuffmanNode* temp = heap->array[0];
    heap->array[0] = heap->array[heap->size - 1];
    heap->size--;
    min_heapify(heap, 0);
    return temp;
}

// Insert node into min heap
void insert_min_heap(MinHeap* heap, HuffmanNode* node) {
    int i = heap->size++;
    while (i && node->frequency < heap->array[(i - 1) / 2]->frequency) {
        heap->array[i] = heap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    heap->array[i] = node;
}

// Build min heap from frequencies
MinHeap* build_min_heap(unsigned char data[], unsigned freq[], int size) {
    MinHeap* heap = create_min_heap(size);
    for (int i = 0; i < size; ++i)
        heap->array[i] = create_node(data[i], freq[i]);
    heap->size = size;
    for (int i = (size - 1) / 2; i >= 0; --i)
        min_heapify(heap, i);
    return heap;
}

// Construct Huffman Tree
HuffmanNode* build_huffman_tree(unsigned char data[], unsigned freq[], int size) {
    HuffmanNode *left, *right, *top;
    MinHeap* heap = build_min_heap(data, freq, size);

    while (heap->size > 1) {
        left = extract_min(heap);
        right = extract_min(heap);
        top = create_node('$', left->frequency + right->frequency);
        top->left = left;
        top->right = right;
        insert_min_heap(heap, top);
    }
    HuffmanNode* root = heap->array[0];
    free(heap->array);
    free(heap);
    return root;
}

// Generate Huffman Codes
void generate_codes(HuffmanNode* root, int arr[], int top, char codes[256][MAX_TREE_HT], int code_lens[]) {
    if (root->left) {
        arr[top] = 0;
        generate_codes(root->left, arr, top + 1, codes, code_lens);
    }
    if (root->right) {
        arr[top] = 1;
        generate_codes(root->right, arr, top + 1, codes, code_lens);
    }
    if (!(root->left) && !(root->right)) {
        code_lens[root->data] = top;
        for (int i = 0; i < top; ++i) {
            codes[root->data][i] = arr[i] + '0';
        }
        codes[root->data][top] = '\0';
    }
}

// Free Huffman Tree memory
void free_huffman_tree(HuffmanNode* root) {
    if (!root) return;
    free_huffman_tree(root->left);
    free_huffman_tree(root->right);
    free(root);
}

/* ================================================================= */
/* FILE OPERATIONS & UTILITIES                                       */
/* ================================================================= */

void create_sample_log_if_needed() {
    FILE* f = fopen("device_log.txt", "r");
    if (!f) {
        f = fopen("device_log.txt", "w");
        if (f) {
            fprintf(f, "DEVICE_ID=RWA_014\nSTATUS=ONLINE\nTEMP=28.4\nBATTERY=87\nSENSOR=OK\nNETWORK=CONNECTED\nALERT=NONE\n");
            fclose(f);
            printf("[Setup] Created default 'device_log.txt'.\n");
        }
    } else {
        fclose(f);
    }
}

// 1. Compress Log
void compress_log() {
    create_sample_log_if_needed();
    FILE* file = fopen("device_log.txt", "rb");
    if (!file) {
        printf("[Error] Could not open 'device_log.txt' for reading.\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    long original_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (original_size == 0) {
        printf("[Error] Log file is empty.\n");
        fclose(file);
        return;
    }

    unsigned char* buffer = (unsigned char*)malloc(original_size);
    fread(buffer, 1, original_size, file);
    fclose(file);

    // Frequency analysis
    unsigned freq[256] = {0};
    for (long i = 0; i < original_size; i++) {
        freq[buffer[i]]++;
    }

    printf("\n--- Character Frequency Analysis ---\n");
    int unique_chars = 0;
    unsigned char unique_data[256];
    unsigned unique_freq[256];
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            unique_data[unique_chars] = (unsigned char)i;
            unique_freq[unique_chars] = freq[i];
            if (i == '\n') printf(" '\\n' : %u\n", freq[i]);
            else printf(" '%c' : %u\n", i, freq[i]);
            unique_chars++;
        }
    }
    printf("------------------------------------\n");

    HuffmanNode* root = build_huffman_tree(unique_data, unique_freq, unique_chars);

    char codes[256][MAX_TREE_HT] = {{0}};
    int code_lens[256] = {0};
    int arr[MAX_TREE_HT];
    generate_codes(root, arr, 0, codes, code_lens);

    // Write compressed file: device_log.huff
    FILE* out = fopen("device_log.huff", "wb");
    if (!out) {
        printf("[Error] Could not create compressed file.\n");
        free(buffer);
        free_huffman_tree(root);
        return;
    }

    fwrite(freq, sizeof(unsigned), 256, out);
    fwrite(&original_size, sizeof(long), 1, out); // FIXED: Added count argument '1'

    // Pack bits into bytes
    unsigned char current_byte = 0;
    int bit_count = 0;
    long compressed_bytes_count = 0;

    for (long i = 0; i < original_size; i++) {
        char* code = codes[buffer[i]];
        for (int j = 0; code[j] != '\0'; j++) {
            current_byte = (current_byte << 1) | (code[j] - '0');
            bit_count++;
            if (bit_count == 8) {
                fwrite(&current_byte, 1, 1, out);
                compressed_bytes_count++;
                current_byte = 0;
                bit_count = 0;
            }
        }
    }
    // Flush remaining bits
    if (bit_count > 0) {
        current_byte = current_byte << (8 - bit_count);
        fwrite(&current_byte, 1, 1, out);
        compressed_bytes_count++;
    }

    fclose(out);
    free(buffer);
    free_huffman_tree(root);

    long compressed_size = sizeof(unsigned) * 256 + sizeof(long) + compressed_bytes_count;
    double ratio = ((double)(original_size - compressed_size) / original_size) * 100.0;

    printf("\n=== COMPRESSION SUCCESSFUL ===\n");
    printf("Original size   : %ld bytes\n", original_size);
    printf("Compressed size : %ld bytes\n", compressed_size);
    printf("Compression ratio: %.2f%%\n", ratio);
    printf("Saved to        : device_log.huff\n");
    printf("==============================\n");
}

// 2. Decompress Log
void decompress_log() {
    FILE* in = fopen("device_log.huff", "rb");
    if (!in) {
        printf("[Error] Compressed file 'device_log.huff' not found. Please compress first.\n");
        return;
    }

    unsigned freq[256];
    long original_size;
    fread(freq, sizeof(unsigned), 256, in);
    fread(&original_size, sizeof(long), 1, in); // FIXED: Added count argument '1'

    int unique_chars = 0;
    unsigned char unique_data[256];
    unsigned unique_freq[256];
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            unique_data[unique_chars] = (unsigned char)i;
            unique_freq[unique_chars] = freq[i];
            unique_chars++;
        }
    }

    HuffmanNode* root = build_huffman_tree(unique_data, unique_freq, unique_chars);

    FILE* out = fopen("device_log_restored.txt", "wb");
    if (!out) {
        printf("[Error] Could not create restored file.\n");
        fclose(in);
        free_huffman_tree(root);
        return;
    }

    HuffmanNode* current = root;
    long decoded_bytes = 0;
    unsigned char byte;

    while (decoded_bytes < original_size && fread(&byte, 1, 1, in) == 1) {
        for (int i = 7; i >= 0 && decoded_bytes < original_size; i--) {
            int bit = (byte >> i) & 1;
            if (bit == 0) current = current->left;
            else current = current->right;

            if (!(current->left) && !(current->right)) {
                fputc(current->data, out);
                decoded_bytes++;
                current = root;
            }
        }
    }

    fclose(in);
    fclose(out);
    free_huffman_tree(root);

    printf("\n=== DECOMPRESSION SUCCESSFUL ===\n");
    printf("Recovered file saved as: device_log_restored.txt\n");
    printf("================================\n");
}

// 3. Verify Restored Log
void verify_log() {
    FILE* f1 = fopen("device_log.txt", "rb");
    FILE* f2 = fopen("device_log_restored.txt", "rb");

    if (!f1 || !f2) {
        printf("\nVerification: FAILED\n");
        printf("[Error] Missing original or restored log files. Please compress and decompress first.\n");
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return;
    }

    fseek(f1, 0, SEEK_END);
    fseek(f2, 0, SEEK_END);
    long size1 = ftell(f1);
    long size2 = ftell(f2);
    fseek(f1, 0, SEEK_SET);
    fseek(f2, 0, SEEK_SET);

    if (size1 != size2) {
        printf("\nVerification: FAILED\n");
        printf("The restored file differs in size from the original.\n");
        fclose(f1);
        fclose(f2);
        return;
    }

    int identical = 1;
    for (long i = 0; i < size1; i++) {
        if (fgetc(f1) != fgetc(f2)) {
            identical = 0;
            break;
        }
    }

    fclose(f1);
    fclose(f2);

    if (identical) {
        printf("\nVerification: SUCCESS\n");
        printf("The restored file is identical to the original.\n");
    } else {
        printf("\nVerification: FAILED\n");
        printf("The restored file differs from the original.\n");
    }
}

/* ================================================================= */
/* MAIN INTERFACE                                                    */
/* ================================================================= */

void print_menu() {
    printf("\n=== EMBEDDED FIRMWARE LOG COMPRESSION ===\n");
    printf(" 1. Compress log\n");
    printf(" 2. Decompress log\n");
    printf(" 3. Verify restored log\n");
    printf(" 4. Exit\n");
    printf("Enter choice: ");
}

int main() {
    int choice;
    create_sample_log_if_needed();

    do {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            printf("[Error] Invalid input. Try again.\n");
            while (getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1:
                compress_log();
                break;
            case 2:
                decompress_log();
                break;
            case 3:
                verify_log();
                break;
            case 4:
                printf("[Exit] Terminating program safely.\n");
                return 0;
            default:
                printf("[Error] Invalid menu choice. Please select between 1 and 4.\n");
        }
    } while (choice != 4);

    return 0;
}
