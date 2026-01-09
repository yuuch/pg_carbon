#ifndef PG_CARBON_RULE_H
#define PG_CARBON_RULE_H

#include "../../common/memory.h"
#include "../memo/expression.h"
#include "../memo/memo.h"

namespace pg_carbon {

// Enum for Rule Types
enum RuleType {
    RULE_GET_TO_SEQ_SCAN,
    RULE_GET_TO_INDEX_SCAN,
    RULE_FILTER_TO_PHYSICAL,
    RULE_PROJECTION_TO_PHYSICAL,
    RULE_LIMIT_TO_PHYSICAL,
};

class Rule : public PgObject {
public:
    Rule(RuleType rule_id) : rule_id_(rule_id) {}
    virtual ~Rule() = default;

    // Pattern is accessed via GetPattern but stored in pattern_
    // Subclasses should initialize pattern_ in their constructor.
    virtual Expression *GetPattern() const { return pattern_; }

    virtual bool Matches(Expression * /*expr*/) const { return true; }
    virtual PgVector<Expression *> Transform(Expression *expr) const = 0;
    virtual std::string ToString() const = 0;

    int GetId() const { return static_cast<int>(rule_id_); }

protected:
    RuleType rule_id_;
    mutable Expression *pattern_ = nullptr; // Cache for pattern
};

} // namespace pg_carbon

#endif // PG_CARBON_RULE_H
