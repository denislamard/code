#include "json.h"

namespace json
{

/****************************************************************
* tockenizer class
****************************************************************/
namespace
{

enum {
	tock_eof = 255,
	tock_err,
	tock_str,
	tock_number,
	tock_true,
	tock_false,
	tock_null
};

typedef enum {
	st_init = 0,
	st_object_or_array_or_value_expected = 0 ,
	st_object_key_or_close_expected,
	st_object_colon_expected,
	st_object_value_expected,
	st_object_close_or_comma_expected,
	st_array_value_or_close_expected,
	st_array_close_or_comma_expected,
	st_error,
	st_done
} state_typedef;

struct tockenizer {
private:
	std::istream &_in;
private:
	bool check(const char* value)
	{
		char ch;

		while(*value && _in.get(ch) && ch==*value)
			value++;
		return *value==0;
	}
	bool parse_number()
	{
		_in >> number;
		return !_in.fail();
	}
	bool parse_string()
	{
		char ch;

		string.clear();
		if(!_in.get(ch) || ch!='"')
			return false;
		for (;;)
		{
			if(!_in.get(ch))
				return false;
			if (ch == '"')
				return true;
			if (std::isalnum(ch))
				string += ch;
		}
		return false;
	}
public:
	tockenizer(std::istream &in)
	:_in(in), number(0)
	{
	}
	uint32_t next()
	{
		char ch;

		for (;;)
		{
			if (!_in.get(ch))
				return tock_eof;
			switch (ch) {
				case '{':
				case '}':
				case '[':
				case ']':
				case ':':
				case ',':
					return ch;
					break;
				case '"':
					_in.unget();
					if (parse_string())
						return tock_str;
					return tock_err;
				case ' ':
				case '\t':
				case '\r':
				case '\n':
					break;
				case 't':
					if(check("rue"))
						return tock_true;
					return tock_err;
				case 'n':
					if(check("ull"))
						return tock_null;
					return tock_err;
				case 'f':
					if(check("alse"))
						return tock_false;
					return tock_err;
				case '-':
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					_in.unget();
					if (parse_number())
						return tock_number;
					return tock_err;
				default:
					return tock_err;
			}
		}
		return tock_err;
	}
public:
	std::string string;
	double number;
};

} /* anonymous */

/****************************************************************
* Value class
****************************************************************/
Value::Value()
{
	_type = js_undefined;
	_data = nullptr;
}

Value::Value(json_type type)
{
	_type = type;
	_data = nullptr;
	init();
}

Value::Value(Value const &other)
{
	if (&other != this)
	{
		_type = other._type;
		_data = other._data;
	}
}

Value const& Value::operator=(Value const &other)
{
	return *this;
}

Value::~Value()
{

}

json_type Value::type() const
{
	return _type;
}

std::string Value::to_string()
{
	if (_type == js_null)
		return "null";
	else if (_type == js_number)
	{
		double d = number();
		char buf[64];

		if (std::floor(d) == d)
			snprintf(buf, 64, "%d", (int)d);
		else
			sprintf(buf, "%f", d);
		return std::string(buf);
	}
	else if (_type == js_boolean)
		return boolean() == true ? "true" : "false";
	else if (_type == js_string)
		return string();
	else if (_type == js_object)
	{
		return "object";
	}
	else if (_type == js_array)
	{
		return "array";
	}
	else
		throw json_exception("failed to convert Value to string", __LINE__);
}

bool Value::is_null() const
{
	if (_data != nullptr && _type == js_null)
		return _type == js_null ? true : false;
	else
		throw json_exception("Bad json type", __LINE__);
}

void Value::null()
{
	if (_data != nullptr && _type == js_null)
		*(static_cast<json_null*>(_data)) = json_null();
	else
		throw json_exception("Bad json type", __LINE__);
}

std::string const& Value::string() const
{
	if (_data != nullptr && _type == js_string)
		return *(static_cast<std::string*>(_data));
	else
		throw json_exception("Bad json type", __LINE__);
}

void Value::string(std::string const &value)
{
	if (_data != nullptr && _type == js_string)
		*(static_cast<std::string*>(_data)) = value;
	else
		throw json_exception("Bad json type", __LINE__);
}

bool const& Value::boolean() const
{
	if (_data != nullptr && _type == js_boolean)
		return *(static_cast<bool*>(_data));
	else
		throw json_exception("Bad json type", __LINE__);
}

void Value::boolean(bool const& value)
{
	if (_data != nullptr && _type == js_boolean)
		*(static_cast<bool*>(_data)) = value;
	else
		throw json_exception("Bad json type", __LINE__);
}

double const& Value::number() const
{
	if (_data != nullptr && _type == js_number)
		return *(static_cast<double*>(_data));
	else
		throw json_exception("Bad json type", __LINE__);
}

void Value::number(double const& value)
{
	if (_data != nullptr && _type == js_number)
		*(static_cast<double*>(_data)) = value;
	else
		throw json_exception("Bad json type", __LINE__);
}

objects_typedef const& Value::object() const
{
	if (_data != nullptr && _type == js_object)
		return *(static_cast<objects_typedef*>(_data));
	else
		throw json_exception("Bad json type", __LINE__);
}

void Value::object(objects_typedef const& value)
{
	if (_data != nullptr && _type == js_object)
		*(static_cast<objects_typedef*>(_data)) = value;
	else
		throw json_exception("Bad json type", __LINE__);
}

array_typedef const& Value::array() const
{
	if (_data != nullptr && _type == js_array)
		return *(static_cast<array_typedef*>(_data));
	else
		throw json_exception("Bad json type", __LINE__);
}

void Value::array(array_typedef const &value)
{
	if (_data != nullptr && _type == js_array)
		*(static_cast<array_typedef*>(_data)) = value;
	else
		throw json_exception("Bad json type", __LINE__);
}

bool Value::parse_stream(std::istream &in, Value &out)
{
	uint32_t c;
	state_typedef state = st_init;
	tockenizer token(in);
	Value &root = out;

	while (state != st_done && state !=st_error)
	{
		c = token.next();
		switch (state)
		{
			case st_object_or_array_or_value_expected:
				break;
			case st_object_key_or_close_expected:
				break;
			case st_object_colon_expected:
				break;
			case st_object_value_expected:
				break;
			case st_object_close_or_comma_expected:
				break;
			case st_array_value_or_close_expected:
				break;
			case st_array_close_or_comma_expected:
				break;
			case st_done:
			case st_error:
				break;
		}
	}
	if(state==st_done)
	{
		return true;
	}
	return false;
}

void Value::init()
{
	switch (_type) {
		case js_null:
			_data = new json_null();
			break;
		case js_boolean:
			_data = new bool;
			break;
		case js_number:
			_data = new double;
			break;
		case js_string:
			_data = new std::string();
			break;
		case js_object:
			_data = new objects_typedef();
			break;
		case js_array:
			_data = new array_typedef();
			break;
		default:
			throw;
	}
}

void Value::deinit()
{
	switch (_type) {
		case js_null:
			delete (static_cast<json_null*>(_data));
			break;
		case js_boolean:
			delete (static_cast<bool*>(_data));
			break;
		case js_number:
			delete (static_cast<double*>(_data));
			break;
		case js_string:
			delete (static_cast<std::string*>(_data));
			break;
		case js_object:
			delete (static_cast<objects_typedef*>(_data));
			break;
		case js_array:
			delete (static_cast<array_typedef*>(_data));
			break;
		default:
			throw;
	}
}

} /* namespace json */
