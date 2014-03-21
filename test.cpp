
#include "libdistributed.hpp"


int main (int argc, char **argv)
{
    Distributed::Node node(8888);
    node.accept();
}

