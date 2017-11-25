#include <deque>
#include <stdexcept>

#include "json_parser.hpp"

namespace json_parser {

  // token impl
  enum class token_type { TRUE, FALSE, NULL_TOKEN, STRING, NUMBER, LBRACE, RBRACE, LBRACKET, RBRACKET, COLON, COMMA };

  struct token {
    std::string text;
    token_type type;

    token(std::string in_text, token_type in_type) :
      text(in_text), type(in_type)
    {}
  };

  // tokenize impl
  std::deque<token> tokenize(std::istream& input);
  std::string read_null(std::istream& input);
  std::string read_false(std::istream& input);
  std::string read_true(std::istream& input);
  std::string read_number(std::istream& input);
  std::string read_string(std::istream& input);

  std::deque<token> tokenize(std::istream& input)
  {
    char current;
    std::deque<token> tokens;

    while(true) {
      while (isspace(input.peek())) {
        input.get();
      }

      current = input.get();

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
        tokens.emplace_back(std::string(1, current), token_type::LBRACE);
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
  
  std::string read_null(std::istream& input)
  {
    char str[5];
    input.get(str, 5);

    if (std::string(str) != "null") {
      throw std::runtime_error(std::string("Expected null, got: ") + std::string(str));
    }

    return "null";
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

  std::string read_true(std::istream& input)
  {
    char str[5];
    input.get(str, 5);

    if (std::string(str) != "true") {
      throw std::runtime_error(std::string("Expected true, got: ") + std::string(str));
    }

    return "true";
  }

  std::string read_number(std::istream& input)
  {
    std::string accum;
    char first = input.get();

    if (first == '-' || (isdigit(first))) {
      accum += first;
    } else {
      throw std::runtime_error(std::string("Expected number, got: ") + char(first));
    }

    if (accum  == "0") {
      if (isdigit(input.peek())) {
        throw std::runtime_error("Invalid leading zero");      
      }
    } else {
      while (isdigit(input.peek())) {
        accum += input.get();
      }
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


  // value impl
  value value::at(std::string key)
  {
    return boost::strict_get<std::map<std::string,value>>(data).at(key);
  }

  value value::at(int i)
  {
    return boost::strict_get<std::vector<value>>(data).at(i);
  }

  std::string value::to_string()
  {
    return boost::strict_get<std::string>(data);
  }

  double value::to_number()
  {
    return boost::strict_get<double>(data);
  }

  bool value::to_bool()
  {
    return boost::strict_get<bool>(data);
  }

  bool value::is_object()
  {
    return data.which() == 5;
  }

  bool value::is_array()
  {
    return data.which() == 4;
  }

  bool value::is_string()
  {
    return data.which() == 3;
  }

  bool value::is_number()
  {
    return data.which() == 2;
  }

  bool value::is_boolean()
  {
    return data.which() == 1;
  }

  bool value::is_null()
  {
    return data.which() == 0;
  }

  // parse_json impl
  value parse_value(std::deque<token>& tokens);
  std::vector<value> read_array(std::deque<token>& tokens);
  std::map<std::string, value> read_object(std::deque<token>& tokens);

  value parse(std::istream& input)
  {
    auto tokens = tokenize(input);
    return parse_value(tokens);
  }

  value parse_value(std::deque<token>& tokens)
  {
    auto tok = tokens.front();
    tokens.pop_front();
    
    switch(tok.type) {
    case token_type::LBRACKET :
      return value(read_object(tokens));
    case token_type::LBRACE :
      return value(read_array(tokens));
    case token_type::STRING :
      return value(tok.text);
    case token_type::NUMBER :
      return value(std::stod(tok.text));
    case token_type::TRUE :
      return value(true);
    case token_type::FALSE :
      return value(false);
    case token_type::NULL_TOKEN :
      return value();
    default:
      throw std::runtime_error(std::string("Invalid token, expected value, got: ") +  tok.text);
    }
  }

  std::vector<value> read_array(std::deque<token>& tokens)
  {
    std::vector<value> values;

    while(tokens.front().type != token_type::RBRACE) {
      values.push_back(parse_value(tokens));

      if (tokens.front().type == token_type::RBRACE) {
        continue;
      }
    
      token comma = tokens.front();
      if (comma.type != token_type::COMMA) {
        throw std::runtime_error(std::string("Invalid token, expected comma, got: ") + comma.text);
      }
      tokens.pop_front();
    }

    tokens.pop_front();
    return values;
  }

  std::map<std::string, value> read_object(std::deque<token>& tokens)
  {
    std::map<std::string, value> object;

    while(tokens.front().type != token_type::RBRACKET) {
      token key = tokens.front();
      if (key.type != token_type::STRING) {
        throw std::runtime_error(std::string("Invalid token, expected string, got: ") +  key.text);
      }
      tokens.pop_front();

      token colon = tokens.front();
      if (colon.type != token_type::COLON) {
        throw std::runtime_error(std::string("Invalid token, expected colon, got: ") + colon.text);
      }
      tokens.pop_front();

      value v = parse_value(tokens);

      object[key.text] = v;

      if (tokens.front().type == token_type::RBRACKET) {
        continue;
      }

      token comma = tokens.front();
      if (comma.type != token_type::COMMA) {
        throw std::runtime_error(std::string("Invalid token, expected comma, got: ") + comma.text);
      }
      tokens.pop_front();
    }

    tokens.pop_front();
    return object;
  }
}
