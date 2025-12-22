#include "binder.h"

namespace pg_carbon {

Expression *Binder::Next() {
    if (Advance()) {
        return BuildExpression();
    }
    return nullptr;
}

bool Binder::Advance() {
    if (!pattern_ || !root_expr_)
        return false;

    // Initialization phase
    if (!initialized_) {
        initialized_ = true;

        // 1. PatternLeaf case: matches anything
        if (dynamic_cast<PatternLeaf *>(pattern_->GetOperator())) {
            return true;
        }

        // 2. Structural Check: Verify Arity
        // If pattern operator is not leaf, it must match root structure
        // We assume Rule::Matches already validated top-level operator type
        // compatibility if strict, or we can add strict check here. PatternLeaf is
        // loose. Let's assume generic matching. PatternLeaf handled above. If
        // pattern has children but root doesn't -> fail (arity check covers this).

        const auto &pattern_children = pattern_->GetChildren();
        const auto &group_children = root_expr_->GetChildren();

        if (pattern_children.size() != group_children.size()) {
            return false;
        }

        // 3. Pattern with no children (Leaf Operator like SCAN)
        if (pattern_children.empty()) {
            return true;
        }

        // 4. Initialize Child States
        children_states_.reserve(pattern_children.size());
        for (size_t i = 0; i < pattern_children.size(); ++i) {
            ChildState state;
            state.group = group_children[i];
            state.pattern_child = pattern_children[i];
            state.current_expr_idx = -1;
            state.sub_binder = nullptr;
            state.current_match = nullptr;
            children_states_.push_back(state);
        }

        // 5. Find FIRST valid match for all children
        for (size_t i = 0; i < children_states_.size(); ++i) {
            if (!FindNextMatchForChild(i)) {
                // If any child cannot be matched, the whole thing fails
                return false;
            }
        }

        return true;
    }

    // If we already yielded the single match (Leaf/PatternLeaf), stop
    if (dynamic_cast<PatternLeaf *>(pattern_->GetOperator()) || pattern_->GetChildren().empty()) {
        return false;
    }

    // Backtracking logic
    int cursor = children_states_.size() - 1;
    while (cursor >= 0) {
        if (AdvanceChild(cursor)) {
            // Success advancing this child. Reset subsequent bindings.
            bool reset_success = true;
            for (size_t right = cursor + 1; right < children_states_.size(); ++right) {
                ResetChild(right);
                if (!FindNextMatchForChild(right)) {
                    reset_success = false;
                    break;
                }
            }

            if (reset_success) {
                return true;
            }
            // If reset failed, continue loop (backtrack further)
        }
        cursor--;
    }

    return false;
}

bool Binder::FindNextMatchForChild(int child_idx) {
    ChildState &state = children_states_[child_idx];
    const auto &exprs = state.group->GetLogicalExpressions();

    while (true) {
        // 1. Try existing binder
        if (state.sub_binder) {
            Expression *match = state.sub_binder->Next();
            if (match) {
                state.current_match = match;
                return true;
            }
            // Exhausted, cleanup
            delete state.sub_binder;
            state.sub_binder = nullptr;
            state.current_match = nullptr;
        }

        // 2. Move to next expression in Group
        state.current_expr_idx++;
        if (state.current_expr_idx >= (int)exprs.size()) {
            return false; // No more expressions
        }

        // 3. Create new binder
        GroupExpression *gexpr = exprs[state.current_expr_idx];
        // Note: We need deeper checks here? For example, if gexpr's operator type
        // doesn't match pattern_child's operator type (unless pattern_child is
        // leaf), we could optimize by skipping Binder creation. But
        // Binder::Advance() handles type/arity checks inside init.
        state.sub_binder = new Binder(state.pattern_child, gexpr);
        // Loop continues to step 1
    }
}

bool Binder::AdvanceChild(int child_idx) {
    // Just try to get next from current sub_binder inside FindNextMatchForChild
    // loop logic Actually FindNextMatchForChild loops until it finds a match or
    // exhausts expressions. If we have an active sub_binder, calling
    // FindNextMatchForChild will simply resume from it. Wait,
    // FindNextMatchForChild loops `while (true)`. It checks `state.sub_binder`.
    // If valid, calls Next(). So yes, simply calling it works.
    return FindNextMatchForChild(child_idx);
}

void Binder::ResetChild(int child_idx) {
    ChildState &state = children_states_[child_idx];
    if (state.sub_binder) {
        delete state.sub_binder;
        state.sub_binder = nullptr;
    }
    state.current_match = nullptr;
    state.current_expr_idx = -1;
}

Expression *Binder::BuildExpression() {
    if (dynamic_cast<PatternLeaf *>(pattern_->GetOperator())) {
        auto res = new Expression(new PatternLeaf());
        if (root_expr_->GetGroup()) {
            res->SetGroup(root_expr_->GetGroup());
        }
        return res;
    }

    PgVector<Expression *> child_exprs;
    for (const auto &state : children_states_) {
        child_exprs.push_back(state.current_match);
    }

    // Clone root operator
    return new Expression(root_expr_->GetOperator(), child_exprs);
}

} // namespace pg_carbon
