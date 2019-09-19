#include<iostream>
#include<string>
#include<vector>
#include<sstream>
#include<stack>

struct ele {
    std::string value;
    bool isVar;
};

struct node {
    ele content;
    std::vector<node*> children;
};


//越大越优先
int getPriority(const std::string& s) {
    if (s=="&&") return 90;
    else if (s=="||") return 70;
    else if (s=="!") return 100;
    else if (s=="^") return 80;
    else if (s=="(" || s==")") return 1000;
    return -1;
}
//获取运算符元数
int getArity(const std::string& s) {
    if (s=="&&" || s=="||" || s=="^") return 2;
    else if (s=="!") return 1;
}

std::stringstream& operator >> (std::stringstream& in, ele& x) {
    x.isVar = isalnum(in.peek());
    x.value.clear();
    char c;
    if (x.isVar) {
        while(c=in.peek(), ~c&&isalnum(c))
            x.value.push_back(in.get());
    } else {
        c = in.get();
        if (c=='+' || c=='-' || c=='*' || c=='/' || c=='(' || c==')' || c=='^' || c=='!') x.value.push_back(c);
        else if (c=='&' || c=='|') {
            char c2 = in.get();
            if (c=='&' && c2=='&') x.value += "&&";
            else if (c=='|' && c2=='|') x.value += "||";
            else in.unget();
        }
    }
    return in;
}


void popOper(std::stack<ele> &ostk, std::stack<node*> &nstk) {
    node *nd = new node;
    nd->content = ostk.top();ostk.pop();
    int ary = getArity(nd->content.value);
    nd->children.resize(ary);
    for (int i=1; i<=ary; ++i) {
        nd->children[ary-i] = nstk.top();
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
    while(ss >> e) {
        if (e.isVar) nstk.push(new node{e});
        else if (e.value==")") {
            while(!ostk.empty() && ostk.top().value!="(") popOper(ostk, nstk);
            if (!ostk.empty()) ostk.pop();
        } else {
            while(!ostk.empty() && ostk.top().value!="(" && getPriority(ostk.top().value)>=getPriority(e.value)) 
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

void deleteExpTree(node* nd) {
    if (!nd->children.empty()) {
        for (node* cnd : nd->children) deleteExpTree(cnd);
    }
    delete nd;
}

long long calc(node *nd) {
    if (nd->content.isVar) return std::stoll(nd->content.value);
    if (nd->content.value=="!") {
        return !calc(nd->children[0]);
    } else if (nd->content.value=="^") {
        return calc(nd->children[0])^calc(nd->children[1]);
    } else if (nd->content.value=="&&") {
        return calc(nd->children[0])&&calc(nd->children[1]);
    } else if (nd->content.value=="||") {
        return calc(nd->children[0])||calc(nd->children[1]);
    }
}


int main() {
    node* nd = buildExpTree("!(A||!B)");
    post_out(nd);
    deleteExpTree(nd);
}