#include <vector>
#include <queue>
#include <algorithm>
#include <map>
#include <iostream>
#include <string>
#include <strstream>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

using namespace std;

int main() {

    string str, item;
    long long i  = 0;
    while(getline(cin, str))
    {
        istrstream istrs((char *)str.c_str());
        long long edge[3] = {0,0,0};
        uint edge_pos = 0;
        bool is_edge = false;
        while(istrs >> item)
        {
            if(edge_pos == 0 && item == "a")
            {
               is_edge = true;
            }
            if(is_edge && (edge_pos > 0))
            {
              edge[edge_pos - 1] = atoll(item.c_str());
            }
            edge_pos++;
            if(edge_pos >= 4) break;
        }

        if(is_edge)
        {
          cout << edge[0] << "\t" <<  edge[1] << "\t" << edge[2] << endl;
          i++;
        }
    }

}
