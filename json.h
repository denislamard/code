#ifndef JSON_H_
#define JSON_H_

#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <map>
#include <stack>
#include <vector>
#include <sstream>
#include <cmath>

namespace json
{

typedef enum {
	js_undefined = 0,
	js_null,
	js_boolean,
	js_number,
	js_string,
	js_object,
	js_array
} json_type;

class Value;

typedef struct json_null {
public:
	std::string to_string()
	{
		return "null";
	}
} json_null;

typedef void* data_typedef;
typedef std::vector<Value> array_typedef;
typedef std::map<std::string, json::Value> objects_typedef;

template<typename T>
struct traits {
	static T get(Value const &value);
	static void set(Value &object,T const &value);
};

class json_exception : public std::exception
{
private:
    std::string _msg;
public:
    json_exception( const char * msg, int Line )
    {
        std::ostringstream oss;

        oss << "Failed line: " << Line << "=" << msg;
        _msg = oss.str();
    }

    virtual ~json_exception() throw()
    {
    }

    virtual const char * what() const throw()
    {
        return _msg.c_str();
    }
};


class Value
{
private:
	json_type _type;
	data_typedef _data;
	void init();
	void deinit();
public:
	Value();
	Value(json_type type);
	Value(Value const &other);
	Value const &operator=(Value const &other);
	virtual ~Value();
public:
	template<typename T>
	T get_value() const
	{
		return traits<T>::get(*this);
	}

	template<typename T>
	void set_value(T const &v)
	{
		traits<T>::set(*this,v);
	}
public:
	json_type type() const;
	std::string to_string();

	bool is_null() const;
	void null();

	std::string const& string() const;
	void string(std::string const &value);

	bool const& boolean() const;
	void boolean(bool const& value);

	double const& number() const;
	void number(double const& value);

	objects_typedef const& object() const;
	void object(objects_typedef const& value);

	array_typedef const& array() const;
	void array(array_typedef const &value);
public:
	static bool parse_stream(std::istream &in, Value &out);
};


/******************************************************
 * Traits Structure
 *****************************************************/

#define JSON_TRAITS_TYPE(type,method) 					\
template<>												\
struct traits<type> {									\
	static type get(Value const &object)				\
	{													\
		return object.method();							\
	}													\
	static void set(Value &object, const type &value)	\
	{													\
		object.method(value);							\
	}													\
}

JSON_TRAITS_TYPE(std::string, string);
JSON_TRAITS_TYPE(bool, boolean);
JSON_TRAITS_TYPE(double, number);
JSON_TRAITS_TYPE(uint32_t, number);
JSON_TRAITS_TYPE(int32_t, number);
JSON_TRAITS_TYPE(uint16_t, number);
JSON_TRAITS_TYPE(int16_t, number);
JSON_TRAITS_TYPE(uint8_t, number);
JSON_TRAITS_TYPE(int8_t, number);
JSON_TRAITS_TYPE(objects_typedef, object);
JSON_TRAITS_TYPE(array_typedef, array);



} /* namespace json */

#endif /* JSON_H_ */
