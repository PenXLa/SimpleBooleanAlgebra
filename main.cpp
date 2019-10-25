#include<iostream>
#include<string>
#include<vector>
#include<sstream>
#include<stack>
#include<map>
#include<functional>
#include<set>
#include<cstring>
#include"cxxopts.hpp"
#include"exceptions.hpp"
#include"utils.hpp"

struct ele {
    std::string value;
    bool isVar;
    int pos;//在线性式中的位置，方便定位错误
};

struct node {
    ele content;
    std::vector<node*> children;
    node *parent=nullptr;
};

//用于解析中缀表达式时提供运算符的信息
//有了这个，就足够让表达式树正确建立
struct operator_info {
    std::string val;
    int ary, pri;//运算符元数 和 优先级（越大越优先）
    bool lf=1;//是否是左结合
    const std::string mj="";//mathjax里一些运算符需要转义符号
};



const operator_info opis[] = { // Ary Pri
                            {"||",2,70}, {"+",2,70},
                            {"!||",2,70}, {"!+",2,70},{"↓",2,70},
                            {"&&",2,90,true,"\\&\\&"}, {"*",2,90},
                            {"↑", 2, 90}, {"!&&", 2, 90}, {"!*", 2, 90},
                            {"!",1,100,false}, 
                            {"(",0,1000}, {")",0,1000}, 
                            {"->",2,60}, {"→",2,60}, 
                            {"!->",2,60}, {"!→",2,60},
                            /*{"⇄",2,50},*/{"<->",2,50}, {"==",2,50},
                            /*{"!⇄",2,50},*/{"!<->",2,50}, {"!=",2,50}, {"^",2,80}, 
                            };



//如果没有找到，throw运算符前缀与s相等的个数
operator_info getOpi(std::string s) {
    int n = sizeof(opis)/sizeof(operator_info);
    int presame = 0, slen = s.length();
    for (int i=0; i<n; ++i) {
        if (s == opis[i].val) return opis[i];
        else presame += (opis[i].val.length()>slen && opis[i].val.substr(0,slen)==s);
    }
    throw presame;
}


std::stringstream& operator >> (std::stringstream& in, ele& x) {
    char c;
    while(~(c = in.peek()) && c==' ') in.get();//吃掉前面空格
    if (in.peek()==-1) return in;
    x.isVar = isalnum(in.peek());
    x.value.clear();
    x.pos = in.tellg()+1;
    if (x.isVar) {
        while(c=in.peek(), ~c&&isalnum(c))
            x.value.push_back(in.get());
    } else {
        std::string tmp = "";
        bool isok = false;
        while(~(c = in.peek()) && c!=' ') {
            in.get();
            tmp.push_back(c);
            try{
                getOpi(tmp);
                x.value = tmp;
                isok=true;
            } catch(int presame){
                if (!presame) break;
            }
        }
        for (int tn = tmp.length(), on = x.value.length(); tn>on; --tn) in.unget();//将多余的字符unget回去

        if (!isok) throw unknown_operator(tmp, in.tellg()==-1?in.str().length():(int)in.tellg()+1);
    }
    return in;
}


void deleteExpTree(node* nd) {
    if (!nd->children.empty()) {
        for (node* cnd : nd->children) deleteExpTree(cnd);
    }
    delete nd;
}

node *copyTree(node *nd) {
    node *r = new node;
    r->content = nd->content;
    for (node* cnd : nd->children) 
        r->children.emplace_back(copyTree(cnd));
    return r;
}

void popOper(std::stack<ele> &ostk, std::stack<node*> &nstk) {
    node *nd = new node;
    nd->content = ostk.top();ostk.pop();
    if(nd->content.value=="(") return;

    int ary = getOpi(nd->content.value).ary;
    nd->children.resize(ary);
    for (int i=1; i<=ary; ++i) {
        if (nstk.empty()) throw missing_operand(nd->content.value, nd->content.pos);
        nd->children[ary-i] = nstk.top();
        nstk.top()->parent = nd;
        nstk.pop();
    }
    nstk.emplace(nd);
}


//解析过程中出现错误，释放内存
void giveupMem(std::stack<node*> &nstk) {
    while(!nstk.empty()) {
        deleteExpTree(nstk.top());
        nstk.pop();
    }
}


node* buildExpTree(const std::string &exp) {
    std::stringstream ss;
    ss << exp;
    ele e;
    std::stack<ele> ostk;
    std::stack<node*> nstk;
    try {
        while(1) {
            //*******↓下一个变量或运算符*****
            try{
                if (!(ss>>e)) break;
            } catch(unknown_operator e) {
                giveupMem(nstk);
                throw e;
            }
            //********↑获取下一个变量或运算符**********
            //********↓栈操作*******************
            if (e.isVar) nstk.push(new node{e});
            else if (e.value==")") {
                while(1) {
                    if (ostk.empty()) giveupMem(nstk), throw missing_operator();//没有遇到左括号，错误
                    if (ostk.top().value=="(") {ostk.pop(); break;}//遇到了左括号，退出
                    popOper(ostk, nstk);
                }
            } else {
                while(!ostk.empty() && ostk.top().value!="(" && 
                                //下面两个条件的意思是，对于左结合运算符，出栈 优先级>=自己的，对于右结合运算符，出栈 优先级>自己的
                                (getOpi(ostk.top().value).pri>getOpi(e.value).pri || 
                                getOpi(e.value).lf&&getOpi(ostk.top().value).pri==getOpi(e.value).pri))
                    popOper(ostk, nstk);
                ostk.push(e);
            }
        }
        while(!ostk.empty()) popOper(ostk, nstk);
    } catch (missing_operand mo) {
        giveupMem(nstk);
        throw mo;
    }
    //********↑栈操作*******************
    if (nstk.size()!=1) giveupMem(nstk), throw missing_operator();
    return nstk.top();
}


void post_out(node* nd) {
    if (!nd->children.empty()) {
        for (node* cnd : nd->children) post_out(cnd);
    }
    std::cout << nd->content.value;
}

//逻辑表达式的基础常量
std::map<std::string, bool> cons = {{"1", true}, {"0", false}, {"true", true}, {"false", false}, {"TRUE", true}, {"FALSE", false}, {"T", true}, {"F", false}};
inline bool readVar(std::string s, std::map<std::string, bool> &vars) {
    if (cons.count(s)) return cons[s];
    else if (vars.count(s)) return vars[s];
    throw unknown_varible();
}
//遍历表达式树，将遇到的变量作为key加入到map中
void getVars(node *nd, std::map<std::string, bool> &vars) {
    if (nd->content.isVar && !cons.count(nd->content.value)) vars[nd->content.value] = 0;
    for (node* cnd : nd->children) getVars(cnd, vars);
}
//遍历表达式树，将遇到的变量加入set
void getVars(node *nd, std::set<std::string> &vars) {
    if (nd->content.isVar && !cons.count(nd->content.value)) vars.emplace(nd->content.value);
    for (node* cnd : nd->children) getVars(cnd, vars);
}
//遍历表达式树，将遇到的变量加入vector
void getVars(node *nd, std::vector<std::string> &vars) {
    std::set<std::string> vset;
    getVars(nd, vset);
    for (const auto &it:vset) vars.emplace_back(it);
}


bool calc(node *nd, std::map<std::string, bool> &vars) {
    if (nd->content.isVar) return readVar(nd->content.value, vars);
    if (nd->content.value=="!") {
        return !calc(nd->children[0], vars);
    } else if (nd->content.value=="^" || /*nd->content.value=="!⇄" ||*/ nd->content.value=="!<->" || nd->content.value=="!=") {
        return calc(nd->children[0], vars)^calc(nd->children[1], vars);
    } else if (nd->content.value=="&&" || nd->content.value=="*") {
        return calc(nd->children[0], vars)&&calc(nd->children[1], vars);
    } else if (nd->content.value=="||" || nd->content.value=="+") {
        return calc(nd->children[0], vars)||calc(nd->children[1], vars);
    } else if (nd->content.value=="->" || nd->content.value=="→") {
        return !calc(nd->children[0], vars)||calc(nd->children[1], vars);
    } else if (nd->content.value=="==" || nd->content.value=="<->" /*|| nd->content.value=="⇄"*/ ) {
        return calc(nd->children[0], vars)==calc(nd->children[1], vars); 
    } else if (nd->content.value=="↓" || nd->content.value=="!||" || nd->content.value=="!+") {
        return !(calc(nd->children[0], vars)||calc(nd->children[1], vars));
    } else if (nd->content.value=="↑" || nd->content.value=="!&&" || nd->content.value=="!*") {
        return !(calc(nd->children[0], vars)&&calc(nd->children[1], vars));
    } else if (nd->content.value=="!->" || nd->content.value=="!→") {
        return !(!calc(nd->children[0], vars)||calc(nd->children[1], vars));
    }
}

//lambda是出一个结果后的回调函数，bool参数是计算结果，map存储各变量以及取值
void truth_table(std::map<std::string, bool>::iterator it, std::map<std::string, bool> &vars, node *nd, std::function<void(bool, std::map<std::string, bool>&)> lambda) {
    if (it!=vars.end()) {
        auto it2 = it; ++it2;
        it->second = false;
        truth_table(it2, vars, nd, lambda);
        it->second = true;
        truth_table(it2, vars, nd, lambda);
    } else {
        lambda(calc(nd, vars), vars);
    }
}
void truth_table(node *nd, std::function<void(bool, std::map<std::string, bool>&)> lambda) {
    std::map<std::string, bool> vars;
    getVars(nd, vars);
    truth_table(vars.begin(), vars, nd, lambda);
}

void output_truth_table(node* nd) {
    truth_table(nd, [](bool res, std::map<std::string, bool> vars){
        for (const auto &it:vars) 
            std::cout << it.first << "=" << it.second << ' ';
        std::cout << " result=" << res << '\n';
    });
}


//用于判断b的非0项是否可以覆盖s的非0项
bool isSub(const std::vector<char> &s, const std::vector<char> &b) {
    int len = s.size();
    for (int i=0; i<len; ++i) 
        if (~s[i] && s[i]!=b[i]) return false;
    return true;
}

//判断vector中是否全是某个值
bool isAllValue(const std::vector<char> &vs, char v) {
    for (const int it:vs) if (it!=v) return false;
    return true;
}

//表达式树转线性式
void tree2linear(node *nd, std::stringstream &ss) {
    if (nd->children.empty()) {
        ss << nd->content.value;
    } else {
        bool needBrackets = nd->parent && 
                (getOpi(nd->content.value).pri < getOpi(nd->parent->content.value).pri
                || !getOpi(nd->content.value).lf && getOpi(nd->content.value).pri == getOpi(nd->parent->content.value).pri
                );//意思是如果运算符是左结合，优先级相等的情况下不加括号，如果是右结合则加括号

        if (needBrackets) ss << "(";
        if (nd->content.value=="!") ss << "!";
        tree2linear(nd->children[0], ss);
        if (nd->content.value!="!") ss << nd->content.value;
        for (int i=1; i<nd->children.size(); ++i) tree2linear(nd->children[i], ss);
        if (needBrackets) ss << ")";
    }
}
//表达式树转线性式
std::string tree2linear(node *nd) {
    std::stringstream ss;
    tree2linear(nd,ss);
    return ss.str();
}


void linear2mathjax(node *nd, std::stringstream &ss) {
    if (nd->children.empty()) {
        ss << nd->content.value;
    } else {
        bool needBrackets = nd->parent && 
                (getOpi(nd->content.value).pri < getOpi(nd->parent->content.value).pri
                || !getOpi(nd->content.value).lf && getOpi(nd->content.value).pri == getOpi(nd->parent->content.value).pri
                );//意思是如果运算符是左结合，优先级相等的情况下不加括号，如果是右结合则加括号

        if (needBrackets) ss << "(";
        if (nd->content.value=="!") ss << "\\overline{";
        linear2mathjax(nd->children[0], ss);
        if (nd->content.value=="!") {
            ss << "}";
        } else {
            ss << (getOpi(nd->content.value).mj==""?nd->content.value:getOpi(nd->content.value).mj);
        }
        for (int i=1; i<nd->children.size(); ++i) linear2mathjax(nd->children[i], ss);
        if (needBrackets) ss << ")";
    }
    
}

//线性式转mathjax
std::string linear2mathjax(const std::string exp) {
    node *nd = buildExpTree(exp);
    std::stringstream ss;
    ss<<"$$";
    linear2mathjax(nd,ss);
    deleteExpTree(nd);
    ss<<"$$";
    return ss.str();
}

//根据选择的组合输出表达式
std::string group2exp(const std::vector<std::vector<char> > &used, const std::vector<std::string> &vars) {
    std::stringstream ss;
    if(used.empty()) //判断是否没有结果，如果没有结果就是0
        ss << '0';
    else {
        bool firstOr = true;
        for (const auto &sels : used) {
            if (isAllValue(sels, -1)) //判断是否是永真式，如果是，应该就只有这一项，会自然退出循环
                ss << '1';
            else {
                int n = sels.size();
                if (!firstOr) ss << "||"; firstOr = false;

                bool firstAnd = true;
                for (int i=0; i<n; ++i) {
                    if (sels[i]!=-1) {
                        if (!firstAnd) ss << "&&"; firstAnd = false;
                        if (!sels[i]) ss << "!";
                        ss << vars[i];
                    }
                }
            }
        }
    }
    
    return ss.str();
}

//d是递归深度，seln是应该选择几个, n是总变量数, ndtt是n维真值表
void simplify_dfs(int d, int seln, std::map<std::vector<char>, bool> &ndtt, std::vector<std::vector<char> > &used, std::vector<char> &sel) {
    if (d>=0) {
        sel[d] = -1;
        simplify_dfs(d-1, seln, ndtt, used, sel);
        if (seln>0) {
            sel[d] = 0;
            simplify_dfs(d-1, seln-1, ndtt, used, sel);
            sel[d] = 1;
            simplify_dfs(d-1, seln-1, ndtt, used, sel);
        }
    } else if (seln==0) {
        for (const auto &it:ndtt) 
            if (isSub(sel, it.first) && !it.second) return;
        for (const auto &it:used) 
            if (isSub(it, sel)) return;
        used.emplace_back(sel);
    }
}
//化简逻辑表达式，原理是列出真值表然后建立n维真值表，逐组合遍历是否可行，和卡诺图法类似
std::string simplify(node *nd) {
    std::map<std::vector<char>, bool> ndtt;//n维真值表
    truth_table(nd, [&ndtt](bool res, std::map<std::string, bool> vars){
        std::vector<char> v;
        for (const auto &it:vars) v.push_back(it.second);
        ndtt[v] = res;
    });
    std::vector<std::string> vars;
    getVars(nd, vars);

    std::vector<std::vector<char> > used;
    std::vector<char> sel(vars.size(), -1);//sel表示对应的变量取值，0表示false, 1表示true, -1表示无关
    for (int i=0; i<=vars.size(); ++i) 
        simplify_dfs(vars.size()-1, i, ndtt, used, sel);

    //************************************
    return group2exp(used, vars);
}



void changeOperatorStyle(node *nd, std::map<std::string, std::string> &mp) {
    if (!nd->content.isVar && mp.count(nd->content.value)) {
        nd->content.value = mp[nd->content.value];
    }
    for (auto it : nd->children) {
        changeOperatorStyle(it,mp);
    }
}
//改变表达式树中的运算符，参数列表中{A,A2,B,B2...}表示A变成A2，B变成B2
void changeOperatorStyle(node *nd, std::initializer_list<std::string> tr) {
    std::map<std::string, std::string> mp;
    for (auto it = tr.begin(); it!=tr.end(); it+=2) {
        mp[*it] = *(it+1);
    }
    changeOperatorStyle(nd, mp);
}

//按照指定的格式转换线性式
std::string formatLinearExp(const std::string exp, const std::string style, bool mathjax) {
    node *nd = buildExpTree(exp);
    if (style == "c") {
        changeOperatorStyle(nd, {"+","||","*","&&"});
    } else if (style == "am") {
        changeOperatorStyle(nd, {"||","+","&&","*"});
    }
    std::string ans = tree2linear(nd);
    if (mathjax) ans = linear2mathjax(ans);
    deleteExpTree(nd);
    return ans;
}

bool checkOption(std::string opt, cxxopts::ParseResult &res) {
    if (res.count(opt)) return true;
    else {
        std::cout << "missing option " << opt;
        exit(-1);
    }
}


//out选项：c表示c语言风格输出，am表示用+和*替换输出
int main(int argc, char* argv[]) {
    cxxopts::Options options("Simple Boolean Algebra", "A simple boolean algebra system");
    options.add_options()
    ("e,exp", "input the expression", cxxopts::value<std::string>())
    ("f,function", "tell me what you want", cxxopts::value<std::string>()->default_value("null"))
    ("o,out", "output style", cxxopts::value<std::string>()->default_value("c"))
    ("m,mathjax", "use mathjax format to output");
    options.parse_positional({"exp", "function", "out"});

    std::string exp;
    try {
        auto res = options.parse(argc,argv);
        std::string oper = res["function"].as<std::string>();
        if (oper == "simplify" || oper == "s") {
            checkOption("exp", res);
            exp = res["exp"].as<std::string>();

            node *nd = buildExpTree(exp);
            std::string sim = simplify(nd);
            deleteExpTree(nd);
            std:: cout << formatLinearExp(sim, res["out"].as<std::string>(), res["mathjax"].as<bool>());
            return 0;
        } else if (oper == "postexp" || oper == "pe") {
            checkOption("exp", res);
            exp = res["exp"].as<std::string>();

            node *nd = buildExpTree(exp);
            post_out(nd);
            deleteExpTree(nd);
            return 0;
        } else if (oper == "table" || oper == "t") {
            checkOption("exp", res);
            exp = res["exp"].as<std::string>();

            node *nd = buildExpTree(exp);
            output_truth_table(nd);
            deleteExpTree(nd);
            return 0;
        } else if (oper == "null") {
            checkOption("exp", res);
            exp = res["exp"].as<std::string>();

            std::cout << formatLinearExp(exp, res["out"].as<std::string>(), res["mathjax"].as<bool>());
            return 0;
        } else if (oper == "varn") {
            checkOption("exp", res);
            exp = res["exp"].as<std::string>();

            node *nd = buildExpTree(exp);
            std::vector<std::string> vs;
            getVars(nd, vs);
            std::cout << vs.size();
            return 0;
        }

    } catch (missing_operator) {
        std::cout << "missing operator.";
        exit(-1);
    } catch (missing_operand mo) {
        std::cout << "Missing operand of '" << mo.op << "' on col " << mo.pos << ", " << getOpi(mo.op).ary << " operand(s) expected.\n";
        print_error_col(exp, mo.pos);
        exit(-1);
    } catch (unknown_operator e) {
        if (e.pos == 0) {
            puts("wrong expression.");
        } else {
            std::stringstream err;
            err << "unknown operator '" << e.op << "' on col " << e.pos << '.';
            std::cout << err.str() <<'\n';
            print_error_col(exp, e.pos);
        }
        exit(-1);
    }
    catch (cxxopts::OptionParseException e) {
        std::cout << "unknown option";
        exit(-1);
    }
}