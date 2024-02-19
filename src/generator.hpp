#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <experimental/optional>
#include <map>
#include <algorithm>
#include <assert.h>
//#include "./tokenization.hpp"
//#include "./parser.hpp"

class Generator {
public:
    explicit Generator(NodeProg prog) : m_prog(std::move(prog)) {
    }

    void gen_term(const NodeTerm* term) {
        struct TermVisitor {
            Generator* gen;
            void operator()(const NodeTermIntLit* term_int_lit) const {
                gen->m_out << "    mov rax, " <<term_int_lit->int_lit.value.value() << "\n";
                gen->push("rax"); //ontop of stack
            }
            void operator()(const NodeTermIdent* term_ident) const {
                //Extracting Variable Contents (get from top of stack)
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var& var){
                    return var.name == term_ident->ident.value.value();
                });
                if (it == gen->m_vars.cend()) {
                    std::cerr << "Undeclared identitfier: " << term_ident->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                //%rsp is the guy// //(*it) is the variable :)//
                std::stringstream offset;
                offset << "QWORD [rsp + " << (gen->m_stack_size - (*it).stack_location - 1) * 8 << "]";
                gen->push(offset.str());
            }
            void operator()(const NodeTermFunc* term_func) const {
                //Extracting Variable Contents (get from top of stack)
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var& var){
                    return var.name == term_func->ident.value.value();
                });
                if (it == gen->m_vars.cend()) {
                    std::cerr << "Undeclared identitfier/function: " << term_func->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                if (term_func->params.has_value()) {
                    auto param = term_func->params.value()->params;
                    //std::reverse(param.begin(), param.end());
                    for (const NodeExpr* expr : param){
                        gen->gen_expr(expr);
                    }
                }
                if (it->param_num.has_value()) {
                    if (!(term_func->params.has_value()) || (term_func->params.value()->params.size() != it->param_num.value())){
                        std::cerr << "Inncorrect num of parameters given for func call, " << it->param_num.value() << " required!" << std::endl;
                        exit(EXIT_FAILURE);
                    }
                }
                gen->m_out << "    call " << term_func->ident.value.value() <<"\n";
                gen->push("rax");
            }
            void operator()(const NodeTermParen* term_paren) const {
                gen->gen_expr(term_paren->expr);
            }
        };
        TermVisitor visitor({.gen = this});
        std::visit(visitor, term->var);
    }

    void gen_bin_expr(const NodeBinExpr* bin_expr) {
        struct BinVisitor {
            Generator * gen;

            void operator()(const NodeBinExprSub* sub_expr) {
                gen->gen_expr(sub_expr->r);
                gen->gen_expr(sub_expr->l);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_out << "    sub rax, rbx\n";
                gen->push("rax");
            }
            void operator()(const NodeBinExprAdd* add_expr) {
                gen->gen_expr(add_expr->r);
                gen->gen_expr(add_expr->l);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_out << "    add rax, rbx\n";
                gen->push("rax");
            }
            void operator()(const NodeBinExprMult* mult_expr) {
                gen->gen_expr(mult_expr->r);
                gen->gen_expr(mult_expr->l);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_out << "    mul rbx\n";
                gen->push("rax");
            }
            void operator()(const NodeBinExprDiv* div_expr) {
                gen->gen_expr(div_expr->r);
                gen->gen_expr(div_expr->l);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_out << "    div rbx\n";
                gen->push("rax");
            }
            void operator()(const NodeBinExprEq* eq_expr) { //returns 0 or 1
                //0 == false | 1 == true //
                gen->gen_expr(eq_expr->r);
                gen->gen_expr(eq_expr->l);
                gen->pop("rax"); //stores left
                gen->pop("rbx"); //stores right
                std::string lab = gen->gen_label();
                std::string end_lab = gen->gen_label();
                gen->m_out << "    cmp rax, rbx\n";
                gen->m_out << "    je " << lab << "\n"; //if equal
                gen->m_out << "    mov rax, 0\n"; //return 0 if not
                gen->m_out << "    jmp " << end_lab << "\n";
                gen->m_out << lab << ":\n";
                gen->m_out << "    mov rax, 1\n"; //return 1 if
                gen->m_out << end_lab << ":\n";
                gen->push("rax"); //put rax on the end
            }
            void operator()(const NodeBinExprGrEq* greq_expr) { //returns 0 or 1
                //0 == false | 1 == true //
                gen->gen_expr(greq_expr->r);
                gen->gen_expr(greq_expr->l);
                gen->pop("rax"); //stores left
                gen->pop("rbx"); //stores right
                std::string lab = gen->gen_label();
                std::string end_lab = gen->gen_label();
                gen->m_out << "    cmp rax, rbx\n";
                gen->m_out << "    jge " << lab << "\n"; //if equal
                gen->m_out << "    mov rax, 0\n"; //return 0 if not
                gen->m_out << "    jmp " << end_lab << "\n";
                gen->m_out << lab << ":\n";
                gen->m_out << "    mov rax, 1\n"; //return 1 if
                gen->m_out << end_lab << ":\n";
                gen->push("rax"); //put rax on the end
            }
            void operator()(const NodeBinExprLsEq* lseq_expr) { //returns 0 or 1
                //0 == false | 1 == true //
                gen->gen_expr(lseq_expr->r);
                gen->gen_expr(lseq_expr->l);
                gen->pop("rax"); //stores left
                gen->pop("rbx"); //stores right
                std::string lab = gen->gen_label();
                std::string end_lab = gen->gen_label();
                gen->m_out << "    cmp rax, rbx\n";
                gen->m_out << "    jle " << lab << "\n"; //if equal
                gen->m_out << "    mov rax, 0\n"; //return 0 if not
                gen->m_out << "    jmp " << end_lab << "\n";
                gen->m_out << lab << ":\n";
                gen->m_out << "    mov rax, 1\n"; //return 1 if
                gen->m_out << end_lab << ":\n";
                gen->push("rax"); //put rax on the end
            }
            void operator()(const NodeBinExprLs* ls_expr) { //returns 0 or 1
                //0 == false | 1 == true //
                gen->gen_expr(ls_expr->r);
                gen->gen_expr(ls_expr->l);
                gen->pop("rax"); //stores left
                gen->pop("rbx"); //stores right
                std::string lab = gen->gen_label();
                std::string end_lab = gen->gen_label();
                gen->m_out << "    cmp rax, rbx\n";
                gen->m_out << "    jl " << lab << "\n"; //if equal
                gen->m_out << "    mov rax, 0\n"; //return 0 if not
                gen->m_out << "    jmp " << end_lab << "\n";
                gen->m_out << lab << ":\n";
                gen->m_out << "    mov rax, 1\n"; //return 1 if
                gen->m_out << end_lab << ":\n";
                gen->push("rax"); //put rax on the end
            }
            void operator()(const NodeBinExprGr* gr_expr) { //returns 0 or 1
                //0 == false | 1 == true //
                gen->gen_expr(gr_expr->r);
                gen->gen_expr(gr_expr->l);
                gen->pop("rax"); //stores left
                gen->pop("rbx"); //stores right
                std::string lab = gen->gen_label();
                std::string end_lab = gen->gen_label();
                gen->m_out << "    cmp rax, rbx\n";
                gen->m_out << "    jg " << lab << "\n"; //if equal
                gen->m_out << "    mov rax, 0\n"; //return 0 if not
                gen->m_out << "    jmp " << end_lab << "\n";
                gen->m_out << lab << ":\n";
                gen->m_out << "    mov rax, 1\n"; //return 1 if
                gen->m_out << end_lab << ":\n";
                gen->push("rax"); //put rax on the end
            }
        };

        BinVisitor visitor{.gen=this};
        std::visit(visitor, bin_expr->var);
    }

    void gen_expr(const NodeExpr* expr) {
        struct ExprVisitor {
            Generator* gen;

            void operator()(const NodeTerm* term) {
                gen->gen_term(term);
            }
            void operator()(const NodeBinExpr* bin_expr) const {
                //Get both sides of bin expression THEN expression
                gen->gen_bin_expr(bin_expr);
            }
        };

        ExprVisitor visitor{.gen=this};
        std::visit(visitor, expr->var);
    }

    void gen_scope(const NodeScope* scope) {
        st_scope();
        for (const NodeStmt* stmt : scope->stmts){
            gen_stmt(stmt);
        }

        ex_scope();
    }

    void gen_params_dec(const NodeParamsDec* param, const NodeScope* scope){
        //gen_scope but with local_var declarations//
        st_scope(param->params.size());
        for (Token ident : param->params) {
            auto it = std::find_if(m_vars.cbegin(), m_vars.cend(), [&](const Var& var){
                return var.name == ident.value.value();
            });
            if(it != m_vars.end()){ //equiv to .contains//
                std::cerr << "Ident already used: " << ident.value.value() << std::endl;
                exit(EXIT_FAILURE);
            }
            
            m_vars.push_back({.name = ident.value.value(), .stack_location = m_stack_size-1});
            m_stack_size++;
        }
        for (const NodeStmt* stmt : scope->stmts){
            gen_stmt(stmt);
        }

        ex_scope();
    }

    void gen_if_pred(const NodeIfPred* pred, const std::string& end_lab) {
        struct PredVisitor {
            Generator* gen;
            const std::string& end_lab;
            void operator()(const NodeIfPredWif* wif) {
                gen->gen_expr(wif->expr); //puts expr on top of stack
                gen->pop("rax");
                std::string lab = gen->gen_label();
                gen->m_out << "    test rax, rax\n";
                if (wif->pred.has_value()) { //If another wif or else
                    gen->m_out << "    jz " << lab << "\n"; //checks if 0
                } else { //if no else is found
                    gen->m_out << "    jz " << end_lab << "\n"; //checks if 0
                }
                gen->gen_scope(wif->scope);
                gen->m_out << "    jmp " << end_lab << "\n";
                if (wif->pred.has_value()){
                    gen->m_out << lab << ":\n";
                    gen->gen_if_pred(wif->pred.value(), end_lab);
                }
            }
            void operator()(const NodeIfPredElse* _else) {
                gen->gen_scope(_else->scope);
            }
        };

        PredVisitor visitor {.gen = this, .end_lab = end_lab};
        std::visit(visitor, pred->var);
    }

    void gen_stmt(const NodeStmt* stmt) {
        struct StmtVisitor {
            Generator* gen;
            void operator()(const NodeStmtExit* stmt_ret) {
                gen->gen_expr(stmt_ret->expr);
                gen->m_out << "    mov rax, 60\n";
                gen->pop("rdi");
                gen->m_out << "    syscall\n";
            }
            void operator()(const NodeStmtGive* stmt_give) {
                gen->gen_expr(stmt_give->expr);
                gen->pop("rax");
                gen->m_out << "    ret\n";
                //gen->ex_scope(); //should exit scope as well :)
            }
            void operator()(const NodeStmtVar* stmt_var) {
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var& var){
                    return var.name == stmt_var->ident.value.value();
                });
                if(it != gen->m_vars.end()){ //equiv to .contains//
                    std::cerr << "Ident already used: " << stmt_var->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen->m_vars.push_back({.name = stmt_var->ident.value.value(), .stack_location = gen->m_stack_size});
                gen->gen_expr(stmt_var->expr);
            }
            void operator()(const NodeStmtAssign* stmt_assign) {
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var& var){
                    return var.name == stmt_assign->ident.value.value();
                });
                if (it == gen->m_vars.end()) {
                    //Didn't find variable
                    std::cerr << "Identifier not declared: " << stmt_assign->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen->gen_expr(stmt_assign->expr); //top of stack expression
                gen->pop("rax");
                gen->m_out << "    mov [rsp +" << (gen->m_stack_size - it->stack_location - 1) * 8  << "], rax \n";
            }
            void operator()(const NodeScope* scope) {
                gen->gen_scope(scope);
            }
            void operator()(const NodeStmtIf* stmt_if) const {
                gen->gen_expr(stmt_if->expr); //puts expr on top of stack
                gen->pop("rax");
                std::string lab = gen->gen_label();
                gen->m_out << "    test rax, rax\n";
                gen->m_out << "    jz " << lab << "\n"; //checks if 0
                gen->gen_scope(stmt_if->scope);
                if (stmt_if->pred.has_value()) {
                    const std::string end_lab = gen->gen_label();
                    gen->m_out << "    jmp " << end_lab << "\n";
                    gen->m_out << lab << ":\n";
                    gen->gen_if_pred(stmt_if->pred.value(), end_lab);
                    gen->m_out << end_lab << ":\n";
                } else {
                    gen->m_out << lab<< ":\n";
                }
            } void operator()(const NodeStmtWhile* stmt_while) const {
                std::string start_lab = gen->gen_label();
                std::string end_lab = gen->gen_label();
                gen->m_out << start_lab << ":\n";
                gen->gen_expr(stmt_while->expr); //puts expr on top of stack
                gen->pop("rax");
                gen->m_out << "    test rax, rax\n";
                gen->m_out << "    jz " << end_lab << "\n"; //checks if 0
                gen->gen_scope(stmt_while->scope);
                gen->m_out << "    jmp " << start_lab << "\n";
                gen->m_out << end_lab << ":\n";
            } void operator()(const NodeStmtFunc* stmt_func) const {
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var& var){
                    return var.name == stmt_func->ident.value.value();
                });
                if (it != gen->m_vars.end()) {
                    //Found Var
                    std::cerr << "Identifier declared already!" << stmt_func->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                //checks for function name used alreday? ^^^
                //start of file function
                const std::string &temp = gen->m_out.str();
                gen->m_out.seekp(0);
                gen->m_out << stmt_func->ident.value.value() << ":\n";
                if (stmt_func->params.has_value()) {
                    gen->m_vars.push_back({.name = stmt_func->ident.value.value(), .stack_location = gen->m_stack_size,
                        .param_num = stmt_func->params.value()->params.size()});
                    //reserves function name ident ^^
                    gen->gen_params_dec(stmt_func->params.value(), stmt_func->scope);
                } else {
                    gen->gen_scope(stmt_func->scope);
                }
                gen->m_out << "    mov rax, 0 ;;return 0 at end\n"; //return 0 if nothing else ret :)
                gen->m_out << "    ret\n";
                gen->m_out << "\n";
                gen->m_out << temp;
            } void operator()(const NodeStmtFuncCall* stmt_func_call) const {
                auto it = std::find_if(gen->m_vars.cbegin(), gen->m_vars.cend(), [&](const Var& var){
                    return var.name == stmt_func_call->ident.value.value();
                });
                if (it == gen->m_vars.cend()) {
                    std::cerr << "Undeclared identitfier/function: " << stmt_func_call->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                if (stmt_func_call->params.has_value()) {
                    auto param = stmt_func_call->params.value()->params;
                    //std::reverse(param.begin(), param.end());
                    for (const NodeExpr* expr : param){
                        gen->gen_expr(expr);
                    }
                }
                if (it->param_num.has_value()) {
                    if (!(stmt_func_call->params.has_value()) || (stmt_func_call->params.value()->params.size() != it->param_num.value())){
                        std::cerr << "Inncorrect num of parameters given for func call, " << it->param_num.value() << " required!" << std::endl;
                        exit(EXIT_FAILURE);
                    }
                }
                gen->m_out << "    call " << stmt_func_call->ident.value.value() <<"\n";
            }
        };

        StmtVisitor visitor {.gen = this};
        std::visit(visitor, stmt->var);
    }

    std::string generate_prog() {
        m_out << "global _start:\n_start:\n";

        for (const NodeStmt* stmt : m_prog.stmts) {
            gen_stmt(stmt);
        }

        //Exits with 0 by default
        m_out << "    mov rax, 60\n";
        m_out << "    mov rdi, 0\n";
        m_out << "    syscall\n";
        return m_out.str();
    }

private:

    void push(const std::string& reg) { //Pushing onto the stack
        m_out << "    push " << reg << "\n";
        m_stack_size++;
    }

    void pop(const std::string& reg) { //Popping off the stack
        m_out << "    pop " << reg << "\n";
        m_stack_size--;
    }

    void st_scope(int param_num = 0) { //Start Scope
        m_scopes.push_back(m_vars.size()+param_num);
    }

    void ex_scope() { //End Scope
        //Pop off vars until get to last part of scope
        size_t pop_count = m_vars.size() - m_scopes.back();
        m_out << "    add rsp, " << pop_count * 8 << "\n"; //stack grows so popping is add instead of sub
        m_stack_size -= pop_count;
        for (int i = 0; i < pop_count; i++) {
            m_vars.pop_back();
        }
        m_scopes.pop_back();
    }

    std::string gen_label() { //if statement generator :)
        std::stringstream ss;
        ss << "label" << m_label_count++;
        return ss.str();
    }

    struct Var {
        std::string name;
        int stack_location;
        std::optional<int> param_num;
    };

    const NodeProg m_prog;
    std::stringstream m_out;
    int m_stack_size = 0;
    std::vector<size_t> m_scopes {};
    int m_label_count = 0;

    //NEED A VECTOR FOR THE VARIABLES KEEPING//
    std::vector<Var> m_vars;
};