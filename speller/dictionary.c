// Implements a dictionary's functionality

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include "dictionary.h"

// Represents a node in a hash table
typedef struct node
{
    char word[LENGTH + 1];
    struct node *next;
}
node;

// Number of buckets in hash table
const unsigned int N = 999983;

// Hash table
node *table[N];

// Dictionary word counter
int wordCount = 0;

// Returns true if word is in dictionary else false
bool check(const char *word)
{
    // Hash input word
    int index = hash(word);
    
    // Create a head pointer and a cursor pointer starting at head
    node *head = table[index];
    node *cursor = table[index];
    
    // Traverse linked list 
    while (cursor != NULL)
    {
        if (strcasecmp(cursor->word, word) == 0)
        {
            return true;
        }
        
        cursor = cursor->next;
    }
    
    return false;
}

// Hashes word to a number
unsigned int hash(const char *word)
{
    // PJW hash function by Peter J. Weinberger of AT&T Bell Labs adjusted for a 32-bit system (https://bit.ly/3oABVAp). Modified to work case-insentively.
    unsigned int hash = 0;
    unsigned int x    = 0;
    unsigned int i    = 0;
    unsigned int length = strlen(word);
    char lcaseWord[length + 1];

    // Converts word into lowercase
    for (int n = 0; n <= length; n++)
    {
        lcaseWord[n] = tolower(word[n]);
    }
    
    for (i = 0; i < length; ++i)
    {
        hash = (hash << 4) + (lcaseWord[i]);

        if ((x = hash & 0xF0000000L) != 0)
        {
            hash ^= (x >> 24);
        }

        hash &= ~x;
    }

    return hash & N;
}

// Loads dictionary into memory, returning true if successful else false
bool load(const char *dictionary)
{
    FILE *dict = fopen(dictionary, "r");
    if (dict == NULL)
    {
        printf("Dictonary is invalid\n");
        return false;
    }

    int *add, *add1;
    char readWord[LENGTH + 1];
    
        
    while (fscanf(dict, "%s", readWord) != EOF)
    {
        node *transfer = malloc(sizeof(node));
        if (transfer == NULL)
        {
            free(transfer);
            printf("Out of memory for dictionary.\n");
            return false;
        }
        node *newnode = malloc(sizeof(node));
        if (newnode == NULL)
        {
            free(newnode);
            printf("Out of memory for dictionary. \n");
            return false;
        }
        
        // Copies words from Dictionary file into a new node
        strcpy(transfer->word, readWord);
        transfer->next = NULL;
        int index = hash(readWord);
        
        node *head = table[index];
        int counterp = 0;
        
        if (table[index] == NULL)
        {
            table[index] = transfer;
            free(newnode);
        }
        else
        {
            newnode->next = head;
            strcpy(newnode->word, readWord);
            table[index] = newnode;
            free(transfer);
        }

        wordCount++;
    }
    fclose(dict);
    return true;
}

// Returns number of words in dictionary if loaded else 0 if not yet loaded
unsigned int size(void)
{
    return wordCount;
}

// Unloads dictionary from memory, returning true if successful else false
bool unload(void)
{
    // Iterate all of the nodes in a linked list
    for (unsigned int i = 0; i <= N; i++)
    {
        //printf("%i: %s\n", i, table[i]->word);
        node *head = table[i];
        node *cursor = head;
        node *tmp = head;
        
        // Traverse linked list and free up nodes
        while (cursor != NULL)
        {
            tmp = cursor;
            cursor = cursor->next;
            free(tmp);
        }
    }
    return true;
}
