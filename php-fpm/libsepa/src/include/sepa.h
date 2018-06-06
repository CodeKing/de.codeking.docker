/*  _ _ _
 * | (_) |__ ___ ___ _ __  __ _   SEPA library · www.libsepa.com
 * | | | '_ (_-</ -_) '_ \/ _` |  Copyright (c) 2013-2014 Keppler IT GmbH.
 * |_|_|_.__/__/\___| .__/\__,_|____________________________________________
 *                  |_|
 * $Id: sepa.h 317 2018-03-21 20:33:55Z kk $
 */

/*! \file sepa.h
 *  \brief libsepa header (C/C++ API)
 *
 *  This include file defines all exported functions required for using libsepa.
 */

#ifndef __SEPA_H
#define __SEPA_H

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SEPA_BIC_LENGTH 11
#define SEPA_IBAN_MAXLENGTH 34		/*!< maximum length of a valid IBAN number */

typedef struct sepa_t sepa_t;		/*!< abstract type for a libsepa instance handle */

typedef enum {
	SEPA_ERROR = 0,
	SEPA_OK = 1
} sepa_status_t;

typedef enum {
	SEPA_LOOKUP_OK			= 1,
	SEPA_LOOKUP_NOTSUPPORTED = 0,
	SEPA_LOOKUP_FORMATERROR = -1,
	SEPA_LOOKUP_NOTFOUND	= -2,
	SEPA_LOOKUP_FAILED		= -3,
	SEPA_LOOKUP_AMBIGUOUS	= -4
} sepa_lookup_status_t;

typedef enum {
	SEPA_INIT_LICUSER		= 1,	/*!< set license user */
	SEPA_INIT_LICCODE		= 2,	/*!< set license code */
	SEPA_INIT_RULEBOOK		= 3		/*!< select SEPA rulebook */
} sepa_init_t;

typedef enum {
	SEPA_MSGTYPE_UNKNOWN	= 0,
	SEPA_MSGTYPE_DDI		= 1,	/*!< Direct Debit Initiation - pain.008.003.02 */
	SEPA_MSGTYPE_CTI		= 2		/*!< Credit Transfer Initiation - pain.001.003.03 */
} sepa_msgtype_t;

typedef enum {
	SEPA_DDTYPE_CORE	= 0,		/*!< Direct Debit 'LocalInstrumentCode': CORE */
	SEPA_DDTYPE_COR1	= 1,		/*!< Direct Debit 'LocalInstrumentCode': COR1 */
	SEPA_DDTYPE_B2B		= 2			/*!< Direct Debit 'LocalInstrumentCode': B2B */
} sepa_ddtype_t;

typedef enum {
	SEPA_STMT_FORMAT_NONE = 0,		/*!< format not initialized yet */
	SEPA_STMT_FORMAT_MT940 = 1,		/*!< SWIFT MT940 */
	SEPA_STMT_FORMAT_CAMT052 = 2,	/*!< SEPA camt.052 */
	SEPA_STMT_FORMAT_CAMT053 = 3	/*!< SEPA camt.053 */
} sepa_stmt_format_t;

typedef struct sepa_stmt_tx_t {
	struct tm ts_valuta, ts_booked;
	unsigned char storno;
	double amount;
	char code[4];		/* Buchungsschluessel */
	char *ref;			/* Kundenreferenz */
	char *bankref;
	char *txtext;		/* Buchungstext */
	char *primanota;	/* Primanota */
	unsigned int gvc;	/* Geschäftsvorfallcode */
	unsigned int extcode;	/* erweiterter SEPA-Code bei GVC 108(109/159/181/184 */
	char bank[13];		/* BLZ/BIC */
	char account[35];	/* Konto/IBAN */
	char name[55];		/* 32/33: Name */
	char *purpose;		/* Verwendungszweck (non-SEPA) */
	char *eref;			/* Ende-zu-Ende-Referenz */
	char *kref;			/* Kundenreferenz */
	char *mref;			/* Mandatsreferenz */
	char *cred;			/* Gläubiger-ID */
	char *debt;			/* Originators Identification */
	char *coam;			/* Compensation Amount */
	char *oamt;			/* Original Amount */
	char *svwz;			/* SEPA-Verwendungszweck */
	char *abwa;			/* abweichender Auftraggeber */
	char *abwe;			/* abweichender Empfänger */
	struct sepa_stmt_tx_t *next;
} sepa_stmt_tx_t;

typedef struct sepa_stmt_t {
	char *ref;
	char *account;
	unsigned int number;
	unsigned int page;
	struct tm ts_open, ts_close;
	double saldo_open, saldo_close;
	char curr_open[4], curr_close[4];
	sepa_stmt_tx_t *tx_first, *tx_last;
	struct sepa_stmt_t *next;
} sepa_stmt_t;

typedef struct {
	const char *key;
	const char *value;
} sepa_keyvalue_t;

typedef struct {
	char *name;
	char *zip;
	char *city;
	char *country;
	char *bic;
	uint16_t flags;
	char __priv[];
} sepa_bankinfo_t;

typedef struct sepa_stmt_parser_t sepa_stmt_parser_t;
typedef struct sepa_camt053_t sepa_camt053_t;

#define SEPA_SCL_SCT	1		/*!< sepa_bankinfo_t.flags: support for SEPA cash transfer (according to SCL) */
#define SEPA_SCL_SDD	2		/*!< sepa_bankinfo_t.flags: support for SEPA direct debit (according to SCL) */
#define SEPA_SCL_COR1	4		/*!< sepa_bankinfo_t.flags: support for SEPA direct debit COR1 (according to SCL) */
#define SEPA_SCL_B2B	8		/*!< sepa_bankinfo_t.flags: support for SEPA direct debit B2B (according to SCL) */

#ifdef SEPA_DLL
	#ifdef SEPA_DLL_EXPORT
		#define _SEPA_EXT extern __declspec(dllexport)
	#else
		#define _SEPA_EXT extern __declspec(dllimport)
	#endif
#else
	#define _SEPA_EXT extern
#endif

#define SEPA_VERSION "2.13"
#define SEPA_VERSION_MAJOR 2
#define SEPA_VERSION_MINOR 13
#define SEPA_VERSION_PATCH 0
#define SEPA_VERSION_INT 21300
_SEPA_EXT const char *sepa_version_str;
_SEPA_EXT const int sepa_version_int;

_SEPA_EXT sepa_status_t sepa_init(sepa_init_t option, const char *value);
_SEPA_EXT sepa_t *sepa_new(sepa_msgtype_t msgtype);
_SEPA_EXT void sepa_free(sepa_t *s);
_SEPA_EXT char *sepa_toXML(sepa_t *s);
_SEPA_EXT sepa_status_t sepa_setIBAN(sepa_t *s, const char *iban);
_SEPA_EXT sepa_status_t sepa_setBIC(sepa_t *s, const char *bic);
_SEPA_EXT sepa_status_t sepa_setName(sepa_t *s, const char *name);
_SEPA_EXT sepa_status_t sepa_setCreditorIdentifier(sepa_t *s, const char *ci);
_SEPA_EXT sepa_status_t sepa_setDate(sepa_t *s, const char *date);
_SEPA_EXT sepa_status_t sepa_setDDType(sepa_t *s, sepa_ddtype_t ddtype);
_SEPA_EXT sepa_status_t sepa_setBatchBooking(sepa_t *s, unsigned char mode);
_SEPA_EXT const char *sepa_getError(sepa_t *s);
_SEPA_EXT sepa_status_t sepa_add(sepa_t *s, sepa_keyvalue_t kv[]);
_SEPA_EXT sepa_lookup_status_t sepa_iban_getBank(const char *iban, sepa_bankinfo_t **info);
_SEPA_EXT sepa_lookup_status_t sepa_iban_convert(const char *country, const char *account, const char *bankid, char iban[SEPA_IBAN_MAXLENGTH+1], char *status);
_SEPA_EXT sepa_lookup_status_t sepa_iban_check(const char *iban);
_SEPA_EXT sepa_lookup_status_t sepa_iban_getBIC(const char *iban, char bic[SEPA_BIC_LENGTH+1]);
_SEPA_EXT sepa_lookup_status_t sepa_bic_getBank(const char *bic, sepa_bankinfo_t **info);			/*!< return data from SCL for this BIC (bank name & SCL flags) */

/* Statement parser */
_SEPA_EXT sepa_stmt_parser_t *sepa_stmt_parser_new(void);
_SEPA_EXT const char *sepa_stmt_parser_getError(sepa_stmt_parser_t *p);
_SEPA_EXT sepa_status_t sepa_stmt_parser_load(sepa_stmt_parser_t *p, sepa_stmt_format_t format, const char *filename);
_SEPA_EXT void sepa_stmt_parser_free(sepa_stmt_parser_t *p);
_SEPA_EXT sepa_stmt_t *sepa_stmt_parser_getStatement(sepa_stmt_parser_t *p);

/* these functions are used rather internally: */
_SEPA_EXT int sepa_account_check(const char *account, unsigned char rule);
_SEPA_EXT int sepa_account_check2(const char *account, const char *bankid, unsigned char rule);
_SEPA_EXT sepa_lookup_status_t sepa_bankid_check(const char *country, const char *bankid, sepa_bankinfo_t **info);
_SEPA_EXT void sepa_cleanup(void);

/* camt.053 parser */
_SEPA_EXT sepa_camt053_t *sepa_camt053_new(const char **errmsg);
_SEPA_EXT void sepa_camt053_free(sepa_camt053_t *ct);
_SEPA_EXT sepa_status_t sepa_camt053_load(sepa_camt053_t *ct, const char *filename);
_SEPA_EXT sepa_status_t sepa_camt053_parse(sepa_camt053_t *ct, const char *data, size_t len);
_SEPA_EXT const char *sepa_camt053_getError(sepa_camt053_t *ct);

/* utility functions, used eg. by PHP module */
/* sepa.c */
void __sepa_d2str(char *ptr, size_t sz, double d);

#ifdef __cplusplus
} // extern "C" {...}
#endif

#endif /* !__SEPA_H */

/* <EOF>_________________________________________________________________ */
