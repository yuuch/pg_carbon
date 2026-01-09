#ifndef PG_CARBON_OPTIMIZER_TASKS_APPLY_RULE_H
#define PG_CARBON_OPTIMIZER_TASKS_APPLY_RULE_H

#include "scheduler.h"

namespace pg_carbon {

// 5. ApplyRule
class ApplyRule : public Task {
public:
    ApplyRule(Rule *rule, GroupExpression *expr, Context *context, bool /*exploring*/)
        : rule_(rule), expr_(expr), context_(context) {}
    void perform(TaskScheduler *scheduler) override;

    int promise_ = 0; // For sorting if needed

private:
    Rule *rule_;
    GroupExpression *expr_;
    Context *context_;
    // bool exploring_; // Unused
};

} // namespace pg_carbon

#endif // PG_CARBON_OPTIMIZER_TASKS_APPLY_RULE_H
