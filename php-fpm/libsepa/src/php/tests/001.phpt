--TEST--
Create SEPA message for DD (direct debit)
--SKIPIF--
<?php if (!extension_loaded("SEPA")) print "skip"; ?>
--POST--
--GET--
--FILE--
<?php

$sepa = new SEPA(SEPA_MSGTYPE_DDI);

?>
===DONE===
--EXPECT--
===DONE===
