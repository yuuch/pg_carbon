#ifndef PG_CARBON_OPTIMIZER_TASKS_SCHEDULER_H
#define PG_CARBON_OPTIMIZER_TASKS_SCHEDULER_H

#include "../../common/memory.h"
#include "../memo/memo.h"
#include "../rules/rules.h"
#include <stack>

namespace pg_carbon {

class TaskScheduler;

// Context for optimization (e.g., cost limits, required properties)
// Context for optimization (e.g., cost limits, required properties)
class Context : public PgObject {
public:
    Context(Memo *memo) : memo_(memo) {}
    Memo *GetMemo() const { return memo_; }

private:
    Memo *memo_;
};

class Task : public PgObject {
public:
    virtual ~Task() = default;
    // perform returns void in this design, logic is inside.
    virtual void perform(TaskScheduler *scheduler) = 0;
};

class TaskScheduler : public PgObject {
public:
    void ScheduleTask(Task *task);
    void Run();
    void SetRules();
    Task *GetTask();

    // Helper to get available rules, effectively part of the scheduler/optimizer
    // context
    const PgVector<Rule *> &GetRules() const;

private:
    PgStack<Task *> task_stack_;
    PgVector<Rule *> rules_; // Simplification: Rules stored here
};

} // namespace pg_carbon

#endif // PG_CARBON_OPTIMIZER_TASKS_SCHEDULER_H
