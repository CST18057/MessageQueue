#include "ojSocket.h"
#include "configure.h"
#include "fileio.h"
#include "log.h"
using namespace std;
int main()
{
    Scheduler sch = Scheduler();
    int serverFd = createServer();
    polling(serverFd, sch);
    closeSocket(serverFd);
}