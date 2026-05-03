#include <string>
#include <unordered_set>

struct Tag {
  std::string name;
  std::unordered_set<std::string> groups;
};
