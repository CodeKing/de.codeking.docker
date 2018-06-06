/*  _ _ _
 * | (_) |__ ___ ___ _ __  __ _   SEPA library · www.libsepa.com
 * | | | '_ (_-</ -_) '_ \/ _` |  Copyright (c) 2013-2014 Keppler IT GmbH.
 * |_|_|_.__/__/\___| .__/\__,_|____________________________________________
 *                  |_|
 * $Id: sepa.c 291 2016-11-18 09:20:59Z kk $
 * PHP interface to libsepa
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <php.h>
#include <zend_exceptions.h>
#include <ext/standard/info.h>
#include "php_verdep.h"

#include <sepa.h>
#include "stmt.h"
 
#define PHP_SEPA_VERSION SEPA_VERSION
#define PHP_SEPA_EXTNAME "SEPA"

// #define PHP_SEPA_FEATURE_BACKTRACE
#ifdef PHP_SEPA_FEATURE_BACKTRACE
#include <syslog.h>
#include <execinfo.h>
#define _SEPA_INIT_PHP_SEGFAULT -1	/* special init constant for PHP SEGFAULT handler */
static int	btCount;				/* number of entries in address list */
static void	*btList[32];			/* backtrace address list */
#endif /* PHP_SEPA_FEATURE_BACKTRACE */

extern zend_module_entry sepa_module_entry;
#define phpext_sepa_ptr &sepa_module_entry
 
zend_class_entry *sepa_ce_exception;

zend_class_entry *sepa_ce;

zend_object_handlers sepa_object_handlers;

typedef struct {
	zend_object std;
	sepa_t *sepa;
} sepa_object;

#ifndef convert_to_cstring
#define convert_to_cstring(op) if ((op)->type != IS_STRING) { _convert_to_cstring((op) ZEND_FILE_LINE_CC); }
void _convert_to_cstring(zval *op ZEND_FILE_LINE_DC) /* {{{ */
{
	double dval;
	switch (Z_TYPE_P(op)) {
		case IS_DOUBLE: {
			char buf[20];
			TSRMLS_FETCH();
			dval = Z_DVAL_P(op);
			zval_dtor(op);
			//Z_STRLEN_P(op) = zend_spprintf(&Z_STRVAL_P(op), 0, "%.*H", (int) EG(precision), dval);
			/* %H already handles removing trailing zeros from the fractional part, yay */
			__sepa_d2str(buf, sizeof(buf), dval);
			Z_STRVAL_P(op) = estrndup_rel(buf, strlen(buf));
			Z_STRLEN_P(op) = strlen(buf);
			break;
		}
		default:
			convert_to_string(op ZEND_FILE_LINE_CC);
	}
	Z_TYPE_P(op) = IS_STRING;
}
/* }}} */
#endif

#ifdef PHP_SEPA_FEATURE_BACKTRACE
static void create_backtrace(int s) {
	char **strings;
	int i;
	btCount = backtrace(btList, sizeof(btList) / sizeof(void *));
	strings = backtrace_symbols(btList, btCount);
	for (i=0; i<btCount; i++) {
		syslog(LOG_ERR, "libsepa backtrace: [%2i] '%p' (%s)", i, btList[i], strings == NULL ? "-" : strings[i]);
	}
	if (strings != NULL) free(strings);

} /* create_backtrace() */
#endif /* PHP_SEPA_FEATURE_BACKTRACE */

/* ---------------------------------------------------------------------- */
static void sepa_free_storage(void *object TSRMLS_DC) {
	sepa_object *obj = (sepa_object*)object;
	sepa_free(obj->sepa);
	obj->sepa = NULL;

	zend_hash_destroy(obj->std.properties);
	FREE_HASHTABLE(obj->std.properties);
	efree(obj);
}

/* ---------------------------------------------------------------------- */
static zend_object_value sepa_create_handler(zend_class_entry *type TSRMLS_DC) {
	zval *tmp;
	zend_object_value retval;

	sepa_object *obj = (sepa_object *)emalloc(sizeof(sepa_object));
	memset(obj, 0, sizeof(sepa_object));
	obj->std.ce = type;

	ALLOC_HASHTABLE(obj->std.properties);
	zend_hash_init(obj->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
#if PHP_VERSION_ID < 50399
	zend_hash_copy(obj->std.properties, &type->default_properties, (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));
#else
	object_properties_init((zend_object*) &(obj->std), type);
#endif
	retval.handle = zend_objects_store_put(obj, NULL, sepa_free_storage, NULL TSRMLS_CC);
	retval.handlers = &sepa_object_handlers;

	return(retval);
}

/* -------------------------------------------------------------------------
 * Constructor
 * ---------------------------------------------------------------------- */
PHP_METHOD(SEPA, __construct) {
	sepa_t *sepa = NULL;
	sepa_object *obj = NULL;
	zval *object = getThis();
	long msgtype;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &msgtype) == FAILURE) {
		RETURN_NULL();
	}

	if (msgtype != SEPA_MSGTYPE_DDI && msgtype != SEPA_MSGTYPE_CTI) {
		zend_throw_exception(sepa_ce_exception, "invalid/unknown message type", 42 TSRMLS_CC);
		// ###BAUSTELLE### Altenative:
		// zend_throw_exception_ex(sqlite_ce_exception, 0 TSRMLS_CC, "Could not execute %s::%s()", class_name, ce->constructor->common.function_name);
		RETURN_NULL();
	}

	sepa = sepa_new(msgtype);
	if (sepa == NULL) {
		zend_throw_exception(sepa_ce_exception, "can't create SEPA message object", 42 TSRMLS_CC);
		RETURN_NULL();
	}
	obj = (sepa_object *)zend_object_store_get_object(object TSRMLS_CC);
	obj->sepa = sepa;
}

/* -------------------------------------------------------------------------
 * setIBAN()
 * ---------------------------------------------------------------------- */
PHP_METHOD(SEPA, setIBAN) {
	sepa_object *obj = (sepa_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	sepa_t *sepa = obj->sepa;
	char *iban = NULL;
	int iban_len;
	sepa_status_t st;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &iban, &iban_len) == FAILURE) {
		RETURN_NULL();
	}

	st = sepa_setIBAN(sepa, iban);
	if (st == SEPA_OK) {
		RETVAL_BOOL(1);
	} else {
		zend_throw_exception(sepa_ce_exception, (char*)sepa_getError(sepa), 0 TSRMLS_CC);
	}
}

/* -------------------------------------------------------------------------
 * setBIC()
 * ---------------------------------------------------------------------- */
PHP_METHOD(SEPA, setBIC) {
	sepa_object *obj = (sepa_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	sepa_t *sepa = obj->sepa;
	char *bic = NULL;
	int bic_len;
	sepa_status_t st;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &bic, &bic_len) == FAILURE) {
		RETURN_NULL();
	}

	st = sepa_setBIC(sepa, bic);
	if (st == SEPA_OK) {
		RETVAL_BOOL(1);
	} else {
		zend_throw_exception(sepa_ce_exception, (char*)sepa_getError(sepa), 0 TSRMLS_CC);
	}
}

/* -------------------------------------------------------------------------
 * setName()
 * ---------------------------------------------------------------------- */
PHP_METHOD(SEPA, setName) {
	sepa_object *obj = (sepa_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	sepa_t *sepa = obj->sepa;
	char *name = NULL;
	int name_len;
	sepa_status_t st;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
		RETURN_NULL();
	}

	st = sepa_setName(sepa, name);
	if (st == SEPA_OK) {
		RETVAL_BOOL(1);
	} else {
		zend_throw_exception(sepa_ce_exception, (char*)sepa_getError(sepa), 0 TSRMLS_CC);
	}
}

/* -------------------------------------------------------------------------
 * setCreditorIdentifier()
 * ---------------------------------------------------------------------- */
PHP_METHOD(SEPA, setCreditorIdentifier) {
	sepa_object *obj = (sepa_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	sepa_t *sepa = obj->sepa;
	char *ci = NULL;
	int ci_len;
	sepa_status_t st;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &ci, &ci_len) == FAILURE) {
		RETURN_NULL();
	}

	st = sepa_setCreditorIdentifier(sepa, ci);
	if (st == SEPA_OK) {
		RETVAL_BOOL(1);
	} else {
		zend_throw_exception(sepa_ce_exception, (char*)sepa_getError(sepa), 0 TSRMLS_CC);
	}
}

/* -------------------------------------------------------------------------
 * setDate()
 * ---------------------------------------------------------------------- */
PHP_METHOD(SEPA, setDate) {
	sepa_object *obj = (sepa_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	sepa_t *sepa = obj->sepa;
	char *date = NULL;
	int date_len;
	sepa_status_t st;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &date, &date_len) == FAILURE) {
		RETURN_NULL();
	}

	st = sepa_setDate(sepa, date);
	if (st == SEPA_OK) {
		RETVAL_BOOL(1);
	} else {
		zend_throw_exception(sepa_ce_exception, (char*)sepa_getError(sepa), 0 TSRMLS_CC);
	}
}

/* -------------------------------------------------------------------------
 * setDDType()
 * ---------------------------------------------------------------------- */
PHP_METHOD(SEPA, setDDType) {
	sepa_object *obj = (sepa_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	sepa_t *sepa = obj->sepa;
	long ddtype;
	sepa_status_t st;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &ddtype) == FAILURE) {
		RETURN_NULL();
	}

	st = sepa_setDDType(sepa, ddtype);
	if (st == SEPA_OK) {
		RETVAL_BOOL(1);
	} else {
		zend_throw_exception(sepa_ce_exception, (char*)sepa_getError(sepa), 0 TSRMLS_CC);
	}
}

/* -------------------------------------------------------------------------
 * setBatchBooking()
 * ---------------------------------------------------------------------- */
PHP_METHOD(SEPA, setBatchBooking) {
	sepa_object *obj = (sepa_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	sepa_t *sepa = obj->sepa;
	zend_bool mode;
	sepa_status_t st;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &mode) == FAILURE) {
		RETURN_NULL();
	}

	st = sepa_setBatchBooking(sepa, mode == 0 ? 0 : 1);
	if (st == SEPA_OK) {
		RETVAL_BOOL(1);
	} else {
		zend_throw_exception(sepa_ce_exception, (char*)sepa_getError(sepa), 0 TSRMLS_CC);
	}
}

/* -------------------------------------------------------------------------
 * add()
 * ---------------------------------------------------------------------- */
PHP_METHOD(SEPA, add) {
	sepa_object *obj = (sepa_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	sepa_t *sepa = obj->sepa;
	zval *arr, **data;
	HashTable *arr_hash;
	HashPosition pointer;
	int count;
	sepa_status_t st;
	int i=0;
	sepa_keyvalue_t *kv;
	zval *temp;
	char *idx;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &arr) == FAILURE) {
		RETURN_NULL();
	}

	arr_hash = Z_ARRVAL_P(arr);

	/* get number of elements in hash array */
	count = zend_hash_num_elements(arr_hash);

	/* heap-based arrays for libsepa key-value list, temporary ZVALs and temporary index strings */
//	sepa_keyvalue_t kv[count+1];
//	zval temp[count];
//	char idx[count][10];
	kv = (sepa_keyvalue_t*)malloc((count+1) * sizeof(sepa_keyvalue_t));
	temp = (zval*)malloc(count * sizeof(zval));
	idx = (char*)malloc(count * 10);

	/* iterate over hash array */
	for (zend_hash_internal_pointer_reset_ex(arr_hash, &pointer);
		 zend_hash_get_current_data_ex(arr_hash, (void**)&data, &pointer) == SUCCESS;
		 zend_hash_move_forward_ex(arr_hash, &pointer), i++) {
		char *key;
		int key_len;
		long index;

		/* get key name or index */
		if (zend_hash_get_current_key_ex(arr_hash, &key, &key_len, &index, 0, &pointer) == HASH_KEY_IS_STRING) {
			kv[i].key = key;
		} else {
			/* convert index number into string (static string array) */
			snprintf(&idx[i*10], 9, "%ld", index);
			kv[i].key = &idx[i*10];
		}

		/* copy value into temporary ZVAL */
		temp[i] = **data;
		zval_copy_ctor(&temp[i]);
		/* convert into string */
		convert_to_cstring(&temp[i]);
		/* set value */
		kv[i].value = Z_STRVAL(temp[i]);
	}

	/* terminate key-value list with NULL elements */
	kv[i].key = NULL;
	kv[i].value = NULL;

	/* add transaction */
	st = sepa_add(sepa, kv);

	/* release temporary ZVALs */
	for (i=0; i<count; i++) {
		zval_dtor(&temp[i]);
	}

	free(idx);
	free(temp);
	free(kv);

	if (st == SEPA_OK) {
		RETVAL_BOOL(1);
	} else {
		zend_throw_exception(sepa_ce_exception, (char*)sepa_getError(sepa), 0 TSRMLS_CC);
	}
}

/* -------------------------------------------------------------------------
 * Return SEPA XML message
 * ---------------------------------------------------------------------- */
PHP_METHOD(SEPA, toXML) {
	sepa_object *obj = (sepa_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	sepa_t *sepa = obj->sepa;
	char *xml;

	xml = sepa_toXML(sepa);
	if (xml == NULL) {
		zend_throw_exception(sepa_ce_exception, (char*)sepa_getError(sepa), 0 TSRMLS_CC);
		RETURN_NULL();
	}

	/* duplicate XML for PHP memory manager */
	RETVAL_STRING(xml, 1);
	free(xml);
}

/* -------------------------------------------------------------------------
 * Initialize libsepa settings
 * ---------------------------------------------------------------------- */
PHP_METHOD(SEPA, init) {
	long option;
	char *value = NULL;
	int value_len;
	sepa_status_t st;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &option, &value, &value_len) == FAILURE) {
		RETURN_NULL();
	}

#ifdef PHP_SEPA_FEATURE_BACKTRACE
	if (option == _SEPA_INIT_PHP_SEGFAULT) {
		/* special case for PHP: register SEGFAULT handler */
		openlog("libsepa", 0, LOG_USER);
	}
#endif /* PHP_SEPA_FEATURE_BACKTRACE */

	st = sepa_init(option, value);

	if (st == SEPA_OK) {
		RETVAL_BOOL(1);
	} else {
		RETVAL_BOOL(0);
	}
}

/* -------------------------------------------------------------------------
 * generate IBAN from account id and bank id
 * ---------------------------------------------------------------------- */
#ifdef ZEND_ENGINE_2
ZEND_BEGIN_ARG_INFO(IBAN_convert_arginfo, 0)
	ZEND_ARG_INFO(0, country)
	ZEND_ARG_INFO(0, account)
	ZEND_ARG_INFO(0, bank_id)
	ZEND_ARG_INFO(1, status)
	ZEND_END_ARG_INFO()
#else /* ZE 1 */
static unsigned char IBAN_convert_arginfo[] = { 4, BYREF_NONE, BYREF_NONE, BYREF_NONE, BYREF_FORCE };
#endif
PHP_METHOD(SEPA, IBAN_convert) {
	char *cc, *accountid = NULL, *bankid = NULL;
	int cc_len, accountid_len, bankid_len;
	zval *code = NULL;
	sepa_lookup_status_t ls;
	char iban[SEPA_IBAN_MAXLENGTH+1];
	char status;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss|z", &cc, &cc_len, &accountid, &accountid_len, &bankid, &bankid_len, &code) == FAILURE) {
		RETURN_NULL();
	}

	ls = sepa_iban_convert(cc, accountid, bankid, (char*)&iban, &status);
	if (code != NULL && Z_ISREF_P(code)) {
		/* we have a reference to a "code" variable */
		convert_to_null(code);	/* destroy the value that was passed in */
		ZVAL_LONG(code, status);
	}
	if (ls == SEPA_LOOKUP_OK) {
		RETVAL_STRING(iban, 1);
		return;
	}

	RETURN_NULL();
}

/* -------------------------------------------------------------------------
 * get BIC from IBAN
 * ---------------------------------------------------------------------- */
PHP_METHOD(SEPA, IBAN_getBIC) {
	char *iban = NULL;
	int iban_len;
	sepa_lookup_status_t ls;
	char bic[SEPA_BIC_LENGTH+1];

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &iban, &iban_len) == FAILURE) {
		RETURN_NULL();
	}

	ls = sepa_iban_getBIC(iban, (char*)&bic);
	if (ls == SEPA_LOOKUP_OK) {
		RETVAL_STRING(bic, 1);
		return;
	}

	RETURN_NULL();
}

/* -------------------------------------------------------------------------
 * check IBAN
 * ---------------------------------------------------------------------- */
PHP_METHOD(SEPA, IBAN_check) {
	char *iban = NULL;
	int iban_len;
	sepa_lookup_status_t ls;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &iban, &iban_len) == FAILURE) {
		RETURN_NULL();
	}

	ls = sepa_iban_check(iban);
	if (ls == SEPA_LOOKUP_OK) {
		RETVAL_BOOL(1);
	} else {
		RETVAL_BOOL(0);
	}
}

/* -------------------------------------------------------------------------
 * get bank name from SCL (using BIC)
 * ---------------------------------------------------------------------- */
PHP_METHOD(SEPA, BIC_getBankName) {
	char *bic = NULL;
	int bic_len;
	sepa_bankinfo_t *bank;
	sepa_status_t st;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &bic, &bic_len) == FAILURE) {
		RETURN_NULL();
	}

	st = sepa_bic_getBank(bic, &bank);
	if (st == SEPA_OK) {
		RETVAL_STRING(bank->name, 1);
		free(bank);
		return;
	}

	RETURN_NULL();
}

/* -------------------------------------------------------------------------
 * get SCL flags (using BIC)
 * ---------------------------------------------------------------------- */
PHP_METHOD(SEPA, BIC_getBankFlags) {
	char *bic = NULL;
	int bic_len;
	sepa_bankinfo_t *bank;
	sepa_status_t st;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &bic, &bic_len) == FAILURE) {
		RETURN_NULL();
	}

	st = sepa_bic_getBank(bic, &bank);
	if (st == SEPA_OK) {
		RETVAL_LONG((long)bank->flags);
		free(bank);
		return;
	}

	RETURN_NULL();
}

/* -------------------------------------------------------------------------
 * Method list
 * ---------------------------------------------------------------------- */
zend_function_entry sepa_methods[] = {
	PHP_ME(SEPA, __construct,			NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)		/* ZEND_ACC_CTOR: constructor! */
	PHP_ME(SEPA, setIBAN,				NULL, ZEND_ACC_PUBLIC)						/* sepa_setIBAN() */
	PHP_ME(SEPA, setBIC,				NULL, ZEND_ACC_PUBLIC)						/* sepa_setBIC() */
	PHP_ME(SEPA, setName,				NULL, ZEND_ACC_PUBLIC)						/* sepa_setName() */
	PHP_ME(SEPA, setCreditorIdentifier,	NULL, ZEND_ACC_PUBLIC)						/* sepa_setCreditorIdentifier() */
	PHP_ME(SEPA, setDate,				NULL, ZEND_ACC_PUBLIC)						/* sepa_setDate() */
	PHP_ME(SEPA, setDDType,				NULL, ZEND_ACC_PUBLIC)						/* sepa_setDDType() */
	PHP_ME(SEPA, setBatchBooking,		NULL, ZEND_ACC_PUBLIC)						/* sepa_setBatchBooking() */
	PHP_ME(SEPA, add,					NULL, ZEND_ACC_PUBLIC)						/* sepa_add() */
	PHP_ME(SEPA, toXML,					NULL, ZEND_ACC_PUBLIC)						/* sepa_toXML() */
	PHP_ME(SEPA, init,					NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)	/* sepa_init() */
	PHP_ME(SEPA, IBAN_convert,			IBAN_convert_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)	/* sepa_iban_convert() */
	PHP_ME(SEPA, IBAN_getBIC,			NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)	/* sepa_iban_getBIC() */
	PHP_ME(SEPA, IBAN_check,			NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)	/* sepa_iban_check() */
	PHP_ME(SEPA, BIC_getBankName,		NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)	/* sepa_bic_getBank() */
	PHP_ME(SEPA, BIC_getBankFlags,		NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)	/* sepa_bic_getBank() */
	{NULL, NULL, NULL}
};

/* -------------------------------------------------------------------------
 * Register SEPA class
 * ---------------------------------------------------------------------- */
PHP_MINIT_FUNCTION(sepa) {
	/* register SEPA class */
	zend_class_entry ce;
	zend_class_entry ce_ex;

	INIT_CLASS_ENTRY(ce, "SEPA", sepa_methods);
	sepa_ce = zend_register_internal_class(&ce TSRMLS_CC);
	sepa_ce->create_object = sepa_create_handler;
	memcpy(&sepa_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	sepa_object_handlers.clone_obj = NULL;

	/* register exception class */
	INIT_CLASS_ENTRY(ce_ex, "SEPAException", NULL);
	sepa_ce_exception = zend_register_internal_class_ex(&ce_ex, (zend_class_entry*)zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);

	/* register constants */
	REGISTER_LONG_CONSTANT("SEPA_DDTYPE_CORE",		SEPA_DDTYPE_CORE,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SEPA_DDTYPE_COR1",		SEPA_DDTYPE_COR1,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SEPA_DDTYPE_B2B",		SEPA_DDTYPE_B2B,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SEPA_MSGTYPE_CTI",		SEPA_MSGTYPE_CTI,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SEPA_MSGTYPE_DDI",		SEPA_MSGTYPE_DDI,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SEPA_INIT_LICUSER",		SEPA_INIT_LICUSER,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SEPA_INIT_LICCODE",		SEPA_INIT_LICCODE,	CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SEPA_INIT_RULEBOOK",	SEPA_INIT_RULEBOOK,	CONST_CS | CONST_PERSISTENT);
#ifdef PHP_SEPA_FEATURE_BACKTRACE
	REGISTER_LONG_CONSTANT("SEPA_INIT_PHP_SEGFAULT",	_SEPA_INIT_PHP_SEGFAULT,	CONST_CS | CONST_PERSISTENT);
#endif /* PHP_SEPA_FEATURE_BACKTRACE */
	REGISTER_LONG_CONSTANT("SEPA_SCL_SCT",			SEPA_SCL_SCT,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SEPA_SCL_SDD",			SEPA_SCL_SDD,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SEPA_SCL_COR1",			SEPA_SCL_COR1,		CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SEPA_SCL_B2B",			SEPA_SCL_B2B,		CONST_CS | CONST_PERSISTENT);

	sepaStP_init();
	REGISTER_LONG_CONSTANT("SEPA_STMT_FORMAT_MT940",	SEPA_STMT_FORMAT_MT940,		CONST_CS | CONST_PERSISTENT);

	return(SUCCESS);
}

/* -------------------------------------------------------------------------
 * Display informations about this extension in phpinfo()
 * ---------------------------------------------------------------------- */
PHP_MINFO_FUNCTION(sepa) {
	php_info_print_table_start();
	php_info_print_table_header(2, "libsepa", "enabled");
	php_info_print_table_row(2, "Version", SEPA_VERSION);
	php_info_print_table_end();
}

/* -------------------------------------------------------------------------
 * Module entry
 * ---------------------------------------------------------------------- */
// the following code creates an entry for the module and registers it with Zend.
zend_module_entry sepa_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_SEPA_EXTNAME,
    NULL,				/* functions */
    PHP_MINIT(sepa),	/* name of the MINIT function or NULL if not applicable */
    NULL,				/* name of the MSHUTDOWN function or NULL if not applicable */
    NULL,				/* name of the RINIT function or NULL if not applicable */
    NULL,				/* name of the RSHUTDOWN function or NULL if not applicable */
    PHP_MINFO(sepa),	/* name of the MINFO function or NULL if not applicable */
#if ZEND_MODULE_API_NO >= 20010901
    PHP_SEPA_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};
 
ZEND_GET_MODULE(sepa)
 
/* <EOF>_________________________________________________________________ */
