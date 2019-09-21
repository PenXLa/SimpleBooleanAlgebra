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

const int UNKNOWN_OPERATOR = 1;
const int UNKNOWN_VARIABLE = 2;
const int MISSING_OPERAND = 3;

struct ele {
    std::string value;
    bool isVar;
};

struct node {
    ele content;
    std::vector<node*> children;
    node *parent=nullptr;
};

//���ڽ�����׺���ʽʱ�ṩ���������Ϣ
//������������㹻�ñ��ʽ����ȷ����
struct operator_info {
    std::string val;
    int ary, pri;//�����Ԫ�� �� ���ȼ���Խ��Խ���ȣ�
    bool lf=1;//�Ƿ�������
    const std::string mj="";//mathjax��һЩ�������Ҫת�����
};

const operator_info opis[] = { // Ary Pri
                            {"||",2,70}, {"&&",2,90,true,"\\&\\&"}, {"!",1,100,false}, {"^",2,80}, {"||",2,70}, 
                            {"(",0,1000}, {")",0,1000}, {"->",2,60}, {"��",2,60}, {"?",2,50},
                            {"<->",2,50}, {"==",2,50}, {"+",2,70}, {"*",2,90}
                            };



//���û���ҵ���throw�����ǰ׺��s��ȵĸ���
operator_info getOpi(std::string s) {
    int n = sizeof(opis)/sizeof(operator_info);
    int presame = 0, slen = s.length();
    for (int i=0; i<n; ++i) {
        if (s == opis[i].val) return opis[i];
        else presame += (opis[i].val.length()>slen && opis[i].val.substr(0,slen)==s);
    }
    throw presame;
}

//���ʶ�����throw�к�
std::stringstream& operator >> (std::stringstream& in, ele& x) {
    if (in.peek()==-1) return in;
    x.isVar = isalnum(in.peek());
    x.value.clear();
    char c;
    if (x.isVar) {
        while(c=in.peek(), ~c&&isalnum(c))
            x.value.push_back(in.get());
    } else {
        while(~(c = in.peek()) && c==' ') in.get();//�Ե�ǰ��ո�
        std::string tmp = "";
        bool isok = false;
        int cnt = 0;//��ʾ��ȡ�Ķ����ַ�����֮��Ҫunget��ȥ
        while(~(c = in.get()) && c!=' ') {
            tmp.push_back(c);++cnt;
            try{
                getOpi(tmp);
                x.value = tmp;
                isok=true;
                cnt -= tmp.length();
            } catch(int presame){
                if (!presame) break;
            }
        }
        for (;cnt>0; --cnt) in.unget();

        if (!isok) throw (int)(in.tellg()+1);
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
        if (nstk.empty()) throw MISSING_OPERAND;
        nd->children[ary-i] = nstk.top();
        nstk.top()->parent = nd;
        nstk.pop();
    }
    nstk.emplace(nd);
}



node* buildExpTree(const std::string &exp) {
    std::stringstream ss;
    ss << exp;
    ele e;
    std::stack<ele> ostk;
    std::stack<node*> nstk;
    while(1) {
        //*******����һ�������������*****
        try{
            if (!(ss>>e)) break;
        } catch(int col) {
            if (col == 0) {
                 puts("wrong expression.");
            } else {
                std::stringstream err;
                err << "unknown operator on col " << col << '.';
                std::cout << err.str() <<'\n';
                std::cout << exp << '\n';
                for (int i=0; i<col-1; ++i) putchar(' ');
                puts("^");
            }
            

            while(!nstk.empty()) {
                deleteExpTree(nstk.top());
                nstk.pop();
            }
            throw UNKNOWN_OPERATOR;
        }
        //********����ȡ��һ�������������**********
        //********��ջ����*******************
        try {
            if (e.isVar) nstk.push(new node{e});
            else if (e.value==")") {
                while(!ostk.empty() && ostk.top().value!="(") popOper(ostk, nstk);
                if (!ostk.empty()) ostk.pop();
            } else {
                while(!ostk.empty() && ostk.top().value!="(" && 
                                //����������������˼�ǣ������������������ջ ���ȼ�>=�Լ��ģ������ҽ�����������ջ ���ȼ�>�Լ���
                                (getOpi(ostk.top().value).pri>getOpi(e.value).pri || 
                                getOpi(e.value).lf&&getOpi(ostk.top().value).pri==getOpi(e.value).pri))
                    popOper(ostk, nstk);
                ostk.push(e);
            }
        } catch (int err) {
            if (err == MISSING_OPERAND) std::cout << "Missing operand";
            while(!nstk.empty()) {
                deleteExpTree(nstk.top());
                nstk.pop();
            }
            throw MISSING_OPERAND;
        }
        
    }
    while(!ostk.empty()) popOper(ostk, nstk);
    //********��ջ����*******************
    return nstk.top();
}


void post_out(node* nd) {
    if (!nd->children.empty()) {
        for (node* cnd : nd->children) post_out(cnd);
    }
    std::cout << nd->content.value;
}

//�߼����ʽ�Ļ�������
std::map<std::string, bool> cons = {{"1", true}, {"0", false}, {"true", true}, {"false", false}, {"TRUE", true}, {"FALSE", false}, {"T", true}, {"F", false}};
inline bool readVar(std::string s, std::map<std::string, bool> &vars) {
    if (cons.count(s)) return cons[s];
    else if (vars.count(s)) return vars[s];
    throw UNKNOWN_VARIABLE;
}
//�������ʽ�����������ı�����Ϊkey���뵽map��
void getVars(node *nd, std::map<std::string, bool> &vars) {
    if (nd->content.isVar && !cons.count(nd->content.value)) vars[nd->content.value] = 0;
    for (node* cnd : nd->children) getVars(cnd, vars);
}
//�������ʽ�����������ı�������set
void getVars(node *nd, std::set<std::string> &vars) {
    if (nd->content.isVar && !cons.count(nd->content.value)) vars.emplace(nd->content.value);
    for (node* cnd : nd->children) getVars(cnd, vars);
}
//�������ʽ�����������ı�������vector
void getVars(node *nd, std::vector<std::string> &vars) {
    std::set<std::string> vset;
    getVars(nd, vset);
    for (const auto &it:vset) vars.emplace_back(it);
}


bool calc(node *nd, std::map<std::string, bool> &vars) {
    if (nd->content.isVar) return readVar(nd->content.value, vars);
    if (nd->content.value=="!") {
        return !calc(nd->children[0], vars);
    } else if (nd->content.value=="^") {
        return calc(nd->children[0], vars)^calc(nd->children[1], vars);
    } else if (nd->content.value=="&&" || nd->content.value=="*") {
        return calc(nd->children[0], vars)&&calc(nd->children[1], vars);
    } else if (nd->content.value=="||" || nd->content.value=="+") {
        return calc(nd->children[0], vars)||calc(nd->children[1], vars);
    } else if (nd->content.value=="->" || nd->content.value=="��") {
        return !calc(nd->children[0], vars)||calc(nd->children[1], vars);
    } else if (nd->content.value=="==" || nd->content.value=="<->" || nd->content.value=="?" ) {
        return calc(nd->children[0], vars)==calc(nd->children[1], vars); 
    }
}

//lambda�ǳ�һ�������Ļص�������bool�����Ǽ�������map�洢�������Լ�ȡֵ
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


//�����ж�b�ķ�0���Ƿ���Ը���s�ķ�0��
bool isSub(const std::vector<char> &s, const std::vector<char> &b) {
    int len = s.size();
    for (int i=0; i<len; ++i) 
        if (~s[i] && s[i]!=b[i]) return false;
    return true;
}

//�ж�vector���Ƿ�ȫ��ĳ��ֵ
bool isAllValue(const std::vector<char> &vs, char v) {
    for (const int it:vs) if (it!=v) return false;
    return true;
}

//���ʽ��ת����ʽ
void tree2linear(node *nd, std::stringstream &ss) {
    if (nd->children.empty()) {
        ss << nd->content.value;
    } else {
        bool needBrackets = nd->parent && getOpi(nd->content.value).pri <= getOpi(nd->parent->content.value).pri;
        if (needBrackets) ss << "(";
        if (nd->content.value=="!") ss << "!";
        tree2linear(nd->children[0], ss);
        if (nd->content.value!="!") ss << nd->content.value;
        for (int i=1; i<nd->children.size(); ++i) tree2linear(nd->children[i], ss);
        if (needBrackets) ss << ")";
    }
}
//���ʽ��ת����ʽ
std::string tree2linear(node *nd) {
    std::stringstream ss;
    tree2linear(nd,ss);
    return ss.str();
}


void linear2mathjax(node *nd, std::stringstream &ss) {
    if (nd->children.empty()) {
        ss << nd->content.value;
    } else {
        bool needBrackets = nd->parent && getOpi(nd->content.value).pri <= getOpi(nd->parent->content.value).pri;
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

//����ʽתmathjax
std::string linear2mathjax(const std::string exp) {
    node *nd = buildExpTree(exp);
    std::stringstream ss;
    ss<<"$$";
    linear2mathjax(nd,ss);
    deleteExpTree(nd);
    ss<<"$$";
    return ss.str();
}

//����ѡ������������ʽ
std::string group2exp(const std::vector<std::vector<char> > &used, const std::vector<std::string> &vars) {
    std::stringstream ss;
    if(used.empty()) //�ж��Ƿ�û�н�������û�н������0
        ss << '0';
    else {
        bool firstOr = true;
        for (const auto &sels : used) {
            if (isAllValue(sels, -1)) //�ж��Ƿ�������ʽ������ǣ�Ӧ�þ�ֻ����һ�����Ȼ�˳�ѭ��
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

//d�ǵݹ���ȣ�seln��Ӧ��ѡ�񼸸�, n���ܱ�����, ndtt��nά��ֵ��
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
//�����߼����ʽ��ԭ�����г���ֵ��Ȼ����nά��ֵ������ϱ����Ƿ���У��Ϳ�ŵͼ������
std::string simplify(node *nd) {
    std::map<std::vector<char>, bool> ndtt;//nά��ֵ��
    truth_table(nd, [&ndtt](bool res, std::map<std::string, bool> vars){
        std::vector<char> v;
        for (const auto &it:vars) v.push_back(it.second);
        ndtt[v] = res;
    });
    std::vector<std::string> vars;
    getVars(nd, vars);

    std::vector<std::vector<char> > used;
    std::vector<char> sel(vars.size(), -1);//sel��ʾ��Ӧ�ı���ȡֵ��0��ʾfalse, 1��ʾtrue, -1��ʾ�޹�
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
//�ı���ʽ���е�������������б���{A,A2,B,B2...}��ʾA���A2��B���B2
void changeOperatorStyle(node *nd, std::initializer_list<std::string> tr) {
    std::map<std::string, std::string> mp;
    for (auto it = tr.begin(); it!=tr.end(); it+=2) {
        mp[*it] = *(it+1);
    }
    changeOperatorStyle(nd, mp);
}

//����ָ���ĸ�ʽת������ʽ
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


//outѡ�c��ʾc���Է�������am��ʾ��+��*�滻���
int main(int argc, char* argv[]) {
    cxxopts::Options options("Simple Boolean Algebra", "A simple boolean algebra system");
    options.add_options()
    ("e,exp", "input the expression", cxxopts::value<std::string>())
    ("f,function", "tell me what you want", cxxopts::value<std::string>()->default_value("null"))
    ("o,out", "output style", cxxopts::value<std::string>()->default_value("c"))
    ("m,mathjax", "use mathjax format to output");
    options.parse_positional({"exp", "function", "out"});

    try {
        auto res = options.parse(argc,argv);
        std::string oper = res["function"].as<std::string>();
        if (oper == "simplify" || oper == "s") {
            checkOption("exp", res);
            std::string exp = res["exp"].as<std::string>();

            try {
                node *nd = buildExpTree(exp);
                std::string sim = simplify(nd);
                deleteExpTree(nd);
                std:: cout << formatLinearExp(sim, res["out"].as<std::string>(), res["mathjax"].as<bool>());
                return 0;
            } catch(...) {
                exit(-1);
            }
        } else if (oper == "postexp" || oper == "pe") {
            checkOption("exp", res);
            std::string exp = res["exp"].as<std::string>();

            try {
                node *nd = buildExpTree(exp);
                post_out(nd);
                deleteExpTree(nd);
                return 0;
            } catch(...) {
                exit(-1);
            }
        } else if (oper == "table" || oper == "t") {
            checkOption("exp", res);
            std::string exp = res["exp"].as<std::string>();

            try {
                node *nd = buildExpTree(exp);
                output_truth_table(nd);
                deleteExpTree(nd);
                return 0;
            } catch(...) {
                exit(-1);
            }
        } else if (oper == "null") {
            checkOption("exp", res);
            std::string exp = res["exp"].as<std::string>();

            try {
                std::cout << formatLinearExp(exp, res["out"].as<std::string>(), res["mathjax"].as<bool>());
                return 0;
            } catch(...) {
                exit(-1);
            }
        }

    } catch (cxxopts::OptionParseException e) {
        std::cout << "unknown option";
        exit(-1);
    }
}