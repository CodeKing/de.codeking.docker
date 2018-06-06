/*  _ _ _
 * | (_) |__ ___ ___ _ __  __ _   SEPA library · www.libsepa.com
 * | | | '_ (_-</ -_) '_ \/ _` |  Copyright (c) 2013-2014 Keppler IT GmbH.
 * |_|_|_.__/__/\___| .__/\__,_|____________________________________________
 *                  |_|
 * $Id: SEPA.xs 277 2016-06-07 18:34:45Z kk $
 */

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include <sepa.h>

typedef sepa_t * SEPA;
typedef sepa_stmt_parser_t * SEPA__StatementParser;
typedef int bool_t;

__attribute__((destructor))
static void destroy_libsepa() {
  sepa_cleanup();
}

MODULE = SEPA		PACKAGE = SEPA		PREFIX = sepa_

# PROTOTYPES: ENABLE

BOOT:
{
#define IV_CONST(X) newCONSTSUB(stash, #X, newSViv(X))
	HV *stash = gv_stashpv("SEPA", TRUE);
	IV_CONST(SEPA_MSGTYPE_CTI);
	IV_CONST(SEPA_MSGTYPE_DDI);
	IV_CONST(SEPA_DDTYPE_CORE);
	IV_CONST(SEPA_DDTYPE_COR1);
	IV_CONST(SEPA_DDTYPE_B2B);
	IV_CONST(SEPA_INIT_LICUSER);
	IV_CONST(SEPA_INIT_LICCODE);
	IV_CONST(SEPA_INIT_RULEBOOK);
	IV_CONST(SEPA_SCL_SCT);
	IV_CONST(SEPA_SCL_SDD);
	IV_CONST(SEPA_SCL_COR1);
	IV_CONST(SEPA_SCL_B2B);
}

# __________________________________________________________________________
sepa_status_t
sepa_init(option, value)
	sepa_init_t option
	const char *value

# __________________________________________________________________________
char*
sepa_IBAN_convert(country, account, bankid, ...)
	const char *country
	const char *account
	const char *bankid
PPCODE:
	char iban[SEPA_IBAN_MAXLENGTH+1];
	char status;

	if (sepa_iban_convert(country, account, bankid, (char*)&iban, &status) == SEPA_LOOKUP_OK) {
		XPUSHs(sv_2mortal(newSVpv(iban, 0)));
	}
	if (items > 3 && SvROK(ST(3))) {
		sv_setsv(SvRV(ST(3)), newSViv(status));
	}

# __________________________________________________________________________
char*
sepa_IBAN_getBIC(iban)
	const char *iban
PPCODE:
	char bic[SEPA_BIC_LENGTH+1];
	if (sepa_iban_getBIC(iban, (char*)&bic) == SEPA_LOOKUP_OK) {
		EXTEND(SP, 1);
		PUSHs(sv_2mortal(newSVpv(bic, 0)));
	}

# __________________________________________________________________________
bool_t
sepa_IBAN_check(iban)
	const char *iban
CODE:
	RETVAL = sepa_iban_check(iban) > 0 ? 1 : 0;
OUTPUT:
	RETVAL

# __________________________________________________________________________
char*
sepa_BIC_getBankName(bic)
	const char *bic
PPCODE:
	sepa_bankinfo_t *bank;
	if (sepa_bic_getBank(bic, &bank) == SEPA_OK) {
		EXTEND(SP, 1);
		PUSHs(sv_2mortal(newSVpv(bank->name, 0)));
		free(bank);
	}

# __________________________________________________________________________
int
sepa_BIC_getBankFlags(bic)
	const char *bic
PPCODE:
	sepa_bankinfo_t *bank;
	if (sepa_bic_getBank(bic, &bank) == SEPA_OK) {
		EXTEND(SP, 1);
		PUSHs(sv_2mortal(newSViv(bank->flags)));
		free(bank);
	}

# __________________________________________________________________________
SEPA
sepa_new(class, msgtype)
	char *class
	sepa_msgtype_t msgtype
CODE:
	RETVAL = sepa_new(msgtype);
	if (RETVAL == NULL) {
		croak("Out of memory for %s", class);
	}
OUTPUT:
	RETVAL

# __________________________________________________________________________
void
sepa_DESTROY(sepa)
	SEPA sepa
CODE:
	sepa_free(sepa);

# __________________________________________________________________________
sepa_status_t
sepa_setIBAN(sepa, iban)
	SEPA sepa
	const char *iban
POSTCALL:
	if (RETVAL != SEPA_OK) {
		croak("%s", sepa_getError(sepa));
	}

# __________________________________________________________________________
sepa_status_t
sepa_setBIC(sepa, bic)
	SEPA sepa
	const char *bic
POSTCALL:
	if (RETVAL != SEPA_OK) {
		croak("%s", sepa_getError(sepa));
	}

# __________________________________________________________________________
sepa_status_t
sepa_setName(sepa, name)
	SEPA sepa
	const char *name
POSTCALL:
	if (RETVAL != SEPA_OK) {
		croak("%s", sepa_getError(sepa));
	}

# __________________________________________________________________________
sepa_status_t
sepa_setCreditorIdentifier(sepa, ci)
	SEPA sepa
	const char *ci
POSTCALL:
	if (RETVAL != SEPA_OK) {
		croak("%s", sepa_getError(sepa));
	}

# __________________________________________________________________________
sepa_status_t
sepa_setDate(sepa, date)
	SEPA sepa
	const char *date
POSTCALL:
	if (RETVAL != SEPA_OK) {
		croak("%s", sepa_getError(sepa));
	}

# __________________________________________________________________________
sepa_status_t
sepa_setDDType(sepa, ddtype)
	SEPA sepa
	sepa_ddtype_t ddtype
POSTCALL:
	if (RETVAL != SEPA_OK) {
		croak("%s", sepa_getError(sepa));
	}

# __________________________________________________________________________
sepa_status_t
sepa_setBatchBooking(sepa, mode)
	SEPA sepa
	unsigned char mode
POSTCALL:
	if (RETVAL != SEPA_OK) {
		croak("%s", sepa_getError(sepa));
	}

# __________________________________________________________________________
sepa_status_t
sepa_add(sepa, hash)
	SEPA sepa
	HV * hash
PREINIT:
	sepa_status_t st;
	HE *elem;
	char *key;
	I32 keylen;
	SV *value;
	int i, count;
CODE:
	count = HvUSEDKEYS(hash);
	hv_iterinit(hash);
	sepa_keyvalue_t kv[count+1];
	for (i=0; i<count; i++) {
		value = hv_iternextsv(hash, &key, &keylen);
		kv[i].key = key;
		kv[i].value = SvPV_nolen(value);
	}
	kv[i].key = NULL;
	kv[i].value = NULL;
	RETVAL = sepa_add(sepa, kv);
OUTPUT:
	RETVAL
POSTCALL:
	if (RETVAL != SEPA_OK) {
		croak("Can't add transaction: %s", sepa_getError(sepa));
	}

# __________________________________________________________________________
const char *
sepa_toXML(sepa)
	SEPA sepa
PPCODE:
	char *xml = sepa_toXML(sepa);
	if (xml == NULL) {
		croak("%s", sepa_getError(sepa));
	}
	EXTEND(SP, 1);
	PUSHs(sv_2mortal(newSVpv(xml, 0)));
	free(xml);

# --------------------------------------------------------------------------
# SEPA::StatementParser
# --------------------------------------------------------------------------
MODULE = SEPA		PACKAGE = SEPA::StatementParser		PREFIX = sepa_stmt_parser_

BOOT:
{
#define IV_CONST(X) newCONSTSUB(stash, #X, newSViv(X))
	HV *stash = gv_stashpv("SEPA::StatementParser", TRUE);
	IV_CONST(SEPA_STMT_FORMAT_MT940);
}

# __________________________________________________________________________
SEPA::StatementParser
sepa_stmt_parser_new(class)
	char *class
CODE:
	RETVAL = sepa_stmt_parser_new();
	if (RETVAL == NULL) {
		croak("Out of memory for %s", class);
	}
OUTPUT:
	RETVAL

# __________________________________________________________________________
void
sepa_stmt_parser_DESTROY(parser)
	SEPA::StatementParser parser
CODE:
	sepa_stmt_parser_free(parser);

# __________________________________________________________________________
sepa_status_t
sepa_stmt_parser_load(parser, fmt, filename)
	SEPA::StatementParser parser
	sepa_stmt_format_t fmt
	const char *filename
POSTCALL:
	if (RETVAL != SEPA_OK) {
		croak("%s", sepa_stmt_parser_getError(parser));
	}

# __________________________________________________________________________
AV *
sepa_stmt_parser_getStatements(parser)
	SEPA::StatementParser parser
CODE:
	sepa_stmt_t *stmt;
	stmt = sepa_stmt_parser_getStatement(parser);
	if (stmt == NULL) {
		croak("%s", sepa_stmt_parser_getError(parser));
	}
	RETVAL = newAV();
	while (stmt != NULL) {
		SV *sv = NULL;		// temporary Scalar Value
		HV *hv = newHV();	// Hash Value for statement
		AV *atx = newAV();	// Array Value for transactions
		char datebuf[11];
		sepa_stmt_tx_t *tx;

		if (stmt->ref != NULL) hv_store(hv, "ref", 3, newSVpv(stmt->ref, strlen(stmt->ref)), 0);
		if (stmt->account != NULL) hv_store(hv, "account", 7, newSVpv(stmt->account, strlen(stmt->account)), 0);
		if (stmt->number > 0) hv_store(hv, "number", 6, newSViv(stmt->number), 0);
		if (stmt->page > 0) hv_store(hv, "page", 4, newSViv(stmt->page), 0);
		hv_store(hv, "openSaldo", 9, newSVnv(stmt->saldo_open), 0);
		hv_store(hv, "openCurrency", 12, newSVpv(stmt->curr_open, strlen(stmt->curr_open)), 0);
		strftime(datebuf, sizeof(datebuf), "%Y-%m-%d", &stmt->ts_open);
		hv_store(hv, "openDate", 8, newSVpv(datebuf, strlen(datebuf)), 0);
		hv_store(hv, "closeSaldo", 10, newSVnv(stmt->saldo_close), 0);
		hv_store(hv, "closeCurrency", 13, newSVpv(stmt->curr_close, strlen(stmt->curr_close)), 0);
		strftime(datebuf, sizeof(datebuf), "%Y-%m-%d", &stmt->ts_close);
		hv_store(hv, "closeDate", 8, newSVpv(datebuf, strlen(datebuf)), 0);

		for (tx=stmt->tx_first; tx != NULL; tx=tx->next) {
			HV *txdata = newHV();		// new Hash Value for transaction details

			strftime(datebuf, sizeof(datebuf), "%Y-%m-%d", &tx->ts_valuta);
			hv_store(txdata, "valuta", 6, newSVpv(datebuf, strlen(datebuf)), 0);
			if (tx->ts_booked.tm_year > 0) {
				strftime(datebuf, sizeof(datebuf), "%Y-%m-%d", &tx->ts_booked);
				hv_store(txdata, "booked", 6, newSVpv(datebuf, strlen(datebuf)), 0);
			}
			hv_store(txdata, "amount", 6, newSVnv(tx->amount), 0);
			hv_store(txdata, "code", 4, newSVpv(tx->code, strlen(tx->code)), 0);
			if (tx->ref != NULL) hv_store(txdata, "ref", 3, newSVpv(tx->ref, strlen(tx->ref)), 0);
			if (tx->bankref != NULL) hv_store(txdata, "bankref", 7, newSVpv(tx->bankref, strlen(tx->bankref)), 0);
			hv_store(txdata, "gvc", 3, newSViv(tx->gvc), 0);
			if (tx->extcode > 0) hv_store(txdata, "extcode", 7, newSViv(tx->extcode), 0);
			if (tx->txtext != NULL) hv_store(txdata, "txtext", 6, newSVpv(tx->txtext, strlen(tx->txtext)), 0);
			if (tx->primanota != NULL) hv_store(txdata, "primanota", 9, newSVpv(tx->primanota, strlen(tx->primanota)), 0);
			hv_store(txdata, "bank", 4, newSVpv(tx->bank, strlen(tx->bank)), 0);
			hv_store(txdata, "account", 7, newSVpv(tx->account, strlen(tx->account)), 0);
			hv_store(txdata, "name", 4, newSVpv(tx->name, strlen(tx->name)), 0);
			if (tx->purpose != NULL) hv_store(txdata, "purpose", 7, newSVpv(tx->purpose, strlen(tx->purpose)), 0);
			if (tx->eref != NULL) hv_store(txdata, "eref", 4, newSVpv(tx->eref, strlen(tx->eref)), 0);
			if (tx->kref != NULL) hv_store(txdata, "kref", 4, newSVpv(tx->kref, strlen(tx->kref)), 0);
			if (tx->mref != NULL) hv_store(txdata, "mref", 4, newSVpv(tx->mref, strlen(tx->mref)), 0);
			if (tx->cred != NULL) hv_store(txdata, "cred", 4, newSVpv(tx->cred, strlen(tx->cred)), 0);
			if (tx->debt != NULL) hv_store(txdata, "debt", 4, newSVpv(tx->debt, strlen(tx->debt)), 0);
			if (tx->coam != NULL) hv_store(txdata, "coam", 4, newSVpv(tx->coam, strlen(tx->coam)), 0);
			if (tx->oamt != NULL) hv_store(txdata, "oamt", 4, newSVpv(tx->oamt, strlen(tx->oamt)), 0);
			if (tx->svwz != NULL) hv_store(txdata, "svwz", 4, newSVpv(tx->svwz, strlen(tx->svwz)), 0);
			if (tx->abwa != NULL) hv_store(txdata, "abwa", 4, newSVpv(tx->abwa, strlen(tx->abwa)), 0);
			if (tx->abwe != NULL) hv_store(txdata, "abwe", 4, newSVpv(tx->abwe, strlen(tx->abwe)), 0);

			sv = newRV_noinc((SV*)txdata);
			av_push(atx, sv);
		}

		sv = newRV_noinc((SV*)atx);		// get reference to transaction array without incrementing reference counter
		hv_store(hv, "tx", 2, sv, 0);	// store Array reference into statement hash

		sv = newRV_noinc((SV*)hv);	// get reference to Hash Value without incrementing reference counter
		av_push(RETVAL, sv);		// store Hash reference into array
		stmt = stmt->next;
	}
	sv_2mortal((SV*)RETVAL);
OUTPUT:
	RETVAL


# __________________________________________________________________________
