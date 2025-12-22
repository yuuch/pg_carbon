#include "apply_rule.h"
#include "../memo/binder.h"
#include "../memo/expression.h"
#include "../memo/memo.h"
#include "../rules/rules.h"
#include "expl_expr.h"
#include "opt_inputs.h"

namespace pg_carbon {

// 5. ApplyRule
void ApplyRule::perform(TaskScheduler *scheduler) {
    // Mark rule as applied
    expr_->AddAppliedRule(rule_);

    // Apply Rule using match pattern (Binder)
    auto *pattern = rule_->GetPattern();
    Binder binder(pattern, expr_);

    while (Expression *bound_expr = binder.Next()) {
        if (rule_->Matches(bound_expr)) {
            auto new_exprs = rule_->Transform(bound_expr);

            // Iterate transformed expressions
            for (auto *new_expr : new_exprs) {
                // Process children to ensure they are in Memo
                PgVector<Group *> child_groups;
                for (auto *child : new_expr->GetChildren()) {
                    // CopyIn creates groups for new sub-trees or returns existing ones
                    // for bound leaves
                    Group *child_group = context_->GetMemo()->CopyIn(child);
                    child_groups.push_back(child_group);
                }

                // Create new GroupExpression
                auto *new_group_expr = new GroupExpression(new_expr->GetOperator(), child_groups);

                // Add to CURRENT group
                Group *current_group = expr_->GetGroup();

                // Optimization: Check for duplicates? Memo/Group logic should handle
                // it? Using InsertExpression might put it in a NEW group if we used
                // Memo::Insert. But here we enforce it to be in current_group.

                if (!current_group) {
                    // Should not happen if expr_ is in a group
                    // Log error?
                    continue;
                }

                // We manually set group and add.
                new_group_expr->SetGroup(current_group);
                current_group->AddExpression(new_group_expr);

                // Schedule Follow-up Tasks
                if (new_expr->GetOperator()->IsLogical()) {
                    scheduler->ScheduleTask(new ExplExpr(new_group_expr, context_));
                } else {
                    scheduler->ScheduleTask(new OptInputs(new_group_expr, context_));
                }
            }
        }
    }
}

} // namespace pg_carbon
