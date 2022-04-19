#include "ojSocket.h"
#include "configure.h"
#include "fileio.h"
#include "log.h"
#define DEBUG_JUDGE
int main()
{
    Scheduler sch = Scheduler();
    sch.createQueue("queue");
    sch.addMessage("queue","123");
}