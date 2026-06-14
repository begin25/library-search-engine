\# Library Search Engine



A command-line search engine built in C++ that indexes text documents and enables fast keyword-based retrieval using custom data structures and search algorithms.



\## Features



\* Keyword extraction and normalization

\* Inverted index construction

\* Custom hash table with separate chaining

\* Dynamic hash table resizing

\* Binary-search-based retrieval engine

\* Fast document lookup by keyword

\* Performance comparison between Hash Table and Binary Search approaches

\* Command-line interface for searching indexed documents



\## Motivation



The goal of this project was to understand how real search systems organize and retrieve information efficiently. Instead of relying entirely on STL containers, core indexing functionality was implemented using custom data structures to explore collision handling, hashing, indexing, and search complexity in practice.



\## System Design



\### Document Processing



1\. Read all `.txt` files from the `books/` directory.

2\. Extract keywords from document contents.

3\. Convert words to lowercase.

4\. Remove short tokens (< 3 characters).

5\. Build an inverted index mapping keywords to document IDs.



\### Search Engines



\#### Hash Table Engine



\* Custom hash table implementation

\* Separate chaining for collision resolution

\* Dynamic resizing when load factor exceeds threshold

\* Average query complexity: \*\*O(1)\*\*



\#### Binary Search Engine



\* Stores `(keyword, document\_id)` pairs

\* Sorts index during initialization

\* Uses binary search for retrieval

\* Query complexity: \*\*O(log n)\*\*



\## Example Usage



Search using Hash Table:



```bash

./engine --search algorithm --engine hash

```



Search using Binary Search:



```bash

./engine --search algorithm --engine binary

```



Compare both engines:



```bash

./engine --search algorithm --engine all

```



\## Sample Output



```text

Search term: algorithm



Engine: Hash Table

Time: 1200 ns



ID    Title

1     Introduction\_to\_Algorithms

3     Data\_Structures\_Handbook

```



\## Complexity Analysis



| Operation   | Hash Table   | Binary Search |

| ----------- | ------------ | ------------- |

| Insert      | O(1) Average | O(1)          |

| Build Index | O(n)         | O(n log n)    |

| Search      | O(1) Average | O(log n)      |



\## Technologies



\* C++17

\* STL

\* File System Library

\* Hashing

\* Binary Search

\* Dynamic Data Structures



\## Future Improvements



\* Phrase search support

\* TF-IDF ranking

\* Multi-keyword queries

\* Fuzzy matching

\* Trie-based autocomplete

\* Persistent index storage



\## Learning Outcomes



Through this project I gained hands-on experience with:



\* Hash table design

\* Collision handling techniques

\* Dynamic resizing strategies

\* Inverted indexing

\* Binary search optimization

\* Time-complexity analysis

\* File handling in C++

\* Search engine fundamentals



