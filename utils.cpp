#include"utils.hpp"
#include<iostream>
void print_error_col(const std::string exp, int col) {
    std::cout << exp << '\n';
    for (int i=0; i<col-1; ++i) putchar(' ');
    puts("^");
}