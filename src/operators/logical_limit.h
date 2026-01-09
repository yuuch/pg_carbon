#ifndef PG_CARBON_LOGICAL_LIMIT_H
#define PG_CARBON_LOGICAL_LIMIT_H

#include "operator.h"

namespace pg_carbon {

class LogicalLimit : public LogicalOperator {
public:
    LogicalLimit(Node *limit_offset, Node *limit_count)
        : limit_offset_(limit_offset), limit_count_(limit_count) {}

    std::string ToString() const override { return "LogicalLimit"; }
    Node *GetLimitOffset() const { return limit_offset_; }
    Node *GetLimitCount() const { return limit_count_; }

    LogicalProperties *DeriveLogicalProps(Memo *memo,
                                          const PgVector<Group *> &input_groups) const override;

private:
    Node *limit_offset_;
    Node *limit_count_;
};

} // namespace pg_carbon

#endif // PG_CARBON_LOGICAL_LIMIT_H
