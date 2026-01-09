#ifndef PG_CARBON_LOGICAL_INNER_JOIN_H
#define PG_CARBON_LOGICAL_INNER_JOIN_H

#include "operator.h"

namespace pg_carbon {

class LogicalInnerJoin : public LogicalOperator {
public:
    LogicalInnerJoin() = default;

    std::string ToString() const override { return "LogicalInnerJoin"; }

    LogicalProperties *DeriveLogicalProps(Memo *memo,
                                          const PgVector<Group *> &input_groups) const override;
};

} // namespace pg_carbon

#endif // PG_CARBON_LOGICAL_INNER_JOIN_H
