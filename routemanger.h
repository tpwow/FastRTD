#ifndef ROUTEMANGER_H
#define ROUTEMANGER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

class RouteManger
{
public:
    RouteManger();
    ~RouteManger();

    int read_data_files(char *filepath);

    enum RouteParamater
    {
        MAX_ROUTE_NUM=10000,
        MAX_ROUTE_LEN=40,
        MAX_DIFFER_LEN=1000,
        MAX_LEVEL=5,
        ZERO_LEVEL=0,
        IP_LEVEL=1,
        SUB_LEVEL=2,
        AS_LEVEL=3,
        GEO_LEVEL=4

    };

    typedef struct st_Node
    {
       unsigned int ID[MAX_LEVEL];
    }Node;

    typedef struct st_Route
    {
       struct st_Node route[MAX_ROUTE_LEN];
       int valid_route_len;
    }Route;

    struct st_Route *all_routes=new struct st_Route[MAX_ROUTE_NUM];
    int valid_route_num;
    int current_level;
    int prun_unvalid;
    int prun_bigger;
    int prun_same;
    int prun_trace;
    int total_nodes;

    unsigned int diff_id_list[MAX_LEVEL][MAX_DIFFER_LEN];


    int inits_variables();
    int add_to_id_list(unsigned intid_num,int level);
    unsigned int ip_to_int(char *ip_addr);
    int int_to_ip(unsigned int ip,unsigned char*ip_addr);
    int setLevel(int level);


    int max_level;
    bool is_compare_finished;
    typedef struct st_PairNode
    {
        unsigned int *index_in_seuqences=new unsigned int[MAX_ROUTE_NUM];
        unsigned int key;
        int parent;
        int index;
        int level;
        char state;
    }PairNode;

    typedef struct st_IndexSequence
    {
        int index_table[MAX_DIFFER_LEN][MAX_ROUTE_LEN];

    }IndexSequence;

    struct st_IndexSequence *Index_Sequence=new struct st_IndexSequence[MAX_ROUTE_NUM];
    struct st_PairNode *Pairs_table=new struct st_PairNode[MAX_ROUTE_LEN*MAX_ROUTE_NUM];

    int level[MAX_ROUTE_LEN];
    FILE*log;
    int construct_index_sequence();
    int construct_pairs_tree();
    int level_tree_generate(int lev);
    int pair_testify(PairNode & p,unsigned int key);
    int find_previous(int key,int pos);
    int pair_index_compare(PairNode p,PairNode q);
    int  level_tree_prunning(int &child_start,int& child_end,PairNode &pair);
    int trace_back_lcs();
    int clear();
    bool is_algorithm_improved;

};

#endif // ROUTEMANGER_H
