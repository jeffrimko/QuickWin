#include <iostream>
#include <vector>
#include <algorithm>

#include "cmdformatter.h"

using namespace std;

string get_cmd(string chk) {
    static const vector<string> cmd_types = {"help", "set", "get", "delete", "title", "number", "executable"};
    for(string cmd : cmd_types) {
        if(0 == cmd.find(chk)) {
            return(cmd);
        }
    }
    return("");
}

bool is_cmd(string chk) {
    return("" != get_cmd(chk));
}

/** Trims whitespace from edges of string. */
string trim(string &str)
{
    if(!str.size()) {
        return("");
    }
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');
    if(first == string::npos) {
        // Bail since string is all whitespace; return blank string.
        return("");
    }
    return(str.substr(first, (last-first+1)));
}

void update_cmds(cmds_t &cmds, string cmd, string arg, bool semi) {
    cmd = trim(cmd);
    arg = trim(arg);
    if(0 == cmds.size() && !semi && arg == "") {
        // Erase existing data otherwise insert will do nothing if duplicate.
        cmds.erase("title");
        cmds.insert(pair<string,string>("title", cmd));
    } else if(is_cmd(cmd)) {
        // Erase existing data otherwise insert will do nothing if duplicate.
        cmd = get_cmd(cmd);
        cmds.erase(cmd);
        cmds.insert(pair<string,string>(cmd, arg));
    }
}

void format_cmds(cmds_t &cmds, string cmd_text) {
    string cmd;
    string arg;
    bool has_semi = false;
    bool in_cmd = false;
    bool on_arg = false;

    for(char &c : cmd_text) {
        if(!in_cmd) {
            if(' ' != c) {
                in_cmd = true;
            }
        }
        if(in_cmd) {
            if(';' != c) {
                if(' ' == c) {
                    cmd = trim(cmd);
                    if(is_cmd(cmd) && has_semi) {
                        on_arg = true;
                    }
                }
                if(on_arg) {
                    arg += c;
                } else {
                    cmd += c;
                }
            } else {
                update_cmds(cmds, cmd, arg, has_semi);
                cmd.erase();
                arg.erase();
                in_cmd = false;
                on_arg = false;
                has_semi = true;
            }
        }
    }
    update_cmds(cmds, cmd, arg, has_semi);
}
