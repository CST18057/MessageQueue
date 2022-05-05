#include "ojSocket.h"
#include "configure.h"
#include "fileio.h"
#include "log.h"
#define DEBUG_JUDGE
int main()
{
    Scheduler sch = Scheduler();
    // sch.createQueue("queue");
    // sch.addMessage("queue1", "123");
    // sch.addMessage("queue", "123");
    // sch.readMessage("queue", 0, 1, 0);
    sch.createGroup("queue", "group", true);
    sch.addMessage("queue", "1");
    sch.addMessage("queue", "2");
    sch.addMessage("queue", "3");
    sch.addMessage("queue", "4");
    sch.readMessageGroup("queue", "group", "consumer", -1);
    sch.readMessageGroup("queue", "group", "consumer", -1);
}