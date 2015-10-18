#include<stdio.h>
#include<stdlib.h>
#include "routemanger.h"

int main(int argc,char*argv[])
{
    RouteManger route;
    route.inits_variables();
    route.setLevel(3);
    route.read_data_files(argv[1]);
    route.construct_index_sequence();
    route.construct_pairs_tree();

    route.clear();
    route.setLevel(2);
    route.construct_pairs_tree();

    route.clear();
    route.setLevel(1);
    route.construct_pairs_tree();
    return 0;
}
