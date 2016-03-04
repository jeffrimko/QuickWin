

#ifndef _CMDFORMATTER_H_  /* Define include guard. */
#define _CMDFORMATTER_H_

#include <map>
#include <string>

typedef std::map<std::string,std::string> cmds_t;

void format_cmds(cmds_t &cmds, std::string cmd_text);

#endif  /* #ifndef include guard */