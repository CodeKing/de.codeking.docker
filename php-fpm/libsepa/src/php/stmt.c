/*  _ _ _
 * | (_) |__ ___ ___ _ __  __ _   SEPA library · www.libsepa.com
 * | | | '_ (_-</ -_) '_ \/ _` |  Copyright (c) 2013-2014 Keppler IT GmbH.
 * |_|_|_.__/__/\___| .__/\__,_|____________________________________________
 *                  |_|
 * $Id: stmt.c 279 2016-06-07 19:54:13Z kk $
 * PHP interface to libsepa (statements parser)
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <time.h>	/* strftime() */
#include <php.h>
#include <zend_exceptions.h>
#include <ext/standard/info.h>
#include "php_verdep.h"

#include <sepa.h>
#include "stmt.h"

extern zend_class_entry *sepa_ce_exception;

zend_class_entry *sepaStP_ce;
zend_object_handlers sepaStP_object_handlers;

typedef struct {
	zend_object std;
	sepa_stmt_parser_t *parser;
} sepaStP_object;

/* ---------------------------------------------------------------------- */
static void sepaStP_free_storage(void *object TSRMLS_DC) {
	sepaStP_object *obj = (sepaStP_object*)object;
	sepa_stmt_parser_free(obj->parser);
	obj->parser = NULL;

	zend_hash_destroy(obj->std.properties);
	FREE_HASHTABLE(obj->std.properties);
	efree(obj);
}

/* ---------------------------------------------------------------------- */
static zend_object_value sepaStP_create_handler(zend_class_entry *type TSRMLS_DC) {
	zval *tmp;
	zend_object_value retval;

	sepaStP_object *obj = (sepaStP_object *)emalloc(sizeof(sepaStP_object));
	memset(obj, 0, sizeof(sepaStP_object));
	obj->std.ce = type;

	ALLOC_HASHTABLE(obj->std.properties);
	zend_hash_init(obj->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
#if PHP_VERSION_ID < 50399
	zend_hash_copy(obj->std.properties, &type->default_properties, (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));
#else
	object_properties_init((zend_object*) &(obj->std), type);
#endif
	retval.handle = zend_objects_store_put(obj, NULL, sepaStP_free_storage, NULL TSRMLS_CC);
	retval.handlers = &sepaStP_object_handlers;

	return(retval);
}

/* -------------------------------------------------------------------------
 * Constructor
 * ---------------------------------------------------------------------- */
PHP_METHOD(SEPAStatementParser, __construct) {
	sepa_stmt_parser_t *parser = NULL;
	sepaStP_object *obj = NULL;
	zval *object = getThis();

	parser = sepa_stmt_parser_new();
	if (parser == NULL) {
		zend_throw_exception(sepa_ce_exception, "can't create SEPAStatementParser object", 42 TSRMLS_CC);
		RETURN_NULL();
	}
	obj = (sepaStP_object *)zend_object_store_get_object(object TSRMLS_CC);
	obj->parser = parser;

}

/* -------------------------------------------------------------------------
 * load()
 * ---------------------------------------------------------------------- */
PHP_METHOD(SEPAStatementParser, load) {
	sepaStP_object *obj = (sepaStP_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	sepa_stmt_parser_t *parser = obj->parser;
	long format;
	char *fname = NULL;
	int fname_len;
	sepa_status_t st;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &format, &fname, &fname_len) == FAILURE) {
		RETURN_NULL();
	}

	if (format != SEPA_STMT_FORMAT_MT940) {
		zend_throw_exception(sepa_ce_exception, "invalid/unknown statement format", 42 TSRMLS_CC);
		// ###BAUSTELLE### Altenative:
		// zend_throw_exception_ex(sqlite_ce_exception, 0 TSRMLS_CC, "Could not execute %s::%s()", class_name, ce->constructor->common.function_name);
		RETURN_NULL();
	}

	st = sepa_stmt_parser_load(parser, (sepa_stmt_format_t)format, fname);
	if (st == SEPA_OK) {
		RETVAL_BOOL(1);
	} else {
		zend_throw_exception(sepa_ce_exception, (char*)sepa_stmt_parser_getError(parser), 0 TSRMLS_CC);
	}
}

/* -------------------------------------------------------------------------
 * load()
 * ---------------------------------------------------------------------- */
PHP_METHOD(SEPAStatementParser, getStatements) {
	sepaStP_object *obj = (sepaStP_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	sepa_stmt_parser_t *parser = obj->parser;
	sepa_stmt_t *stmt = NULL;

	stmt = sepa_stmt_parser_getStatement(parser);
	if (stmt == NULL) {
		RETURN_NULL();
	}

	array_init(return_value);

	while (stmt != NULL) {
		char datebuf[11];
		zval *subarray, *tx_arr;
		sepa_stmt_tx_t *tx;

		MAKE_STD_ZVAL(subarray);
		array_init(subarray);

		if (stmt->ref != NULL) add_assoc_string(subarray, "ref", stmt->ref, 1);
		if (stmt->account != NULL) add_assoc_string(subarray, "account", stmt->account, 1);
		if (stmt->number > 0) add_assoc_long(subarray, "number", stmt->number);
		if (stmt->page > 0) add_assoc_long(subarray, "page", stmt->page);
		add_assoc_double(subarray, "openSaldo", stmt->saldo_open);
		add_assoc_string(subarray, "openCurrency", stmt->curr_open, 1);
		strftime(datebuf, sizeof(datebuf), "%Y-%m-%d", &stmt->ts_open);
		add_assoc_string(subarray, "openDate", datebuf, 1);
		add_assoc_double(subarray, "closeSaldo", stmt->saldo_close);
		add_assoc_string(subarray, "closeCurrency", stmt->curr_close, 1);
		strftime(datebuf, sizeof(datebuf), "%Y-%m-%d", &stmt->ts_close);
		add_assoc_string(subarray, "closeDate", datebuf, 1);

		MAKE_STD_ZVAL(tx_arr);
		array_init(tx_arr);

		for (tx=stmt->tx_first; tx != NULL; tx=tx->next) {
			zval *txdata;

			MAKE_STD_ZVAL(txdata);
			array_init(txdata);

			strftime(datebuf, sizeof(datebuf), "%Y-%m-%d", &tx->ts_valuta);
			add_assoc_string(txdata, "valuta", datebuf, 1);
			if (tx->ts_booked.tm_year > 0) {
				strftime(datebuf, sizeof(datebuf), "%Y-%m-%d", &tx->ts_booked);
				add_assoc_string(txdata, "booked", datebuf, 1);
			}
			add_assoc_double(txdata, "amount", tx->amount);
			add_assoc_string(txdata, "code", tx->code, 1);
			if (tx->ref != NULL) add_assoc_string(txdata, "ref", tx->ref, 1);
			if (tx->bankref != NULL) add_assoc_string(txdata, "bankref", tx->ref, 1);
			add_assoc_long(txdata, "gvc", tx->gvc);
			if (tx->extcode > 0) add_assoc_long(txdata, "extcode", tx->extcode);
			if (tx->txtext != NULL) add_assoc_string(txdata, "txtext", tx->txtext, 1);
			if (tx->primanota != NULL) add_assoc_string(txdata, "primanota", tx->primanota, 1);
			add_assoc_string(txdata, "bank", tx->bank, 1);
			add_assoc_string(txdata, "account", tx->account, 1);
			add_assoc_string(txdata, "name", tx->name, 1);
			if (tx->purpose != NULL) add_assoc_string(txdata, "purpose", tx->purpose, 1);
			if (tx->eref != NULL) add_assoc_string(txdata, "eref", tx->eref, 1);
			if (tx->kref != NULL) add_assoc_string(txdata, "kref", tx->kref, 1);
			if (tx->mref != NULL) add_assoc_string(txdata, "mref", tx->mref, 1);
			if (tx->cred != NULL) add_assoc_string(txdata, "cred", tx->cred, 1);
			if (tx->debt != NULL) add_assoc_string(txdata, "debt", tx->debt, 1);
			if (tx->coam != NULL) add_assoc_string(txdata, "coam", tx->coam, 1);
			if (tx->oamt != NULL) add_assoc_string(txdata, "oamt", tx->oamt, 1);
			if (tx->svwz != NULL) add_assoc_string(txdata, "svwz", tx->svwz, 1);
			if (tx->abwa != NULL) add_assoc_string(txdata, "abwa", tx->abwa, 1);
			if (tx->abwe != NULL) add_assoc_string(txdata, "abwe", tx->abwe, 1);

			add_next_index_zval(tx_arr, txdata);

		}

		add_assoc_zval(subarray, "tx", tx_arr);

		add_next_index_zval(return_value, subarray);
		stmt = stmt->next;
	}

}

/* -------------------------------------------------------------------------
 * Method list
 * ---------------------------------------------------------------------- */
static zend_function_entry sepaStP_methods[] = {
	PHP_ME(SEPAStatementParser, __construct,	NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)		/* ZEND_ACC_CTOR: constructor! */
	PHP_ME(SEPAStatementParser, load,			NULL, ZEND_ACC_PUBLIC)						/* sepa_stmt_parser_load() */
	PHP_ME(SEPAStatementParser, getStatements,	NULL, ZEND_ACC_PUBLIC)						/* sepa_stmt_parser_getStatement() */
	{NULL, NULL, NULL}
};

void sepaStP_init(void) {
	/* register SEPA class */
	zend_class_entry ce;
	zend_class_entry ce_ex;
	TSRMLS_FETCH();

	INIT_CLASS_ENTRY(ce, "SEPAStatementParser", sepaStP_methods);
	sepaStP_ce = zend_register_internal_class(&ce TSRMLS_CC);
	sepaStP_ce->create_object = sepaStP_create_handler;
	memcpy(&sepaStP_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	sepaStP_object_handlers.clone_obj = NULL;

}

/* <EOF>_________________________________________________________________ */
