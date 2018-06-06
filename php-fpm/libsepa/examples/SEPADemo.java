//  _ _ _
// | (_) |__ ___ ___ _ __  __ _   SEPA library - www.libsepa.com
// | | | '_ (_-</ -_) '_ \/ _` |  Copyright (c) 2013-2016 Keppler IT GmbH.
// |_|_|_.__/__/\___| .__/\__,_|_____________________________________________
//                  |_|
// java/SEPADemo.java
// Example for libsepa usage with Java (JNI)
// $Id: SEPADemo.java 277 2016-06-07 18:34:45Z kk $

// Compile: javac -classpath dist/SEPA.jar:. SEPADemo.java
// Test: java -Djava.library.path=dist/ -classpath dist/SEPA.jar:. SEPADemo

import com.libsepa.*;

public class SEPADemo {

	public static void main( String[] args ) {

		// un-comment the following lines to activate your license:
		//SEPA.init(SEPA_INIT_LICUSER, "YOUR NAME");
		//SEPA.init(SEPA_INIT_LICCODE, "YOUR LICENSE CODE");

		String iban   = SEPA.IBAN_convert("DE", "1234567890", "51010800");
		String bic    = SEPA.IBAN_getBIC(iban);
		boolean valid = SEPA.IBAN_check(iban);

		System.out.println("IBAN=" + iban + ", BIC=" + bic + ", VALID=" + (valid ? "YES" : "NO"));

		// Null pointer test
		//iban = SEPA.IBAN_convert("DE", null, "51010800");

		SEPA s = new SEPA(SEPA.SEPA_MSGTYPE_DDI);
		s.setIBAN("DE56510108001234567890");
		s.setBIC("BANKDEFFXXX");
		s.setName("Mustermann u. Co. KG");
		s.setCreditorIdentifier("DE98ZZZ09999999999");
		s.setBatchBooking(false);
		String[][] tx = {
			{ "seq",	"FRST" },
			{ "id",		"R2017742-1" },
			{ "name",	"Carl Customer" },
			{ "mref",	"(MandateReference)" },
			{ "mdate",	"2013-09-24" },
			{ "amount",	"123.45" },
			{ "iban",	"DE56510108001234567890" },
			{ "bic",	"BANKDEZZXXX" },
			{ "ref",	"Invoice R2017742 from 17/10/2013" },
		};
		s.add(tx);
		String xml = s.toXML();
		System.out.println(xml);
	}

}

// <EOF>_____________________________________________________________________
