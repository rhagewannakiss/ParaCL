#include "gtest/gtest.h"
#include "AST/AST.hpp"
#include "Visitors/Interpreter.hpp"

TEST(PrintNodeUnitTest, CheckNodeTypesInExpr) {
    ast::Interpreter interpreter;

    {
        ast::PrintNode node(std::make_unique<ast::ScopeNode>());
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::PrintNode node(std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(1)
        ));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::PrintNode node(std::make_unique<ast::WhileNode>(
            std::make_unique<ast::ValueNode>(0),
            std::make_unique<ast::ExprNode>(std::make_unique<ast::ValueNode>(1))
        ));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::PrintNode node(std::make_unique<ast::InputNode>(
            std::make_unique<ast::ValueNode>(1)
        ));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::PrintNode node(std::make_unique<ast::VarDeclNode>(
            "x",
            std::make_unique<ast::ValueNode>(1)
        ));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::PrintNode node(std::make_unique<ast::PrintNode>(
            std::make_unique<ast::ValueNode>(1)
        ));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::PrintNode node(std::make_unique<ast::IfNode>(
            std::make_unique<ast::ValueNode>(1),
            std::make_unique<ast::ExprNode>(std::make_unique<ast::ValueNode>(1)),
            nullptr
        ));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }
}

TEST(InputNodeUnitTest, CheckAvailableNodesForInput) {
    ast::Interpreter interpreter;

    {
        ast::InputNode node(std::make_unique<ast::ValueNode>(69));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::InputNode node(std::make_unique<ast::UnOpNode>(
            ast::unop_node_type::neg,
            std::make_unique<ast::ValueNode>(1)
        ));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::InputNode node(std::make_unique<ast::BinArithOpNode>(
            ast::bin_arith_op_type::add,
            std::make_unique<ast::ValueNode>(1),
            std::make_unique<ast::ValueNode>(2)
        ));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::InputNode node(std::make_unique<ast::BinLogicOpNode>(
            ast::bin_logic_op_type::greater,
            std::make_unique<ast::ValueNode>(1),
            std::make_unique<ast::ValueNode>(2)
        ));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::InputNode node(std::make_unique<ast::ExprNode>(
            std::make_unique<ast::ValueNode>(1)
        ));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::InputNode node(std::make_unique<ast::PrintNode>(
            std::make_unique<ast::ValueNode>(1)
        ));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::InputNode node(std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(1)
        ));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::InputNode node(std::make_unique<ast::VarDeclNode>(
            "x",
            std::make_unique<ast::ValueNode>(1)
        ));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::InputNode node(std::make_unique<ast::ScopeNode>());
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::InputNode node(std::make_unique<ast::WhileNode>(
            std::make_unique<ast::ValueNode>(0),
            std::make_unique<ast::ExprNode>(std::make_unique<ast::ValueNode>(1))
        ));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::InputNode node(std::make_unique<ast::IfNode>(
            std::make_unique<ast::ValueNode>(1),
            std::make_unique<ast::ExprNode>(std::make_unique<ast::ValueNode>(1)),
            nullptr
        ));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::InputNode node(std::make_unique<ast::InputNode>(
            std::make_unique<ast::VarNode>("x")
        ));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }
}

inline ast::BaseNode::NodePtr MakeThenExpr() {
    return std::make_unique<ast::ExprNode>(
            std::make_unique<ast::ValueNode>(1)
            );
}

inline ast::BaseNode::NodePtr MakeWhileBodyScope() {
    return std::make_unique<ast::ScopeNode>();
}

inline ast::BaseNode::NodePtr MakeForBodyScope() {
    return std::make_unique<ast::ScopeNode>();
}

TEST(IfNodeUnitTest, CheckAvailableNodesForCondition) {
    ast::Interpreter interpreter;

    {
        ast::IfNode node(
            std::make_unique<ast::AssignNode>(
                std::make_unique<ast::VarNode>("x"),
                std::make_unique<ast::ValueNode>(1)
            ),
            MakeThenExpr(),
            nullptr
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::IfNode node(
            std::make_unique<ast::PrintNode>(
                std::make_unique<ast::ValueNode>(1)
            ),
            MakeThenExpr(),
            nullptr
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::IfNode node(
            std::make_unique<ast::ScopeNode>(),
            MakeThenExpr(),
            nullptr
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::IfNode node(
            std::make_unique<ast::WhileNode>(
                std::make_unique<ast::ValueNode>(0),
                std::make_unique<ast::ExprNode>(std::make_unique<ast::ValueNode>(1))
            ),
            MakeThenExpr(),
            nullptr
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::IfNode node(
            std::make_unique<ast::InputNode>(
                std::make_unique<ast::VarNode>("x")
            ),
            MakeThenExpr(),
            nullptr
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::IfNode node(
            std::make_unique<ast::VarDeclNode>(
                "x",
                std::make_unique<ast::ValueNode>(1)
            ),
            MakeThenExpr(),
            nullptr
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::IfNode node(
            std::make_unique<ast::IfNode>(
                std::make_unique<ast::ValueNode>(0),
                std::make_unique<ast::ExprNode>(std::make_unique<ast::ValueNode>(1)),
                nullptr
            ),
            MakeThenExpr(),
            nullptr
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }
}

TEST(WhileNodeUnitTest, CheckAvailableNodesForCondition) {
    ast::Interpreter interpreter;

    {
        ast::WhileNode node(
            std::make_unique<ast::AssignNode>(
                std::make_unique<ast::VarNode>("x"),
                std::make_unique<ast::ValueNode>(1)
            ),
            MakeWhileBodyScope()
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::WhileNode node(
            std::make_unique<ast::PrintNode>(
                std::make_unique<ast::ValueNode>(1)
            ),
            MakeWhileBodyScope()
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::WhileNode node(
            std::make_unique<ast::ScopeNode>(),
            MakeWhileBodyScope()
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::WhileNode node(
            std::make_unique<ast::WhileNode>(
                std::make_unique<ast::ValueNode>(0),
                std::make_unique<ast::ScopeNode>()
            ),
            MakeWhileBodyScope()
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::WhileNode node(
            std::make_unique<ast::InputNode>(
                std::make_unique<ast::VarNode>("x")
            ),
            MakeWhileBodyScope()
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::WhileNode node(
            std::make_unique<ast::VarDeclNode>(
                "x",
                std::make_unique<ast::ValueNode>(1)
            ),
            MakeWhileBodyScope()
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::WhileNode node(
            std::make_unique<ast::IfNode>(
                std::make_unique<ast::ValueNode>(0),
                std::make_unique<ast::ExprNode>(std::make_unique<ast::ValueNode>(1)),
                nullptr
            ),
            MakeWhileBodyScope()
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }
}

TEST(WhileNodeUnitTest, CheckAvailableNodesForBody) {
    ast::Interpreter interpreter;

    {
        ast::WhileNode node(
            std::make_unique<ast::ValueNode>(1),
            std::make_unique<ast::ExprNode>(
                std::make_unique<ast::ValueNode>(1)
            )
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::WhileNode node(
            std::make_unique<ast::ValueNode>(1),
            std::make_unique<ast::AssignNode>(
                std::make_unique<ast::VarNode>("x"),
                std::make_unique<ast::ValueNode>(1)
            )
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }
}

TEST(ForNodeUnitTest, CheckAvailableNodesForCondition) {
    ast::Interpreter interpreter;

    {
        ast::ForNode node(
            nullptr,
            std::make_unique<ast::AssignNode>(
                std::make_unique<ast::VarNode>("x"),
                std::make_unique<ast::ValueNode>(1)
            ),
            nullptr,
            MakeForBodyScope()
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::ForNode node(
            nullptr,
            std::make_unique<ast::PrintNode>(
                std::make_unique<ast::ValueNode>(1)
            ),
            nullptr,
            MakeForBodyScope()
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::ForNode node(
            nullptr,
            std::make_unique<ast::IfNode>(
                std::make_unique<ast::ValueNode>(0),
                std::make_unique<ast::ExprNode>(std::make_unique<ast::ValueNode>(1)),
                nullptr
            ),
            nullptr,
            MakeForBodyScope()
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }
}

TEST(ForNodeUnitTest, CheckAvailableNodesForBody) {
    ast::Interpreter interpreter;

    {
        ast::ForNode node(
            nullptr,
            std::make_unique<ast::ValueNode>(1),
            nullptr,
            std::make_unique<ast::ExprNode>(
                std::make_unique<ast::ValueNode>(1)
            )
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::ForNode node(
            nullptr,
            std::make_unique<ast::ValueNode>(1),
            nullptr,
            std::make_unique<ast::AssignNode>(
                std::make_unique<ast::VarNode>("x"),
                std::make_unique<ast::ValueNode>(1)
            )
        );
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }
}
