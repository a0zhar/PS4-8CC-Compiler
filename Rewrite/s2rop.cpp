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


void do_exchange_regs(unordered_map<string, string>& mapping) {
    if (mapping.empty()) return;
    if (mapping.size() == 1 && mapping.count("rax") && mapping["rax"].find(' ') == string::npos) {
        cout << "mov rax, " << mapping["rax"] << endl;
        return;
    }
    if (mapping.count("rax") == 0) mapping["rax"] = "rax";
    cout << "# do_exchange_regs: " << endl;
    unordered_map<string, string> labels_dst;
    for (const auto& kv : mapping) {
        labels_dst[kv.first] = make_label(_static_cntr);
    }
    for (const auto& kv : mapping) {
        if (kv.second == "rax") {
            cout << "pop rsi" << endl;
            cout << "dp " << labels_dst[kv.first] << endl;
            cout << "mov [rsi], rax" << endl;
        }
    }
    string cur_rax = "rax";
    for (const auto& kv : mapping) {
        if (kv.second != "rax" && kv.second.find(' ') == string::npos && kv.first != "rax") {
            cur_rax = kv.second;
            cout << "mov rax, " << kv.second << endl;
            cout << "pop rsi" << endl;
            cout << "dp " << labels_dst[kv.first] << endl;
            cout << "mov [rsi], rax" << endl;
        }
    }
    if (mapping["rax"].find(' ') == string::npos && mapping["rax"] != "rax" && mapping["rax"] != cur_rax && mapping.count("r11") == 0) {
        cout << "mov rax, " << mapping["rax"] << endl;
    }
    if (mapping.count("r11")) {
        cout << "pop r11 ; mov rax, rdi" << endl;
        cout << labels_dst["r11"] << ":" << endl;
        if (mapping["r11"].find(' ') != string::npos) cout << mapping["r11"] << endl;
        else cout << "dq 0" << endl;
    }
    for (const auto& kv : mapping) {
        assert(kv.first.find(' ') == string::npos);
        if ((kv.first != "rax" || kv.second.find(' ') != string::npos || kv.second == "rax" || mapping.count("r11")) && kv.first != "rsp" && kv.first != "r11") {
            cout << "pop " << kv.first << endl;
            cout << labels_dst[kv.first] << ":" << endl;
            if (kv.second.find(' ') != string::npos) cout << kv.second << endl;
            else cout << "dq 0" << endl;
        }
    }
    if (mapping.count("rsp")) {
        cout << "pop rsp" << endl;
        cout << labels_dst["rsp"] << ":" << endl;
        if (mapping["rsp"].find(' ') != string::npos) cout << mapping["rsp"] << endl;
        else cout << "dq 0" << endl;
    }
}

unordered_map<string, string> cur_exchange;

void exchange_regs(const unordered_map<string, string>& mapping) { // {dst: src, ...}
    if (mapping.empty()) {
        do_exchange_regs(cur_exchange);
        cur_exchange.clear();
        return;
    }
    unordered_map<string, string> new_cur;
    for (const auto& kv : cur_exchange) {
        new_cur[kv.first] = kv.second;
    }
    for (const auto& kv : mapping) {
        string v = kv.first;
        if (mapping.count(v)) v = mapping.at(v);
        if (cur_exchange.count(v)) v = cur_exchange.at(v);
        if (v != kv.first) new_cur[kv.first] = v;
    }
    cout << "# exchange_regs" << endl;
    cur_exchange.clear();
    cur_exchange = new_cur;
}

}

unordered_map<string, string> cur_exchange;

void exchange_regs(const unordered_map<string, string>& mapping) { 
    if (mapping.empty()) {
        do_exchange_regs(cur_exchange);
        cur_exchange.clear();
        return;
    }
    unordered_map<string, string> new_cur;
    for (const auto& kv : cur_exchange) {
        new_cur[kv.first] = kv.second;
    }
    for (const auto& kv : mapping) {
        string v = kv.first;
        if (mapping.count(v)) v = mapping.at(v);
        if (cur_exchange.count(v)) v = cur_exchange.at(v);
        if (v != kv.first) new_cur[kv.first] = v;
    }
    cout << "# exchange_regs" << endl;
    cur_exchange.clear();
    cur_exchange = new_cur;
}

void emit_instr(const vector<string>& args) {
    assert(cur_exchange.empty());
    for (const auto& arg : args) {
        cout << arg << " ";
    }
    cout << endl;
}

void emit_mov(const string& reg_dst, const string& reg_src) {
    if (reg_dst == "rax" && cur_exchange.empty()) {
        if (reg_src != "rax") {
            emit_instr({"mov rax,", reg_src});
        }
    } else {
        if (reg_src != "rax") {
            warn("CHECK_FALLBACK", {"mov " + reg_dst + ", " + reg_src + ": fallback"});
        }
        unordered_map<string, string> exchange;
        exchange[reg_dst] = reg_src;
        exchange_regs(exchange);
    }
}


// MORE COMING
