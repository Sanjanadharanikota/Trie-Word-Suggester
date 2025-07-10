#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define ALPHABET_SIZE 26
#define MAX_WORD_LENGTH 100
#define MAX_SUGGESTIONS 10
#define MAX_WORDS 1000
#define MAX_LEVENSHTEIN_DISTANCE 2

// Trie Node
typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    bool isEndOfWord;
    char* originalWord;
    int frequency; // Added for frequency-based suggestions
} TrieNode;

// Suggestion structure for ranking
typedef struct {
    char* word;
    int distance;
    int frequency;
} Suggestion;

// Suggestion List
typedef struct {
    Suggestion suggestions[MAX_SUGGESTIONS];
    int count;
} SuggestionList;

// Dictionary structure
typedef struct {
    char* words[MAX_WORDS];
    int count;
} Dictionary;

// Create new Trie node
TrieNode* createTrieNode() {
    TrieNode* node = (TrieNode*)calloc(1, sizeof(TrieNode));
    if (!node) {
        perror("Failed to create Trie node");
        exit(EXIT_FAILURE);
    }
    node->isEndOfWord = false;
    node->originalWord = NULL;
    node->frequency = 0;
    return node;
}

// Convert string to lowercase in place
void toLowerCase(char* str) {
    for (; *str; ++str) {
        *str = tolower((unsigned char)*str);
    }
}

// Create lowercase copy of string
char* strtolower(const char* str) {
    char* lower = strdup(str);
    if (!lower) return NULL;
    toLowerCase(lower);
    return lower;
}

// Check if word contains only letters
bool isValidWord(const char* str) {
    for (; *str; ++str) {
        if (!isalpha((unsigned char)*str)) return false;
    }
    return true;
}

// Insert word into Trie with optional frequency
void insertWord(TrieNode* root, const char* word, int frequency) {
    if (!root || !word || !*word) return;

    char* lowerWord = strtolower(word);
    if (!lowerWord) return;

    TrieNode* current = root;
    for (int i = 0; lowerWord[i]; ++i) {
        int index = lowerWord[i] - 'a';
        if (current->children[index] == NULL) {
            current->children[index] = createTrieNode();
        }
        current = current->children[index];
    }

    current->isEndOfWord = true;
    // Only update if new word or higher frequency
    if (!current->originalWord || frequency > current->frequency) {
        free(current->originalWord);
        current->originalWord = strdup(word);
        current->frequency = frequency;
    }
    free(lowerWord);
}

// Initialize suggestion list
void initSuggestionList(SuggestionList* list) {
    list->count = 0;
    for (int i = 0; i < MAX_SUGGESTIONS; ++i) {
        list->suggestions[i].word = NULL;
        list->suggestions[i].distance = INT_MAX;
        list->suggestions[i].frequency = 0;
    }
}

// Add suggestion to list if it's better than existing ones
void addSuggestion(SuggestionList* list, const char* word, int distance, int frequency) {
    if (list->count < MAX_SUGGESTIONS) {
        list->suggestions[list->count].word = strdup(word);
        list->suggestions[list->count].distance = distance;
        list->suggestions[list->count].frequency = frequency;
        list->count++;
    } else {
        // Find the worst suggestion to replace
        int worst_idx = 0;
        for (int i = 1; i < MAX_SUGGESTIONS; ++i) {
            if (list->suggestions[i].distance > list->suggestions[worst_idx].distance ||
                (list->suggestions[i].distance == list->suggestions[worst_idx].distance && 
                 list->suggestions[i].frequency < list->suggestions[worst_idx].frequency)) {
                worst_idx = i;
            }
        }
        
        // Replace if new suggestion is better
        if (distance < list->suggestions[worst_idx].distance ||
            (distance == list->suggestions[worst_idx].distance && 
             frequency > list->suggestions[worst_idx].frequency)) {
            free(list->suggestions[worst_idx].word);
            list->suggestions[worst_idx].word = strdup(word);
            list->suggestions[worst_idx].distance = distance;
            list->suggestions[worst_idx].frequency = frequency;
        }
    }
}

// Compare function for qsort (prioritize lower distance, then higher frequency)
int compareSuggestions(const void* a, const void* b) {
    const Suggestion* sa = (const Suggestion*)a;
    const Suggestion* sb = (const Suggestion*)b;
    
    if (sa->distance != sb->distance) {
        return sa->distance - sb->distance;
    }
    return sb->frequency - sa->frequency;
}

// Collect suggestions from Trie with prefix
void collectSuggestions(TrieNode* node, const char* prefix, SuggestionList* suggestions) {
    if (!node) return;

    if (node->isEndOfWord) {
        addSuggestion(suggestions, node->originalWord, 0, node->frequency);
    }

    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        if (node->children[i]) {
            collectSuggestions(node->children[i], prefix, suggestions);
        }
    }
}

// Search words by prefix
void searchWordsByPrefix(TrieNode* root, const char* prefix) {
    if (!root || !prefix) return;

    char* lowerPrefix = strtolower(prefix);
    if (!lowerPrefix) return;

    TrieNode* current = root;
    for (int i = 0; lowerPrefix[i]; ++i) {
        int index = lowerPrefix[i] - 'a';
        if (!current->children[index]) {
            free(lowerPrefix);
            printf("No suggestions found for \"%s\".\n", prefix);
            return;
        }
        current = current->children[index];
    }

    SuggestionList suggestions;
    initSuggestionList(&suggestions);
    collectSuggestions(current, prefix, &suggestions);
    qsort(suggestions.suggestions, suggestions.count, sizeof(Suggestion), compareSuggestions);

    if (suggestions.count == 0) {
        printf("No suggestions found for \"%s\".\n", prefix);
    } else {
        printf("Suggestions for \"%s\":\n", prefix);
        for (int i = 0; i < suggestions.count && i < MAX_SUGGESTIONS; ++i) {
            printf("%2d. %s (frequency: %d)\n", i+1, suggestions.suggestions[i].word, 
                   suggestions.suggestions[i].frequency);
        }
    }

    // Free suggestion strings
    for (int i = 0; i < suggestions.count; ++i) {
        free(suggestions.suggestions[i].word);
    }
    free(lowerPrefix);
}

// Levenshtein Distance for Spell Correction
int levenshteinDistance(const char* s, const char* t) {
    int lenS = strlen(s), lenT = strlen(t);
    
    // Use dynamic memory allocation for larger strings
    int* prev = (int*)malloc((lenT + 1) * sizeof(int));
    int* curr = (int*)malloc((lenT + 1) * sizeof(int));
    
    if (!prev || !curr) {
        free(prev);
        free(curr);
        return INT_MAX; // Return max distance on allocation failure
    }

    for (int j = 0; j <= lenT; ++j) prev[j] = j;

    for (int i = 1; i <= lenS; ++i) {
        curr[0] = i;
        for (int j = 1; j <= lenT; ++j) {
            int cost = (s[i-1] == t[j-1]) ? 0 : 1;
            curr[j] = (prev[j] + 1 < curr[j-1] + 1) ? 
                      (prev[j] + 1 < prev[j-1] + cost ? prev[j] + 1 : prev[j-1] + cost) :
                      (curr[j-1] + 1 < prev[j-1] + cost ? curr[j-1] + 1 : prev[j-1] + cost);
        }
        // Swap prev and curr
        int* temp = prev;
        prev = curr;
        curr = temp;
    }

    int result = prev[lenT];
    free(prev);
    free(curr);
    return result;
}

// Collect all words in Trie for spell correction
void collectAllWords(TrieNode* node, Dictionary* dict) {
    if (!node) return;

    if (node->isEndOfWord && node->originalWord && dict->count < MAX_WORDS) {
        dict->words[dict->count++] = strdup(node->originalWord);
    }

    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        if (node->children[i]) {
            collectAllWords(node->children[i], dict);
        }
    }
}

// Suggest similar words based on Levenshtein distance
void suggestSimilarWords(const char* input, Dictionary* dict) {
    if (!input || !dict || dict->count == 0) return;

    char* lowerInput = strtolower(input);
    if (!lowerInput) return;

    SuggestionList suggestions;
    initSuggestionList(&suggestions);

    for (int i = 0; i < dict->count; ++i) {
        char* lowerDictWord = strtolower(dict->words[i]);
        if (!lowerDictWord) continue;

        int distance = levenshteinDistance(lowerInput, lowerDictWord);
        free(lowerDictWord);

        if (distance <= MAX_LEVENSHTEIN_DISTANCE) {
            // For spell correction, we don't have frequency info, so use 0
            addSuggestion(&suggestions, dict->words[i], distance, 0);
        }
    }

    qsort(suggestions.suggestions, suggestions.count, sizeof(Suggestion), compareSuggestions);

    if (suggestions.count > 0) {
        printf("Did you mean:\n");
        for (int i = 0; i < suggestions.count && i < MAX_SUGGESTIONS; ++i) {
            printf("%2d. %s (distance: %d)\n", i+1, suggestions.suggestions[i].word, 
                   suggestions.suggestions[i].distance);
        }
    } else {
        printf("No similar words found.\n");
    }

    // Free suggestion strings and input copy
    for (int i = 0; i < suggestions.count; ++i) {
        free(suggestions.suggestions[i].word);
    }
    free(lowerInput);
}

// Free Trie memory
void freeTrie(TrieNode* node) {
    if (!node) return;
    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        freeTrie(node->children[i]);
    }
    free(node->originalWord);
    free(node);
}

// Free dictionary memory
void freeDictionary(Dictionary* dict) {
    for (int i = 0; i < dict->count; ++i) {
        free(dict->words[i]);
    }
}

// Interactive menu
void showMenu() {
    printf("\nMenu:\n");
    printf("1. Search by prefix\n");
    printf("2. Show all words\n");
    printf("3. Exit\n");
    printf("Choose an option: ");
}

int main() {
    TrieNode* root = createTrieNode();
    Dictionary dict = { .count = 0 };
    int n;

    printf("Trie-Based Word Suggestion System\n");
    printf("How many words do you want to enter? (1-%d): ", MAX_WORDS);
    
    while (scanf("%d", &n) != 1 || n <= 0 || n > MAX_WORDS) {
        printf("Invalid input. Enter a number between 1 and %d: ", MAX_WORDS);
        while (getchar() != '\n'); // Clear input buffer
    }

    printf("Enter words (one per line) with optional frequency (word:freq):\n");
    for (int i = 0; i < n; ) {
        char input[MAX_WORD_LENGTH * 2]; // Allow space for frequency
        int frequency = 0;
        
        if (scanf("%s", input) != 1) {
            printf("Error reading input. Try again.\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }

        // Check for frequency suffix (word:frequency)
        char* colon = strchr(input, ':');
        if (colon) {
            *colon = '\0'; // Split the string
            frequency = atoi(colon + 1);
        }

        if (strlen(input) >= MAX_WORD_LENGTH || !isValidWord(input)) {
            printf("Invalid word. Try again.\n");
            continue;
        }

        insertWord(root, input, frequency);
        i++;
    }

    // Collect all words for spell correction
    collectAllWords(root, &dict);

    int choice;
    do {
        showMenu();
        while (scanf("%d", &choice) != 1) {
            printf("Invalid input. Enter a number (1-3): ");
            while (getchar() != '\n');
        }

        switch (choice) {
            case 1: {
                char prefix[MAX_WORD_LENGTH];
                printf("Enter prefix to search: ");
                if (scanf("%s", prefix) == 1) {
                    if (!isValidWord(prefix)) {
                        printf("Invalid prefix. Only letters allowed.\n");
                    } else {
                        char* lowerPrefix = strtolower(prefix);
                        TrieNode* current = root;
                        bool found = true;

                        for (int i = 0; lowerPrefix[i]; ++i) {
                            int index = lowerPrefix[i] - 'a';
                            if (!current->children[index]) {
                                found = false;
                                break;
                            }
                            current = current->children[index];
                        }

                        if (found) {
                            searchWordsByPrefix(root, prefix);
                        } else {
                            printf("No words with prefix \"%s\". Trying spell correction...\n", prefix);
                            suggestSimilarWords(prefix, &dict);
                        }
                        free(lowerPrefix);
                    }
                }
                break;
            }
            case 2: {
                printf("\nAll words in the Trie:\n");
                char buffer[MAX_WORD_LENGTH];
                Dictionary allWords = { .count = 0 };
                collectAllWords(root, &allWords);
                
                // Sort words alphabetically
                qsort(allWords.words, allWords.count, sizeof(char*), 
                      (int (*)(const void*, const void*))strcmp);
                
                for (int i = 0; i < allWords.count; ++i) {
                    printf("%3d. %s\n", i+1, allWords.words[i]);
                }
                freeDictionary(&allWords);
                break;
            }
            case 3:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice. Try again.\n");
        }
    } while (choice != 3);

    freeTrie(root);
    freeDictionary(&dict);
    return 0;
}
