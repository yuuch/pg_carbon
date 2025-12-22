#ifndef PG_CARBON_MEMO_EXPRESSION_H
#define PG_CARBON_MEMO_EXPRESSION_H

#include "../../common/memory.h"
#include "../../operators/operators.h"

namespace pg_carbon {

class Group; // Forward declaration

class Expression : public PgObject {
public:
    Expression(Operator *op) : op_(op) {}
    Expression(Operator *op, const PgVector<Expression *> &children)
        : op_(op), children_(children) {}

    Operator *GetOperator() const { return op_; }
    const PgVector<Expression *> &GetChildren() const { return children_; }
    void AddChild(Expression *child) { children_.push_back(child); }

    void SetGroup(Group *group) { group_ = group; }
    Group *GetGroup() const { return group_; }

private:
    Operator *op_;
    PgVector<Expression *> children_;
    Group *group_ = nullptr; // Reference to the Group if this is a leaf/bound node
};

} // namespace pg_carbon

#endif // PG_CARBON_MEMO_EXPRESSION_H
