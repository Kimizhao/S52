/*
 * Copyright © 2011 Ryan Lortie
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * licence, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 *
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#if defined(G_DISABLE_SINGLE_INCLUDES) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __G_ATOMIC_H__
#define __G_ATOMIC_H__

#include <glib/gtypes.h>

G_BEGIN_DECLS

gint                    g_atomic_int_get                      (volatile gint  *atomic);
void                    g_atomic_int_set                      (volatile gint  *atomic,
                                                               gint            newval);
void                    g_atomic_int_inc                      (volatile gint  *atomic);
gboolean                g_atomic_int_dec_and_test             (volatile gint  *atomic);
gboolean                g_atomic_int_compare_and_exchange     (volatile gint  *atomic,
                                                               gint            oldval,
                                                               gint            newval);
gint                    g_atomic_int_add                      (volatile gint  *atomic,
                                                               gint            val);
guint                   g_atomic_int_and                      (volatile guint *atomic,
                                                               guint           val);
guint                   g_atomic_int_or                       (volatile guint *atomic,
                                                               guint           val);
guint                   g_atomic_int_xor                      (volatile guint *atomic,
                                                               guint           val);

gpointer                g_atomic_pointer_get                  (volatile void  *atomic);
void                    g_atomic_pointer_set                  (volatile void  *atomic,
                                                               gpointer        newval);
gboolean                g_atomic_pointer_compare_and_exchange (volatile void  *atomic,
                                                               gpointer        oldval,
                                                               gpointer        newval);
gssize                  g_atomic_pointer_add                  (volatile void  *atomic,
                                                               gssize          val);
gsize                   g_atomic_pointer_and                  (volatile void  *atomic,
                                                               gsize           val);
gsize                   g_atomic_pointer_or                   (volatile void  *atomic,
                                                               gsize           val);
gsize                   g_atomic_pointer_xor                  (volatile void  *atomic,
                                                               gsize           val);

#ifndef G_DISABLE_DEPRECATED
gint                    g_atomic_int_exchange_and_add         (volatile gint  *atomic,
                                                               gint            val);
#endif

G_END_DECLS

#if defined(__GNUC__) && defined(G_ATOMIC_OP_USE_GCC_BUILTINS)

#define g_atomic_int_get(atomic) \
  (G_GNUC_EXTENSION ({                                                          \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gint));                     \
    (void) (0 ? *(atomic) ^ *(atomic) : 0);                                  \
    __sync_synchronize ();                                                   \
    (gint) *(atomic);                                                        \
  }))
#define g_atomic_int_set(atomic, newval) \
  (G_GNUC_EXTENSION ({                                                          \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gint));                     \
    (void) (0 ? *(atomic) ^ (newval) : 0);                                   \
    *(atomic) = (newval);                                                    \
    __sync_synchronize ();                                                   \
  }))
#define g_atomic_int_inc(atomic) \
  (G_GNUC_EXTENSION ({                                                          \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gint));                     \
    (void) (0 ? *(atomic) ^ *(atomic) : 0);                                  \
    (void) __sync_fetch_and_add ((atomic), 1);                               \
  }))
#define g_atomic_int_dec_and_test(atomic) \
  (G_GNUC_EXTENSION ({                                                          \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gint));                     \
    (void) (0 ? *(atomic) ^ *(atomic) : 0);                                  \
    __sync_fetch_and_sub ((atomic), 1) == 1;                                 \
  }))
#define g_atomic_int_compare_and_exchange(atomic, oldval, newval) \
  (G_GNUC_EXTENSION ({                                                          \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gint));                     \
    (void) (0 ? *(atomic) ^ (newval) ^ (oldval) : 0);                        \
    (gboolean) __sync_bool_compare_and_swap ((atomic), (oldval), (newval));  \
  }))
#define g_atomic_int_add(atomic, val) \
  (G_GNUC_EXTENSION ({                                                          \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gint));                     \
    (void) (0 ? *(atomic) ^ (val) : 0);                                      \
    (gint) __sync_fetch_and_add ((atomic), (val));                           \
  }))
#define g_atomic_int_and(atomic, val) \
  (G_GNUC_EXTENSION ({                                                          \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gint));                     \
    (void) (0 ? *(atomic) ^ (val) : 0);                                      \
    (guint) __sync_fetch_and_and ((atomic), (val));                          \
  }))
#define g_atomic_int_or(atomic, val) \
  (G_GNUC_EXTENSION ({                                                          \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gint));                     \
    (void) (0 ? *(atomic) ^ (val) : 0);                                      \
    (guint) __sync_fetch_and_or ((atomic), (val));                           \
  }))
#define g_atomic_int_xor(atomic, val) \
  (G_GNUC_EXTENSION ({                                                          \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gint));                     \
    (void) (0 ? *(atomic) ^ (val) : 0);                                      \
    (guint) __sync_fetch_and_xor ((atomic), (val));                          \
  }))

#define g_atomic_pointer_get(atomic) \
  (G_GNUC_EXTENSION ({                                                          \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gpointer));                 \
    __sync_synchronize ();                                                   \
    (gpointer) *(atomic);                                                    \
  }))
#define g_atomic_pointer_set(atomic, newval) \
  (G_GNUC_EXTENSION ({                                                          \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gpointer));                 \
    (void) (0 ? (gpointer) *(atomic) : 0);                                   \
    *(atomic) = (__typeof__ (*(atomic))) (gsize) (newval);                   \
    __sync_synchronize ();                                                   \
  }))
#define g_atomic_pointer_compare_and_exchange(atomic, oldval, newval) \
  (G_GNUC_EXTENSION ({                                                          \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gpointer));                 \
    (void) (0 ? (gpointer) *(atomic) : 0);                                   \
    (gboolean) __sync_bool_compare_and_swap ((atomic), (oldval), (newval));  \
  }))
#define g_atomic_pointer_add(atomic, val) \
  (G_GNUC_EXTENSION ({                                                          \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gpointer));                 \
    (void) (0 ? (gpointer) *(atomic) : 0);                                   \
    (void) (0 ? (val) ^ (val) : 0);                                          \
    (gssize) __sync_fetch_and_add ((atomic), (val));                         \
  }))
#define g_atomic_pointer_and(atomic, val) \
  (G_GNUC_EXTENSION ({                                                          \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gpointer));                 \
    (void) (0 ? (gpointer) *(atomic) : 0);                                   \
    (void) (0 ? (val) ^ (val) : 0);                                          \
    (gsize) __sync_fetch_and_and ((atomic), (val));                          \
  }))
#define g_atomic_pointer_or(atomic, val) \
  (G_GNUC_EXTENSION ({                                                          \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gpointer));                 \
    (void) (0 ? (gpointer) *(atomic) : 0);                                   \
    (void) (0 ? (val) ^ (val) : 0);                                          \
    (gsize) __sync_fetch_and_or ((atomic), (val));                           \
  }))
#define g_atomic_pointer_xor(atomic, val) \
  (G_GNUC_EXTENSION ({                                                          \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gpointer));                 \
    (void) (0 ? (gpointer) *(atomic) : 0);                                   \
    (void) (0 ? (val) ^ (val) : 0);                                          \
    (gsize) __sync_fetch_and_xor ((atomic), (val));                          \
  }))

#else /* defined(__GNUC__) && defined(G_ATOMIC_OP_USE_GCC_BUILTINS) */

#define g_atomic_int_get(atomic) \
  (g_atomic_int_get ((gint *) (atomic)))
#define g_atomic_int_set(atomic, newval) \
  (g_atomic_int_set ((gint *) (atomic), (gint) (newval)))
#define g_atomic_int_compare_and_exchange(atomic, oldval, newval) \
  (g_atomic_int_compare_and_exchange ((gint *) (atomic), (oldval), (newval)))
#define g_atomic_int_add(atomic, val) \
  (g_atomic_int_add ((gint *) (atomic), (val)))
#define g_atomic_int_and(atomic, val) \
  (g_atomic_int_and ((gint *) (atomic), (val)))
#define g_atomic_int_or(atomic, val) \
  (g_atomic_int_or ((gint *) (atomic), (val)))
#define g_atomic_int_xor(atomic, val) \
  (g_atomic_int_xor ((gint *) (atomic), (val)))
#define g_atomic_int_inc(atomic) \
  (g_atomic_int_inc ((gint *) (atomic)))
#define g_atomic_int_dec_and_test(atomic) \
  (g_atomic_int_dec_and_test ((gint *) (atomic)))

#define g_atomic_pointer_get(atomic) \
  (g_atomic_pointer_get (atomic))
#define g_atomic_pointer_set(atomic, newval) \
  (g_atomic_pointer_set ((atomic), (gpointer) (newval)))
#define g_atomic_pointer_compare_and_exchange(atomic, oldval, newval) \
  (g_atomic_pointer_compare_and_exchange ((atomic), (gpointer) (oldval), (gpointer) (newval)))
#define g_atomic_pointer_add(atomic, val) \
  (g_atomic_pointer_add ((atomic), (gssize) (val)))
#define g_atomic_pointer_and(atomic, val) \
  (g_atomic_pointer_and ((atomic), (gsize) (val)))
#define g_atomic_pointer_or(atomic, val) \
  (g_atomic_pointer_or ((atomic), (gsize) (val)))
#define g_atomic_pointer_xor(atomic, val) \
  (g_atomic_pointer_xor ((atomic), (gsize) (val)))

#endif /* defined(__GNUC__) && defined(G_ATOMIC_OP_USE_GCC_BUILTINS) */

#endif /* __G_ATOMIC_H__ */