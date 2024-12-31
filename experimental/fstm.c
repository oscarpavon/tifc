#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STATES 100
#define ALPHABET_SIZE 127

typedef int state_id;

typedef struct state
{
   state_id id;
   void *param;
} state;

typedef struct fstm
{
    state    *cur;
    state_id trmap[MAX_STATES][ALPHABET_SIZE];
    state_id end_state_id;
    char     states[];
} fstm;

/*
 * |    state      |     signal     |  next state   |
 * +---------------^----------------^---------------+
 *         S0      |       a        |       S1
 *         S1      |       b        |       S0   
 *        -++-     |       c        |       S2
 *         S2      |       b        |      - X -          
 */


fstm* fstm_alloc(state_id states_num)
{
    fstm *sm = (fstm*) malloc(sizeof(fstm) + sizeof(state) * states_num);
    if (!sm) return 0;
    
    memset(sm->trmap, -1, sizeof(sm->trmap));
    
    state* states = (state*)sm->states;

    for (state_id id = 0; id < states_num; ++id)
    {
        states[id].id = id;
    }

    sm->end_state_id = states_num - 1;
    sm->cur = (state*) sm->states;
    
    return sm;
}

void fstm_free(fstm *sm)
{
    free(sm);
}

void fstm_set_transit(fstm *sm, int c, state_id from, state_id to)
{
    sm->trmap[from][c] = to;
}

static state* next_state(fstm *sm, int c);
bool fstm_feed(fstm *sm, int c)
{
    state *next = next_state(sm, c);

    if (! next)
    {
        fprintf(stderr, "Invalid transition!\n");
        exit(EXIT_FAILURE);
    }
    
    sm->cur = next;

    return (next->id != sm->end_state_id);
}

static state* next_state(fstm *sm, int c)
{
    state_id cur = sm->cur->id;
    state_id next = sm->trmap[cur][c];

    if (-1 == next)
    {
        return 0;
    }

    state* states = (state*) sm->states;
    
    return &states[next];
}


int main(void)
{
    fstm *automat = fstm_alloc(3);

    fstm_set_transit(automat, 'a', 0, 1);
    fstm_set_transit(automat, 'b', 1, 0);
    fstm_set_transit(automat, 'c', 1, 2); /* exit */
    
    fstm_feed(automat, 'a');
    fstm_feed(automat, 'b');
    fstm_feed(automat, 'a');
    fstm_feed(automat, 'b');
    fstm_feed(automat, 'c');
    
    bool res = fstm_feed(automat, 'a');
    printf("res = %s\n", !res ? "end" : "has next");
    
    res = fstm_feed(automat, 'c');
    printf("res = %s\n", !res ? "end" : "has next");
    
    fstm_free(automat);

    return EXIT_SUCCESS;
}
