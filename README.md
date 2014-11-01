ANPA 1312 Data Extracter
========================

The anpa1312 command, will extract each ANPA-1312 encoded text file
into "extracted form" text on standard output. When multiple input
files are provided, each is separated by a FF LF characters, unless
the -f option is used:

    $ ./anpa1312 -h
    Usage:	./anpa1312 [-f] [-h] file1 [filen]
    	-f	Suppress FF between story output
    	-h	This usage info.
    
The header information is extracted and put into an email like text
file, with header lines separated from the extracted body. The 
separator is a blank line:


    SERVICE-LEVEL-DESIGNATOR: B
    STORY-NUMBER: 1488
    UPI: 
    SELECTOR-CODE: cpstf
    PRIORITY: r
    CATEGORY: b
    BODY: text
    KEYWORDS: Dollar-Markets
    VERSION: 7thWritethru
    REFERENCE: TB06
    FILING-DATE: 31-10
    WORD-COUNT: 588
    TRANSMISSION-DATE: 16:35ET 31-10-14<
    
    ^Dollar-Markets, 7th Writethru@<
    code:2<
    ^Update:ADDS details@<
    ^INDEX: Economy, Business@<
    ^SYMBOL:TSX:IMO@<
    ^HL:Stock markets advance as Bank of Japan seeks to fire up world's
    No. 3 economy@<
    ^By Malcolm Morrison@<
    ^THE CANADIAN PRESS@<
    	   TORONTO _ The Toronto stock market ran ahead sharply Friday on
    relief that another major central bank is stepping up to help keep
    the global economic recovery on track.<
    ...etc...

