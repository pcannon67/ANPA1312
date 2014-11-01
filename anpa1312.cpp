//////////////////////////////////////////////////////////////////////
// anpa1312.cpp -- Decode ANPA-1312 Format Files
// Date: Sat Nov  1 14:38:58 2014  (C) Warren W. Gay VE3WWG 
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <string>
#include <sstream>

#define SOH	0x01
#define STX	0x02
#define ETX	0x03
#define EOT	0x04
#define SYN	0x16
#define FS	0x1C
#define GS	0x1D
#define RS	0x1E
#define US	0x1F
#define FF	0x0C

#define DC1	0x11
#define DC2	0x12
#define DC3	0x13
#define DC4	0x14

#define XIT_CMDLINE	1
#define XIT_FOPEN	2
#define XIT_FORMAT	3

extern char *optarg;
extern int optind;

static const char *pathname = 0;
static FILE *in = 0;
static long offset = 0;

static bool opt_noFF = false;

static int
getc() {
	assert(in);
	++offset;
	return fgetc(in);
}

static void
format_error(const char *what) {
	fprintf(stderr,"Format Error: Expected %s in file %s at byte offset %ld.\n",
		what,
		pathname,
		offset-1);
	exit(XIT_FORMAT);
}

static void
extract(const char *path) {
	int ch;
	struct s_info {
		char		service_level_designator;
		unsigned	story_number;
		std::string	UPI;
		std::string	selector_code;
		char		priority;
		char		category;
		bool		agate;
		bool		agate_tabular;
		bool		body;
		bool		body_text;
		std::string	keywords;
		std::string	version;
		std::string	reference;
		std::string	filing_date;
		unsigned	word_count;
		std::string	text;
		std::string	date_transmission;
	} info;

	if ( offset != 0 && !opt_noFF ) {
		putchar(FF);		// Separate multiple stories with FF, unless suppressed
		putchar('\n');
	}

	offset = 0;
	in = fopen(pathname=path,"r");
	if ( !in ) {
		fprintf(stderr,"%s: Opening %s for read.\n",
			strerror(errno),
			pathname);
		exit(XIT_FOPEN);
	}

	while ( (ch = getc()) != -1 && ( ch == SYN || ch == '\n' || ch == '\r' ) )
		;

	if ( ch != SOH )
		format_error("SOH");

	//////////////////////////////////////////////////////////////
	// Extract service level designator
	//////////////////////////////////////////////////////////////

	ch = getc();
	if ( isalpha(ch) ) {
		info.service_level_designator = ch;
		ch = getc();
	} else	info.service_level_designator = 0x00;

	if ( !isdigit(ch) )
		format_error("<digit>");

	//////////////////////////////////////////////////////////////
	// Extract story number
	//////////////////////////////////////////////////////////////
	{
		std::stringstream ss;
		unsigned count = 1;

		ss << char(ch);
		while ( (ch = getc()) != -1 && isdigit(ch) && ++count <= 4 )
			ss << char(ch);
	
		const std::string val = ss.str();
		info.story_number = unsigned(atoi(val.c_str()));
	}

	//////////////////////////////////////////////////////////////
	// Extract optional UPI string
	//////////////////////////////////////////////////////////////

	if ( ch != US ) {
		std::stringstream ss;
		unsigned count = 0;
		
		ss << char(ch);
		while ( (ch = getc()) != -1 && ch != US && count <= 15 )
			ss << char(ch);
		info.UPI = ss.str();

		if ( count > 15 )
			format_error("US");
	}

	if ( ch != US )
		format_error("US");

	//////////////////////////////////////////////////////////////
	// Extract selector code
	//////////////////////////////////////////////////////////////
	{
		std::stringstream ss;
		unsigned count = 0;

		while ( (ch = getc()) != -1 && ( isalpha(ch) || ch == '-' ) && ++count <= 5 )
			ss << char(ch);

		info.selector_code = ss.str();
		if ( count > 5 )
			format_error("LF");
	}

	if ( ch == '\r' )
		ch = getc();
	if ( ch != '\n' )
		format_error("LF");


	//////////////////////////////////////////////////////////////
	// Extract Priority of Story
	//////////////////////////////////////////////////////////////

	ch = getc();
	if ( !isalpha(ch) )
		format_error("<alpha>");

	info.priority = ch;

	ch = getc();
	if ( ch != ' ' )
		format_error("<space>");

	info.category = (ch = getc());
	if ( !isalpha(ch) )
		format_error("<alpha>");

	//////////////////////////////////////////////////////////////
	// Get format identifier
	//////////////////////////////////////////////////////////////

	info.agate = info.agate_tabular = false;
	info.body = info.body_text = false;

	ch = getc();

	if ( ch == DC3 ) {
		info.body = true;
		ch = getc();
		if ( ch == DC1 )
			info.body_text = true;
		else if ( ch != DC2 )
			format_error("DC1/DC2");
	} else if ( ch == DC4 ) {
		info.agate = true;
		ch = getc();
		if ( ch == DC2 )
			info.agate_tabular = true;
		else if ( ch != DC1 )
			format_error("DC1/DC2");
	} else	{
		format_error("DC3/DC4");
	}
	
	ch = getc();
	if ( ch != ' ' )
		format_error("SPACE");

	//////////////////////////////////////////////////////////////
	// Get keywords
	//////////////////////////////////////////////////////////////

	{
		std::stringstream ss;
		unsigned count = 0;

		while ( (ch = getc()) != -1 && ( isalnum(ch) || ch == '-' ) && ++count <= 24 )
			ss << char(ch);

		const std::string kw = ss.str();
		info.keywords = kw;
	}

	if ( ch != ' ' )
		format_error("SPACE");

	//////////////////////////////////////////////////////////////
	// Extract optional version
	//////////////////////////////////////////////////////////////

	{
		std::stringstream ss;

		while ( (ch = getc()) != -1 && ch != ' ' )
			ss << char(ch);

		info.version = ss.str();
	}

	if ( ch != ' ' )
		format_error("SPACE");

	//////////////////////////////////////////////////////////////
	// Extract optional reference
	//////////////////////////////////////////////////////////////

	{
		std::stringstream ss;

		if ( info.version == "" ) {
			// For some reason, when version is null, there is an extra space
			ch = getc();
			if ( ch != ' ' ) {
				ungetc(ch,in);
				--offset;
			}
		}

		while ( (ch = getc()) != -1 && ch != ' ' )
			ss << char(ch);

		info.reference = ss.str();
	}

	if ( ch != ' ' )
		format_error("SPACE");

	//////////////////////////////////////////////////////////////
	// Extract filing date
	//////////////////////////////////////////////////////////////

	{
		std::stringstream ss;
		unsigned count = 0;

		while ( (ch = getc()) != -1 && ( isdigit(ch) || ch == '-' ) && ++count <= 5 )
			ss << char(ch);

		info.filing_date = ss.str();
	}

	if ( ch != ' ' )
		format_error("SPACE");

	//////////////////////////////////////////////////////////////
	// Extract the word count
	//////////////////////////////////////////////////////////////

	{
		std::stringstream ss;
		unsigned count = 0;

		while ( (ch = getc()) != -1 && isdigit(ch) && ++count <= 4 )
			ss << char(ch);

		info.word_count = unsigned(atoi(ss.str().c_str()));
	}

	if ( ch == '\r' )
		ch = getc();

	if ( ch == '\n' )
		ch = getc();

	if ( ch != STX ) 
		format_error("STX");

	//////////////////////////////////////////////////////////////
	// Now extract the body
	//////////////////////////////////////////////////////////////
	{
		std::stringstream ss;

		while ( (ch = getc()) != -1 && ch != ETX )
			ss << char(ch);

		info.text = ss.str();
	}

	if ( ch != ETX )
		format_error("ETX");

	//////////////////////////////////////////////////////////////
	// Extract the time of transmission
	//////////////////////////////////////////////////////////////
	
	{
		std::stringstream ss;

		while ( (ch = getc()) != -1 && (ch == '\r' || ch == '\n') )
			;

		if ( ch != -1 ) {
			do	{
				ss << char(ch);
			} while ( (ch = getc()) != -1 && ch != '\r' && ch != '\n' );
		}

		info.date_transmission = ss.str();
	}

	while ( ch == '\r' || ch == '\n' )
		ch = getc();

	if ( ch != EOT )
		format_error("EOT");

	while ( (ch = getc()) != -1 && ( ch == '\r' || ch == '\n' ) )
		;

	if ( (ch = getc()) != -1 )
		format_error("EOF");

	fclose(in);

	printf("SERVICE-LEVEL-DESIGNATOR: %c\n",info.service_level_designator);	
	printf("STORY-NUMBER: %u\n",info.story_number);
	printf("UPI: %s\n",info.UPI.c_str());
	printf("SELECTOR-CODE: %s\n",info.selector_code.c_str());
	printf("PRIORITY: %c\n",info.priority);
	printf("CATEGORY: %c\n",info.category);
	if ( info.agate )
		printf("AGATE: %s\n",info.agate_tabular ? "tabular" : "text");
	if ( info.body )
		printf("BODY: %s\n",info.body_text ? "text" : "tabular");		
	printf("KEYWORDS: %s\n",info.keywords.c_str());
	printf("VERSION: %s\n",info.version.c_str());
	printf("REFERENCE: %s\n",info.reference.c_str());
	printf("FILING-DATE: %s\n",info.filing_date.c_str());
	printf("WORD-COUNT: %u\n",info.word_count);
	printf("TRANSMISSION-DATE: %s\n",info.date_transmission.c_str());
	printf("\n%s\n",info.text.c_str());
}


static void
usage(const char *cmdpath) {
	fprintf(stderr,"Usage:\t%s [-f] [-h] file1 [filen]\n"
		"\t-f\tSuppress FF between story output\n"
		"\t-h\tThis usage info.\n",
		cmdpath);
}

int
main(int argc,char **argv) {
	static const char cmdopts[] = ":fh";
	bool opt_errs = false;
	int optch;
	
	while ( (optch = getopt(argc,argv,cmdopts)) != -1 ) {
		switch ( optch ) {
		case 'h':
			usage(argv[0]);
			exit(0);
		case 'f':
			opt_noFF = true;	// Suppress FF between stories
			break;
		case '?':
			fprintf(stderr,"Unsupported optioin -%c\n",optch);
			opt_errs = true;
			break;
		case ':':
			fprintf(stderr,"Option -%c requires an argument.\n",optopt);
			opt_errs = true;
			break;
		default:
			fprintf(stderr,"Unsupported option: -%c\n",optch);
			opt_errs = true;
		}
	}

	if ( opt_errs || optind >= argc ) {
		usage(argv[0]);
		exit(XIT_CMDLINE);
	}

	for ( ; optind < argc; ++optind )
		extract(argv[optind]);

	return 0;
}

// End anpa1312.cpp
