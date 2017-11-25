#ifndef PACKRAT_JSON
#define PACKRAT_JSON

#include <map>
#include <vector>

#include <string>
#include <iostream>

#include <boost/variant.hpp>
#include <boost/none.hpp>

namespace json_parser {
  class value {
  public:
    value(std::map<std::string, value> m) : data(m) {}
    value(std::vector<value> v) : data(v) {}
    value(std::string s) : data(s) {}
    value(double d) : data(d) {}
    value(bool b) : data(b) {}
    value() : data(boost::none) {}
  
    value at(std::string key);
    value at(int i);
    std::string to_string();
    double to_number();
    bool to_bool();

    bool is_object();
    bool is_array();
    bool is_string();
    bool is_number();
    bool is_boolean();
    bool is_null();

  private:
    boost::variant<boost::none_t, bool, double, std::string, std::vector<value>, std::map<std::string, value>> data;
  };

  value parse(std::istream& input);
}

#endif
