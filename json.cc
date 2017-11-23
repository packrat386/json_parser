#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <deque>
#include <string>
#include <stdexcept>
#include <cctype>
#include <map>
#include <optional>
#include <memory>

enum class token_type { TRUE, FALSE, NULL_TOKEN, STRING, NUMBER, LBRACE, RBRACE, LBRACKET, RBRACKET, COLON, COMMA };

struct token {
  std::string text;
  token_type type;

  token(std::string in_text, token_type in_type) :
    text(in_text), type(in_type)
  {}
};

std::string read_number(std::istream& input)
{
  std::string accum;
  char first = input.get();

  if (first == '-' || (isdigit(first) && first != '0')) {
    accum += first;
  } else {
    throw std::runtime_error(std::string("Expected number, got: ") + char(first));
  }

  while (isdigit(input.peek())) {
    accum += input.get();
  }

  if (input.peek() != '.') {
    return accum;
  } else {
    accum += input.get();
  }

  while (isdigit(input.peek())) {
    accum += input.get();
  }

  if ((input.peek() == 'e') || (input.peek() == 'E')) {
    accum += input.get();

    if ((input.peek() == '+') || (input.peek() == '-')) {
      accum += input.get();
    }

  }

  if (!isdigit(input.peek())) {
    throw std::runtime_error(std::string("Expected digit, got: ") + char(input.get()));
  }

  while (isdigit(input.peek())) {
    accum += input.get();
  }

  return accum;
}

std::string read_true(std::istream& input)
{
  char str[5];
  input.get(str, 5);

  if (std::string(str) != "true") {
    throw std::runtime_error(std::string("Expected true, got: ") + std::string(str));
  }

  return "true";
}

std::string read_false(std::istream& input)
{
  char str[6];
  input.get(str, 6);

  if (std::string(str) != "false") {
    throw std::runtime_error(std::string("Expected false, got: ") + std::string(str));
  }

  return "false";
}

std::string read_null(std::istream& input)
{
  char str[5];
  input.get(str, 5);

  if (std::string(str) != "null") {
    throw std::runtime_error(std::string("Expected null, got: ") + std::string(str));
  }

  return "null";
}

std::string read_string(std::istream& input)
{
  std::string accum;
  char current;

  while(input.good()) {
    current = input.get();

    if (current == '\\') {
      char escaped = input.get();
      switch(escaped) {
      case '"': 
        current = '"';
        break;
      case '\\':
        current = '\\';
        break;
      case '/':
        current = '/';
        break;
      case 'b':
        current = '\b';
        break;
      case 'f':
        current = '\f';
        break;
      case 'n':
        current = '\n';
        break;
      case 'r':
        current = '\r';
        break;
      case 't':
        current = '\t';
        break;
      case 'u':
        throw std::runtime_error("Unicode not supported");  
      default:
        throw std::runtime_error(std::string("Unexpected character in escape sequence: ") + escaped);  
      }
    }
    
    if (current == '"') {
      return accum;
    }

    accum += current;
  }

  throw std::runtime_error(std::string("Unexpected EOF"));  
}

std::deque<token> tokenize(std::istream& input)
{
  char current;
  std::deque<token> tokens;
  
  while(true) {
    while (isspace(input.peek())) {
      input.get();
    }

    current = input.get();
    std::cout << current << std::endl;

    if (!input.good()) {
      break;
    }
    
    switch(current) {
    case '{':
      tokens.emplace_back(std::string(1, current), token_type::LBRACKET);
      break;
    case '}':
      tokens.emplace_back(std::string(1, current), token_type::RBRACKET);
      break;
    case '[':
      tokens.emplace_back(std::string(1, current), token_type::RBRACE);
      break;
    case ']':
      tokens.emplace_back(std::string(1, current), token_type::RBRACE);
      break;
    case ':':
      tokens.emplace_back(std::string(1, current), token_type::COLON);
      break;
    case ',':
      tokens.emplace_back(std::string(1, current), token_type::COMMA);
      break;
    case '"':
      tokens.emplace_back(read_string(input), token_type::STRING);
      break;
    case 't':
      input.unget();
      tokens.emplace_back(read_true(input), token_type::TRUE);
      break;
    case 'f':
      input.unget();
      tokens.emplace_back(read_false(input), token_type::FALSE);
      break;      
    case 'n':
      input.unget();
      tokens.emplace_back(read_null(input), token_type::NULL_TOKEN);
      break;
    default:
      input.unget();
      tokens.emplace_back(read_number(input), token_type::NUMBER);
      break;
    }    
  }

  return tokens;
}

struct json_null {};

class value {
public:
  virtual std::optional<std::map<std::string, value>> to_object() { return std::nullopt; }
  virtual std::optional<std::vector<value>> to_array() { return std::nullopt; }
  virtual std::optional<std::string> to_string() { return std::nullopt; }
  virtual std::optional<double> to_number() { return std::nullopt; }
  virtual std::optional<bool> to_bool() { return std::nullopt; }
  virtual std::optional<json_null> to_null() { return std::nullopt; }
};

class object_value : public value {
public:
  object_value(std::map<std::string, value> in_data) :
    data(in_data)
  {}
  
  virtual std::optional<std::map<std::string, value>> to_object() { return data; }

private:
  std::map<std::string, value> data;
};

class array_value : public value {
public:
  array_value(std::vector<value> in_data) :
    data(in_data)
  {}
  
  virtual std::optional<std::vector<value>> to_array() { return data; }

private:
  std::vector<value> data;
};

class string_value : public value {
public:
  string_value(std::string in_data) :
    data(in_data)
  {}

  virtual std::optional<std::string> to_string() { return data; }
private:
  std::string data;
};

class number_value : public value {
public:
  number_value(double in_data) :
    data(in_data)
  {}

  virtual std::optional<double> to_number() { return data; }
private:
  double data;
};

class null_value : public value {
public:
  virtual std::optional<json_null> to_null() { return json_null{}; }
};

value parse_value(deque<token> tokens)
{
  auto tok = tokens.pop_front();

  switch(tok.type) {
  case token_type::STRING :
    reu
  }
}

int main()
{
  std::ifstream input("test.json");
  auto tokens = tokenize(input);

  std::cout << "Printing tokens" << std::endl;
  for (auto tok : tokens) {
    std::cout << tok.text << std::endl;
  }
}
