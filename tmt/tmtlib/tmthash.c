#ifdef __cplusplus
extern "C" {
#endif
#include "tmthash.h"

guint 
tmt_uint64_hash( gconstpointer key )
{
#if 0
  guint64 v;
  guint h;
  v = *((const guint64 *) key);
  h = ((guint) (v >> 32)) ^ ((guint)(v));
  return (h);
#else
  /* this method may be faster */
  const guint32 *v = (const guint32 *)key;
  return (v[0]^v[1]);
#endif
}

gboolean 
tmt_uint64_equal( gconstpointer a, gconstpointer b )
{
  return *((const guint64*) a) == *((const guint64*) b);
}
#ifdef __cplusplus
}
#endif
