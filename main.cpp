#include<iostream>
#include<string>
#include<vector>
#include<sstream>
#include<stack>
#include<map>

struct ele {
    std::string value;
    bool isVar;
};

struct node {
    ele content;
    std::vector<node*> children;
};

//用于解析中缀表达式时提供运算符的信息
//有了这个，就足够让表达式树正确建立
struct operator_info {
    std::string val;
    int ary, pri;//运算符元数 和 优先级（越大越优先）
};

const operator_info opis[] = { // Ary Pri
                            {"||",2,70}, {"&&",2,90}, {"!",1,100}, {"^",2,80}, {"||",2,70}, 
                            {"(",0,1000}, {")",0,1000}, {"->",2,60}, {"→",2,60}, {"?",2,50},
                            {"<->",2,50}, {"==",2,50}
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

//如果识别出错，throw列号
std::stringstream& operator >> (std::stringstream& in, ele& x) {
    if (in.peek()==-1) return in;
    x.isVar = isalnum(in.peek());
    x.value.clear();
    char c;
    if (x.isVar) {
        while(c=in.peek(), ~c&&isalnum(c))
            x.value.push_back(in.get());
    } else {
        std::string tmp = "";
        bool isok = false;
        int cnt = 0;//表示读取的多余字符数，之后要unget回去
        while(~(c = in.get())) {
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

void popOper(std::stack<ele> &ostk, std::stack<node*> &nstk) {
    node *nd = new node;
    nd->content = ostk.top();ostk.pop();
    if(nd->content.value=="(") return;

    int ary = getOpi(nd->content.value).ary;
    nd->children.resize(ary);
    for (int i=1; i<=ary; ++i) {
        nd->children[ary-i] = nstk.top();
        nstk.pop();
    }
    nstk.emplace(nd);
}


const int UNKNOWN_OPERATOR = 1;
node* buildExpTree(const std::string &exp) {
    std::stringstream ss;
    ss << exp;
    ele e;
    std::stack<ele> ostk;
    std::stack<node*> nstk;
    while(1) {
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
        
        if (e.isVar) nstk.push(new node{e});
        else if (e.value==")") {
            while(!ostk.empty() && ostk.top().value!="(") popOper(ostk, nstk);
            if (!ostk.empty()) ostk.pop();
        } else {
            while(!ostk.empty() && ostk.top().value!="(" && getOpi(ostk.top().value).pri>=getOpi(e.value).pri) 
                popOper(ostk, nstk);
            ostk.push(e);
        }
    }
    while(!ostk.empty()) popOper(ostk, nstk);
    return nstk.top();
}


void post_out(node* nd) {
    if (!nd->children.empty()) {
        for (node* cnd : nd->children) post_out(cnd);
    }
    std::cout << nd->content.value;
}



bool calc(node *nd, std::map<std::string, bool> vars) {
    if (nd->content.isVar) return vars[nd->content.value];
    if (nd->content.value=="!") {
        return !calc(nd->children[0], vars);
    } else if (nd->content.value=="^") {
        return calc(nd->children[0], vars)^calc(nd->children[1], vars);
    } else if (nd->content.value=="&&") {
        return calc(nd->children[0], vars)&&calc(nd->children[1], vars);
    } else if (nd->content.value=="||") {
        return calc(nd->children[0], vars)||calc(nd->children[1], vars);
    } else if (nd->content.value=="->" || nd->content.value=="→") {
        return !calc(nd->children[0], vars)||calc(nd->children[1], vars);
    } else if (nd->content.value=="==" || nd->content.value=="<->" || nd->content.value=="?" ) {
        return calc(nd->children[0], vars)==calc(nd->children[1], vars); 
    }
}

//遍历表达式树，将遇到的变量作为key加入到map中
void getVars(node *nd, std::map<std::string, bool> &vars) {
    if (nd->content.isVar) vars[nd->content.value] = 0;
    for (node* cnd : nd->children) getVars(cnd, vars);
}

void output_truth_table(std::map<std::string, bool>::iterator it, std::map<std::string, bool> &vars, node *nd) {
    if (it!=vars.end()) {
        auto it2 = it; ++it2;
        it->second = false;
        output_truth_table(it2, vars, nd);
        it->second = true;
        output_truth_table(it2, vars, nd);
    } else {
        bool res = calc(nd, vars);
        for (auto it:vars) 
            std::cout << it.first << "=" << it.second << ' ';
        std::cout << " result=" << res << '\n';
    }
}

void output_truth_table(node* nd) {
    std::map<std::string, bool> vars;
    getVars(nd, vars);
    output_truth_table(vars.begin(), vars, nd);
}

void expand(node *nd) {

}


int main() {
    try{
        node* nd = buildExpTree("Q->(P||(P&&Q))");
        post_out(nd);
        putchar('\n');
        output_truth_table(nd);
        deleteExpTree(nd);
    } catch(...){}
    
}