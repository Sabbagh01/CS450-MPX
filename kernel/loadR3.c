#include <mpx/processes.h>
#include <mpx/pcb.h>
#include <mpx/context.h>
#include <mpx/loadR3.h>
#include <mpx/sys_req.h>
int loadR3(){
    struct pcb* pro1 = pcb_setup("proc1", KERNEL, 1);
    pro1 -> pctxt = pro1->pstackseg + MPX_PCB_STACK_SZ - sizeof(struct context);
    struct context* pr1 = pro1 -> pctxt;
    pr1 -> ss = 0x10;
    pr1 -> ds = 0x10;
    pr1 -> es = 0x10;
    pr1 -> fs = 0x10;
    pr1 -> gs = 0x10;
    pr1 -> cs = 0x08;
    pr1 -> esp = (unsigned long) pro1->pctxt;
    pr1 -> ebp = (unsigned long) pro1->pstackseg + MPX_PCB_STACK_SZ - 1;
    pr1 -> eip = (unsigned long) proc1;
    pr1 -> eflags = 0x0202;
    pr1 -> edi = 0;
    pr1 -> esi = 0;
    pr1 -> ebp = 0;
    pr1 -> esp = 0;
    pr1 -> ebx = 0;
    pr1 -> edx = 0;
    pr1 -> ecx = 0;
    pr1 -> eax = 0;
    pcb_insert(pro1);

    struct pcb* pro2 = pcb_setup("proc2",KERNEL,1);
     pro2 -> pctxt = pro2->pstackseg + MPX_PCB_STACK_SZ - sizeof(struct context);
    struct context* pr2 = pro2 -> pctxt;
    pr2 -> ss = 0x10;
    pr2 -> ds = 0x10;
    pr2 -> es = 0x10;
    pr2 -> fs = 0x10;
    pr2 -> gs = 0x10;
    pr2 -> cs = 0x08;
    pr2 -> esp = (unsigned long) pro2->pctxt;
    pr2 -> ebp = (unsigned long) pro2->pstackseg + MPX_PCB_STACK_SZ - 1;
    pr2 -> eip = (unsigned long) proc2;
    pr2 -> eflags = 0x0202;
    pr2 -> edi = 0;
    pr2 -> esi = 0;
    pr2 -> ebp = 0;
    pr2 -> esp = 0;
    pr2 -> ebx = 0;
    pr2 -> edx = 0;
    pr2 -> ecx = 0;
    pr2 -> eax = 0;
    pcb_insert(pro2);


    struct pcb* pro3 = pcb_setup("proc3",KERNEL,1);
     pro3 -> pctxt = pro3->pstackseg + MPX_PCB_STACK_SZ - sizeof(struct context);
    struct context* pr3 = pro3 -> pctxt;
    pr3 -> ss = 0x10;
    pr3 -> ds = 0x10;
    pr3 -> es = 0x10;
    pr3 -> fs = 0x10;
    pr3 -> gs = 0x10;
    pr3 -> cs = 0x08;
    pr3 -> esp = (unsigned long) pro3->pctxt;
    pr3 -> ebp = (unsigned long) pro3->pstackseg + MPX_PCB_STACK_SZ - 1;
    pr3 -> eip = (unsigned long) proc3;
    pr3 -> eflags = 0x0202;
    pr3 -> edi = 0;
    pr3 -> esi = 0;
    pr3 -> ebp = 0;
    pr3 -> esp = 0;
    pr3 -> ebx = 0;
    pr3 -> edx = 0;
    pr3 -> ecx = 0;
    pr3 -> eax = 0;
    pcb_insert(pro3);


    struct pcb* pro4 = pcb_setup("proc4",KERNEL,1);
    pro4 -> pctxt = pro4->pstackseg + MPX_PCB_STACK_SZ - sizeof(struct context);
    pcb_insert(pro4);
    struct context* pr4 = pro4 -> pctxt;
    pr4 -> ss = 0x10;
    pr4 -> ds = 0x10;
    pr4 -> es = 0x10;
    pr4 -> fs = 0x10;
    pr4 -> gs = 0x10;
    pr4 -> cs = 0x08;
    pr4 -> esp = (unsigned long) pro4->pctxt;
    pr4 -> ebp = (unsigned long) pro4->pstackseg + MPX_PCB_STACK_SZ - 1;
    pr4 -> eip = (unsigned long) proc4;
    pr4 -> eflags = 0x0202;
    pr4 -> edi = 0;
    pr4 -> esi = 0;
    pr4 -> ebp = 0;
    pr4 -> esp = 0;
    pr4 -> ebx = 0;
    pr4 -> edx = 0;
    pr4 -> ecx = 0;
    pr4 -> eax = 0;

    struct pcb* pro5 = pcb_setup("proc5",KERNEL,1);
         pro5 -> pctxt = pro5->pstackseg + MPX_PCB_STACK_SZ - sizeof(struct context);
    pcb_insert(pro5);
    struct context* pr5 = pro5 -> pctxt;
    pr5 -> ss = 0x10;
    pr5 -> ds = 0x10;
    pr5 -> es = 0x10;
    pr5 -> fs = 0x10;
    pr5 -> gs = 0x10;
    pr5 -> cs = 0x08;
    pr5 -> esp = (unsigned long) pro5->pctxt;
    pr5 -> ebp = (unsigned long) pro5->pstackseg + MPX_PCB_STACK_SZ - 1;
    pr5 -> eip = (unsigned long) proc5;
    pr5 -> eflags = 0x0202;
    pr5 -> edi = 0;
    pr5 -> esi = 0;
    pr5 -> ebp = 0;
    pr5 -> esp = 0;
    pr5 -> ebx = 0;
    pr5 -> edx = 0;
    pr5 -> ecx = 0;
    pr5 -> eax = 0;
    
    // struct pcb* proc3 = pcb_setup("proc3",1,1);
    // struct pcb* proc4 = pcb_setup("proc4",1,1);
    // struct pcb* proc5 = pcb_setup("proc5",1,1);
    // struct pcb* sysIdle = pcb_setup("sysIdle",1,1);
    // struct pcb* comwrite = pcb_setup("comwrite",1,1);
    // struct pcb* comread = pcb_setup("comread",1,1);
    // struct pcb* icom25 = pcb_setup("icom25",1,1);
    // struct pcb* iocom = pcb_setup("iocom",1,1);
    return 0;
}
int yield(){
 sys_req(IDLE);
 return 0;
}
