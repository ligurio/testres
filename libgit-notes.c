#include <git2.h>
#include <stdio.h>

#define GIT_REPO "."
#define DEFAULT_NOTES_REF "refs/notes/commits"

/**
 * gcc notes.c -o notes -L/usr/local/lib -lgit2 -I/usr/local/include
 * https://github.com/libgit2/libgit2/tree/master/tests/notes
 */

static int note_list_cb(
	const git_oid *blob_id, const git_oid *annotated_obj_id, void *payload)
{
	printf("note_list_cb\n");
	git_repository *repo = (git_repository*)payload;
	git_note *git_note;
	int rc = 0;
	const char *note_msg;
	rc = git_note_read(&git_note, repo, DEFAULT_NOTES_REF, blob_id);
	if (rc != 0) {
		const git_error *e = giterr_last();
		printf("Error %d/%d: %s\n", rc, e->klass, e->message);
		exit(rc);
	}
	note_msg = git_note_message(git_note);
	printf("%s\n", note_msg);
	/*
	const git_signature *signature = NULL;
	signature = git_note_author(git_note);
	if (signature != NULL) {
		printf("author %s\n", signature->name);
	}
	*/

	return 0;
}

struct payload {
	git_repository *repo;
	const char *notes_ref;
};

int main (int argc, char** argv)
{
	if (!(git_repository_open_ext(
        NULL, GIT_REPO, GIT_REPOSITORY_OPEN_NO_SEARCH, NULL) == 0)) {
        exit(-1);
	}
	printf("%s is a GIT repository\n", GIT_REPO);
	git_libgit2_init();
	git_repository *repo = NULL;
	int rc = 0;
	rc = git_repository_open(&repo, GIT_REPO);
	if (rc < 0) {
		const git_error *e = giterr_last();
		printf("Error %d/%d: %s\n", rc, e->klass, e->message);
		exit(rc);
	}

	git_oid *annotated_id, *note_id;

/*
	git_note_iterator *iter;
	rc = git_note_iterator_new(&iter, repo, NULL);
	// rc = git_note_iterator_new(&iter, repo, "refs/notes/commits");
	if (rc < 0) {
		const git_error *e = giterr_last();
		printf("Error git_note_iterator_new() %d/%d: %s\n", rc, e->klass, e->message);
		exit(rc);
	}
	printf("iterator\n");
	rc = 0;
	while(rc != GIT_ITEROVER) {
		rc = git_note_next(note_id, annotated_id, iter);
		if (rc == GIT_ITEROVER) {
			break;
		}
		if (rc < 0) {
			const git_error *e = giterr_last();
			printf("Error git_note_next() %d/%d: %s\n", rc, e->klass, e->message);
			continue;
		}
		printf("XXX\n");
		const char *note_msg;
		git_note *git_note;
		git_note_read(&git_note, repo, "refs/notes/commits", note_id);
		note_msg = git_note_message(git_note);
		printf("%s\n", note_msg);
	}
	git_note_iterator_free(iter);
*/

	/*
	git_note_iterator *iter;
	git_note *note;
	git_commit *notes_commit;
	const char* note_message;
	int i, err;

	git_note_commit_iterator_new(&iter, notes_commit);
	for (i = 0; (err = git_note_next(note_id, annotated_id, iter)) >= 0; ++i) {
		git_note_commit_read(&note, repo, notes_commit, annotated_id);
		note_message = git_note_message(note);
		git_note_free(note);
	}
	git_note_iterator_free(iter);
	git_commit_free(notes_commit);
	*/

	/*
	git_buf *notes_ref;
	rc = git_note_default_ref(notes_ref, repo);
	if (rc != 0) {
		const git_error *e = giterr_last();
		printf("Error git_note_next() %d/%d: %s\n", rc, e->klass, e->message);
		exit(rc);
	}
	printf("%s\n", (const char*)notes_ref);
	*/

	git_note_foreach(repo, DEFAULT_NOTES_REF, note_list_cb, repo);
	git_libgit2_shutdown();
}
