#ifndef PG_CARBON_LOGICAL_GET_H
#define PG_CARBON_LOGICAL_GET_H

#include "operator.h"

namespace pg_carbon {

class LogicalGet : public LogicalOperator {
public:
    LogicalGet(Oid table_oid, Index rtindex) : table_oid_(table_oid), rtindex_(rtindex) {}

    std::string ToString() const override {
        return "LogicalGet(" + std::to_string(table_oid_) + ")";
    }
    Oid GetTableOid() const { return table_oid_; }
    Index GetRtIndex() const { return rtindex_; }

    LogicalProperties *DeriveLogicalProps(Memo *memo,
                                          const PgVector<Group *> &input_groups) const override;

private:
    Oid table_oid_;
    Index rtindex_;
};

} // namespace pg_carbon

#endif // PG_CARBON_LOGICAL_GET_H
