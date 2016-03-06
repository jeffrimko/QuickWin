

#ifndef _CMDFORMATTER_H_  /* Define include guard. */
#define _CMDFORMATTER_H_

#include <map>
#include <vector>
#include <string>

typedef std::map<std::string,std::string> cmds_t;

void format_cmds(cmds_t &cmds, std::string cmd_text);
const std::vector<std::string> list_cmd_types(void);

bool is_cmd(std::string chk);
std::string get_cmd(std::string chk);

#endif  /* #ifndef include guard */
