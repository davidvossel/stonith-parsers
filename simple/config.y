%{
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include "standalone_config.h"


extern int yylex (void);
int yyerror(const char *foo);
static void handle_line_value(void);
static void reset_line_value();

enum line_type {
	STANDALONE_LINE_DEVICE = 0,
	STANDALONE_LINE_OPTION,
	STANDALONE_LINE_PRIORITY,
	STANDALONE_LINE_PORT
};

static struct {
	enum line_type type;
	char *name;
	char *agent;
	char *keys[STANDALONE_CFG_MAX_KEYVALS];
	char *vals[STANDALONE_CFG_MAX_KEYVALS];
	char val_count;
} line_val = { 0, };

%}

%token <sval> T_VAL
%token T_DEVICE T_CONNECT T_PORTMAP T_PRIO
%token T_EQ T_ENDL T_UNFENCE T_OPTIONS
%left T_VAL

%start stuff

%union {
	char *sval;
	int ival;
}

%%

//unfline:
//	T_UNFENCE T_VAL T_ENDL {
//		printf("unfence\n");
//	}

devline:
	T_DEVICE T_VAL T_VAL T_ENDL {
		line_val.name = $2;
		line_val.agent = $3;
		line_val.type = STANDALONE_LINE_DEVICE;
		handle_line_value();
	} |
	T_DEVICE T_VAL T_VAL assigns T_ENDL {
		line_val.name = $2;
		line_val.agent = $3;
		line_val.type = STANDALONE_LINE_DEVICE;
		handle_line_value();
	}
	;

optline:
	T_OPTIONS T_VAL assigns T_ENDL {
		line_val.name = $2;
		line_val.type = STANDALONE_LINE_OPTION;
		handle_line_value();
	}
	;

prioline:
	T_PRIO T_VAL vals T_ENDL {
		line_val.name = $2;
		line_val.type = STANDALONE_LINE_PRIORITY;
		handle_line_value();
	}
	;

portline:
	T_PORTMAP T_VAL portinfo T_ENDL {
		line_val.name = $2;
		line_val.type = STANDALONE_LINE_PORT;
		handle_line_value();
	}
	;

vals:
	T_VAL vals |
	T_VAL {
		if (line_val.val_count < STANDALONE_CFG_MAX_KEYVALS) {
			line_val.vals[line_val.val_count] = $1;
			line_val.val_count++;
		}
	}
	;

portinfo:
	assign portinfo |
	T_VAL portinfo |
	assign |
	T_VAL {
		if (line_val.val_count < STANDALONE_CFG_MAX_KEYVALS) {
			line_val.vals[line_val.val_count] = $1;
			line_val.val_count++;
		}
	}
	;

assign:
	T_VAL T_EQ T_VAL {
		if (line_val.val_count < STANDALONE_CFG_MAX_KEYVALS) {
			line_val.vals[line_val.val_count] = $3;
			line_val.keys[line_val.val_count] = $1;
			line_val.val_count++;
		}
	}
	;

assigns:
	assigns assign |
	assign 
	;

stuff:
	T_ENDL stuff |
	//unfline stuff |
	devline stuff |
	portline stuff |
	optline stuff |
	prioline stuff |
	//unfline |
	portline |
	devline |
	optline |
	prioline |
	T_ENDL
	;

%%

extern int _line_count;

int
yyerror(const char *foo)
{
	printf("%s on line %d\n", foo, _line_count);
	return 0;
}

static void
reset_line_value()
{
	int i;
	free(line_val.name);
	free(line_val.agent);

	for (i = 0; i < line_val.val_count; i++) {
		free(line_val.keys[i]);
		free(line_val.vals[i]);
	}

	memset(&line_val, 0, sizeof(line_val));
}

static void
handle_line_value(void)
{
	int i;
	const char *type;

	switch (line_val.type) {
	case STANDALONE_LINE_DEVICE:
		standalone_cfg_add_device(line_val.name, line_val.agent);
		/* fall through */
	case STANDALONE_LINE_OPTION:
		for (i = 0; i < line_val.val_count; i++) {
			standalone_cfg_add_device_options(line_val.name,
				line_val.keys[i],
				line_val.vals[i]);
		}
		break;
	case STANDALONE_LINE_PRIORITY:
		for (i = 0; i < line_val.val_count; i++) {
			standalone_cfg_add_node_priority(line_val.name, 
				line_val.keys[i],
				atoi(line_val.vals[i]));
		}
		break;
	case STANDALONE_LINE_PORT:
		for (i = 0; i < line_val.val_count; i++) {
			standalone_cfg_add_node(line_val.name, 
				line_val.keys[i],
				line_val.vals[i]); /* vals may be NULL if no port specified */
		}
		break;
	}
	reset_line_value();
}

