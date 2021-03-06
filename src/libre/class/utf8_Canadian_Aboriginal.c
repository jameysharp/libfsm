/* Generated by libfsm */

#include LF_HEADER

#include <stddef.h>

#include <fsm/fsm.h>

struct fsm *
utf8_Canadian_Aboriginal_fsm(const struct fsm_options *opt)
{
	struct fsm *fsm;
	size_t i;

	struct fsm_state *s[6] = { 0 };

	fsm = fsm_new(opt);
	if (fsm == NULL) {
		return NULL;
	}

	for (i = 0; i < 6; i++) {
		s[i] = fsm_addstate(fsm);
		if (s[i] == NULL) {
			goto error;
		}
	}

	if (!fsm_addedge_literal(fsm, s[0], s[1], 0xe1)) { goto error; }
	for (i = 0x90; i <= 0x99; i++) {
		if (!fsm_addedge_literal(fsm, s[1], s[2], i)) { goto error; }
	}
	if (!fsm_addedge_literal(fsm, s[1], s[3], 0xa2)) { goto error; }
	if (!fsm_addedge_literal(fsm, s[1], s[4], 0xa3)) { goto error; }
	for (i = 0x80; i <= 0xbf; i++) {
		if (!fsm_addedge_literal(fsm, s[2], s[5], i)) { goto error; }
	}
	for (i = 0xb0; i <= 0xbf; i++) {
		if (!fsm_addedge_literal(fsm, s[3], s[5], i)) { goto error; }
	}
	for (i = 0x80; i <= 0xb5; i++) {
		if (!fsm_addedge_literal(fsm, s[4], s[5], i)) { goto error; }
	}

	fsm_setstart(fsm, s[0]);
	fsm_setend(fsm, s[5], 1);

	return fsm;

error:

	fsm_free(fsm);

	return NULL;
}

