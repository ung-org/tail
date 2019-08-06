/*
 * UNG's Not GNU
 *
 * Copyright (c) 2011-2019, Jakob Kaivo <jkk@ung.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define _POSIX_C_SOURCE 2
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum { BYTES, LINES };

static int tail(const char *path, int follow, int unit, intmax_t count)
{
	FILE *f = stdin;
	if (path && strcmp(path, "-")) {
		f = fopen(path, "r");
	}

	if (f == NULL) {
		fprintf(stderr, "tail: %s: %s\n", path, strerror(errno));
		return 1;
	}

	int c;
	while (count > 0) {
		c = fgetc(f);
		if (unit == BYTES || c == '\n') {
			count--;
		}
	}

	while (count == 0 && (c = fgetc(f)) != EOF) {
		putchar(c);
	}

	while (count < 0) {
		/* work from the end */
	}

	while (follow) {
		char buf[BUFSIZ];
		ssize_t nread = read(fileno(f), buf, sizeof(buf));
		write(STDOUT_FILENO, buf, nread);
	}

	if (f != stdin) {
		fclose(f);
	}

	return 0;
}

void fixobsolete(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--")) {
			return;
		}

		if (!strcmp(argv[i], "-n")) {
			i++;
			continue;
		}

		if (!strcmp(argv[i], "-c")) {
			i++;
			continue;
		}

		if (argv[i][0] == '+') {
			if (!isdigit(argv[i][1])) {
				argv[i][0] = '-';
				continue;
			}

			fprintf(stderr, "tail: +# is obsolete; use -n or -l\n");
			/* TODO: convert */
		}

		if (argv[i][0] == '-' && isdigit(argv[i][1])) {
			fprintf(stderr, "tail: -# is obsolete; use -n or -l\n");
			char *opt = malloc(strlen(argv[i]) + 3);
			if (opt == NULL) {
				perror("tail");
				exit(1);
			}
			/* TODO */
			/* no suffix or 'l' => -l */
			/* 'c' => -c */
			/* 'b' => -c 512 * n */
		}
	}
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");

	int follow = 0;
	int type = LINES;
	intmax_t count = -10;

	int c;
	char *end;
	while ((c = getopt(argc, argv, "c:fn:")) != -1) {
		switch (c) {
		case 'c':
		case 'n':
			type = (c == 'c') ? BYTES : LINES;
			count = -(imaxabs(strtoimax(optarg, &end, 10)) - 1);
			if (*end != '\0') {
				fprintf(stderr, "tail: invalid count %s\n", optarg);
				return 1;
			}
			if (optarg[0] == '+') {
				count = -count;
			}
			break;

		case 'f':
			follow = 1;
			break;

		default:
			return 1;
		}
	}

	return tail(argv[optind], follow, type, count);
}
