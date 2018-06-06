--TEST--
Convert german bank account & bank id to IBAN
--SKIPIF--
<?php if (!extension_loaded("SEPA")) print "skip"; ?>
--POST--
--GET--
--FILE--
<?php

$status = -1;
var_dump(SEPA::IBAN_convert("DE", "7325022", "26580070", $status));
var_dump($status);
var_dump(SEPA::IBAN_convert("DE", "960994510", "55090500"));
var_dump(SEPA::IBAN_convert("DE", "7325022", "26580070"));
var_dump(SEPA::IBAN_getBIC("DE60550905000960994510"));
var_dump(SEPA::IBAN_getBIC("DE32265800700732502200"));
var_dump(SEPA::IBAN_check("DE22380200904030000000"));
var_dump(SEPA::IBAN_check("DE22380200904030000001"));
var_dump(SEPA::BIC_getBankName("BYLADEM1ERH"));
var_dump(SEPA::BIC_getBankFlags("BYLADEM1ERH"));
var_dump(SEPA::BIC_getBankName("BANKDEZZXXX"));

?>
--EXPECT--
string(22) "DE32265800700732502200"
int(1)
string(22) "DE60550905000960994510"
string(22) "DE32265800700732502200"
string(11) "GENODEF1S01"
string(11) "DRESDEFF265"
bool(true)
bool(false)
string(34) "STADT- UND KREISSPARKASSE ERLANGEN"
int(11)
NULL
