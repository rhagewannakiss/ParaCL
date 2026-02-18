#include <memory>
#include <stdexcept>
#include "gtest/gtest.h"
#include "AST/AST.hpp"
#include "Visitors/Visitor.hpp"

TEST(BaseSmokeTest, ValueNodeTest) {
    ast::ValueNode v(42);
    EXPECT_EQ(v.value(), 42);
    EXPECT_EQ(v.node_type(), ast::base_node_type::value);
}

TEST(BaseSmokeTest, VarNodeTest) {
    ast::VarNode var("a");
    EXPECT_EQ(var.name(), "a");
    EXPECT_EQ(var.node_type(), ast::base_node_type::var);
}

TEST(BaseSmokeTest, UnOpNodeTest) {
    auto op = std::make_unique<ast::VarNode>("a");
    ast::UnOpNode uo(ast::unop_node_type::neg, std::move(op));

    EXPECT_EQ(uo.node_type(), ast::base_node_type::unop);
    EXPECT_EQ(uo.op(), ast::unop_node_type::neg);
    ASSERT_NE(uo.operand(), nullptr);
    EXPECT_EQ(uo.operand()->node_type(), ast::base_node_type::var);
}

TEST(BaseSmokeTest, PrintNodeTest) {
    auto op = std::make_unique<ast::ValueNode>(69); 
    ast::PrintNode p(std::move(op));

    EXPECT_EQ(p.node_type(), ast::base_node_type::print);
    EXPECT_NE(p.expr(), nullptr);
}

TEST(BaseSmokeTest, ExprNodeTest) {
    auto v = std::make_unique<ast::ValueNode>(69);
    ast::ExprNode e(std::move(v));

    EXPECT_EQ(e.node_type(), ast::base_node_type::expr);
    EXPECT_NE(e.expr(), nullptr);
}

TEST(BaseSmokeTest, InputNodeTest) {
    auto var = std::make_unique<ast::VarNode>("var");
    ast::InputNode i(std::move(var));

    EXPECT_EQ(i.node_type(), ast::base_node_type::input);
    EXPECT_EQ(i.lhs()->node_type(), ast::base_node_type::var);
}

TEST(BaseSmokeTest, AssignNodeTest) {
    auto lhs = std::make_unique<ast::VarNode>("var");
    auto rhs = std::make_unique<ast::ValueNode>(69);
    ast::AssignNode a(std::move(lhs), std::move(rhs));
    
    EXPECT_EQ(a.node_type(), ast::base_node_type::assign);
    EXPECT_NE(a.lhs(), nullptr);
    EXPECT_NE(a.rhs(), nullptr);
}

TEST(BaseSmokeTest, IfNodeTest) {
    auto v11 = std::make_unique<ast::ValueNode>(69);
    auto v21 = std::make_unique<ast::ValueNode>(96);
    auto c1 = std::make_unique<ast::BinLogicOpNode>(
            ast::bin_logic_op_type::greater,
            std::move(v11),
            std::move(v21)
            );
    auto v31 = std::make_unique<ast::ValueNode>(69);
    auto v41 = std::make_unique<ast::ValueNode>(96);
    auto th1 = std::make_unique<ast::BinArithOpNode> (
            ast::bin_arith_op_type::add,
            std::move(v31),
            std::move(v41)
            );
    
    ast::IfNode i1(std::move(c1), std::move(th1), nullptr);
    EXPECT_NE(i1.condition(), nullptr);
    EXPECT_NE(i1.then_branch(), nullptr);
    EXPECT_EQ(i1.else_branch(), nullptr);

   
    auto v12 = std::make_unique<ast::ValueNode>(69);
    auto v22 = std::make_unique<ast::ValueNode>(96);
    auto c2 = std::make_unique<ast::BinLogicOpNode>(
            ast::bin_logic_op_type::greater,
            std::move(v12),
            std::move(v22)
            );
    auto v32 = std::make_unique<ast::ValueNode>(69);
    auto v42 = std::make_unique<ast::ValueNode>(96);
    auto th2 = std::make_unique<ast::BinArithOpNode> (
            ast::bin_arith_op_type::add,
            std::move(v32),
            std::move(v42)
            );
    auto v52 = std::make_unique<ast::ValueNode>(69);
    auto v62 = std::make_unique<ast::ValueNode>(96);
    auto e2 = std::make_unique<ast::BinArithOpNode> (
            ast::bin_arith_op_type::sub,
            std::move(v52),
            std::move(v62)
            );    

    ast::IfNode i2(std::move(c2), std::move(th2), std::move(e2));
    EXPECT_NE(i2.condition(), nullptr);
    EXPECT_NE(i2.then_branch(), nullptr);
    EXPECT_NE(i2.else_branch(), nullptr);
}

TEST(BaseSmokeTest, WhileNodeTest) {
    auto v1 = std::make_unique<ast::ValueNode>(69);
    auto v2 = std::make_unique<ast::ValueNode>(96);
    auto c = std::make_unique<ast::BinLogicOpNode>(
            ast::bin_logic_op_type::greater,
            std::move(v1),
            std::move(v2)
            );
    auto v3 = std::make_unique<ast::ValueNode>(69);
    auto v4 = std::make_unique<ast::ValueNode>(96);
    auto b = std::make_unique<ast::BinArithOpNode> (
            ast::bin_arith_op_type::add,
            std::move(v3),
            std::move(v4)
            );

    ast::WhileNode w(std::move(c), std::move(b));

    EXPECT_EQ(w.node_type(), ast::base_node_type::while_node);
    EXPECT_NE(w.condition(), nullptr);
    EXPECT_NE(w.body(), nullptr);
}

TEST(BaseSmokeTest, BinArithOpNode) {
    auto v1 = std::make_unique<ast::ValueNode>(69);
    auto v2 = std::make_unique<ast::ValueNode>(96);
    ast::BinArithOpNode b(
            ast::bin_arith_op_type::add,
            std::move(v1),
            std::move(v2)
            );

    EXPECT_EQ(b.node_type(), ast::base_node_type::bin_arith_op);
    EXPECT_EQ(b.op(), ast::bin_arith_op_type::add);
    EXPECT_NE(b.left(), nullptr);
    EXPECT_NE(b.right(), nullptr);
}

TEST(BaseSmokeTest, BinLogicOpNode) {
    auto v1 = std::make_unique<ast::ValueNode>(69);
    auto v2 = std::make_unique<ast::ValueNode>(96);
    ast::BinLogicOpNode b(
            ast::bin_logic_op_type::greater,
            std::move(v1),
            std::move(v2)
            );

    EXPECT_EQ(b.node_type(), ast::base_node_type::bin_logic_op);
    EXPECT_EQ(b.op(), ast::bin_logic_op_type::greater);
    EXPECT_NE(b.left(), nullptr);
    EXPECT_NE(b.right(), nullptr);
}

TEST(BaseSmokeTest, ScopeNodeTest) {
    auto var1 = std::make_unique<ast::VarNode>("a");
    auto val1 = std::make_unique<ast::ValueNode>(69);
    auto a1 = std::make_unique<ast::AssignNode>(
            std::move(var1),
            std::move(val1)
            );

    auto var2 = std::make_unique<ast::VarNode>("b");
    auto val2 = std::make_unique<ast::ValueNode>(6969);
    auto a2 = std::make_unique<ast::AssignNode>(
            std::move(var2),
            std::move(val2)
            );

    std::vector<ast::BaseNode::NodePtr> v;
    v.push_back(std::move(a1));
    v.push_back(std::move(a2));

    ast::ScopeNode s(std::move(v));
    
    EXPECT_EQ(s.node_type(), ast::base_node_type::scope);
    EXPECT_NE(s.statements().empty(), true);
    EXPECT_EQ(s.statements().size(), 2);
    EXPECT_NE(s.statements()[0], nullptr);
    EXPECT_NE(s.statements()[1], nullptr);
    EXPECT_EQ(s.statements()[0]->parent(), &s);
    EXPECT_EQ(s.statements()[1]->parent(), &s);
}

TEST(BaseSmokeTest, VarDeclNodeTest) {
    auto val1 = std::make_unique<ast::ValueNode>(69);
    ast::VarDeclNode vd1("var1", std::move(val1));

    EXPECT_EQ(vd1.node_type(), ast::base_node_type::var_decl);
    EXPECT_NE(vd1.init_expr(), nullptr);

    ast::VarDeclNode vd2("var2", nullptr);
    EXPECT_EQ(vd2.init_expr(), nullptr);
}

TEST(BaseSmokeTest, ForNodeFullTest) {
    auto i = std::make_unique<ast::AssignNode>(
        std::make_unique<ast::VarNode>("var"), 
        std::make_unique<ast::ValueNode>(69)
    );
    auto c = std::make_unique<ast::BinLogicOpNode>(
        ast::bin_logic_op_type::less,
        std::make_unique<ast::VarNode>("var"),
        std::make_unique<ast::ValueNode>(96)
    );
    auto s = std::make_unique<ast::BinArithOpNode>(
        ast::bin_arith_op_type::add,
        std::make_unique<ast::VarNode>("var"),
        std::make_unique<ast::ValueNode>(1)
    );
    auto b = std::make_unique<ast::PrintNode>(
        std::make_unique<ast::VarNode>("var") 
    );

    ast::ForNode f(std::move(i), std::move(c), std::move(s), std::move(b));
    EXPECT_NE(f.get_init(), nullptr);
    EXPECT_NE(f.get_cond(), nullptr);
    EXPECT_NE(f.get_step(), nullptr);
    EXPECT_NE(f.get_body(), nullptr);
}

TEST(BaseSmokeTest, ForNodeMinimalTest) {
    auto c = std::make_unique<ast::BinLogicOpNode>(
        ast::bin_logic_op_type::less,
        std::make_unique<ast::VarNode>("var"),
        std::make_unique<ast::ValueNode>(96)
    );

    auto b = std::make_unique<ast::PrintNode>(
        std::make_unique<ast::VarNode>("var") 
    );

    ast::ForNode f(nullptr, std::move(c), nullptr, std::move(b));
    EXPECT_EQ(f.get_init(), nullptr);
    EXPECT_NE(f.get_cond(), nullptr);
    EXPECT_EQ(f.get_step(), nullptr);
    EXPECT_NE(f.get_body(), nullptr);
}

TEST(BaseSmokeTest, ASTTest) {
    auto v = std::make_unique<ast::ValueNode>(69);
    ast::AST a;
    a.set_root(std::move(v));

    EXPECT_NE(a.root(), nullptr);

    ast::AST b = a;
    EXPECT_NE(b.root(), nullptr);
    EXPECT_NE(&a, &b);
}

TEST(BaseSmokeTest, BaseNodeSafetyTest) {
    ast::ScopeNode s;

    EXPECT_THROW(s.add_statement(nullptr), 
                std::runtime_error);

    
    class TempNode : public ast::BaseNode {
        public:    
        explicit TempNode() : 
            BaseNode(ast::base_node_type::base) {}
        
        TempNode(const TempNode& other) : 
            BaseNode(other) {}
    
        TempNode& operator=(const TempNode& other) {
            if (this == &other) return *this;
            BaseNode::operator=(other);
            return *this;
        }
    
        TempNode(TempNode&& other) noexcept = default;
        TempNode& operator=(TempNode&& other) noexcept = default;
            
        void ask_add_child_front(ast::BaseNode::NodePtr np) {
            add_child_front(std::move(np));
        }

        inline void accept(ast::Visitor& v) override {
            (void)v;
            return;
        }
    
        NodePtr clone() const override {
            return std::make_unique<TempNode>(*this);
        }
    };
    
    TempNode tn;
    EXPECT_THROW(tn.ask_add_child_front(nullptr), 
                std::runtime_error);
}
