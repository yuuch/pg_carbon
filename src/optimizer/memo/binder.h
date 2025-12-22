#ifndef PG_CARBON_MEMO_BINDER_H
#define PG_CARBON_MEMO_BINDER_H

#include "../../common/memory.h"
#include "expression.h"
#include "memo.h"

namespace pg_carbon {

class Binder {
public:
    Binder(Expression *pattern, GroupExpression *root_expr)
        : pattern_(pattern), root_expr_(root_expr) {}

    // Get the next valid bound expression tree. Returns nullptr if exhausted.
    Expression *Next();

private:
    bool Advance();                // Advance internal state to next valid match
    Expression *BuildExpression(); // Build expression from current state

    // Helpers for backtracking
    bool FindNextMatchForChild(int child_idx);
    bool AdvanceChild(int child_idx);
    void ResetChild(int child_idx);

    Expression *pattern_;
    GroupExpression *root_expr_;

    struct ChildState {
        Group *group;
        Expression *pattern_child;
        int current_expr_idx = -1;           // Index in Group's logical_expressions
        Binder *sub_binder = nullptr;        // Recursive binder
        Expression *current_match = nullptr; // Cache for current match
    };

    PgVector<ChildState> children_states_;
    bool initialized_ = false;
};

} // namespace pg_carbon

#endif // PG_CARBON_MEMO_BINDER_H
