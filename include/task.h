#ifndef TASK_H
#define TASK_H
#include <types.h>
#include <spinlock.h>
#include <mm.h>

struct task_struct {
	void *stack;
	int state;
	int id;
	int cpu;
	int prio;
	int counter;
	int reschedule;

	spin_lock_t spin_lock;
	struct memory_space mm;

	struct task_struct *parent;
	struct task_struct *sibling;
	struct list_head list;
};

#define TASK_STATE_RUNNING 0
#define TASK_STATE_SLEEPING 1
#define TASK_STATE_KILLED 2
#define TASK_STATE_ZOMBIE 3

struct rq {
	spin_lock_t spin_lock;
	u64 length;
	struct task_struct *current;
	struct task_struct *idle;
	struct list_head task_list;
};

struct sq {
	spin_lock_t spin_lock;
	u64 length;
	struct list_head task_list;
};

struct rq_flags {
};

struct task_struct *
switch_to(struct task_struct *prev, struct task_struct *next);

void task_init();
struct task_struct *get_current_task();
u64 get_current_task_stack();
void arch_init_kstack(struct task_struct *task, void (*fptr)(void), u64 stack, bool kernel);
struct task_struct *
__create_task(void (*fun)(void), int prio, int kstack_size, int kernel, int cpu);

struct task_struct *
create_task(void (*fun)(void), int prio, int kstack_size, int kernel, int cpu);
int task_timer_tick(int irq, void *data);





#endif

