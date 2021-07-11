#ifndef YASL_YASL_TYPES_H_
#define YASL_YASL_TYPES_H_

extern const char *const TABLE_NAME;
extern const char *const LIST_NAME;

//Keep up to date with the YASL_TYPE_NAMES
enum YASL_Types {
	Y_END = -1,
	Y_UNDEF,
	Y_FLOAT,
	Y_INT,
	Y_BOOL,
	Y_STR,
	//Y_STR_W,
	Y_LIST,
	//Y_LIST_W,
	Y_TABLE,
	Y_FN,
	Y_CLOSURE,
	Y_CFN,
	Y_USERPTR,
	Y_USERDATA,
};

#endif
