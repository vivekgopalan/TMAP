#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "fmap_error.h"
#include "fmap_alloc.h"
#include "fmap_definitions.h"
#include "fmap_fq.h"

inline fmap_fq_t *
fmap_fq_init()
{
  fmap_fq_t *s = fmap_calloc(1, sizeof(fmap_fq_t), "s");
  s->name = fmap_string_init(0);
  s->comment = fmap_string_init(0);
  s->seq = fmap_string_init(0);
  s->qual= fmap_string_init(0);
  s->is_int = 0;

  return s;
}

inline void
fmap_fq_destroy(fmap_fq_t *seq)
{
  fmap_string_destroy(seq->name);
  fmap_string_destroy(seq->comment);
  fmap_string_destroy(seq->seq);
  fmap_string_destroy(seq->qual);
  free(seq);
}

inline fmap_fq_t*
fmap_fq_clone(fmap_fq_t *seq)
{
  fmap_fq_t *ret = fmap_calloc(1, sizeof(fmap_fq_t), "ret");

  ret->name = fmap_string_clone(seq->name);
  ret->comment = fmap_string_clone(seq->comment);
  ret->seq = fmap_string_clone(seq->seq);
  ret->qual = fmap_string_clone(seq->qual);
  ret->is_int = seq->is_int;

  return ret;
}

void
fmap_fq_reverse_compliment(fmap_fq_t *fq)
{
  fmap_string_reverse_compliment(fq->seq, fq->is_int);
  fmap_string_reverse(fq->qual);
}

void
fmap_fq_to_int(fmap_fq_t *seq)
{
  int i;
  if(1 == seq->is_int) return;
  for(i=0;i<seq->seq->l;i++) {
      seq->seq->s[i] = nt_char_to_int[(int)seq->seq->s[i]];
  }
  seq->is_int = 1;
}

void
fmap_fq_to_char(fmap_fq_t *seq)
{
  int i;
  if(0 == seq->is_int) return;
  for(i=0;i<seq->seq->l;i++) {
      seq->seq->s[i] = "ACGTN"[(int)seq->seq->s[i]];
  }
  seq->is_int = 0;
}
