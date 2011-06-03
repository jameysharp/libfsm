/* $Id$ */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fsm/graph.h> /* XXX */
#include <re/re.h>

#include "parser.h"
#include "internal.h"

#include "libfsm/internal.h" /* XXX */
#include "libre/internal.h" /* XXX */

#include "ast.h"

static void usage(void) {
	printf("usage: lx [-h] [-l <language>] <input>\n");
}

/* TODO: centralise */
static FILE *xopen(int argc, char * const argv[], int i, FILE *f, const char *mode) {
	if (argc <= i || 0 == strcmp("-", argv[i])) {
		return f;
	}

	f = fopen(argv[i], mode);
	if (f == NULL) {
		perror(argv[i]);
		exit(EXIT_FAILURE);
	}

	return f;
}

int main(int argc, char *argv[]) {
	struct ast *ast;
	enum lx_out format = LX_OUT_C;
	FILE *in;

	{
		int c;

		while ((c = getopt(argc, argv, "hvl:")) != -1) {
			switch (c) {
			case 'h':
				usage();
				exit(EXIT_SUCCESS);

			case 'l':
				if (0 == strcmp(optarg, "dot")) {
					format = LX_OUT_DOT;
				} else if (0 == strcmp(optarg, "c")) {
					format = LX_OUT_C;
				} else if (0 == strcmp(optarg, "h")) {
					format = LX_OUT_H;
				} else {
					fprintf(stderr, "unrecognised output language; valid languages are: c, h, dot\n");
					exit(EXIT_FAILURE);
				}
				break;

			case '?':
			default:
				usage();
				exit(EXIT_FAILURE);
			}

			argc -= optind;
			argv += optind;
		}

		if (argc > 1) {
			usage();
			exit(EXIT_FAILURE);
		}

		in = xopen(argc, argv, 1, stdin, "r");
	}

	ast = lx_parse(in);
	if (ast == NULL) {
		return EXIT_FAILURE;
	}

	/*
	 * What happens here is a transformation of the AST. This is equivalent to
	 * compiling to produce an IR, however the IR in lx's case is practically
	 * identical to the AST, so the same data structure is kept for simplicity.
	 *
	 * The important steps are:
	 *  1. Set FSM end state to point to each ast_mapping;
	 *  2. Minimize each mapping's regex;
	 *  3. Union all regexps within one mapping to produce one regexp per zone;
	 *  4. NFA->DFA on each zone's regexp.
	 *
	 * This produces as minimal state machines as possible, without losing track
	 * of which end state is associated with each mapping. In other words,
	 * without losing track of which regexp maps to which token.
	 */
	{
		struct ast_zone    *z;
		struct ast_mapping *m;

		for (z = ast->zl; z != NULL; z = z->next) {
			assert(z->re == NULL);

			z->re = re_new_empty();
			if (z->re == NULL) {
				return EXIT_FAILURE;
			}

			for (m = z->ml; m != NULL; m = m->next) {
				assert(m->re != NULL);
				assert(m->re->fsm != NULL);

				/* XXX: abstraction */
				if (!fsm_minimize(m->re->fsm)) {
					return EXIT_FAILURE;
				}

				/* potentially invalidated by fsm_minimize */
				/* TODO: maybe it would be convenient to re-find this, if neccessary */
				m->re->end = NULL;

				if (!re_union(z->re, m->re)) {
					return EXIT_FAILURE;
				}

				m->re = NULL;   /* TODO: free properly somehow? or clone first? */
			}

			/* TODO: note this makes re->end invalid. that's what i get for breaking abstraction */
			if (!fsm_todfa(z->re->fsm)) {
				return EXIT_FAILURE;
			}
		}
	}

	/*
	 * Semantic checks.
	 */
	{
		struct ast_zone  *z;
		struct fsm_state *s;

		for (z = ast->zl; z != NULL; z = z->next) {
			assert(z->re != NULL);

			for (s = z->re->fsm->sl; s != NULL; s = s->next) {
				if (!fsm_isend(z->re->fsm, s)) {
					continue;
				}

				assert(s->cl != NULL);

				if (s->cl->next != NULL) {
					fprintf(stderr, "conflict\n");  /* TODO: give some useful output */
					return EXIT_FAILURE;
				}
			}

		}
	}

	/* TODO: free ast */

	lx_print(ast, stdout, format);

	return EXIT_SUCCESS;
}

