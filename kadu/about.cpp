/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qvbox.h>
#include <qtextbrowser.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qtextcodec.h>
#include <qfile.h>
#include <qtextstream.h>

#include "about.h"
#include "misc.h"
#include "../config.h"

static char * sun_xpm[] = {
"167 122 45 1",
" 	c None",
".	c #000000",
"+	c #080808",
"@	c #0C0C0C",
"#	c #0D0D0D",
"$	c #020202",
"%	c #050505",
"&	c #040404",
"*	c #090909",
"=	c #0E0E0E",
"-	c #030303",
";	c #0A0A0A",
">	c #070707",
",	c #101010",
"'	c #010101",
")	c #060606",
"!	c #0B0B0B",
"~	c #141414",
"{	c #111111",
"]	c #1C1C1C",
"^	c #0F0F0F",
"/	c #FFF207",
"(	c #131313",
"_	c #FFFFFF",
":	c #121212",
"<	c #1A1A1A",
"[	c #FFE707",
"}	c #FFB707",
"|	c #FF9808",
"1	c #191919",
"2	c #FF2A0A",
"3	c #FF2B09",
"4	c #FF5C09",
"5	c #151515",
"6	c #FF2F09",
"7	c #FF8208",
"8	c #FF4709",
"9	c #FFC807",
"0	c #FF2E09",
"a	c #171717",
"b	c #FF2A09",
"c	c #FF6909",
"d	c #EFEFEF",
"e	c #1D1D1D",
"f	c #EEEEEE",
"                                                                                                                                                                       ",
"                                                                                                                                                                       ",
"                                                                                                                                                                       ",
"                                                                                                                                                                       ",
"                                                                                                                                                                       ",
"                                                                          .+                                                                                           ",
"                                                                          ...                                                                                          ",
"                                                                           @#..                                                                                        ",
"                                                                           $.%..                                              .                                        ",
"                                                                            .&.*.                                            =.                                        ",
"                                                                            -+..;.                                          +.-                                        ",
"                                                                             .&..&                                          $.>                                        ",
"                                                                             ,.$.>'                                        ;.$.,                                       ",
"                                                                              )$>-..                                       .!>&.                                       ",
"                                                                               .'$+'                                      .>...=                                       ",
"                                                                               >.'.+&                                    -...~..                                       ",
"                                                                                >.;.'                                    .+.-..                                        ",
"                                                                                ...!'-                                  @.$..%.                                        ",
"                                                                                $$-..;                                  ..;..$                                         ",
"                                                                                 %.{..                                 '->$&+.                                         ",
"                                       ..&*'                                     %..+$                                .'..'.#.                                         ",
"                                        )..;%.!                                  -%-..                                !$!..-.                                          ",
"                                        -*..%$..).                                .).                                -..'.$)                                           ",
"                                          #%.*.%.%.                                '                                 &''--'.                                           ",
"                                           -.>..!..&.                              .                                ...'.).                                            ",
"                                            +..-.'.;.%                                                              -.%.'>.                                            ",
"                                              -...#..%..                                                            .&.+...                                            ",
"                                               ;'%.*,-.@.                                                          .$...-.                                             ",
"                                                ~.;.....*.                     =.$)..].$$$$$$$$;-.                ;..#.*'&                                             ",
"                                                  ..>$.)...               &&%'+..&-.+.'..........%'.@$            .*..;&.                                              ",
"                                                   .^.-&'~..          .*.;.....*@..%'.%'''''''',-.%+..*'           %;&..$                                              ",
"                                                    ..>.;...        &*%.$>'*.=...*.+.///////////.;'.;.).'>         ..*.-.                                         -#.  ",
"                                                     ..,.*&.     %.%...->..$..//////////////////////').*..).       !..*.                                        .-.$'  ",
"                                                      )..%.'   &.)>'.(...$//////////////////////////////....$+      $.>.                                      .$.%.)   ",
"                                                       ).*._....*...$.'///////////////////////////////////%'...     .%.                                      .%.&'.)   ",
"                                                        .;.-&......@-///////////////////////////////////////!.>.$    .                                     ..*%...(    ",
"                                                         .)...!;..///////////////////////////////////////////-.).$                                       $>$.*..^.     ",
"                                                       ....;$+..///////////////////////////////////////////////..$.                                    .'.&...@.)      ",
"                                                      .,'.*.)%$/////////////////////////////////////////////////)..'                                 +.+..'.+...       ",
"                                                   >.*&.%..+/////////////..-.////////////////////////////////////'.>:                              ..)..):)...&$       ",
"                                                  !..;.&.%////////////+-.$%.+...//////////////////////////////////;..,                          &%.>..)>...-.).        ",
"                                                 &..*-...///////////.&.&'.>'.-...//////////////////////////////////!'..                        $..+.'&.&.)>.$...       ",
"                                               ..&-$..'//////////..;.'>......=.^..-/////////////////////////////////.;).                    ....$>..;.(.+..-$%.        ",
"                                              -@.*..@./////////..{.&.%.%=-//////.).%/////////////////////////////////'.>              -  >..,&.&&.-$-...(%             ",
"                                             .%....%//////////.#.$+.)'>//////////>-'/////////////////////////////////.+'&          ;..$.%.;;..'...'..@&..              ",
"                                            &$-.*&>/////////$...%+...////////////.>.//////////////////////////////////-'..         .(-.'.#..);+..{)$.).#.              ",
"                                           ..-..>./////////..*.$$..//////////////...///////////////////////////////////.>!         ..@.)..-#...*....=                  ",
"                                          =..*)).////////.^+.&%..///.////////////.;-/////////////.#.///////////////////.'.$        &%...@.$.-.*.*..@$                  ",
"                                          .%*../////////.;...%.////.^/.//////////-%.////////////)'.'..//////////////////.--         *.$'.)..*....$)                    ",
"                                         .$.).////////.$.&')../////.....//////////..////////////'..+%.//////////////////.&.$        .%$>....$.) $.                     ",
"                                        ..!'.////////.%.,...///////+>.>%*///////////////////////.%%.);./////////////////)'+.         ......*.,                         ",
"                                       .>..);///////.,.&.';////////.^..$.///////////////////////-.)...^//////////////////%..          ;&.>+.#                          ",
"                                       )'&..///////.+.%&../////////..#..#///////////////////////.-.).*.//////////////////.+..          . .                             ",
"                                      ..!.$ //////.!..'!.//////////@..>..///////////////////////-.>.)$.//////////////////.-%.                                          ",
"                                      ;$$. ///////-...>.///////////%.&'$.///////////////////////...'.'////////////////////..-                                          ",
"                                     %.+.  //////&..'+.////////////+.&.$*///////////////////////$..'.'////////////////////.=.                                          ",
"                                      $.   /////)-.&../////////////.!&..&////////////////////////..'.'////////////////////%.)-                                         ",
"                                      .    ////*.&..^//////////////$.._..////////////////////////..'.'////////////////////..*.                                         ",
"                                           ///%...,!///////////////'._..=////////////////////////..'.'///////////////////*.)..                                         ",
"                                            //'>'.;.////////////////!..>/////////////////////////..'.'///////////////////.-&..                                         ",
"                                            /.+..&.////////#..=/////>%.*/////////////////////////..'.'///////////////////.>.#.                                         ",
"                                            /...=./////////..-..//////&./////////////////////////..'.'///////////////////!.-..                                         ",
"                                            $....'/////////%.).&./////.///////////////////////////-.'%///////////////////.)@.                                          ",
"                                           .,.;.^//////////'-.+.&$////////////////////////////////.@>////////////////////,%.@                                          ",
"                                           %.$.&///////////..!.*..//////////////////////////////////.///////////////////$.&..                                          ",
"                                           .%.$.////////////'.+..'//////////////////////////////////////////////////////.).$%                                          ",
"                                          .*.>.//////////////..')$./////////////////////////////////////////////////////,.'.%                                          ",
"                                          ;'-.$//////////////{..&..@/////////////////////////////..////////////////////)....                                           ",
"                                         ...>.)///////////////%$.:.>.///////////////////////////-..&///////////////////.&*+.                                           ",
"                                         {>..://////////////////.)..+$-///////////////////////...&@)//////////////////$;...                                            ",
"                              .).@.+     .%,..///////////////////.+..;../////////////////////.&).%.///////////////////..(..                                            ",
"                           .<...>...-)   ....)////////////////////%%.@.'..////////////////.')...;.$//////////////////).'..{                                            ",
"                         &.-.&-.-&.$...  &>.$ /////////////////////)..,#'...///////[}|%.&.>...'..$//////////////////!'.&.+                                             ",
"                        .*....%-.%..;.% .'..' //////////////////////>.'..>+..%.)..;>-....'&.&.*'*.//////////////////'.'.$.                                             ",
"                      '&..-;%......-.>. $&.;. ///////////////////////:.'>.'-.$$.>*..'..).).$.@.$///////////////////.-.%&%                                              ",
"                    .$$.1...-.&>..,..#. .&-.. ///////////////////////.&.%=..^..>..$);.++.%>...////////////////////.!.'..   =.                                          ",
"                   .>*...@)>.>'.*..#.&. !.%.) ///////////////////////&)...%...%.-)&..'..#..//////////////////////$..-.;{  '.;..                                        ",
"                  .;..&..&..>-..     .   &$.; ///////////////////////..;%.%'222..@+.'222234////////////////////....5.-.  ^+..).&                                       ",
"                 >>.+'^&.-).+            $... ////////////////////////&..'&.222222222222267/////////////////// %+!...$   ..--.&.#                                      ",
"                .)...,.%-.%              $)>+ ////////////////////////...$.&222222222222289//////////////////-.....$.     .&=.-)..                                     ",
"               .'%.*;._. @               ..%../////////////////////////.>.*=.222222222220>>.////////////////)...&&'%      .&.@.$...                                    ",
"              ..a..$+.@                   ~..= ////////////////////////--.-..2222222222bc>..///////////////....$...       .>.(.>.+$''                                  ",
"             ..(.(..$..                   .$!.&////////////////////////'.%-$.>22222222.5.$../////////////)&.&#.)..         )$.$.1.$.$.                                 ",
"            ..#...#'..                     .... ////////////////////////..'-$.2222222.;.!.+////////////>..)'%...#           %->..%'!...                                ",
"           ..'.*'_..;                      -)$^. ///////////////////////.@....222222.+.$$'///////////>$.+%>$...&             &.-.@..@%..                               ",
"           &.%>....#                        .-.$. ///////////////////////-.!$'.2222*$....)/////////>%..$'%.'#.                ..'''-.$..&                              ",
"          )$.;.$&.                           @)+-. //////////////////////.)..)..2b%.$.%'*////////'.>..^...,.%                   '..+).=>-%                             ",
"          ..=.@$+.                            %.%''  /////////////////////...,..*..'!.).////..~%.$)-!...>+.                      ....!.$.$)                            ",
"         .^'.).'.                             .'..'..*/////////////////////&$....!.....$/+.&$)..'$$..)--.                         ..>...(.>.                           ",
"         .%.-. .                                ..>..#.. ///////////////////.>*!.-&.%&#-.=.*'.%&..*.'%.                            ..!.$.+&&.                          ",
"         $-%-                                    >.>).'*..'   ////////////////....$.%.d&..-.-;.&>..$                                 ....&..e                          ",
"         ...                                       ...@.).'#+''..%    . . 1..#.....'+..&.>@..!..='                                     ;...+.                          ",
"         .=%                                         ....)....--;...*.!>+,..;.)%.f-.+'!.~.&*&.-                                         $&!%..                         ",
"         %.%                                            .$;%.^....&-.+....>)...)..f#.-.+.                                                 ..'                          ",
"         .>                                               .@..!...;..=.)).-..{..=..+.&                                                                                 ",
"         &%                                                   .-$...>...%....%.                                                                                        ",
"                                                    .!.'.                                                                                                              ",
"                                                   .&.'>.                                                                                                              ",
"                                                  %..--.-'                                                                                                             ",
"                                                &@.%...-')                               )$.                                                                           ",
"                                               ;..,.^'+..                               %..*.                                                                          ",
"                                               .%@...$.'$                               ;.*.;*                                                                         ",
"                                              ..*..;!.;+.                               !.!..%                                                                         ",
"                                              ..&.#..;''                                .'..@.                                                                         ",
"                                             $'&.@.%.                                    ...%.                                                                         ",
"                                             .&.*.).                                     +..-.                                                                         ",
"                                            '%.!...                                      '.%...                                                                        ",
"                                            %....!*                                      $).%&&                                                                        ",
"                                           ..#.-*.                                       .....;                                                                        ",
"                                           .+&..)                                         .%%..                                                                        ",
"                                           +'$@)                                          .@..*.                                                                       ",
"                                           .)...                                          ...;..                                                                       ",
"                                           &.;.                                            .$..'$                                                                      ",
"                                           .'@                                             '.$..+.                                                                     ",
"                                           !:                                               -'..%^                                                                     ",
"                                          .$.                                               %.>'...                                                                    ",
"                                           $                                                 ^.=.*#.                                                                   ",
"                                                                                             +..*.>..                                                                  ",
"                                                                                              '.%..(.+*                                                                ",
"                                                                                               ).*>..)                                                                 ",
"                                                                                                 ..-.                                                                  ",
"                                                                                                                                                                       "};

About::About() : QTabDialog(0) {
	setCaption(tr("About"));

	addTab1();
	addTab2();
	addTab3();
	addTab4();
	addTab5();

	setOkButton(tr("&Close"));
	resize(300, 200);
}

void About::addTab1() {
	QVBox *box = new QVBox(this);
	box->setSpacing(10);
	QLabel *l_logo = new QLabel(box);
	l_logo->setPixmap(QPixmap(sun_xpm));
	l_logo->setAlignment(Qt::AlignCenter);
	QString ver;
	ver = QString("<span style=\"font-size: 12pt\">Kadu ") + QString(VERSION)
		+ QString(tr("<br>(c) 2001-2003 Kadu Team</span>"));
	QLabel *l_info = new QLabel(ver, box);
	l_info->setAlignment(Qt::AlignCenter);
	addTab(box, tr("&About"));
}

void About::addTab2() {
	QVBox *box = new QVBox(this);
	box->setSpacing(10);
	QTextBrowser *tb_authors = new QTextBrowser(box);
	tb_authors->setTextFormat(Qt::PlainText);
	tb_authors->setText(loadFile("AUTHORS"));
	addTab(box, tr("&Authors"));
}

void About::addTab3() {
	QVBox *box = new QVBox(this);
	box->setSpacing(10);
	QTextBrowser *tb_thanks = new QTextBrowser(box);
	tb_thanks->setTextFormat(Qt::PlainText);
	tb_thanks->setText(loadFile("THANKS"));
	addTab(box, tr("&Thanks"));
}

void About::addTab4() {
	QVBox *box = new QVBox(this);
	box->setSpacing(10);
	QTextBrowser *tb_history = new QTextBrowser(box);
	tb_history->setTextFormat(Qt::PlainText);
	tb_history->setText(loadFile("HISTORY"));
	addTab(box, tr("&History"));
}

void About::addTab5() {
	QVBox *box = new QVBox(this);
	box->setSpacing(10);
	QTextBrowser *tb_license = new QTextBrowser(box);
	tb_license->setTextFormat(Qt::PlainText);
	tb_license->setText(loadFile("COPYING"));
	addTab(box, tr("&License"));
}

QString About::loadFile(const QString &name) {
	QString data;
	QFile file(QString(DATADIR) + QString("/kadu/") + name);
	if (!file.open(IO_ReadOnly))
		return QString::null;
	QTextStream str(&file);
	str.setCodec(QTextCodec::codecForName("ISO8859-2"));
	data = str.read();
	file.close();
	return data;
}
