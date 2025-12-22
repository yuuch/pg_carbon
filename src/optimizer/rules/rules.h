#ifndef PG_CARBON_RULES_H
#define PG_CARBON_RULES_H

#include "../../common/memory.h"
#include "../memo/expression.h"
#include "../memo/memo.h"

namespace pg_carbon {

class Rule : public PgObject {
public:
    virtual ~Rule() = default;
    virtual Expression *GetPattern() const = 0;
    virtual bool Matches(Expression *expr) const { return true; }
    virtual PgVector<Expression *> Transform(Expression *expr) const = 0;
    virtual std::string ToString() const = 0;

    void SetId(int id) { rule_id_ = id; }
    int GetId() const { return rule_id_; }

protected:
    int rule_id_ = -1;
    mutable Expression *pattern_ = nullptr; // Cache for pattern
};

class RuleGetToScan : public Rule {
public:
    Expression *GetPattern() const override;
    bool Matches(Expression *expr) const override;
    PgVector<Expression *> Transform(Expression *expr) const override;
    std::string ToString() const override { return "RuleGetToScan"; }
};

class RuleFilterToPhysical : public Rule {
public:
    Expression *GetPattern() const override;
    bool Matches(Expression *expr) const override;
    PgVector<Expression *> Transform(Expression *expr) const override;
    std::string ToString() const override { return "RuleFilterToPhysical"; }
};

class RuleProjectionToPhysical : public Rule {
public:
    Expression *GetPattern() const override;
    bool Matches(Expression *expr) const override;
    PgVector<Expression *> Transform(Expression *expr) const override;
    std::string ToString() const override { return "RuleProjectionToPhysical"; }
};

class RuleLimitToPhysical : public Rule {
public:
    Expression *GetPattern() const override;
    bool Matches(Expression *expr) const override;
    PgVector<Expression *> Transform(Expression *expr) const override;
    std::string ToString() const override { return "RuleLimitToPhysical"; }
};

} // namespace pg_carbon

#endif // PG_CARBON_RULES_H
