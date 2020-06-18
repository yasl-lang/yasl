#include "test/yats.h"
#include "yasl.h"
#include "yasl_aux.h"
#include "IO.h"
#include "yasl_state.h"

SETUP_YATS();

#define STR(x) #x

#define ASSERT_VALUE_ERR(code, expected, line) {\
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));\
	YASLX_decllibs(S);\
	S->vm.err.print = io_print_string;\
	S->vm.out.print = io_print_string;\
	ASSERT_SUCCESS(YASL_compile(S));\
	int result = YASL_execute(S);\
	ASSERT_EQ(result, 7);\
	const char *exp_err = "ValueError: " expected ". (line " STR(line) ")\n";\
	ASSERT_EQ(S->vm.out.len, 0);\
	ASSERT_EQ(strlen(exp_err), S->vm.err.len);\
	ASSERT_STR_EQ(exp_err, S->vm.err.string, S->vm.err.len);\
	YASL_delstate(S);\
}

#define ASSERT_DIV_BY_ZERO_ERR(code, line) {\
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));\
	YASLX_decllibs(S);\
	S->vm.err.print = io_print_string;\
	S->vm.out.print = io_print_string;\
	ASSERT_SUCCESS(YASL_compile(S));\
	int result = YASL_execute(S);\
	ASSERT_EQ(result, 6);\
	const char *exp_err = "DivisionByZeroError (line " STR(line) ")\n";\
	ASSERT_EQ(S->vm.out.len, 0);\
	ASSERT_EQ(strlen(exp_err), S->vm.err.len);\
	ASSERT_STR_EQ(exp_err, S->vm.err.string, S->vm.err.len);\
	YASL_delstate(S);\
}

#define ASSERT_TYPE_ERR(code, expected, line) {\
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));\
	YASLX_decllibs(S);\
	S->vm.err.print = io_print_string;\
	S->vm.out.print = io_print_string;\
	ASSERT_SUCCESS(YASL_compile(S));\
	int result = YASL_execute(S);\
	ASSERT_EQ(result, YASL_TYPE_ERROR);\
	const char *exp_err = "TypeError: " expected ". (line " STR(line) ")\n";\
	ASSERT_EQ(S->vm.out.len, 0);\
	ASSERT_EQ(strlen(exp_err), S->vm.err.len);\
	ASSERT_STR_EQ(exp_err, S->vm.err.string, S->vm.err.len);\
	YASL_delstate(S);\
}

#define ASSERT_UNOP_TYPE_ERR(code, op, expr, line) ASSERT_TYPE_ERR(code, op " not supported for operand of type " expr, line)
#define ASSERT_BINOP_TYPE_ERR(code, op, left, right, line) \
	ASSERT_TYPE_ERR(code, op " not supported for operands of types " left " and " right, line)
#define ASSERT_ARG_TYPE_ERR(code, method, exp, actual, arg, line) \
	ASSERT_TYPE_ERR(code, method " expected arg in position " STR(arg) \
	" to be of type " exp ", got arg of type " actual, line)

#define ASSERT_ASSERT_ERR(code, expected, line) {\
	struct YASL_State *S = YASL_newstate_bb(code, strlen(code));\
	YASLX_decllibs(S);\
	S->vm.err.print = io_print_string;\
	S->vm.out.print = io_print_string;\
	ASSERT_SUCCESS(YASL_compile(S));\
	int result = YASL_execute(S);\
	ASSERT_EQ(result, YASL_ASSERT_ERROR);\
	ASSERT_EQ(S->vm.out.len, 0);\
	const char *exp_err = "AssertError: " expected ". (line " #line ")\n";\
	ASSERT_EQ(strlen(exp_err), S->vm.err.len);\
	ASSERT_STR_EQ(exp_err, S->vm.err.string, S->vm.err.len);\
}

////////////////////////////////////////////////////////////////////////////////

int vmtest(void) {
	// assert errors
	ASSERT_ASSERT_ERR("assert false;", "false", 1);
	ASSERT_ASSERT_ERR("assert fn(a, b) { return a + b; }(1.0, 2.0) === 3;", "false", 1);
	ASSERT_ASSERT_ERR("assert undef;", "undef", 1);

	// binary operator type errors
	ASSERT_BINOP_TYPE_ERR(".true | false;", "|", "str", "bool", 1);
	ASSERT_BINOP_TYPE_ERR(".true ^ false;", "^", "str", "bool", 1);
	ASSERT_BINOP_TYPE_ERR(".true & false;", "&", "str", "bool", 1);
	ASSERT_BINOP_TYPE_ERR(".true &^ false;", "&^", "str", "bool", 1);
	ASSERT_BINOP_TYPE_ERR(".true << false;", "<<", "str", "bool", 1);
	ASSERT_BINOP_TYPE_ERR(".true >> false;", ">>", "str", "bool", 1);
	ASSERT_BINOP_TYPE_ERR(".true + false;", "+", "str", "bool", 1);
	ASSERT_BINOP_TYPE_ERR(".true - false;", "-", "str", "bool", 1);
	ASSERT_BINOP_TYPE_ERR(".true * false;", "*", "str", "bool", 1);
	ASSERT_BINOP_TYPE_ERR(".true / false;", "/", "str", "bool", 1);
	ASSERT_BINOP_TYPE_ERR(".true // false;", "//", "str", "bool", 1);
	ASSERT_BINOP_TYPE_ERR(".true % false;", "%", "str", "bool", 1);
	ASSERT_BINOP_TYPE_ERR(".true ** false;", "**", "str", "bool", 1);

	// comparison operator type errors
	ASSERT_BINOP_TYPE_ERR("true < 1;", "<", "bool", "int", 1);
	ASSERT_BINOP_TYPE_ERR("true <= 1;", "<=", "bool", "int", 1);
	ASSERT_BINOP_TYPE_ERR("true >= 1;", ">=", "bool", "int", 1);
	ASSERT_BINOP_TYPE_ERR("true > 1;", ">", "bool", "int", 1);

	// Not callable
	ASSERT_TYPE_ERR("undef();", "undef is not callable", 1);

	// unary operator type errors
	ASSERT_UNOP_TYPE_ERR("+true;", "+", "bool", 1);
	ASSERT_UNOP_TYPE_ERR("-true;", "-", "bool", 1);
	ASSERT_UNOP_TYPE_ERR("^true;", "^", "bool", 1);
	ASSERT_UNOP_TYPE_ERR("len true;", "len", "bool", 1);

	// bool method type errors
	ASSERT_ARG_TYPE_ERR("true.tostr(1);", "bool.tostr", "bool", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("true.tobool(1);", "bool.tobool", "bool", "int", 0, 1);

	// float method type errors
	ASSERT_ARG_TYPE_ERR("0.0.tostr(1);", "float.tostr", "float", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("0.0.toint(1);", "float.toint", "float", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("0.0.tofloat(1);", "float.tofloat", "float", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("0.0.tobool(1);", "float.tobool", "float", "int", 0, 1);

	// int method type errors
	ASSERT_ARG_TYPE_ERR("0.tostr(1.0);", "int.tostr", "int", "float", 0, 1);
	ASSERT_ARG_TYPE_ERR("0.toint(1.0);", "int.toint", "int", "float", 0, 1);
	ASSERT_ARG_TYPE_ERR("0.tofloat(1.0);", "int.tofloat", "int", "float", 0, 1);
	ASSERT_ARG_TYPE_ERR("0.tobool(1.0);", "int.tobool", "int", "float", 0, 1);

	// list method type errors
	ASSERT_ARG_TYPE_ERR("[].push(1, true);", "list.push", "list", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("[].copy(true);", "list.copy", "list", "bool", 0, 1);
	ASSERT_ARG_TYPE_ERR("[].__add(true, []);", "list.__add", "list", "bool", 0, 1);
	ASSERT_ARG_TYPE_ERR("[].__add([], true);", "list.__add", "list", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("[].__add(true, 1);", "list.__add", "list", "int", 1, 1);
	ASSERT_ARG_TYPE_ERR("[] + true;", "list.__add", "list", "bool", 1, 1);
	// ASSERT_ARG_TYPE_ERR("true + [];", "list.__add", "list", "bool", 0);
	// ASSERT_ARG_TYPE_ERR("[].extend(1, []);", "list.extend", "list", "int", 0, 1);
	// ASSERT_ARG_TYPE_ERR("[].extend([], 1);", "list.extend", "list", "int", 1, 1);
	// ASSERT_ARG_TYPE_ERR("[].extend(1, true);", "list.extend", "list", "bool", 1, 1);
	// TODO: __get, __set
	ASSERT_VALUE_ERR("[][2];", "unable to index list with value of type int", 1);
	ASSERT_ARG_TYPE_ERR("[].tostr(1);", "list.tostr", "list", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("[].search(1);", "list.search", "list", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("[].reverse(1);", "list.reverse", "list", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("[].clear(1);", "list.clear", "list", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("[].join(1, .str);", "list.join", "list", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("[].join(1, true);", "list.join", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("[].join([], true);", "list.join", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("[].sort(1);", "list.sort", "list", "int", 0, 1);

	// str method type errors
	ASSERT_VALUE_ERR("''[2];", "unable to index str with value of type int", 1);
	ASSERT_ARG_TYPE_ERR("''.tofloat(1);", "str.tofloat", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.toint(1);", "str.toint", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.isalnum(1);", "str.isalnum", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.isal(1);", "str.isal", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.isnum(1);", "str.isnum", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.isspace(1);", "str.isspace", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.tobool(1);", "str.tobool", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.tostr(1);", "str.tostr", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.toupper(1);", "str.toupper", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.tolower(1);", "str.tolower", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.startswith(1, true);", "str.startswith", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.startswith(1, .true);", "str.startswith", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.startswith(.str, true);", "str.startswith", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''->startswith(1);", "str.startswith", "str", "int", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.endswith(1, true);", "str.endswith", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.endswith(1, .true);", "str.endswith", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.endswith(.str, true);", "str.endswith", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''->endswith(1);", "str.endswith", "str", "int", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.replace(1, true, 1.0);", "str.replace", "str", "float", 2, 1);
	ASSERT_ARG_TYPE_ERR("''.replace(1, true, .str);", "str.replace", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.replace(1, .true, .str);", "str.replace", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.replace(1, .true, 1.0);", "str.replace", "str", "float", 2, 1);
	ASSERT_ARG_TYPE_ERR("''.replace(.tr, true, .str);", "str.replace", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.replace(.tr, .true, 1);", "str.replace", "str", "int", 2, 1);
	ASSERT_ARG_TYPE_ERR("''.replace(.tr, true, 1);", "str.replace", "str", "int", 2, 1);
	ASSERT_ARG_TYPE_ERR("''.search(1, true);", "str.search", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.search(1, .true);", "str.search", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.search(.str, true);", "str.search", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''->search(1);", "str.search", "str", "int", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.count(1, true);", "str.count", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.count(1, .true);", "str.count", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.count(.str, true);", "str.count", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''->count(1);", "str.count", "str", "int", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.split(1, true);", "str.split", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.split(1, .true);", "str.split", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.split(.str, true);", "str.split", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''->split(1);", "str.split", "str", "int", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.ltrim(1, true);", "str.ltrim", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.ltrim(1, .true);", "str.ltrim", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.ltrim(.str, true);", "str.ltrim", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''->ltrim(1);", "str.ltrim", "str", "int", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.rtrim(1, true);", "str.rtrim", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.rtrim(1, .true);", "str.rtrim", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.rtrim(.str, true);", "str.rtrim", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''->rtrim(1);", "str.rtrim", "str", "int", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.trim(1, true);", "str.trim", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.trim(1, .true);", "str.trim", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.trim(.str, true);", "str.trim", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''->trim(1);", "str.trim", "str", "int", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.rep(1, true);", "str.rep", "int", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''.rep(1, 1);", "str.rep", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("''.rep(.str, true);", "str.rep", "int", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("''->rep(true);", "str.rep", "int", "bool", 1, 1);

	// table method type errors
	ASSERT_ARG_TYPE_ERR("{}.remove(1, 2.0);", "table.remove", "table", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("{}.keys(1);", "table.keys", "table", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("{}.values(1);", "table.values", "table", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("{}.copy(1);", "table.copy", "table", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("{}.tostr(1);", "table.tostr", "table", "int", 0, 1);
	// TODO: __get, __set
	ASSERT_ARG_TYPE_ERR("{}.clear(1);", "table.clear", "table", "int", 0, 1);

	// undef method type errors
	ASSERT_ARG_TYPE_ERR("undef.tostr(1);", "undef.tostr", "undef", "int", 0, 1);

	// value errors
	ASSERT_VALUE_ERR("echo []->pop();", "list.pop expected nonempty list as arg 0", 1);
	ASSERT_VALUE_ERR("echo [1, .a]->sort();", "list.sort expected a list of all numbers or all strings", 1);
	ASSERT_VALUE_ERR("echo ''.replace(.tr, '', .sad);", "str.replace expected a nonempty str as arg 1", 1);
	ASSERT_VALUE_ERR("echo 'wasd'->split('');", "str.split expected a nonempty str as arg 1", 1);
	ASSERT_VALUE_ERR("echo 'as'->rep(-1);", "str.rep expected non-negative int as arg 1", 1);

	// division by zero errors
	ASSERT_DIV_BY_ZERO_ERR("echo 1 // 0;", 1);
	ASSERT_DIV_BY_ZERO_ERR("echo 1 % 0;", 1);

	ASSERT_TYPE_ERR("echo { []: .list };", "unable to use mutable object of type list as key", 1);
	ASSERT_TYPE_ERR("const x = {}; x[[]] = .list;", "unable to use mutable object of type list as key", 1);

	ASSERT_VALUE_ERR("const x = [ .a, .b, .c ]; x[3] = .d;", "unable to index list of length 3 with index 3", 1);

	// math type errors
	ASSERT_ARG_TYPE_ERR("math.max(1, 2, .a);", "math.max", "float", "str", 2, 1);
	ASSERT_ARG_TYPE_ERR("math.min(1, 2, .a);", "math.min", "float", "str", 2, 1);
	ASSERT_ARG_TYPE_ERR("math.max(.a, 1);", "math.max", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.min(.a, 1);", "math.min", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.abs(.str);", "math.abs", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.log(.str);", "math.log", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.exp(.str);", "math.exp", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.sqrt(.str);", "math.sqrt", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.cos(.str);", "math.cos", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.sin(.str);", "math.sin", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.tan(.str);", "math.tan", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.acos(.str);", "math.acos", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.asin(.str);", "math.asin", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.atan(.str);", "math.atan", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.ceil(.str);", "math.ceil", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.floor(.str);", "math.floor", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.deg(.str);", "math.deg", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.rad(.str);", "math.rad", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.isprime(.str);", "math.isprime", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.gcd(.str, 1);", "math.gcd", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.gcd(2, .s);", "math.gcd", "float", "str", 1, 1);
	ASSERT_ARG_TYPE_ERR("math.gcd(.a, .b);", "math.gcd", "float", "str", 1, 1);
	ASSERT_ARG_TYPE_ERR("math.lcm(.str, 1);", "math.lcm", "float", "str", 0, 1);
	ASSERT_ARG_TYPE_ERR("math.lcm(2, .s);", "math.lcm", "float", "str", 1, 1);
	ASSERT_ARG_TYPE_ERR("math.lcm(.a, .b);", "math.lcm", "float", "str", 1, 1);

	// collection errors
	ASSERT_TYPE_ERR("echo collections.table({},[]);", "unable to use mutable object of type table as key", 1);
	ASSERT_TYPE_ERR("echo collections.set([], []);", "unable to use mutable object of type list as key", 1);
	ASSERT_TYPE_ERR("const x = collections.set(); x->add([]);", "unable to use mutable object of type list as key", 1);
	ASSERT_TYPE_ERR("const x = collections.set(); collections.set().add(x, []);",
			"unable to use mutable object of type list as key", 1);
	ASSERT_ARG_TYPE_ERR("const x = collections.set(); collections.set().add([], 1);",
			    "collections.set.add", "set", "list", 0, 1);
	ASSERT_ARG_TYPE_ERR("const x = collections.set(); collections.set().remove([], 1);",
			    "collections.set.remove", "set", "list", 0, 1);
	ASSERT_ARG_TYPE_ERR("echo collections.set().tostr(1);", "collections.set.tostr", "set", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("echo collections.set().tolist(1);", "collections.set.tolist", "set", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("echo collections.set().__band(collections.set(), true);",
			    "collections.set.__band", "set", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("echo collections.set().__band(1, collections.set());",
			    "collections.set.__band", "set", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("echo collections.set().__band(1, true);", "collections.set.__band", "set", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("echo collections.set().__bor(collections.set(), true);",
			    "collections.set.__bor", "set", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("echo collections.set().__bor(1, collections.set());",
			    "collections.set.__bor", "set", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("echo collections.set().__bor(1, true);", "collections.set.__bor", "set", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("echo collections.set().__bxor(collections.set(), true);",
			    "collections.set.__bxor", "set", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("echo collections.set().__bxor(1, collections.set());",
			    "collections.set.__bxor", "set", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("echo collections.set().__bxor(1, true);", "collections.set.__bxor", "set", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("echo collections.set().__bandnot(collections.set(), true);",
			    "collections.set.__bandnot", "set", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("echo collections.set().__bandnot(1, collections.set());",
			    "collections.set.__bandnot", "set", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("echo collections.set().__bandnot(1, true);", "collections.set.__bandnot", "set", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("echo collections.set().__len(1);", "collections.set.__len", "set", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("echo collections.set().copy(1);", "collections.set.copy", "set", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("echo collections.set().clear(1);", "collections.set.clear", "set", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("echo collections.set().__get([], true);", "collections.set.__get", "set", "list", 0, 1);

	// io errors
	ASSERT_ARG_TYPE_ERR("let f = io.open('f', true);", "io.open", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("let f = io.open(1, 'w');", "io.open", "str", "int", 0, 1);
	ASSERT_ARG_TYPE_ERR("let f = io.open(1, true);", "io.open", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("let f = io.open(1);", "io.open", "str", "int", 0, 1);
	ASSERT_VALUE_ERR("let f = io.open('f', 'www');", "io.open was passed invalid mode: www", 1);
	ASSERT_VALUE_ERR("let f = io.open('f', 'y+');", "io.open was passed invalid mode: y+", 1);
	ASSERT_VALUE_ERR("let f = io.open('f', 'p');", "io.open was passed invalid mode: p", 1);
	ASSERT_ARG_TYPE_ERR("let f = io.open('test/unit_tests/test_vm/sample.txt'); f->read(false);",
			    "io.file.read", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("let f = io.open('test/unit_tests/test_vm/sample.txt'); f.read([], false);",
			    "io.file.read", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("let f = io.open('test/unit_tests/test_vm/sample.txt'); f.read(f, false);",
			    "io.file.read", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("let f = io.open('test/unit_tests/test_vm/sample.txt'); f.read([], 'l');",
			    "io.file.read", "file", "list", 0, 1);
	ASSERT_ARG_TYPE_ERR("let f = io.open('test/unit_tests/test_vm/sample.txt'); f.read([]);",
			    "io.file.read", "file", "list", 0, 1);
	ASSERT_VALUE_ERR("let f = io.open('test/unit_tests/test_vm/sample.txt'); f->read('y');",
			    "io.file.read was passed invalid mode: y", 1);
	ASSERT_VALUE_ERR("let f = io.open('test/unit_tests/test_vm/sample.txt'); f->read('ysadasd');",
			 "io.file.read was passed invalid mode: ysadasd", 1);
	ASSERT_ARG_TYPE_ERR("let f = io.open('test/unit_tests/test_vm/sample.txt'); f->write(false);",
			    "io.file.write", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("let f = io.open('test/unit_tests/test_vm/sample.txt'); f.write([], false);",
			    "io.file.write", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("let f = io.open('test/unit_tests/test_vm/sample.txt'); f.write(f, false);",
			    "io.file.write", "str", "bool", 1, 1);
	ASSERT_ARG_TYPE_ERR("let f = io.open('test/unit_tests/test_vm/sample.txt'); f.write([], 'l');",
			    "io.file.write", "file", "list", 0, 1);
	ASSERT_ARG_TYPE_ERR("let f = io.open('test/unit_tests/test_vm/sample.txt'); f.flush([]);",
			    "io.file.flush", "file", "list", 0, 1);

	// ASSERT_TYPE_ERR("mt.setmt(1, {});", "cannot set metatable for value of type int", 1);
	// ASSERT_ARG_TYPE_ERR("mt.setmt({}, 1);", "mt.setmt", "table", "int", 1, 1);
	return __YASL_TESTS_FAILED__;
}
