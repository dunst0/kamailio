/**
 * @file ruuid.c
 * @author Rick Barenthin <rick@ng-voice.com>
 * @author Rick Barenthin <dunst0@gmail.com>
 * @date 12.05.2023
 * @brief This file is an example executable,
 *        which is using the ruuid rust library.
 */

#include <stdio.h>
#include <string.h>

#include <ruuid.h>

int main(int argc, char *argv[])
{
	char uuid_string[RUUID_FORMATTING_MAX_LENGTH] = {0};

	if(argc != 2) {
		fprintf(stderr, "ERROR: please give UUID to parse\n");
		return 1;
	}

	int ret = -1;
	ruuid *uuid = ruuid_parse(argv[1]);
	if(!uuid) {
		fprintf(stderr,
				"ERROR: give string \"%s\" could not be parsed as UUID\n",
				argv[1]);
		return -1;
	}

	memset(uuid_string, 0, RUUID_FORMATTING_MAX_LENGTH);
	if(ruuid_get_hyphenated(uuid, uuid_string, RUUID_FORMATTING_MAX_LENGTH)
			> 0) {
		fprintf(stdout, "%s\n", uuid_string);
	}

	ret = ruuid_is_nil(uuid);

	ruuid_destroy(uuid);

	return ret == 0 ? 0 : 1;
}
