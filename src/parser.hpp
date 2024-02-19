#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <optional>
#include <variant>
#include <cassert>
#include "./arena.hpp"
//#include "./tokenization.hpp"

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeExpr;

struct NodeBinExprAdd {
    NodeExpr* l;
    NodeExpr* r;
};

struct NodeBinExprSub {
    NodeExpr* l;
    NodeExpr* r;
};

struct NodeBinExprMult {
    NodeExpr* l;
    NodeExpr* r;
};

struct NodeBinExprDiv {
    NodeExpr* l;
    NodeExpr* r;
};

//Logic Stuff//
struct NodeBinExprEq { //==
    NodeExpr* l;
    NodeExpr* r;
};

struct NodeBinExprGrEq { //>=
    NodeExpr* l;
    NodeExpr* r;
};

struct NodeBinExprLsEq { //<=
    NodeExpr* l;
    NodeExpr* r;
};

struct NodeBinExprGr { //>
    NodeExpr* l;
    NodeExpr* r;
};

struct NodeBinExprLs { //<
    NodeExpr* l;
    NodeExpr* r;
};

struct NodeBinExpr {
    std::variant<NodeBinExprAdd*, NodeBinExprSub*, NodeBinExprMult*, NodeBinExprDiv*,
        NodeBinExprEq*, NodeBinExprGrEq*, NodeBinExprLsEq*, NodeBinExprGr*, NodeBinExprLs*> var;
};

struct NodeTermParen {
    //Parenthasized Expr
    NodeExpr* expr;
};

struct NodeParamsIn {
    std::vector<NodeExpr*> params;
};

struct NodeTermFunc {
    Token ident;
    std::optional<NodeParamsIn*> params;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*, NodeTermFunc*> var;
};

struct NodeExpr {
    std::variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeStmtExit {
    NodeExpr* expr;
};

struct NodeStmtVar {
    Token ident;
    NodeExpr* expr;
};

struct NodeStmt;

struct NodeScope {
    std::vector<NodeStmt*> stmts;
};

struct NodeIfPred;

struct NodeIfPredWif {
    NodeExpr* expr;
    NodeScope* scope;
    std::optional<NodeIfPred*> pred;
};

struct NodeIfPredElse {
    NodeScope* scope;
};

struct NodeIfPred {
    std::variant<NodeIfPredWif*, NodeIfPredElse*> var;
};

struct NodeStmtIf {
    NodeExpr* expr;
    NodeScope* scope;
    std::optional<NodeIfPred*> pred;
};

struct NodeStmtWhile {
    NodeExpr* expr;
    NodeScope* scope;
};

struct NodeStmtAssign {
    Token ident;
    NodeExpr* expr;
};

struct NodeParamsDec {
    std::vector<Token> params;
};

struct NodeStmtGive {
    NodeExpr* expr;
};

struct NodeStmtFunc {
    Token ident;
    std::optional<NodeParamsDec*> params;
    NodeScope* scope;
};

struct NodeStmtFuncCall {
    Token ident;
    std::optional<NodeParamsIn*> params;
};

//using instead of struct?
struct NodeStmt {
    std::variant<NodeStmtExit*, NodeStmtVar*, NodeScope*, NodeStmtIf*, NodeStmtWhile*, 
        NodeStmtAssign*, NodeStmtFunc*, NodeStmtGive*, NodeStmtFuncCall*> var;
};

struct NodeProg {
    std::vector<NodeStmt*> stmts;
};


class Parser {

public:
    inline explicit Parser(const std::vector<Token> tokens) : m_tokens(std::move(tokens)), 
    m_allocator(1024*1024*4) // 4mb
    {
    }

    std::optional<NodeParamsIn*> parse_func_params_in() {
        if (peek().is_value && peek().value.type != TokenType::close_p){
            auto param = m_allocator.alloc<NodeParamsIn>();
            while (peek().is_value && peek().value.type != TokenType::close_p) {
                auto expr = parse_expr();
                if (!expr.has_value()) {
                    std::cerr << "Expected Expression" << std::endl;
                    exit(EXIT_FAILURE);
                }
                param->params.push_back(expr.value());
                if (peek().is_value && peek().value.type != TokenType::close_p) {
                    try_cons(TokenType::comma, "Expected ',' for parameters");
                }
            }
            return param;
        }
        return {};
    }

    std::optional<NodeTerm*> parse_term() {
        if (auto int_lit = try_cons(TokenType::int_literal)) {
            auto term_int_lit = m_allocator.alloc<NodeTermIntLit>(); //Allocating Node
            term_int_lit->int_lit = int_lit.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_int_lit;
            return term;
        } else if (peek().is_value && peek().value.type == TokenType::ident &&
                   peek(1).is_value && peek(1).value.type == TokenType::open_p) {
            auto ident = try_cons(TokenType::ident);
            consume(); //rid of (
            auto term_ident = m_allocator.alloc<NodeTermFunc>(); //Allocating Node
            term_ident->ident = ident.value();
            auto term_param = parse_func_params_in();
            consume(); //rid of )
            term_ident->params = term_param;
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_ident;
            return term;
        } else if (auto ident = try_cons(TokenType::ident)) {
            auto term_ident = m_allocator.alloc<NodeTermIdent>(); //Allocating Node
            term_ident->ident = ident.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_ident;
            return term;
        } else if (auto ident = try_cons(TokenType::open_p)) {
            auto expr = parse_expr();
            if (!expr.has_value()) {
                std::cerr << "Expected Expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_cons(TokenType::close_p, "Expected ')'");
            auto term_paren = m_allocator.alloc<NodeTermParen>();
            term_paren->expr = expr.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_paren;
            return term;
        } else {
            return {};
        }
    }

    std::optional<NodeExpr*> parse_expr(int min_prec = 0) {
        //val debug = peek();
        //int yo = 0;
        std::optional<NodeTerm*> term_l = parse_term();
        if (!term_l.has_value()) {
            return {};
        }

        auto expr_l = m_allocator.alloc<NodeExpr>(); //Left side of bin_expr
        expr_l->var = term_l.value();

        while (true) { //Precedence Tracking! :) Thank you Eli B. :)
            val cur_token = peek();
            std::optional<int> prec; //used throughout func
            if (cur_token.is_value) {
                prec = bin_prec(cur_token.value.type);
                if (!prec.has_value() || prec < min_prec) {
                    break;
                }
            } else {
                break;
            }
            Token oper = consume();
            std::optional<Token> oper2;
            if (peek().is_value && peek().value.type == TokenType::eq) { //checks if 2 chars
                oper2 = consume();
            }
            int next_min_prec = prec.value() + 1;
            auto expr_r = parse_expr(next_min_prec); //recursion :)
            if (!expr_r.has_value()) {
                std::cerr << "Unable to parse expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            auto expr = m_allocator.alloc<NodeBinExpr>();
            auto expr_l2 = m_allocator.alloc<NodeExpr>(); //clone it :) no loops
            if (oper.type == TokenType::plus) {
                auto add = m_allocator.alloc<NodeBinExprAdd>();
                expr_l2->var = expr_l->var; //huh cloning :)
                add->l = expr_l2;
                add->r = expr_r.value();
                expr->var = add;
            } else if (oper.type == TokenType::ast) {
                auto multi = m_allocator.alloc<NodeBinExprMult>();
                expr_l2->var = expr_l->var; //huh cloning :)
                multi->l = expr_l2;
                multi->r = expr_r.value();
                expr->var = multi;
            } else if (oper.type == TokenType::sub) {
                auto sub = m_allocator.alloc<NodeBinExprSub>();
                expr_l2->var = expr_l->var; //huh cloning :)
                sub->l = expr_l2;
                sub->r = expr_r.value();
                expr->var = sub;
            } else if (oper.type == TokenType::f_slash) {
                auto div = m_allocator.alloc<NodeBinExprDiv>();
                expr_l2->var = expr_l->var; //huh cloning :)
                div->l = expr_l2;
                div->r = expr_r.value();
                expr->var = div;
            } else if (oper.type == TokenType::eq && oper2.has_value() && oper2.value().type == TokenType::eq) {
                auto equiv = m_allocator.alloc<NodeBinExprEq>();
                expr_l2->var = expr_l->var; //huh cloning :)
                equiv->l = expr_l2;
                equiv->r = expr_r.value();
                expr->var = equiv;
            } else if (oper.type == TokenType::greater && oper2.has_value() && oper2.value().type == TokenType::eq) {
                auto equiv = m_allocator.alloc<NodeBinExprGrEq>();
                expr_l2->var = expr_l->var; //huh cloning :)
                equiv->l = expr_l2;
                equiv->r = expr_r.value();
                expr->var = equiv;
            } else if (oper.type == TokenType::lesser && oper2.has_value() && oper2.value().type == TokenType::eq) {
                auto equiv = m_allocator.alloc<NodeBinExprLsEq>();
                expr_l2->var = expr_l->var; //huh cloning :)
                equiv->l = expr_l2;
                equiv->r = expr_r.value();
                  expr->var = equiv;
            } else if (oper.type == TokenType::greater) {
                auto equiv = m_allocator.alloc<NodeBinExprGr>();
                expr_l2->var = expr_l->var; //huh cloning :)
                equiv->l = expr_l2;
                equiv->r = expr_r.value();
                expr->var = equiv;
            } else if (oper.type == TokenType::lesser) {
                auto equiv = m_allocator.alloc<NodeBinExprLs>();
                expr_l2->var = expr_l->var; //huh cloning :)
                equiv->l = expr_l2;
                equiv->r = expr_r.value();
                expr->var = equiv;
            } else {
                assert(false); //Unreached
            }
            expr_l->var = expr;
        }

        return expr_l;
    }

    std::optional<NodeScope*> parse_scope() {
        if (!try_cons(TokenType::open_c).has_value()) {
            return {};
        }
        auto scope = m_allocator.alloc<NodeScope>();
        while (auto stmt = parse_stmt()) {
            scope->stmts.push_back(stmt.value());
        }
        try_cons(TokenType::close_c, "Expected }");
        return scope;
    }

    std::optional<NodeIfPred*> parse_if_pred() {
        if (try_cons(TokenType::wif)) {
            try_cons(TokenType::open_p, "Expected '('");
            auto wif = m_allocator.alloc<NodeIfPredWif>();
            if (auto expr = parse_expr()) {
                wif->expr = expr.value();
            } else {
                std::cerr << "Expected expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_cons(TokenType::close_p, "Expected ')'");
            if (auto scope = parse_scope()) {
                wif->scope = scope.value();
            } else {
                std::cerr << "Expected scope" << std::endl;
                exit(EXIT_FAILURE);
            }
            wif->pred = parse_if_pred();
            auto pred = m_allocator.alloc<NodeIfPred>();
            pred->var = wif;
            return pred;
        }
        if (try_cons(TokenType::_else)) {
            auto else_ = m_allocator.alloc<NodeIfPredElse>();
            if (auto scope = parse_scope()) {
                else_->scope = scope.value();
            } else {
                std::cerr << "Expected scope" << std::endl;
                exit(EXIT_FAILURE);
            }
            auto pred = m_allocator.alloc<NodeIfPred>();
            pred->var = else_;
            return pred;
        }
        return {};
    }

    std::optional<NodeParamsDec*> parse_func_params_dec() {
        if (peek().is_value && peek().value.type != TokenType::close_p){
            auto params = m_allocator.alloc<NodeParamsDec>();
            while (peek().is_value && peek().value.type != TokenType::close_p) {
                params->params.push_back(try_cons(TokenType::ident, "Expected identifier!"));
                if (peek().is_value && peek().value.type != TokenType::close_p) {
                    try_cons(TokenType::comma, "Expected ',' for parameter declaration!");
                }
            }
            return params;
        }
        return {};
    }

    std::optional<NodeStmt*> parse_stmt() {
        if (peek().value.type == TokenType::_return && peek(1).is_value && peek(1).value.type == TokenType::open_p) {
            consume();
            consume();
            auto stmt_ret = m_allocator.alloc<NodeStmtExit>();
            if (auto node_expr = parse_expr()) {
                stmt_ret->expr =  node_expr.value();
            } else {
                std::cerr << "Invalid Expression!" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_cons(TokenType::close_p, "Expected ')'");
            try_cons(TokenType::semi, "Expected ';'");
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_ret;
            return stmt;
        } else if (peek().value.type == TokenType::give && peek(1).is_value && peek(1).value.type == TokenType::open_p) {
            consume();
            consume();
            auto stmt_ret = m_allocator.alloc<NodeStmtGive>();
            if (auto node_expr = parse_expr()) {
                stmt_ret->expr =  node_expr.value();
            } else {
                std::cerr << "Invalid Expression!" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_cons(TokenType::close_p, "Expected ')'");
            try_cons(TokenType::semi, "Expected ';'");
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_ret;
            return stmt;
        } else if (peek().is_value && peek().value.type == TokenType::ident && peek(1).is_value &&
            peek(1).value.type == TokenType::eq) {
            auto assign = m_allocator.alloc<NodeStmtAssign>();
            assign->ident = consume();
            consume(); //Rid eq sign//
            if (auto expr = parse_expr()) {
                assign->expr = expr.value();
            } else {
                std::cerr << "Expected expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_cons(TokenType::semi, "Expected ';'");
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = assign;
            return stmt;
        } else if (peek().is_value && peek().value.type == TokenType::func && //function dec
            peek(1).is_value && peek(1).value.type == TokenType::ident &&
            peek(2).is_value && peek(2).value.type == TokenType::open_p) {
            consume(); //rid of func keyword
            auto stmt_func = m_allocator.alloc<NodeStmtFunc>();
            stmt_func->ident = consume();
            consume(); //rid of open_p
            if (auto param = parse_func_params_dec()) { //optional params
                stmt_func->params = param.value();
            }
            try_cons(TokenType::close_p, "Expected ')'"); //rid of close+p
            if (auto scope = parse_scope()) {
                stmt_func->scope = scope.value();
            } else {
                std::cerr << "Invalid scope" << std::endl;
                exit(EXIT_FAILURE);
            }
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_func;
            return stmt;
        } else if (peek().is_value && peek().value.type == TokenType::ident && //function call
            peek(1).is_value && peek(1).value.type == TokenType::open_p) {
            auto stmt_func = m_allocator.alloc<NodeStmtFuncCall>();
            stmt_func->ident = consume();
            consume(); //rid of open_p
            if (auto param = parse_func_params_in()) { //optional params
                stmt_func->params = param.value();
            }
            try_cons(TokenType::close_p, "Expected ')'"); //rid of close+p
            try_cons(TokenType::semi, "Expected ';'"); //rid of semi-colon
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_func;
            return stmt;
        } else if (peek().is_value && peek().value.type == TokenType::var &&
            peek(1).is_value && peek(1).value.type == TokenType::ident &&
            peek(2).is_value && peek(2).value.type == TokenType::eq) {
            consume(); //rid of var
            auto stmt_var = m_allocator.alloc<NodeStmtVar>();
            stmt_var->ident = consume();
            consume(); //rid of eq
            if (auto expr = parse_expr()) {
                stmt_var->expr = expr.value();
            } else {
                std::cerr << "Invalid Expression" <<std::endl;
                exit(EXIT_FAILURE);
            }
            try_cons(TokenType::semi, "Expected ;");
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_var;
            return stmt;
        } else if (peek().is_value && peek().value.type == TokenType::open_c) {
            if (auto scope = parse_scope()) {
                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->var = scope.value();
                return stmt;
            } else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
        } else if (auto _if = try_cons(TokenType::_if)) {
            try_cons(TokenType::open_p, "Expected '('");
            auto stmt_if = m_allocator.alloc<NodeStmtIf>();
            if (auto expr = parse_expr()) {
                stmt_if->expr = expr.value();
            } else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_cons(TokenType::close_p, "Expected ')'");
            if (auto scope = parse_scope()) {
                stmt_if->scope = scope.value();
            } else {
                std::cerr << "Invalid scope" << std::endl;
                exit(EXIT_FAILURE);
            }
            stmt_if->pred = parse_if_pred();

            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_if;
            return stmt;
        } else if (auto _while = try_cons(TokenType::_while)) {
            try_cons(TokenType::open_p, "Expected '('");
            auto stmt_while = m_allocator.alloc<NodeStmtWhile>();
            if (auto expr = parse_expr()) {
                stmt_while->expr = expr.value();
            } else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_cons(TokenType::close_p, "Expected ')'");
            if (auto scope = parse_scope()) {
                stmt_while->scope = scope.value();
            } else {
                std::cerr << "Invalid scope" << std::endl;
                exit(EXIT_FAILURE);
            }

            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_while;
            return stmt;
        } else {
            return {};
        }
    }

    std::optional<NodeProg> parse_prog() {
        NodeProg prog;
        while (peek().is_value) {
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(stmt.value());
            } else {
                std::cerr << "Invalid Statements" <<std::endl;
                exit(EXIT_FAILURE);
            }
        }
        return prog;
    }

private:
    struct val {

        Token value;
        bool is_value;
        
    };

    val peek(int count = 0) const {
        // Peaks ahead to current + count character in src //
        val retVal;
        
        if (m_cur_index + count >= m_tokens.size()) {
            retVal.is_value = false;
            return retVal;
        } else {
            retVal.value = m_tokens.at(m_cur_index + count);
            retVal.is_value = true;
            return retVal;
        }
    };

    inline Token consume() {
        return m_tokens.at(m_cur_index++);
    };

    std::optional<Token> try_cons(TokenType type) {
        if (peek().is_value && peek().value.type == type) {
            return consume();
        } else {
            return {};
        }
    };

    Token try_cons(TokenType type, const std::string& err_msg) {
        if (peek().is_value && peek().value.type == type) {
            return consume();
        } else {
            std::cerr << err_msg << std::endl;
            exit(EXIT_FAILURE);
        }
    };

    const std::vector<Token> m_tokens;
    int m_cur_index = 0;
    ArenaAllocator m_allocator;
};