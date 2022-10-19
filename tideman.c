#include <cs50.h>
#include <stdio.h>
#include <string.h>

// Max number of candidates
#define MAX 9

// preferences[i][j] is number of voters who prefer i over j
int preferences[MAX][MAX];

// locked[i][j] means i is locked in over j
bool locked[MAX][MAX];

// Each pair has a winner, loser
typedef struct
{
    int winner;
    int loser;
}
pair;

// Array of candidates
string candidates[MAX];
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

// Function prototypes
bool vote(int rank, string name, int ranks[]);
void record_preferences(int ranks[]);
void add_pairs(void);
void sort_pairs(void);
void lock_pairs(void);
bool is_cycle(int winner, int loser, int OG_winner);
void print_winner(void);

int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: tideman [candidate ...]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i] = argv[i + 1];
    }

    // Clear graph of locked in pairs
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            locked[i][j] = false;
        }
    }

    pair_count = 0;
    int voter_count = get_int("Number of voters: ");

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];

        // Query for each rank
        for (int j = 0; j < candidate_count; j++)
        {
            string name = get_string("Rank %i: ", j + 1);

            if (!vote(j, name, ranks))
            {
                printf("Invalid vote.\n");
                return 3;
            }
        }

        record_preferences(ranks);

        printf("\n");
    }

    add_pairs();
    sort_pairs();
    lock_pairs();
    print_winner();
    return 0;
}

// Update ranks given a new vote
bool vote(int rank, string name, int ranks[])
{
    /*Look for candidare called name
    If candidate found, update ranks and return true. ranks[i is the voter's ith preferences.
    IF no candidate is found, dont update ranks and return false.
    */
    for (int i = 0; i < candidate_count; i++)
    {
        if (strcmp(name, candidates[i]) == 0)
        {
            ranks[rank] = i;
            return true;
        }
    }

    return false;
}

// Update preferences given one voter's ranks
void record_preferences(int ranks[])
{
    /*Update the preferences array based on the current voter's ranks.
    How many people prefer on candidate over any other candiadate in election? */
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = i + 1; j < candidate_count; j++)
        {
            //Adds to the preference of candidate number over other candidate number
            preferences[ranks[i]][ranks[j]]++;
        }
    }
    return;
}

// Record pairs of candidates where one is preferred over the other
void add_pairs(void)
{
    for (int i = 0; i < candidate_count; i++)
    {   
        for (int j = 0; j < candidate_count; j++)
        {
            //DONT CHANGE this bit is perfect
            if (preferences[i][j] > preferences[j][i])
            {
                pairs[pair_count].winner = i;
                pairs[pair_count].loser = j;
                pair_count++;
            }
        }
    }
    return;
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    //Stores unssorted array info in cache copy
    int temp_winners_unsort[pair_count];
    int temp_losers_unsort[pair_count];
    int temp_pairs[pair_count];
    int x;
    for (int i = 0; i < pair_count; i++)
    {
        temp_pairs[i] = i;
        temp_winners_unsort[i] = pairs[i].winner;
        temp_losers_unsort[i] = pairs[i].loser;
        x = preferences[pairs[i].winner][pairs[i].loser] + preferences[pairs[i].loser][pairs[i].winner];
    }
    
    
    //Receiving array
    int sorted_pairs[pair_count];
    
    
    //Selection Sort
    int l = 0;
    while (x >= 0)    
    {
        int iLoop = 0;
        while (iLoop < pair_count)
        {
            int comp = preferences[pairs[iLoop].winner][pairs[iLoop].loser];
            if (comp == x)
            {
                sorted_pairs[l] = temp_pairs[iLoop];
                iLoop++;
                l++;
            }
            else
            {
                iLoop++;
            }
        }
        x--;
    }
    

    //Sorts pairs array from cache
    for (int i = 0; i < pair_count; i++)
    {
        pairs[i].winner = temp_winners_unsort[sorted_pairs[i]];
        pairs[i].loser = temp_losers_unsort[sorted_pairs[i]];
    }
    
    return;
}

// Lock pairs into the candidate graph in order, without creating cycles
void lock_pairs(void)
{
    //Look through pairs
    for (int i = 0; i < pair_count; i++)
    {
        if (!is_cycle(pairs[i].winner, pairs[i].loser, pairs[i].winner))
        {    
            locked[pairs[i].winner][pairs[i].loser] = true;
        }
    }
    return; 
}

// Check if pair creates a cycle
bool is_cycle(int winner, int loser, int OG_winner)
{
    // Base Case: If loser is the original winner
    if (loser == OG_winner)
    {
        return true;
    }
    
    
    // Recursive Case: Check if loser is a winner at some point
    for (int x = 0; x < candidate_count; x++)
    {
        if (locked[loser][x] == true)
        {
            if (is_cycle(loser, x, OG_winner) == true)
            {
                return true;
            }
        }       
    }
    return false;
}


// Print the winner of the election
void print_winner(void)
{
    
    // Look for source by checking row without any truth values
    int source = 0;
    int counter = 0;
    for (int j = 0; j < candidate_count; j++)
    {
        counter = 0;
        for (int i = 0; i < candidate_count; i++)
        {
            if (locked[i][j] == true)
            {
                counter++;
            }
            
            if (i == (candidate_count - 1) && counter == 0)
            {
                source = j;
            }
        }
    }
    
    // Print source of graph
    printf("%s\n", candidates[source]);
    
    
    return;
}
