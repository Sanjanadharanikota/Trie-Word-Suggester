
# Trie-Word-Suggester

Trie-based word suggestion system in C with prefix search, frequency-ranked suggestions, and spell correction using Levenshtein distance. This project is ideal for practicing data structures, string algorithms, and building command-line tools in C.

---

## Features

- **Trie Data Structure Implementation**  
  Efficiently stores words in a prefix tree (Trie) allowing fast insertion and lookup operations.

- **Prefix-Based Word Suggestions**  
  Given a prefix, the program returns up to 10 matching words stored in the Trie, supporting real-time auto-complete functionality.

- **Frequency Ranking**  
  Each word can be assigned an optional frequency value that represents its popularity or relevance. Suggestions are ranked higher if they have higher frequency.

- **Spell Correction with Levenshtein Distance**  
  When no words match the entered prefix, the program suggests similar words based on the Levenshtein (edit) distance algorithm, allowing correction of typos and misspellings within a distance of 2.

- **Case-Insensitive Input with Original Case Preservation**  
  Inputs are processed without case sensitivity to improve matching, but the original casing of words is preserved for display.

- **Input Validation**  
  Accepts only alphabetic characters for word insertion and searching to ensure clean data.

- **Interactive Command-Line Interface (CLI)**  
  Provides a simple menu-driven interface to enter words, search prefixes, display all stored words, and exit the program.

- **View All Stored Words**  
  Users can list all words currently stored in the Trie, sorted alphabetically for easy browsing.

- **Memory Management**  
  Dynamically allocates and frees memory for Trie nodes and stored words, ensuring efficient resource use.

---

## Getting Started

### Prerequisites

- C compiler installed (e.g., `gcc`)  
- Command-line terminal or console  

### Compilation

Clone or download the repository, then compile the source code:

```bash
gcc main.c -o trie-suggester

** Run the compiled executable:**
./trie-suggester


