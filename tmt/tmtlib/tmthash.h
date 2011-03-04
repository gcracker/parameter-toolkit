#ifdef __cplusplus
extern "C" {
#endif
#ifndef TMT_HASH_H
#define TMT_HASH_H

#include <glib.h>

G_BEGIN_DECLS

guint 
tmt_uint64_hash( gconstpointer key );

gboolean 
tmt_uint64_equal( gconstpointer a, gconstpointer b );

G_END_DECLS

#endif /* TMT_HASH_H */
#ifdef __cplusplus
}
#endif
