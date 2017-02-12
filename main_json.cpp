#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <map>
#include <stack>
#include <vector>
#include <sstream>

#include "json.h"

using namespace std;

/*
 *
 {"id": "0001", "type": "donut", "name": "Cake", "ppu": 0.55, "enable": true}

 */


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

enum
{
	tock_eof = 1024,
	tock_err,
	tock_str,
	tock_number,
	tock_true,
	tock_false,
	tock_null
};

class Value
{
public:
	Value()
	{

	}
	virtual ~Value()
	{

	}
};

typedef std::vector<Value> array_typedef;
typedef std::pair<std::string, Value> object_typedef;
typedef std::vector<object_typedef> objects_typedef;

double number;
std::string str;
typedef std::stack<std::pair<state_typedef, object_typedef>> stack_typedef;


bool parse_string(std::istream& in)
{
	char ch;

	str.clear();
	if(!in.get(ch) || ch!='"')
		return false;
	for (;;)
	{
		if(!in.get(ch))
			return false;
		if (ch == '"')
			return true;
		if (std::isalnum(ch))
			str += ch;
	}
	return false;
}

bool parse_number(std::istream& in)
{
	in >> number;
	return !in.fail();;
}

bool check(std::istream& in, char const *s)
{
	char ch;
	while(*s && in.get(ch) && ch==*s)
		s++;
	return *s==0;
}

uint32_t next(std::istream& in)
{
	char ch;

	for (;;)
	{
		if (!in.get(ch))
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
				in.unget();
				if (parse_string(in))
					return tock_str;
				return tock_err;
				break;
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				break;
			case 't':
				if(check(in, "rue"))
					return tock_true;
				return tock_err;
			case 'n':
				if(check(in, "ull"))
					return tock_null;
				return tock_err;
			case 'f':
				if(check(in, "alse"))
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
				in.unget();
				if (parse_number(in))
					return tock_number;
				return tock_err;
				break;
		}

	}
	return 0;
}

int tabs = 0;
string make_indent(int tabs)
{
	std::string str = "";

	for (int i = 0; i<tabs; i++)
	{
		str += "\t";
	}
	return str;
}

void parse_stream(std::istream& in)
{
	uint32_t c;
	state_typedef state = st_init;
	stack_typedef _stack;

	tabs = 0;


	while (state != st_done && state !=st_error)
	{
		c = next(in);

		switch (state)
		{
			case st_object_or_array_or_value_expected:
				if (c == '{')
				{
					state = st_object_key_or_close_expected;
					cout << make_indent(tabs++) << "object open" << endl;
				}
				else if (c == '[')
				{
					state = st_array_value_or_close_expected;
				}
				break;
			case st_object_key_or_close_expected:
				if (c == '}')
				{
					//pop
					cout << make_indent(tabs++) << "object open" << endl;
				}
				else if (c == tock_str)
				{
					state = st_object_colon_expected;
					cout << make_indent(tabs) << "key:" << str;
				}
				break;
			case st_object_colon_expected:
				if (c == ':')
				{
					state = st_object_value_expected;
				}
				else
					state = st_error;
				break;
			case st_object_value_expected:
				if (c == tock_str)
				{
					state = st_object_close_or_comma_expected;
					cout << " - value=" << str << endl;
				} else if (c == tock_true)
				{
					cout << " - value=true" << endl;
					state = st_object_close_or_comma_expected;
				} else if (c == tock_false)
				{
					cout << " - value=false" << endl;
					state = st_object_close_or_comma_expected;
				} else if (c == tock_number)
				{
					cout << " - value=" << number << endl;
					state = st_object_close_or_comma_expected;
				} else if (c == tock_null)
				{
					state = st_object_close_or_comma_expected;
				} else if (c == '{')
				{
					cout << endl << make_indent(tabs++) << "object open" << endl;
					state = st_object_key_or_close_expected;
				} else if (c == '[')
				{
					cout << endl << make_indent(tabs++) << "array open" << endl;
					state = st_array_value_or_close_expected;
				}
				break;
			case st_object_close_or_comma_expected:
				if (c == ',')
				{
					state = st_object_key_or_close_expected;
				} else if (c == '}')
				{
					cout << make_indent(--tabs) << "object close" << endl;
					//pop
					//state =
				}
				else
					state = st_error;
				break;
			case st_array_value_or_close_expected:
				if (c == ']')
				{
					cout << make_indent(--tabs) << "array close" << endl;
					//pop
				} if (c == tock_str)
				{
					state = st_array_close_or_comma_expected;
				}
				else if (c == tock_true)
				{
					state = st_array_close_or_comma_expected;
				}
				else if (c == tock_false)
				{
					state = st_array_close_or_comma_expected;
				}
				else if (c == tock_number)
				{
					state = st_array_close_or_comma_expected;
				}
				else if (c == tock_null)
				{
					state = st_array_close_or_comma_expected;
				}
				else if (c == '{')
				{
					state = st_object_key_or_close_expected;
				}
				else if (c == '[')
				{
					state = st_array_value_or_close_expected;
				}
				break;
			case st_array_close_or_comma_expected:
				if (c == ']')
				{
					//pop
				}
				else if (c == ',')
				{
					state = st_array_value_or_close_expected;
				}
				else
					state = st_error;

				break;
			case st_done:
			case st_error:
				break;
		}
	}
}

typedef std::map<int, string> data_typedef;

int main()
{

	void* data;
	data = new data_typedef;


	data_typedef list1 = *(static_cast<data_typedef*>(data));

	list1 = *(static_cast<data_typedef*>(data));

	list1.insert(make_pair(1, "aaa"));

	cout << list1.size() << endl;


	data_typedef list2;
	list2 = list1;

	list2.size();




	/*
	 *
	 * typedef std::vector<Value> array_typedef;
typedef std::pair<std::string, Value> object_typedef;
typedef std::map<std::string, json::Value> objects_typedef;
	 *
	 */

	double d = 45;

	std::string sss = std::to_string(d);
	cout << sss << endl;


	json::Value v__(json::js_object);

	json::objects_typedef obj = v__.object();
	std::pair<std::map<std::string, json::Value>::iterator ,bool> res1 = obj.insert(make_pair("id", json::Value(json::js_number)));
	if (res1.second)
	{
		res1.first->second.number(12);
		double d = res1.first->second.number();
		cout << "size:" << obj.size() << "=" << d << endl;
	}


	json::Value v__111(json::js_null);
	v__111.null();


	//objects_typedef obj = v.get_object();

	//cout << obj.size() << endl;

	//res1 = obj.insert(make_pair("id", json::Value(json::js_number)));







	std::map<int, string> list;


	std::pair<std::map<int, string>::iterator ,bool> res= list.insert(make_pair(1, "denis"));
	if (res.second)
		cout << "inserted" << endl;


	ifstream file("json_file//data.json");
	if (file.is_open())
	{
		json::Value value;
		json::Value::parse_stream(file, value);
		parse_stream(file);
		file.close();
	}
	else
		cout << "failed to open file" << endl;
	return 0;
}
