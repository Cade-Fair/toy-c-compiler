//Author: Cade Fair


#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <map>
#include <stdexcept>
#include <iterator>
#include <memory>
#include <utility>


using namespace std;

// Tokenizer: turn characters into tokens
// k = token kinds
/*
 * File ends, identifier, integer, string
 * Keywords
 * Punctuation
 * Operators single char
 * Operators double char
 */
enum class K {
    End, Ident, Num, Str,
    KInt, KIf, KElse, KWhile, KFor, KReturn,
    Lparen, Rparen, Lbrace, Rbrace, Comma, Semi,
    Assign, Plus, Minus, Star, Slash, Percent,
    EqEq, Ne, Lt, Le, Gt, Ge
};

// Token struct: one token -> kind, text, position
struct Tok {
    K k;
    string lex;
    int line, col;
};

// Lex struct: handle reading characters from source code, track position, and will skip comments and whitespace
struct Lex {
    string s;
    size_t i = 0;
    int line = 1, col = 1;
    Lex(string src): s(move(src)) {}
    char peek(int k = 0) const {
        size_t j = i + k; return j<s.size() ? s[j] : '\0';
    }
    char get() {
        if (i >= s.size()) {
            return '\0';
        }
        char c = s[i++];
        if (c == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
        return c;
    }
    void skip() {
        for(;;) {
            while(isspace(peek())) {
                get();
            }
            if (peek() == '/' && peek(1) == '/'){
                while(peek() && peek() != '\n') {
                    get();
                }
                continue;
            }
            if (peek() == '/' && peek(1) == '*') {
                get(); get();
                while (peek() && !(peek() == '*' && peek(1) == '/')) {
                    get();
                }
                if (peek()) {
                    get(); get();
                }
                continue;
            }
            break;
        }
    }
    Tok make(K k, string lx, int l, int c) {
        return Tok{k,lx,l,c};
    }
    // Main token loop
    vector <Tok> run(){
        vector<Tok> v;
        while(true) {
            skip();
            int l = line, c = col;
            char ch = peek();
            if (!ch) {
                v.push_back(make(K::End,"", l, c));
                break;
            }
            if (isalpha(ch) || ch == '_'){   // Identifier
                string id;
                while (isalnum(peek()) || peek() == '_') {
                    id.push_back(get());
                }
                K k = K::Ident;
                // map certain words to their keyword
                if (id == "int") {
                    k = K::KInt;
                } else if (id == "if"){
                    k = K::KIf;
                } else if (id == "else") {
                    k = K::KElse;
                } else if (id == "while"){
                    k = K::KWhile;
                } else if (id == "for") {
                    k = K::KFor;
                } else if (id == "return") {
                    k = K::KReturn;
                }
                v.push_back(make(k,id,l,c));
                continue;
            }
            if (isdigit(ch)){    // number literal
                string n;
                while (isdigit(peek())) {
                    n.push_back(get());
                }
                v.push_back(make(K::Num, n, l,c));
                continue;
            }
            if (ch == '"') { // check for string literal
                get();
                string t;
                while(peek() && peek() != '"') {
                    if (peek() == '\\') {
                        get();
                        char e = get();
                        if (e == 'n') {
                            t.push_back('\n');
                        } else {
                            t.push_back(e);
                        }
                    } else {
                        t.push_back(get());
                    }
                }
                if (peek()!='"') throw runtime_error("unterminated String");
                get(); // collects closing quote
                v.push_back(make(K::Str,t,l,c));
                continue;
            }

            // Check two char operators
            string two;
            two += ch;
            two += peek(1);
            if (two == "==") {
                get(); get();
                v.push_back(make(K::EqEq,"==",l,c));
                continue;
            }
            if (two == "!=") {
                get(); get();
                v.push_back(make(K::Ne, "!=", l,c));
                continue;
            }
            if (two == "<=") {
                get(); get();
                v.push_back(make(K::Le,"<=", l,c));
                continue;
            }
            if (two == ">=") {
                get(); get();
                v.push_back(make(K::Ge,">=", l,c));
                continue;
            }
            // Single char cases
            switch(ch){
                case '(': get(); v.push_back(make(K::Lparen, "(", l,c)); break;
                case ')': get(); v.push_back(make(K::Rparen, ")", l,c)); break;
                case '{': get(); v.push_back(make(K::Lbrace, "{", l,c)); break;
                case '}': get(); v.push_back(make(K::Rbrace, "}", l,c)); break;
                case ',': get(); v.push_back(make(K::Comma, ",", l,c)); break;
                case ';': get(); v.push_back(make(K::Semi, ";", l,c)); break;
                case '=': get(); v.push_back(make(K::Assign, "=", l,c)); break;
                case '+': get(); v.push_back(make(K::Plus, "+", l,c)); break;
                case '-': get(); v.push_back(make(K::Minus, "-", l,c)); break;
                case '*': get(); v.push_back(make(K::Star, "*", l,c)); break;
                case '/': get(); v.push_back(make(K::Slash, "/", l,c)); break;
                case '%': get(); v.push_back(make(K::Percent, "%", l,c)); break;
                case '<': get(); v.push_back(make(K::Lt, "<", l,c)); break;
                case '>': get(); v.push_back(make(K::Gt, ">", l,c)); break;
                default: throw runtime_error("unknown character");
            }
        }
        return v;
    }

};

// Build an AST
struct Expr{ virtual ~Expr() = default; };
struct Num : Expr { int v; explicit Num(int x): v(x){} };
struct Str : Expr { string v; explicit Str(string s): v(move(s)){} };
struct Var : Expr { string n; explicit Var(string s): n(move(s)){} };

enum class B {Add, Sub, Mul, Div, Mod, Eq, Ne, Lt, Le, Gt, Ge}; // binary ops
enum class U {Pos, Neg}; // unary ops
struct Unary : Expr { U op; unique_ptr<Expr> x;
    Unary(U o, unique_ptr<Expr> a) : op(o), x(move(a)){} };
struct Bin : Expr { B op; unique_ptr<Expr> l,r;
    Bin(unique_ptr<Expr> L, B o, unique_ptr<Expr> R) : op(o), l(move(L)), r(move(R)) {} };
struct Call : Expr { string name; vector<unique_ptr<Expr>> args;
    Call(string s, vector<unique_ptr<Expr>> a) : name(move(s)), args(move(a)){} };
struct Assign: Expr { string name; unique_ptr<Expr> val;
    Assign( string n, unique_ptr<Expr> v) : name(move(n)), val(move(v)){} };

struct Stmt { virtual ~Stmt() = default;}; // base of all statements
struct Block : Stmt { vector<unique_ptr<Stmt>> ss; };
struct Decl : Stmt { struct Item { string name; unique_ptr<Expr> init; }; vector<Item> items; };
struct ExprS : Stmt { unique_ptr<Expr> e;
    explicit ExprS(unique_ptr<Expr> x) : e(move(x)){} }; // expression
struct If : Stmt { unique_ptr<Expr> c; unique_ptr<Stmt> t,e;
    If(unique_ptr<Expr> c_, unique_ptr<Stmt>t_, unique_ptr<Stmt>e_) : c(move(c_)), t(move(t_)), e(move(e_)){} };
struct While : Stmt { unique_ptr<Expr> c; unique_ptr<Stmt> b;
    While(unique_ptr<Expr> c_, unique_ptr<Stmt> b_): c(move(c_)), b(move(b_)){} };
struct For : Stmt { unique_ptr<Expr> i,c,s; unique_ptr<Stmt> b;
    For(unique_ptr<Expr> I, unique_ptr<Expr> C, unique_ptr<Expr> S, unique_ptr<Stmt> B)
    : i(move(I)), c(move(C)), s(move(S)), b(move(B)){} };
struct Ret : Stmt { unique_ptr<Expr> v;
    explicit Ret(unique_ptr<Expr> x) : v(move(x)){} };

// Parse through

struct P {
    vector<Tok> t; size_t p=0;
    P(vector<Tok> toks) : t(move(toks)) {}
    Tok& peek(int k=0) {
        size_t j = p + k;
        if(j >= t.size()) {
            return t.back();
        }
        return t[j]; }
    bool match(K k) {
        if(peek().k == k) {
        p++;
        return true;
    }
    return false; }
    Tok& expect(K k, const char* msg) {
        if(peek().k!=k) throw runtime_error(msg);
        return t[p++];
    }

    unique_ptr<Block> block() {
        expect(K::Lbrace, "{");
        auto b = make_unique<Block>();
        while (peek().k != K::Rbrace) b->ss.push_back(stmt());
        expect(K::Rbrace, "}");
        return b;
    }

    unique_ptr<Stmt> stmt() {
        if(peek().k==K::KInt) return decl();
        if(peek().k == K::KIf) return ifs();
        if(peek().k == K::KWhile) return whiles();
        if(peek().k == K::KFor) return fors();
        if(peek().k == K::KReturn) {
            expect(K::KReturn, "return");
            unique_ptr<Expr> e; if (peek().k!=K::Semi) e = expr();
            expect(K::Semi, ";");
            return make_unique<Ret>(move(e));
        }
        if(peek().k == K::Lbrace) return block();
        auto e = expr();
        expect(K::Semi, ";");
        return make_unique<ExprS>(move(e));
    }
    unique_ptr<Stmt> decl(){
        expect(K::KInt, "int");
        auto d = make_unique<Decl>();
        while(true) {
            auto id = expect(K::Ident,"id").lex;
            unique_ptr<Expr> init;
            if(match(K::Assign)) init = expr();
            d->items.push_back({id,move(init)});
            if(match(K::Comma)) continue;
            break;
        }
        expect(K::Semi, ";");
        return d;
    }

    unique_ptr<Stmt> ifs() {
        expect(K::KIf, "if");
        expect(K::Lparen, "(");
        auto c = expr();
        expect(K::Rparen, ")");
        auto tstmt = stmt();
        unique_ptr<Stmt> estmt;
        if(match(K::KElse)) estmt = stmt();
        return make_unique<If>(move(c), move(tstmt), move(estmt));
    }

    unique_ptr<Stmt> whiles(){
        expect(K::KWhile,"while");
        expect(K::Lparen, "(");
        auto c = expr();
        expect(K::Rparen, ")");
        auto b = stmt();
        return make_unique<While>(move(c), move(b));
    }

    unique_ptr<Stmt> fors () {
        expect(K::KFor, "for");
        expect(K::Lparen, "(");
        unique_ptr<Expr> I,C,S;
        if(peek().k != K::Semi) I = expr(); expect(K::Semi, ";");
        if(peek().k != K::Semi) C = expr(); expect(K::Semi, ";");
        if(peek().k != K::Rparen) S = expr(); expect(K::Rparen, ")");
        auto b = stmt();
        return make_unique<For>(move(I), move(C), move(S), move(b));
    }

    //Expression entry point
    unique_ptr<Expr> expr() {
        return assign();
    }
    unique_ptr<Expr> assign(){
        auto L = cmp();
        if (auto v = dynamic_cast<Var*>(L.get()); v && peek().k == K::Assign) {
            expect (K::Assign, "=");
            auto R = assign();
            return make_unique<Assign>(v->n, move(R));
        }
        return L;
    }

    unique_ptr<Expr> cmp() {
        auto e = add();
        while(peek().k == K::EqEq || peek().k == K::Ne || peek().k == K::Lt || peek().k == K::Le || peek().k == K::Gt || peek().k == K::Ge){
            K k = peek().k; p++; auto r = add();
            B op = B::Eq; // map token to enum
            if(k == K::Ne) {
                op = B::Ne;
            } else if (k == K::Lt){
                op = B::Lt;
            } else if (k == K::Le) {
                op = B::Le;
            } else if (k == K::Gt){
                op = B::Gt;
            } else if (k == K::Ge) {
                op = B::Ge;
            }
            e = make_unique<Bin>(move(e), op, move(r)); // build Bin node
        }
        return e;
    }

    // Add -> mul
    unique_ptr<Expr> add(){
        auto e = mul();
        while(peek().k == K::Plus || peek().k == K::Minus){
            K k = peek().k; p++; auto r = mul();
            e = make_unique<Bin>(move(e), k == K::Plus ? B::Add : B::Sub, move(r));
        }
        return e;
    }

    // Mul -> uni
    unique_ptr<Expr> mul() {
        auto e = uni();
        while(peek().k == K::Star || peek().k == K::Slash || peek().k == K::Percent){
            K k = peek().k; p++; auto r = uni();
            B op = (k == K::Star ? B::Mul : (k == K::Slash ? B::Div : B::Mod));
            e = make_unique<Bin>(move(e), op, move(r));
        }
        return e;
    }

    // Uni (+|-)
    unique_ptr<Expr> uni() {
        if(peek().k == K::Plus || peek().k == K::Minus) {
            K k = peek().k; p++;
            return make_unique<Unary>(k == K::Plus ? U::Pos : U::Neg, uni());
        }
        return primary();
    }

    // primary -> number | string | ident| expr
    unique_ptr<Expr> primary(){
        if(peek().k == K::Num) {
            int v = stoi(expect(K::Num, "num").lex);
            return make_unique<Num>(v);
        }
        if(peek().k == K::Str) {
            string s = expect(K::Str, "str").lex;
            return make_unique<Str>(s);
        }
        if(peek().k == K::Ident) {
            string id = expect(K::Ident, "id").lex;
            if(peek().k == K::Lparen){
                p++; vector<unique_ptr<Expr>> args;
                if(peek().k != K::Rparen) {
                    args.push_back(expr());
                    while(peek().k == K::Comma) {
                        p++;
                        args.push_back(expr());
                    }
                }
                expect(K::Rparen,")");
                return make_unique<Call>(id, move(args));
            }
            return make_unique<Var>(id);
        }
        if(peek().k == K::Lparen) {
            p++;
            auto e = expr();
            expect(K::Rparen, ")");
            return e;
        }
        throw runtime_error("Bad expression");
    }
};

// Symbol table to assign each local variable with stack slot

struct Syms{
    map< string, int> off;        // name -> negative offset
    int next = -12;             // start below $fp
    int add(const string& n) {
        int o = next;
        next -=4;
        off[n] = o;
        return o;
    }
    int get(const string& n) {
        return off.at(n);         // lookup offset
    }
    int frame() const {
        return 256;             // Fixed frame
    }

};

// Emitter to collect .data and .text assembly
struct E {
    string data, text;          // separate sections
    int lid = 0;                // label counter
    string L(const string& p) {
        return p + to_string(lid++); // Fresh label names
    }
    void D(const string& s) {
        data += s + "\n";
    }
    void T(const string& s) {
        text += s + "\n";
    }
    string dump() {
        return (data.size() ? ".data\n" + data : "") + "\n.text\n.globl main\n" + text;
    }

};

// Codegen will walk the AST and emits MIPS

struct CG {
    Syms& S;
    E& Emitter;
    vector<string> pool{"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6","$t7"}; // temp registers
    string get() { if(pool.empty()) throw runtime_error("out of temps"); string r = pool.back(); pool.pop_back(); return r; }
    void put(string r) {
        pool.push_back(r);
    }
    static string escape(const string& in) {
        string o;
        for(char c: in) {
            if(c == '\\' || c =='"') {
                o.push_back('\\');
                o.push_back(c);
            } else if (c == '\n') {
                o += "\\n";
            } else {
                o.push_back(c);
            }
        }
        return o;
    }
    string g(Expr* e){
        if(auto n=dynamic_cast<Num*>(e)) {
            string r = get(); Emitter.T("li " + r + ", " + to_string(n->v));
            return r;
        }
        if(auto s = dynamic_cast<Str*>(e)) {
            string r = get();
            static map<string, string> lab;
            if (!lab.count(s->v)) {
                string L = "str_" + to_string((int) lab.size());
                lab[s->v] = L;
                Emitter.D(L + ": .asciiz \"" + escape(s->v) + "\"");
                }
                Emitter.T("la " + r + ", " + lab[s->v]);
                return r;
        }
        if(auto v = dynamic_cast<Var*>(e)) {
            string r = get();
            int o = S.get(v->n);
            Emitter.T("lw " + r + ", " + to_string(o) + "($fp)");
            return r;
        }
        if(auto u = dynamic_cast<Unary*>(e)) {
            string x = g(u->x.get());
            if(u-> op == U::Neg) {
                Emitter.T("sub " + x + ", $zero, " + x);
            }
            return x;
        }
        if(auto a = dynamic_cast<Assign*>(e)) {
            string r = g(a-> val.get());
            int o = S.get(a-> name);
            Emitter.T("sw " + r + ", " + to_string(o) + "($fp)");
            return r;
        }
        if(auto b = dynamic_cast<Bin*>(e)) {
            string Lr = g(b->l.get());
            string Rr = g(b->r.get());
            switch (b->op) {
                case B::Add: Emitter.T("add " + Lr + ", " + Lr + ", " + Rr); break;
                case B::Sub: Emitter.T("sub " + Lr + ", " + Lr + ", " + Rr); break;
                case B::Mul: Emitter.T("mul " + Lr + ", " + Lr + ", " + Rr); break;
                case B::Div: Emitter.T("div " + Lr + ", " + Rr); Emitter.T("mflo " + Lr); break;
                case B::Mod: Emitter.T("div " + Lr + ", " + Rr); Emitter.T("mfhi " + Lr); break;
                case B::Lt: Emitter.T("slt " + Lr + ", " + Lr + ", " + Rr); break;
                case B::Gt: Emitter.T("slt " + Lr + ", " + Rr + ", " + Lr); break;
                case B::Le: { string t = get(); Emitter.T("slt " + t + ", " + Rr + ", " + Lr);
                    Emitter.T("xori " + t + ", " + t + ", 1");
                    put(Lr);
                    Lr = t;
                } break;
                case B::Ge: { string t = get(); Emitter.T("slt " + t + ", " + Lr + ", " + Rr);
                    Emitter.T("xori " + t + ", " + t + ", 1");
                    put(Lr);
                    Lr = t;
                } break;
                case B::Eq: Emitter.T("sub " + Lr + ", " + Lr + ", " + Rr); Emitter.T("sltiu " + Lr + ", " + Lr + ", 1"); break;
                case B::Ne: Emitter.T("sub " + Lr + ", " + Lr + ", " + Rr); Emitter.T("sltu " + Lr + ", $zero, " + Lr); break;
            }
            put(Rr);
            return Lr;
        }
        if(auto c = dynamic_cast<Call*>(e)) {
            if(c->name == "print_int" && c-> args.size() == 1) {
                string r = g(c->args[0].get());
                Emitter.T("move $a0, " + r);
                Emitter.T("jal print_int");
                Emitter.T("nop");
                put(r);
                string z=get();
                Emitter.T("move " + z + ", $zero");
                return z;
            }
            if(c->name == "print_str" && c-> args.size() == 1) {
                string r = g(c->args[0].get());
                Emitter.T("move $a0, " + r);
                Emitter.T("jal print_str");
                Emitter.T("nop");
                put(r);
                string z=get();
                Emitter.T("move " + z + ", $zero");
                return z;
            }
        }
        throw runtime_error("unknown expr");
    }

    void s(Stmt* st) {          // generate code for a Stmt
        if(auto d = dynamic_cast<Decl*>(st)) {
            for(auto& it: d-> items) {
                if(!S.off.count(it.name)) {
                    S.add((it.name));
                }
                if(it.init) {
                    string r = g(it.init.get());
                    Emitter.T("sw " + r + ", " + to_string(S.get(it.name))+ "($fp)");
                    put(r);
                }
            }
            return;
        }
        if(auto e = dynamic_cast<ExprS*>(st)) {
            string r = g(e-> e.get());
            put(r);
            return;
        }
        if(auto r = dynamic_cast<Ret*>(st)) {
            if(r->v) {
                string v = g(r->v.get());
                Emitter.T("move $v0, " + v);
                put(v);
            } else {
                Emitter.T("move $v0, $zero");
            }
            Emitter.T("j __epilogue");
            Emitter.T("nop");
            return;
        }
        if(auto b = dynamic_cast<Block*>(st)) {
            for(auto& ss: b->ss) {
                s(ss.get());
            }
            return;
        }
        if(auto iff = dynamic_cast<If*>(st)){
            string cnd = g(iff->c.get());
            string Lelse = Emitter.L("Lelse_"), Lend = Emitter.L("Lend_");
            Emitter.T("beq " + cnd + ", $zero, " + Lelse);
            Emitter.T("nop");
            put(cnd);
            s(iff->t.get());
            Emitter.T("j " + Lend);
            Emitter.T("nop");
            Emitter.T(Lelse + ":");
            if(iff->e) {
                s(iff->e.get());
            }
            Emitter.T(Lend + ":");
            return;
        }
        if(auto w = dynamic_cast<While*>(st)) {
            string Lc = Emitter.L("Lcond_"), Le = Emitter.L("Lend_");
            Emitter.T(Lc+":");
            string cnd = g(w->c.get());
            Emitter.T("beq " + cnd + ", $zero, " + Le);
            Emitter.T("nop");
            put(cnd);
            s(w->b.get());
            Emitter.T("j " + Lc);
            Emitter.T("nop");
            Emitter.T(Le + ":");
            return;
        }
        if(auto f = dynamic_cast<For*>(st)) {
            if(f->i) {
                string r = g(f->i.get());
                put(r);
            }
            string Lc = Emitter.L("Lfcond_"), Le = Emitter.L("Lfend_");
            Emitter.T(Lc + ":");
            if(f->c) {
                string cnd = g(f->c.get());
                Emitter.T("beq " + cnd + ", $zero, " + Le);
                Emitter.T("nop");
                put(cnd);
            }
            s(f->b.get());
            if(f->s) {
                string r = g(f->s.get());
                put(r);
            }
            Emitter.T("j " + Lc);
            Emitter.T("nop");
            Emitter.T(Le + ":");
            return;
        }

        throw runtime_error("stmt?");
    }

};

// Main writes the .asm

int main(int argc, char** argv) {
    if(argc < 4) {
        cerr<<"usage: toycc input.c -o output.asm\n";
        return 1;
    }
    string in = argv[1], out;
    for(int i = 2; i < argc; i++) {
        if(string(argv[i]) == "-o" && i+1 < argc) {
            out = argv[++i];
        }
    }
    if(out.empty()) {
        cerr<<"-o required\n";
        return 1;
    }

    ifstream ifs(in); if(!ifs){ cerr<<"Cannot open input\n"; return 1;}
    string src((istreambuf_iterator<char>(ifs)), {});

    try {
        Lex L(src);
        auto toks = L.run();

        // parse only: int main()
        size_t p = 0;
        auto expect = [&](K k, const char *m) {
            if (toks[p].k != k) throw runtime_error(m);
            p++;
        };
        expect(K::KInt, "expected int");
        if (toks[p].k != K::Ident || toks[p].lex != "main") throw runtime_error("only int main() supported");
        p++;
        expect(K::Lparen, "(");
        expect(K::Rparen, ")");

        // Hand the block to full parser
        vector <Tok> blockTokens(toks.begin() + p, toks.end());
        P parser(blockTokens);
        auto root = parser.block();        // parse tokens -> ast

        Syms S;
        E E;
        CG cg{S, E};

        int FRAME = S.frame();   // fixed from earlier
        E.T("main:");
        E.T("addi $sp,$sp,-" + to_string(FRAME));
        E.T("sw $ra," + to_string(FRAME - 4) + "($sp)");
        E.T("sw $fp," + to_string(FRAME - 8) + "($sp)");
        E.T("addi $fp,$sp," + to_string(FRAME));

        for (auto &st: root->ss) cg.s(st.get());

        E.T("move $v0,$zero");      //default return

        E.T("__epilogue:");
        E.T("lw $ra," + to_string(FRAME - 4) + "($sp)");
        E.T("lw $fp," + to_string(FRAME - 8) + "($sp)");
        E.T("addi $sp,$sp," + to_string(FRAME));
        E.T("jr $ra");
        E.T("nop");

        ofstream ofs(out);
        ofs << E.dump();
        cout << "wrote " << out << "\n";
    } catch (const exception& e) {
        cerr<<"Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
