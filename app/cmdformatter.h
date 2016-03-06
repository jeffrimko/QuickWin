

#ifndef _CMDFORMATTER_H_  /* Define include guard. */
#define _CMDFORMATTER_H_

#include <map>
#include <vector>
#include <string>

extern const std::vector<std::string> cmd_types;

typedef std::map<std::string,std::string> cmds_t;

void format_cmds(cmds_t &cmds, std::string cmd_text);

bool is_cmd(std::string chk);
std::string get_cmd(std::string chk);

#endif  /* #ifndef include guard */
