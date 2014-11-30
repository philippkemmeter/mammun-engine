<?
if (!isset($_SESSION))
session_start();

class Werbung {
	/**
	 * Zeigt zufällig ein Banner an
	 * @param string   $language  Gibt die Sprache an
	 * @param string[] $exclude	Banner, die NICHT angezeigt werden dürfen
	 * @param string[] $partner     Gibt an, welche Partner zur auswahlt sind.
	 *				Wenn nicht gesetzt, dann wird eine vorgegebene
	 *				Liste verwendet
	 * @return string[] 		Gibt den Ident-String des/der ausgewählten
	 *				Banner(s) zurück
	 */

	function print_banner($language, $exclude=array(), $partner=array()){
			
		/*
		 $r_partner = array();
		 if (empty($partner) && (rand(0,4) == 0)) {
		 echo '<table border=0 cellspacing=0 cellpadding=0><tr><td>'."\n";
		 $r_partner[] = Werbung::print_half_banner($language, $exclude);
		 echo '</td><td>'."\n";
		 $r_partner[] = Werbung::print_half_banner($language, $r_partner);
		 echo '</td></tr></table>';
		 }
		 else {
		 // Wenn $partner nicht angegeben
		 if (empty($partner)) {
		 // hier sind allgemeine Partner für .de und .com
		 $partner = array(
		 'tripel', 'tripel',
		 'allmaxx', 'allmaxx', 'allmaxx',
		 'kabelmeister', 'kabelmeister',
		 'caseking', 'caseking',
		 'conrad',
		 'shirtomat', 'shirtomat', 'shirtomat', 'shirtomat',
		 'fussballgott', 'fussballgott','fussballgott',
		 'freekom', 'freekom',
		 'misco',
		 'partnerprog', 'partnerprog', 'partnerprog', 'partnerprog', 'partnerprog',
		 'uniteddomains', 'uniteddomains',
		 '7mobilepcc','7mobilepcc','7mobilepcc',
		 'auktionsideen', 'auktionsideen',
		 'ron', 'ron', 'ron', 'ron', 'ron', 'ron',
		 'icefighter', 'icefighter', 'icefighter', 'icefighter',
		 'formel1manager', 'formel1manager', 'formel1manager', 'formel1manager',
		 'fussballmanagar','fussballmanagar', 'fussballmanagar',
		 'rabazz', 'rabazz', 'rabazz', 'rabazz',
		 'tankgewinn',
		 'orange_webdesign', 'orange_webdesign', 'orange_webdesign', 'orange_webdesign', 'orange_webdesign',
		 'browsergameclub', 'browsergameclub', 'browsergameclub','browsergameclub', 'browsergameclub', 'browsergameclub','browsergameclub', 'browsergameclub', 'browsergameclub','browsergameclub', 'browsergameclub', 'browsergameclub','browsergameclub', 'browsergameclub', 'browsergameclub','browsergameclub', 'browsergameclub', 'browsergameclub','browsergameclub', 'browsergameclub', 'browsergameclub',
		 'browsergames24','browsergames24', 'browsergames24', 'browsergames24', 'browsergames24', 'browsergames24'
		 );

		 // hier nur für .de
		 $partner_de = array();

		 // und hier nur für .com
		 $partner_com = array(
		 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay',          'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay', 'ebay'
		 );

		 if ($language == 'en')
		 $partner = array_merge($partner, $partner_com);
		 else if ($language == 'de')
		 $partner = array_merge($partner, $partner_de);
		 }

		 do {
		 $r_partner = $partner[rand(0, sizeof($partner)-1)];
		 } while (in_array($r_partner, $exclude));
		  
		 $banner['tripel'] = // 10% S
		 array(
		 '<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=1480&type=b6&bnb=6"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=1480&b=6" border="0" alt="TRIPEL Kabel Shop" width="465" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->',
		 '<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=1480&type=b7&bnb=7"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=1480&b=7" border="0" alt="TRIPEL Kabel Shop" width="465" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->',
		 '<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=1480&type=b8&bnb=8"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=1480&b=8" border="0" alt="TRIPEL Kabel Shop" width="465" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->'
		 );

	  $banner['kabelmeister'] = // 10% S
	  array(
	  '<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=2128&type=b1&bnb=1"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=2128&b=1" border="0" alt="Kabelmeister" width="468" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->',
	  '<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=2128&type=b2&bnb=2"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=2128&b=2" border="0" alt="Kabelmeister" width="468" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->',
	  '<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=2128&type=b10&bnb=10"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=2128&b=10" border="0" alt="Kabelmeister" width="468" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->',
	  '<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=2128&type=b13&bnb=13"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=2128&b=13" border="0" alt="Kabelmeister" width="468" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->',
	  '<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=2128&type=b20&bnb=20"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=2128&b=20" border="0" alt="Kabelmeister" width="468" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->',
	  '<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=2128&type=b22&bnb=22"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=2128&b=22" border="0" alt="Kabelmeister" width="468" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->'
	  );

	  $banner['conrad'] = // 5% S
	  array(
	  '<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=2945&type=b2&bnb=2"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=2945&b=2" border="0" alt="Conrad Electronic" width="468" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->',
	  '<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=2945&type=b22&bnb=22"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=2945&b=22" border="0" alt="Conrad Electronic" width="468" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->'
	  );

	  $banner['misco'] = // 3,5% S
	  array(
	  '<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=3367&type=b173&bnb=173"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=3367&b=173" border="0" alt="MISCO - 25.000 PC / HIFI / VIDEO Produkte" width="468" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->'
	  );

	  $banner['partnerprog'] =	// 0,04 C
	  array(
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=46&type=b1&bnb=1" TARGET="_new">'."\n".'<IMG SRC="http://home.t-online.de/home/partnerprogramme/ani46860.gif" BORDER="0"  alt="Partnerprogramme.com - Geld verdienen im Internet" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=46&type=b4&bnb=4" TARGET="_new">'."\n".'<IMG SRC="http://home.t-online.de/home/partnerprogramme/banner_von_luckysites.gif" BORDER="0"  alt="Partnerprogramme.com - Geld verdienen im Internet" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=46&type=b7&bnb=7" TARGET="_new">'."\n".'<IMG SRC="http://home.t-online.de/home/partnerprogramme/bannertext3.gif" BORDER="0"  alt="Partnerprogramme.com - Geld verdienen im Internet" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=46&type=b8&bnb=8" TARGET="_new">'."\n".'<IMG SRC="http://home.t-online.de/home/partnerprogramme/bannertext2.gif" BORDER="0"  alt="Partnerprogramme.com - Geld verdienen im Internet" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=46&type=b9&bnb=9" TARGET="_new">'."\n".'<IMG SRC="http://home.t-online.de/home/partnerprogramme/haupt.gif" BORDER="0"  alt="Partnerprogramme.com - Geld verdienen im Internet" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
			);

	  $banner['ron'] = // 0,05 C
	  array(
	  '<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=3678&type=b1&bnb=1"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=3678&b=1" border="0" alt="NP Media Marketing" width="468" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->'
	  );

	  $banner['fussballmanager'] = // 0,50 L
	  array(
	  '<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=3437&type=b1&bnb=1"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=3437&b=1" border="0" alt="Fussballmanager.de - der Online-Fussball-Manager" width="468" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->',
	  '<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=3437&type=b19&bnb=19"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=3437&b=19" border="0" alt="Fussballmanager.de - der Online-Fussball-Manager" width="468" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->'
	  );

	  $banner['formel1manager'] =	// 0,05 C
	  array(
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3220&type=b1&bnb=1" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3220&ref=168627&b=1" BORDER="0"  alt="Online Formel-1 Manager" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3220&type=b2&bnb=2" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3220&ref=168627&b=2" BORDER="0"  alt="Online Formel-1 Manager" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
			);
			$banner['caseking'] =	// 6,00 S
			array(
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=2767&type=b4&bnb=4" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=2767&ref=168627&b=4" BORDER="0"  alt="Caseking.de - The Modding Source" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=2767&type=b9&bnb=9" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=2767&ref=168627&b=9" BORDER="0"  alt="Caseking.de - The Modding Source" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=2767&type=b2&bnb=2" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=2767&ref=168627&b=2" BORDER="0"  alt="Caseking.de - The Modding Source" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
			);
			$banner['uniteddomains'] =	// 0,02 C + 2,50 L
			array(
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".''."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=1977&type=b4&bnb=4" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=1977&ref=168627&b=4" BORDER="0"  alt="united-domains.de - Domains weltweit registrieren" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
			);
			$banner['allmaxx'] =	// 3,00 L
			array(
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=1787&type=b6&bnb=6" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=1787&ref=168627&b=6" BORDER="0"  alt="allmaxx.de - Online Shop fuer Studenten" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=1787&type=b4&bnb=4" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=1787&ref=168627&b=4" BORDER="0"  alt="allmaxx.de - Online Shop fuer Studenten" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
			);
			$banner['shirtomat'] =	// 15,00 S
			array(
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3203&type=b3&bnb=3" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3203&ref=168627&b=3" BORDER="0"  alt="ShirtOmat - Die Shirtmaschine im Internet" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3203&type=b16&bnb=16" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3203&ref=168627&b=16" BORDER="0"  alt="ShirtOmat - Die Shirtmaschine im Internet" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3203&type=b4&bnb=4" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3203&ref=168627&b=4" BORDER="0"  alt="ShirtOmat - Die Shirtmaschine im Internet" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
			);
			$banner['fussballgott'] =	// 0,50 L
			array(
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3282&type=b60&bnb=60" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3282&ref=168627&b=60" BORDER="0"  alt="FUSSBALL-GOTT.DE -  DEUTSCHLANDS MANAGER-TURNIER" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3282&type=b69&bnb=69" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3282&ref=168627&b=69" BORDER="0"  alt="FUSSBALL-GOTT.DE -  DEUTSCHLANDS MANAGER-TURNIER" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
			);
			$banner['icefighter'] =	// 0,05 C
			array(
			"<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->\n<A HREF=\"http://partners.webmasterplan.com/click.asp?ref=168627&site=2826&type=b1&bnb=1\" TARGET=\"_new\">\n<IMG SRC=\"http://banners.webmasterplan.com/view.asp?site=2826&ref=168627&b=1\" BORDER=\"0\" ALT=\"IceFighter.com => Online Eishockey Manager\" WIDTH=\"468\" HEIGHT=\"60\"></A><br>\n<!-- END PARTNER PROGRAM -->",
			"<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->\n<A HREF=\"http://partners.webmasterplan.com/click.asp?ref=168627&site=2826&type=b2&bnb=2\" TARGET=\"_new\">\n<IMG SRC=\"http://banners.webmasterplan.com/view.asp?site=2826&ref=168627&b=2\" BORDER=\"0\" ALT=\"IceFighter.com => Online Eishockey Manager\" WIDTH=\"468\" HEIGHT=\"60\"></A><br>\n<!-- END PARTNER PROGRAM -->"
			);
			$banner['7mobilepcc'] =	// 0,03 C
			array(
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3065&type=b9&bnb=9" TARGET="_new"'."\n".'">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3065&ref=168627&b=9" BORDER="0"  alt="7mobile PPC - Handys günstig online kaufen" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3065&type=b10&bnb=10" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3065&ref=168627&b=10" BORDER="0"  alt="7mobile PPC - Handys günstig online kaufen" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3065&type=b14&bnb=14" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3065&ref=168627&b=14" BORDER="0"  alt="7mobile PPC - Handys günstig online kaufen" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3065&type=b26&bnb=26" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3065&ref=168627&b=26" BORDER="0"  alt="7mobile PPC - Handys günstig online kaufen" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3065&type=b31&bnb=31" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3065&ref=168627&b=31" BORDER="0"  alt="7mobile PPC - Handys günstig online kaufen" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3065&type=b33&bnb=33" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3065&ref=168627&b=33" BORDER="0"  alt="7mobile PPC - Handys günstig online kaufen" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
			);
			$banner['tankgewinn'] =	// 0,50 L
			array(
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=2980&type=b66&bnb=66"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=2980&b=66" border="0" alt="Das Shell-Tankgutschein Gewinnspiel" width="468" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=2980&type=b63&bnb=63"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=2980&b=63" border="0" alt="Das Shell-Tankgutschein Gewinnspiel" width="468" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=2980&type=b62&bnb=62"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=2980&b=62" border="0" alt="Das Shell-Tankgutschein Gewinnspiel" width="468" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<a href="http://partners.webmasterplan.com/click.asp?ref=168627&site=2980&type=b60&bnb=60"  target="_blank">'."\n".'<img src="http://banners.webmasterplan.com/view.asp?ref=168627&site=2980&b=60" border="0" alt="Das Shell-Tankgutschein Gewinnspiel" width="468" height="60"/></a><br>'."\n".'<!-- END PARTNER PROGRAM -->'
			);
			$banner['auktionsideen'] =	// 4,00 L + 0,02 C
			array(
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3175&type=b1&bnb=1" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3175&ref=168627&b=1" BORDER="0"  alt="Auktionsideen" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3175&type=b13&bnb=13" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3175&ref=168627&b=13" BORDER="0"  alt="Auktionsideen" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3175&type=b14&bnb=14" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3175&ref=168627&b=14" BORDER="0"  alt="Auktionsideen" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
			);
			$banner['freekom'] =	// 2,00 - 15,00 L + 0,01 C
			array(
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3334&type=b1&bnb=1" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3334&ref=168627&b=1" BORDER="0"  alt="Freekom - einzigartige Mobilfunktarife" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3334&type=b2&bnb=2" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3334&ref=168627&b=2" BORDER="0"  alt="Freekom - einzigartige Mobilfunktarife" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3334&type=b3&bnb=3" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3334&ref=168627&b=3" BORDER="0"  alt="Freekom - einzigartige Mobilfunktarife" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3334&type=b4&bnb=4" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3334&ref=168627&b=4" BORDER="0"  alt="Freekom - einzigartige Mobilfunktarife" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
			);
			$banner['rabazz'] =	// 0,50 L
			array(
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3082&type=b6&bnb=6" TARGET="_top">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3082&ref=168627&b=6" BORDER="0"  alt="www.rabazz.de - powered by E-Plus" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
			'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3082&type=b7&bnb=7" TARGET="_top">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3082&ref=168627&b=7" BORDER="0"  alt="www.rabazz.de - powered by E-Plus" WIDTH="468" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
			);

			$banner['ebay'] = // 0,01 / 1000V
			array(
			'<!-- Begin clixGalore Code-->'."\n".'<img src="http://www.is1.clixgalore.com/cpm.aspx?BID=19872&AfID=81500&AdID=2764" width="0" height="0">'."\n".'<script language=\'JavaScript\' type=\'text/javascript\'>'."\n".'<!--'."\n".'   if (!document.phpAds_used) document.phpAds_used = \',\';'."\n".'   phpAds_random = new String (Math.random()); phpAds_random = phpAds_random.substring(2,11);'."\n".'      document.write ("<" + "script language=\'JavaScript\' type=\'text/javascript\' src=\'");'."\n".'   document.write ("http://ads.co.com/ads/adjs.php?n=" + phpAds_random);'."\n".'   document.write ("&amp;what=zone:165");'."\n".'   document.write ("&amp;exclude=" + document.phpAds_used);'."\n".'   if (document.referer)'."\n".'      document.write ("&amp;referer=" + escape(document.referer));'."\n".'   document.write ("\'><" + "/script>");'."\n".'//-->'."\n".'</script><noscript><a href=\'http://ads.co.com/ads/adclick.php?n=ad2f5a08\' target=\'_blank\'><img src=\'http://ads.co.com/ads/adview.php?what=zone:165&amp;n=ad2f5a08\' border=\'0\' alt=\'\'></a></noscript>'."\n\n".'<!-- End clixGalore Code-->'
			);


			$banner['orange_webdesign'] = // GEFÄLLIGKEIT
			array(
			'<a href="http://www.orange-webdesign.com" target="_new"><img src="http://www.orange-webdesign.com/banners/orange-webdesign_01.jpg" border=0></a>'
			);

	  $banner['browsergameclub'] = // TOPLIST
	  array(
	  '<a target="_blank" href="http://www.browsergameclub.de"><img src="http://www.browsergameclub.de/Banner1.JPG" border="0" alt="Browsergamesclub - Die Browsergame Community" width="468" height="60"></a>'
	  );

	  $banner['browsergames24'] = //TOPLIST
	  array(
	  '<a href="http://www.browsergames24.de" target="_blank"><img src="http://www.browsergames24.de/bannergrossbrowsergames24.jpg" alt="" border="0"></a>'
	  );
	  */
		/*	  echo '<a href="http://www.rabattsparer.de/ad.php?stat=c" target="_blank"><img src="http://www.rabattsparer.de/ad.php?stat=v" alt="Aktuelle Gutscheincodes und Rabattaktionen" height=60 width=468 border=0></a>';//      echo $banner[$r_partner][rand(0, sizeof($banner[$r_partner])-1)];*/

		if ($_SESSION['nick17']) {
			$rand =rand(0,1);

			if ($rand == 1) {
				echo'<a target="_blank" href="http://www.sponsorads.de/index2.php?frame=webmasteranm&refid=13116"><img border="0" src="http://www.sponsorads.de/wm/Banner/aufruf.php" width="468" height="60"></a>';
			}
			else /*if ($rand == 2) {
				echo '<script type="text/javascript" src="http://www.sponsorads.de/script.php?s=41716"></script>';
			}
			else */{
				echo '<object width="468" height="60" data="../pix/weba/premium.swf" type="application/x-shockwave-flash" codebase="http://fpdownload.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,0,0">
 <param name="movie" value="../pix/weba/premium.swf" />
 <param name="quality" value="high" /><img src="" /></object>';
			}
		}
		else {
			//echo '<script type="text/javascript" src="http://www.sponsorads.de/script.php?s=41716"></script>';
			/*$rand = rand(0,1);
			if ($rand == 0)*/
			echo '<a href="http://www.gamestopia.de" target="_blank"><img border=0 src="http://www.gamestopia.de/games/Bilder/banner.jpg" alt="gamestopia.de"></a>';
			/*else if ($rand == 1)
			echo '<script type="text/javascript" src="http://www.sponsorads.de/script.php?s=41716"></script>';*/
			/*else echo'
			 <script type="text/javascript"><!--
			 google_ad_client = "pub-1596307708568599";
			 google_ad_width = 468;
			 google_ad_height = 60;
			 google_ad_format = "468x60_as";
			 google_ad_type = "text_image";
			 //2007-06-28: Project Virus1
			 google_ad_channel = "4740148829";
			 google_color_border = "101040";
			 google_color_bg = "101040";
			 google_color_link = "8080d0";
			 google_color_text = "F0F0F0";
			 google_color_url = "8080d0";
			 google_ui_features = "rc:0";
			 //-->
			 </script>
			 <script type="text/javascript"
			 src="http://pagead2.googlesyndication.com/pagead/show_ads.js">
			 </script>';*/
			//echo'<a target="_blank" href="http://www.sponsorads.de/index2.php?frame=webmasteranm&refid=13116"><img border="0" src="http://www.sponsorads.de/wm/Banner/aufruf.php" width="468" height="60"></a>';
		}
		$r_partner = array('pv');	//      $r_partner = array($r_partner);
		/*
		}
		*/
		return $r_partner;
	}


	/**
	 * Zeigt zufällig ein Banner an
	 * @param string   $language  Gibt die Sprache an
	 * @param string[] $exclude	Banner, die NICHT angezeigt werden dürfen
	 * @param string[] $partner     Gibt an, welche Partner zur auswahlt sind.
	 *				Wenn nicht gesetzt, dann wird eine vorgegebene
	 *				Liste verwendet
	 * @return string 		Gibt den Ident-String des ausgewählten
	 *				Banners zurück
	 */

	function print_half_banner($language, $exclude=array()){
		$partner = array(
		'formel1manager', 'formel1manager', 'formel1manager', 'formel1manager',
		'caseking', 'caseking', 'caseking',
		'uniteddomains', 'uniteddomains',
		'sedo', 'sedo',
		'allmaxx', 'allmaxx', 'allmaxx',
		'shirtomat', 'shirtomat', 'shirtomat', 'shirtomat',
		'fussballgott', 'fussballgott','fussballgott',
		'icefighter', 'icefighter', 'icefighter', 'icefighter',
		'7mobilepcc','7mobilepcc','7mobilepcc',
		'auktionsideen', 'auktionsideen',
		'freekom',
		'rabazz','rabazz','rabazz'
    );

		do {
			$r_partner = $partner[rand(0, sizeof($partner)-1)];
		} while (in_array($r_partner, $exclude));

		$banner['teltex'] =		// 0,08 C
		array(
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3188&type=b9&bnb=9" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3188&ref=168627&b=9" BORDER="0"  alt="www.teltex.de" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3188&type=b1&bnb=1" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3188&ref=168627&b=1" BORDER="0"  alt="www.teltex.de" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3188&type=b23&bnb=23" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3188&ref=168627&b=23" BORDER="0"  alt="www.teltex.de" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
      );
		$banner['abocenter'] =	// 0,05 C
		array(
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=45&type=b7&bnb=7" TARGET="_new">'."\n".'<IMG SRC="http://www.abo-center.de/grafikweb/werbung/banner7.gif" BORDER="0"  alt="Abo-Center.de" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=45&type=b13&bnb=13" TARGET="_new">'."\n".'<IMG SRC="http://www.abo-center.de/grafikweb/werbung/banner13.gif" BORDER="0"  alt="Abo-Center.de" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=45&type=b14&bnb=14" TARGET="_new">'."\n".'<IMG SRC="http://www.abo-center.de/grafikweb/werbung/banner14.gif" BORDER="0"  alt="Abo-Center.de" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
      );
		$banner['fussballgott'] =	// 0,50 L
		array(
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3282&type=b2&bnb=2" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3282&ref=168627&b=2" BORDER="0"  alt="FUSSBALL-GOTT.DE -  DEUTSCHLANDS MANAGER-TURNIER" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3282&type=b9&bnb=9" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3282&ref=168627&b=9" BORDER="0"  alt="FUSSBALL-GOTT.DE -  DEUTSCHLANDS MANAGER-TURNIER" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
      );
		$banner['formel1manager'] =	// 0,05 C
		array(
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3220&type=b3&bnb=3" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3220&ref=168627&b=3" BORDER="0"  alt="Online Formel-1 Manager" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
      );
		$banner['sedo'] =		// 0,02 C + 0,75 L
		array(
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=345&type=b10&bnb=10" TARGET="_new">'."\n".'<IMG SRC="http://www.sedo.de/bannerwerbung/banner23460.gif" BORDER="0"  alt="sedo.de - Domainhandelsbörse" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
      );
		$banner['shirtomat'] =	// 15,00 S
		array(
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3203&type=b13&bnb=13" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3203&ref=168627&b=13" BORDER="0"  alt="ShirtOmat - Die Shirtmaschine im Internet" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
      );
		$banner['uniteddomains'] =	// 0,02 C + 2,50 L
		array(
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=1977&type=b27&bnb=27" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=1977&ref=168627&b=27" BORDER="0"  alt="united-domains.de - Domains weltweit registrieren" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
      );
		$banner['allmaxx'] =		// 3,00 L
		array(
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=1787&type=b41&bnb=41" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=1787&ref=168627&b=41" BORDER="0"  alt="allmaxx.de - Online Shop fuer Studenten" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
      );
		$banner['icefighter'] =	// 0,05 C
		array(
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=2826&type=b5&bnb=5" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=2826&ref=168627&b=5" BORDER="0"  alt="IceFighter.com => Online Eishockey Manager" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
      );
		$banner['7mobilepcc'] =	// 0,03 C
		array(
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3065&type=b21&bnb=21" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3065&ref=168627&b=21" BORDER="0"  alt="7mobile PPC - Handys günstig online kaufen" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3065&type=b22&bnb=22" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3065&ref=168627&b=22" BORDER="0"  alt="7mobile PPC - Handys günstig online kaufen" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3065&type=b19&bnb=19" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3065&ref=168627&b=19" BORDER="0"  alt="7mobile PPC - Handys günstig online kaufen" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3065&type=b20&bnb=20" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3065&ref=168627&b=20" BORDER="0"  alt="7mobile PPC - Handys günstig online kaufen" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3065&type=b24&bnb=24" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3065&ref=168627&b=24" BORDER="0"  alt="7mobile PPC - Handys günstig online kaufen" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
      );
		$banner['auktionsideen'] =	// 0,02 C + 4,- L
		array(
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3175&type=b6&bnb=6" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3175&ref=168627&b=6" BORDER="0"  alt="Auktionsideen" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3175&type=b15&bnb=15" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3175&ref=168627&b=15" BORDER="0"  alt="Auktionsideen" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3175&type=b16&bnb=16" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3175&ref=168627&b=16" BORDER="0"  alt="Auktionsideen" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
      );
		$banner['freekom'] =		// 2,00 - 15,00 L + 0,01 C
		array(
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3334&type=b11&bnb=11" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3334&ref=168627&b=11" BORDER="0"  alt="Freekom - einzigartige Mobilfunktarife" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3334&type=b12&bnb=12" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3334&ref=168627&b=12" BORDER="0"  alt="Freekom - einzigartige Mobilfunktarife" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3334&type=b13&bnb=13" TARGET="_new">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3334&ref=168627&b=13" BORDER="0"  alt="Freekom - einzigartige Mobilfunktarife" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
      );
		$banner['rabazz'] =	// 0,50 L
		array(
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3082&type=b9&bnb=9" TARGET="_top">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3082&ref=168627&b=9" BORDER="0"  alt="www.rabazz.de - powered by E-Plus" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->',
		'<!-- BEGIN PARTNER PROGRAM - DO NOT CHANGE THE PARAMETERS OF THE HYPERLINK -->'."\n".'<A HREF="http://partners.webmasterplan.com/click.asp?ref=168627&site=3082&type=b10&bnb=10" TARGET="_top">'."\n".'<IMG SRC="http://banners.webmasterplan.com/view.asp?site=3082&ref=168627&b=10" BORDER="0"  alt="www.rabazz.de - powered by E-Plus" WIDTH="234" HEIGHT="60"></A><br>'."\n".'<!-- END PARTNER PROGRAM -->'
      );




		echo $banner[$r_partner][rand(0, sizeof($banner[$r_partner])-1)];
		return $r_partner;
	}

	function get_reload_blockings() {
		return array(
		'browsergameclub' => array('reload' => 86400, 'priority' => 100),
		'ebay' => array('reload' => 86400, 'priority' => 100),
		'tripel' => array('reload' => 0, 'priority' => -1),
		'kabelmeister' => array('reload' => 0, 'priority' => -1),
		'conrad' => array('reload' => 0, 'priority' => -1),
		'misco' => array('reload' => 0, 'priority' => -1),
		'ron' => array('reload' => 0, 'priority' => -1),
		'fussballmanager' => array('reload' => 0, 'priority' => -1),
		'formel1manager' => array('reload' => 0, 'priority' => -1),
		'caseking' => array('reload' => 0, 'priority' => -1),
		'uniteddomains' => array('reload' => 0, 'priority' => -1),
		'sedo' => array('reload' => 0, 'priority' => -1),
		'allmaxx' => array('reload' => 0, 'priority' => -1),
		'shirtomat'  => array('reload' => 0, 'priority' => -1),
		'fussballgott'  => array('reload' => 0, 'priority' => -1),
		'icefighter' => array('reload' => 0, 'priority' => -1),
		'7mobilepcc' => array('reload' => 0, 'priority' => -1),
		'auktionsideen' => array('reload' => 0, 'priority' => -1),
		'freekom' => array('reload' => 0, 'priority' => -1),
		'rabazz' => array('reload' => 0, 'priority' => -1),
		'partnerprog' => array('reload' => 0, 'priority' => -1),
		'allmaxx' => array('reload' => 0, 'priority' => -1),
		'shirtomat' => array('reload' => 0, 'priority' => -1),
		'fussballgott' => array('reload' => 0, 'priority' => -1),
		'icefighter' => array('reload' => 0, 'priority' => -1),
		'tankgewinn' => array('reload' => 0, 'priority' => -1),
		'auktionsideen' => array('reload' => 0, 'priority' => -1),
		'freekom' => array('reload' => 0, 'priority' => -1),
		'orangewebdesign' => array('reload' => 0, 'priority' => -1)
		);
	}
}
?>