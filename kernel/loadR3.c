#include <mpx/processes.h>
#include <mpx/pcb.h>
#include <mpx/context.h>
#include <mpx/loadR3.h>
#include <mpx/sys_req.h>

#include <stdint.h>

int loadR3() {
    struct pcb* pro1 = pcb_setup("proc1", USER, 1);
    pcb_context_init(pro1, proc1, NULL, 0);
    pcb_insert(pro1);

    struct pcb* pro2 = pcb_setup("proc2", USER, 1);
    pcb_context_init(pro2, proc2, NULL, 0);
    pcb_insert(pro2);

    struct pcb* pro3 = pcb_setup("proc3", USER, 1);
    pcb_context_init(pro3, proc3, NULL, 0);
    pcb_insert(pro3);

    struct pcb* pro4 = pcb_setup("proc4", USER, 1);
    pcb_context_init(pro4, proc4, NULL, 0);
    pcb_insert(pro4);

    struct pcb* pro5 = pcb_setup("proc5", USER, 1);
    pcb_context_init(pro5, proc5, NULL, 0);
    pcb_insert(pro5);
    
    return 0;
}
