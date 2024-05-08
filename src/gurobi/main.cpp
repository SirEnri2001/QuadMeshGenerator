/* Copyright 2024, Gurobi Optimization, LLC */

/* This example formulates and solves the following simple MIP model:

     maximize    x +   y + 2 z
     subject to  x + 2 y + 3 z <= 4
                 x +   y       >= 1
                 x, y, z binary
*/
#include "gurobi_optimize.h"

int
main(int   argc,
    char* argv[])
{
    
}
