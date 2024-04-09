#include <iostream>
#include <unordered_map>
#include <vector>

using namespace std;

// Constants for register mapping
unordered_map<string, string> reg_map = {
    {"A", "rax"},
    {"B", "rcx"},
    {"C", "r10"},
    {"SP", "rdi"},
    {"BP", "r8"}
};

unordered_set<string> conds = {"eq", "ne", "lt", "le", "gt", "ge"};

// rsi is used as a scratch register for some operation
// r11 is used to back up a register when necessary

string make_label(int& _static_cntr) {
    // all user-provided labels are underscore-prefixed
    string ans = "L" + to_string(_static_cntr);
    _static_cntr++;
    return ans;
}

void warn(const string& check_env, const vector<string>& msg) {
    if (!check_env.empty() && os.environ.count(check_env) == 0) return;
    cerr << "s2rop: warning: ";
    for (const auto& m : msg)
        cerr << m << " ";
    cerr << endl;
}
