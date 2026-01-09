#ifndef PG_CARBON_PHYSICAL_NESTED_LOOP_JOIN_H
#define PG_CARBON_PHYSICAL_NESTED_LOOP_JOIN_H

#include "operator.h"

namespace pg_carbon {

class PhysicalNestedLoopJoin : public PhysicalOperator {
public:
    PhysicalNestedLoopJoin() = default;

    std::string ToString() const override { return "PhysicalNestedLoopJoin"; }
    double ComputeCost(Memo *memo, const PgVector<Group *> &input_groups,
                       GroupExpression *expr = nullptr) const override;
};

} // namespace pg_carbon

#endif // PG_CARBON_PHYSICAL_NESTED_LOOP_JOIN_H
