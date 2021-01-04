#pragma once

enum YASL_Tags {
	T_TABLE = -1,
	T_LIST = -2,
	T_FILE = -3,
	T_SET = -4
};

//Keep up to date with the YASL_TYPE_NAMES
enum YASL_Types {
	Y_END = -1,
	Y_UNDEF,
	Y_FLOAT,
	Y_INT,
	Y_BOOL,
	Y_STR,
	Y_STR_W,
	Y_LIST,
	Y_LIST_W,
	Y_TABLE,
	Y_TABLE_W,
	Y_FN,
	Y_CLOSURE,
	Y_CFN,
	Y_USERPTR,
	Y_USERDATA,
	Y_USERDATA_W,
};
