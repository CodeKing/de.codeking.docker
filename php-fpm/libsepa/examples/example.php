<?php
#  _ _ _
# | (_) |__ ___ ___ _ __  __ _   SEPA library � www.libsepa.com
# | | | '_ (_-</ -_) '_ \/ _` |  Copyright (c) 2013-2016 Keppler IT GmbH.
# |_|_|_.__/__/\___| .__/\__,_|_____________________________________________
#                  |_|
# php/example.php
# Example for libsepa usage with PHP
# $Id: example.php 272 2016-04-05 20:15:05Z kk $

# Test: php -d "extension=modules/sepa.so" example.php

setlocale(LC_ALL, 'de_DE.utf8');
#setlocale(LC_NUMERIC,'C');

$iban = SEPA::IBAN_convert("DE", "1234567890", "51010800");
$bic = SEPA::IBAN_getBIC($iban);
$valid = SEPA::IBAN_check($iban);

print "IBAN=$iban, BIC=$bic, VALID=$valid\n";

$sepa = new SEPA(SEPA_MSGTYPE_DDI);
$sepa->setIBAN("DE56510108001234567890");
$sepa->setBIC("BANKDEFFXXX");
$sepa->setName("Mustermann u. Co. KG");
$sepa->setCreditorIdentifier("DE98ZZZ09999999999");
$sepa->setDDType(SEPA_DDTYPE_B2B);
$tx = [
    'seq' => 'FRST',
    'id' => 'R2017742-1',
    'name' => 'Carl Customer',
    'mref' => '(MandateReference)',
    'mdate' => '2013-09-24',
    'amount' => 123.45,
    'iban' => 'DE56510108001234567890',
    'bic' => 'BANKDEZZXXX',
    'ref' => 'Invoice R2017742 from 17/10/2013',
];
$sepa->add($tx);
$xml = $sepa->toXML();
echo $xml;
?>
