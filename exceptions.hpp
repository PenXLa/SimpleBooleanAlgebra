#include<exception>
struct missing_operand : public std::exception {
    std::string op;
    int pos;
    missing_operand(const std::string &op, int pos):op(op),pos(pos){}

    const char * what () const throw () {
        return "missing operand";
    }
};
struct unknown_operator : public std::exception {
    int pos;
    unknown_operator(int pos):pos(pos){}
    const char * what () const throw () {
        return "unknown operator";
    }
};

struct unknown_varible : public std::exception {
    const char * what () const throw () {
        return "unknown varible";
    }
};


