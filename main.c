#include "ipHeader.h"
#include "tcpHeader.h"
#include "udpHeader.h"

int main(int argc, char* argv[]) {
    //creation raw socket
    int s = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);
    return 0;
}
