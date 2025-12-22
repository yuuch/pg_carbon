#include "opt_expr.h"
#include "apply_rule.h"
#include "expl_grp.h"
#include <typeinfo>

namespace pg_carbon {

// 3. OptExpr (Optimize Expression)
void OptExpr::perform(TaskScheduler *scheduler) {
    // 26: moves <- empty
    PgVector<ApplyRule *> moves;

    // 27: for rule in Rules do
    const auto &rules = scheduler->GetRules();
    for (auto *rule : rules) {
        // 28: if !expr.IsApplied(rule) and rule.CheckPattern(expr)
        // 16: Check if rule is applicable (Top-level operator match)
        Operator *pattern_op = rule->GetPattern()->GetOperator();
        bool is_applicable = false;

        if (dynamic_cast<PatternLeaf *>(pattern_op)) {
            is_applicable = true;
        } else if (typeid(*pattern_op) == typeid(*expr_->GetOperator())) {
            is_applicable = true;
        }

        if (!expr_->HasAppliedRule(rule) && is_applicable) {
            // 29: moves.Add(ApplyRule(expr, rule, promise, limit))
            moves.push_back(new ApplyRule(rule, expr_, context_, false));
        }
    }

    // 30: Sort the moves by promise in ascending order for the LIFO stack
    // (Sorting omitted for now, assuming order is acceptable or promise is equal)

    // 31: for m in moves do
    // 32: tasks.Push(m)
    for (auto *m : moves) {
        scheduler->ScheduleTask(m);
    }

    // 33: for child in inputs of expr do
    const auto &children = expr_->GetChildren();
    for (int i = children.size() - 1; i >= 0; --i) {
        auto *child_group = children[i];
        // 34: grp <- GetGroup(child)
        // 35: if !grp.Explored then
        if (!child_group->IsExplored()) {
            // 36: tasks.Push(ExplGrp(grp, limit))
            scheduler->ScheduleTask(new ExplGrp(child_group, context_));
        }
    }
}

} // namespace pg_carbon
