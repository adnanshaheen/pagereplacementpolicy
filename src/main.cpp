/**
 * Header files
 */
#include "support.h"
#include "log.h"
#include "PageRepPolicy.h"

/**
 * options structure
 * keep track of the following flags
 * debug: to enable/didsable debugging
 * source filename to read information
 */
struct _opts {
	int debug;		/* Debug */
	char* filename;		/* file name of source file */
} opts;

/**
 * Signal handler in case of Ctrl+C or Segmentation fault
 * Not doing anything for the moment
 */
void signal_handler(int arg __attribute__((unused)))
{
	/**
	 * TODO:
	 * Please write some code here
	 */
}

/**
 * Usage:
 * Format of the binary
 * Display help how to use the binary
 */
void Usage()
{
	printf("Usage: prp [options]\n"
		"\n"
		"    Page Repleacement Policy\n"
		"    prp -f <filename>\n"
		"\n"
#ifdef DEBUG
		"    -d, --debug             Show debugging information\n"
#endif // DEBUG
		"\n");
}

/**
 * parse_options:
 * argc: number of parameters provided
 * argv: parameters list
 * 
 * Parse all the available options, and update the requirements
 * If there is any issue, display the help
 */
int parse_options(int argc, char **argv)
{
	debug_log("Entering %s ...", __FUNCTION__);	/* tracing code for debugging */

	const char *pOpt = "-f:"; /* Format of application */
	const struct option cOpt[] = {
#ifdef DEBUG
		{ "debug",	no_argument,		NULL, 'd' },	/* debug */
#endif
		{ "filename",	required_argument,	NULL, 'f' },	/* filename, requires another argument for name */
		{ NULL, 0, NULL, 0 }
	};

	int nRes = 0;
	int c = 0;
	int err = 0;
	bool bIsSource = false;
	memset(&opts, 0, sizeof(opts));

	/* Parse the options */
	while ((c = getopt_long(argc, argv, pOpt, cOpt, NULL)) != -1 && err == 0) {
		switch (c) {
#ifdef DEBUG
		case 'd':
			opts.debug = 1;		/* debugging logs */
			break;
#endif // DEBUG
		case 'f':
			if (bIsSource)
				err = 1;	/* we already have source, this shouldn't happen */
			else {
				bIsSource = true;
				opts.filename = argv[optind-1];	/* get input filename */
			}
			break;
		default:
			perr_printf("Invalid arguments");
			err = 1;
			break;
		}
	}

	if (!bIsSource)	/* Do we have any error? */
		err = 1;

	if (err) {
		Usage();	/* We do have an error, display program usage and return */
		nRes = -1;
	}

	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);	/* exit trace for debugging */

	return nRes;
}

/**
 * main:
 * argc: number of arguments from outside
 * argv: actual list of arguments
 */
int main(int argc, char* argv[])
{
	/* Set SIGINT handler. */
	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		perr_printf("Failed to set SIGINT handler");
		return 1;
	}
	/* Set SIGTERM handler. */
	if (signal(SIGTERM, signal_handler) == SIG_ERR) {
		perr_printf("Failed to set SIGTERM handler");
		return 1;
	}

	/* parse the options */
	if (parse_options(argc, argv)) {
		return 1;
	}

	/* create the class object and call the start function */
	CPageRepPolicy cPolicy(opts.filename);
	return cPolicy.Start();
}
