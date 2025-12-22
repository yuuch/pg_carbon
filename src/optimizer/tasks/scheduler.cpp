#include "scheduler.h"

namespace pg_carbon {

void TaskScheduler::ScheduleTask(Task *task) {
    task_stack_.push(task);
}

void TaskScheduler::SetRules() {
    // Initialize rules (Simplification: just add one rule)
    if (rules_.empty()) {
        rules_.push_back(new RuleGetToScan());
        rules_.push_back(new RuleFilterToPhysical());

        rules_.push_back(new RuleLimitToPhysical());
        rules_.push_back(new RuleProjectionToPhysical());
    }

    // Assign IDs to rules
    for (size_t i = 0; i < rules_.size(); ++i) {
        rules_[i]->SetId(static_cast<int>(i));
    }
}

Task *TaskScheduler::GetTask() {
    if (task_stack_.empty()) {
        return nullptr;
    }
    Task *task = task_stack_.top();
    task_stack_.pop();
    return task;
}

void TaskScheduler::Run() {
    SetRules();

    while (true) {
        Task *task = GetTask();
        if (task == nullptr) {
            break;
        }
        task->perform(this);
        delete task;
    }
}

const PgVector<Rule *> &TaskScheduler::GetRules() const {
    return rules_;
}

} // namespace pg_carbon
