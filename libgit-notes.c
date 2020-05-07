#include <git2.h>
#include <stdio.h>
#include <assert.h>

#define GIT_REPO "."
#define DEFAULT_NOTES_REF "refs/notes/commits"

/**
 * gcc notes.c -o notes -L/usr/local/lib -lgit2 -I/usr/local/include
 * https://github.com/libgit2/libgit2/tree/master/tests/notes
 * https://git-scm.com/docs/git-notes
 *
 * git init
 * git notes add [<object>]
 * git notes get-ref
 */

git_repository* open_repository(const char *path) {
  git_repository *repo = NULL;
  if (!(git_repository_open_ext(&repo, path, GIT_REPOSITORY_OPEN_NO_SEARCH, NULL) == 0)) {
    return NULL;
  }
  printf("%s is a GIT repository\n", path);
  int error = git_repository_open(&repo, path);
  if (error < 0) {
    const git_error *e = giterr_last();
    printf("Error %d/%d: %s\n", error, e->klass, e->message);
    exit(error);
  }

  return repo;
}

void print_notes() {
  git_repository *r = open_repository(GIT_REPO);
  printf("print notes\n");
  git_oid *annotated_id = NULL, *note_id = NULL;
  git_note_iterator *note_iter = NULL;
  int error = git_note_iterator_new(&note_iter, r, "refs/notes/commits");
  git_note_next(note_id, annotated_id, note_iter);
  git_note_iterator_free(note_iter);
  git_repository_free(r);
}

void print_default_note_ref() {
  git_repository *r = open_repository(GIT_REPO);
  printf("print default note ref\n");
  git_buf *out;
  git_note_default_ref(out, r);
  printf("def note reference %s\n", out);
  git_repository_free(r);
}

void print_references() {
  printf("print references\n");
  git_repository *r = open_repository(GIT_REPO);
  int error = 0;
  git_reference_iterator *ref_iter = NULL;
  error = git_reference_iterator_new(&ref_iter, r);
  git_reference *ref = NULL;
  const char *str = NULL;
  while (!(error = git_reference_next(&ref, ref_iter))) {
    error = git_reference_lookup(&ref, r, "refs/heads/master");
    str = git_reference_name(ref);
    printf("ref %s\n", str);
  }
  if (error != GIT_ITEROVER) {
    printf("not a GIT_ITEROVER\n");
  }
  git_repository_free(r);
}

static int show_note_cb(const git_oid *blob_id, const git_oid *annotated_obj_id, void *payload)
{
  /*
  char buf[40];
  git_oid_fmt(buf, annotated_obj_id);
  buf[40] = '\0';
  printf("%s\n", buf);
  git_oid_fmt(buf, annotated_obj_id);
  printf("%s\n", buf);
  */

  git_repository *repo = (git_repository*)payload;
  git_note *git_note;
  int rc = git_note_read(&git_note, repo, DEFAULT_NOTES_REF, annotated_obj_id);
  if (rc != 0) {
    const git_error *e = giterr_last();
    printf("Error %d/%d: %s\n", rc, e->klass, e->message);
    return -1;
  }
  const char *note_msg = git_note_message(git_note);
  printf("Message: %s\n", note_msg);
  const git_signature *signature = NULL;
  signature = git_note_author(git_note);
  if (signature) {
    printf("Author: %s\n", signature->name);
  }
  return 0;
}

struct payload {
  git_repository *repo;
  const char *notes_ref;
};

int main (int argc, char** argv)
{
  git_libgit2_init();
  /* examples */
  print_references();
  git_repository *r = open_repository(GIT_REPO);
  git_note_foreach(r, DEFAULT_NOTES_REF, show_note_cb, r);
  git_repository_free(r);
  //print_default_note_ref();	// FIXME
  //print_notes();		// FIXME

  git_libgit2_shutdown();
}
