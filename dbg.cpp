#include "dbg.hpp"

using namespace std;

typedef struct dbg_mode {
  const char *d_name;
  unsigned long long d_mode;
} dbg_mode_t;

typedef struct dbg_mode_color {
  unsigned long long d_mode;
  const char *color;
} dbg_mode_color_t;

unsigned long long dbg_modes = 0;
short dbg_initiated = 0;

static dbg_mode_color_t dbg_colortab[] = {DBG_COLORTAB};

static dbg_mode_t dbg_nametab[] = {DBG_NAMETAB};

const char *DCOLOR(unsigned long long d_mode) {
  dbg_mode_color_t *mode;
  for (mode = dbg_colortab; mode->d_mode != 0; mode++) {
    if (mode->d_mode & d_mode)
      return mode->color;
  }
  return _BWHITE_;
}

static void dbg_add_mode(const char *name) {
  int cancel;
  dbg_mode_t *mode;
  if (*name == '-') {
    cancel = 1;
    name++;
  } else
    cancel = 0;

  for (mode = dbg_nametab; mode->d_name != NULL; mode++)
    if (strcmp(name, mode->d_name) == 0)
      break;
  if (mode->d_name == NULL) {
    fprintf(stderr,
            "Warning: Unknown debug option: "
            "\"%s\"\n",
            name);
    return;
  }

  if (cancel)
    dbg_modes &= ~mode->d_mode;
  else
    dbg_modes = dbg_modes | mode->d_mode;
}

void dbg_init() {
  char env[256];
  char *name;
  const char *dbg_env;

  dbg_initiated = 1;
  dbg_modes = DBG_ERROR;
  dbg_env = getenv("DBG_MODES");
  if (!dbg_env)
    return;

  strncpy(env, dbg_env, sizeof(255));
  for (name = strtok(env, ","); name; name = strtok(NULL, ","))
    dbg_add_mode(name);
}

void print_nodes_map(std::map<std::string, node_physical_info> m) {
  cout << "virtual mapping is : " << endl;
  for (auto it = m.begin(); !m.empty() && it != m.end(); it++)
    cout << it->first << "  " << it->second.phys_ip << ":"
         << to_string(it->second.port) << endl;
}

void print_distance_table(std::map<int, std::map<int, int>> d) {
  cout << "distance table is : " << endl;
  if (d.begin() == d.end())
    return;
  map<int, int> firstMap = d.begin()->second;
  cout << "     ";
  for (auto it = firstMap.begin(); !firstMap.empty() && it != firstMap.end();
       it++)
    cout << it->first << " ";
  cout << endl;
  for (auto it = d.begin(); !d.empty() && it != d.end(); it++) {
    cout << it->first << " ";
    for (auto it2 = it->second.begin();
         !it->second.empty() && it2 != it->second.end(); it2++)
      cout << it2->second << " ";
    cout << endl;
  }
}

void print_routing_table(std::map<int, struct routing_table_info> r) {
  cout << "routing table is : " << endl;
  for (auto it = r.begin(); !r.empty() && it != r.end(); it++)
    cout << it->first << "  " << it->second.best_route_port << ","
         << to_string(it->second.cost) << endl;
}

void print_creation_time(std::map<int, long> c) {
  cout << "creation time is : " << endl;
  for (auto it = c.begin(); !c.empty() && it != c.end(); it++) {
    cout << it->first << " " << it->second << endl;
  }
}

void print_interfaces(std::vector<interface> interfaces) {
  cout << "id"
       << "     "
       << "rem"
       << "               "
       << "loc" << endl;
  for (int i = 0; i < interfaces.size(); i++) {
    cout << i << "      " << interfaces[i].remote << "       "
         << interfaces[i].local << endl;
  }
}