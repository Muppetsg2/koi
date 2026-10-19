/* koi_image - v1.05 - public domain stb like image loader
               for more extensions - https://github.com/Muppetsg2/koi
               no warranty implied; use at your own risk

   Do this:
      #define KOI_IMAGE_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.

   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define KOI_IMAGE_IMPLEMENTATION
   #include "koi_image.h"

   You can #define KOI_ASSERT(x) before the #include to avoid using assert.h.
   And #define KOI_MALLOC and KOI_FREE to avoid using malloc, free


   QUICK NOTES:
      Primarily of interest to game developers and other people who can
      avoid problematic images and only need the trivial interface

      - decode from memory or through FILE (define KOI_NO_STDIO to remove code)
      - decode from arbitrary I/O callbacks

   Full documentation under "DOCUMENTATION" below.


LICENSE

   See end of file for license information.

RECENT REVISION HISTORY:

      1.05  (2026-10-19) Fixed short callback reads, FILE* position after loading and koi_loadf failure reason
      1.04  (2026-10-18) Fixed QOI run overflow, truncated data and 16-bit NULL handling, build fixes, unified formatting
      1.03  (2026-07-24) Fixed integer conversion warning in QOI loader
      1.02  (2026-04-30) Improved memory safety and made Windows file handling more robust
      1.01  (2025-11-20) Unified formatting of functions and tabs
      1.00  (2025-05-09) QOI file loader

   See end of file for full revision history.


 ============================    Contributors    =========================

   Image formats
      Marceli Antosik (qoi)
*/

#if !defined(KOI_INCLUDE_KOI_IMAGE_H)
#define KOI_INCLUDE_KOI_IMAGE_H

// DOCUMENTATION
//
// Basic usage:
//   int x,y,n;
//   unsigned char *data = koi_load(filename, &x, &y, &n, 0);
//   // ... process data if not NULL ...
//   // ... x = width, y = height, n = # 8-bit components per pixel ...
//   // ... replace '0' with '1'..'4' to force that many components per pixel
//   // ... 'n' will always be the number of channels in file
//   koi_image_free(data);
//
// Standard parameters:
//   int *x                 -- outputs image width in pixels
//   int *y                 -- outputs image height in pixels
//   int *channels_in_file  -- outputs # of image components in image file
//   int desired_channels   -- if non-zero, # of image components requested in result
//
// The return value from an image loader is an 'unsigned char *' which points
// to the pixel data, or NULL on an allocation failure or if the image is
// corrupt or invalid. The pixel data consists of *y scanlines of *x pixels,
// with each pixel consisting of N interleaved 8-bit components; the first
// pixel pointed to is top-left-most in the image. There is no padding between
// image scanlines or between pixels, regardless of format. The number of
// components N is 'desired_channels' if desired_channels is non-zero, or
// *channels_in_file otherwise. If desired_channels is non-zero,
// *channels_in_file has the number of components that _would_ have been
// output otherwise. E.g. if you set desired_channels to 4, you will always
// get RGBA output, but you can check *channels_in_file to see if it's trivially
// opaque because e.g. there were only 3 channels in the source image.
//
// An output image with N components has the following components interleaved
// in this order in each pixel:
//
//    N=#comp   components
//      1       grey
//      2       grey, alpha
//      3       red, green, blue
//      4       red, green, blue, alpha
//
// If image loading fails for any reason, the return value will be NULL,
// and *x, *y, *channels_in_file will be unchanged. The function
// koi_failure_reason() can be queried for an extremely brief, end-user
// unfriendly explanation of why the load failed. Define KOI_NO_FAILURE_STRINGS
// to avoid compiling these strings at all, and KOI_FAILURE_USERMSG to get slightly
// more user-friendly ones.
//
// To query the width, height and component count of an image without having to
// decode the full file, you can use the koi_info family of functions:
//
//   int x,y,n,ok;
//   ok = koi_info(filename, &x, &y, &n);
//   // returns ok=1 and sets x, y, n if image is a supported format,
//   // 0 otherwise.
//
// Note that koi_image pervasively uses ints in its public API for sizes,
// including sizes of memory buffers. This is now part of the API and thus
// hard to change without causing breakage. As a result, the various image
// loaders all have certain limits on image size; these differ somewhat
// by format but generally boil down to either just under 2GB or just under
// 1GB. When the decoded image would be larger than this, koi_image decoding
// will fail.
//
// Additionally, koi_image will reject image files that have any of their
// dimensions set to a larger value than the configurable KOI_MAX_DIMENSIONS,
// which defaults to 2**24 = 16777216 pixels. Due to the above memory limit,
// the only way to have an image with such dimensions load correctly
// is for it to have a rather extreme aspect ratio. Either way, the
// assumption here is that such larger images are likely to be malformed
// or malicious. If you do need to load an image with individual dimensions
// larger than that, and it still fits in the overall size limit, you can
// #define KOI_MAX_DIMENSIONS on your own to be something larger.
//
// ===========================================================================
//
// UNICODE
//
//   If compiling for Windows and you wish to use Unicode filenames, compile
//   with
//      #define KOI_WINDOWS_UTF8
//   and pass utf8-encoded filenames. Call koi_convert_wchar_to_utf8 to convert
//   Windows wchar_t filenames to utf8.
//
// ===========================================================================
//
// PHILOSOPHY
//
// koi libraries are designed with the following priorities:
//
//   1. easy to use
//   2. easy to maintain
//   3. good performance
//
// Sometimes I let "good performance" creep up in priority over "easy to maintain",
// and for best performance I may provide less-easy-to-use APIs that give higher
// performance, in addition to the easy-to-use ones. Nevertheless, it's important
// to keep in mind that from the standpoint of you, a client of this library,
// all you care about is #1 and #3, and koi libraries DO NOT emphasize #3 above all.
//
// Some secondary priorities arise directly from the first two, some of which
// provide more explicit reasons why performance can't be emphasized.
//
//   - Portable ("ease of use")
//   - Small source code footprint ("easy to maintain")
//   - No dependencies ("ease of use")
//
// ===========================================================================
//
// I/O CALLBACKS
//
// I/O callbacks allow you to read from arbitrary sources, like packaged
// files or some other source. Data read from callbacks are processed
// through a small internal buffer (currently 128 bytes) to try to reduce
// overhead.
//
// The two functions you must define are "read" (reads some bytes of data)
// and "skip" (skips some bytes of data).
//
// ===========================================================================
//
// ADDITIONAL CONFIGURATION
//
//  - You can suppress implementation of any of the decoders to reduce
//    your code footprint by #defining one or more of the following
//    symbols before creating the implementation.
//
//      KOI_NO_QOI
//
//  - You can request *only* certain decoders and suppress all other ones
//    (this will be more forward-compatible, as addition of new decoders
//    doesn't require you to disable them explicitly):
//
//      KOI_ONLY_QOI
//
//  - You can suppress implementation of any koi_loadf function to reduce
//    your code footprint by #defining KOI_NO_LINEAR before creating
//    the implementation.
//
//  - If you define KOI_MAX_DIMENSIONS, koi_image will reject images greater
//    than that size (in either width or height) without further processing.
//    This is to let programs in the wild set an upper bound to prevent
//    denial-of-service attacks on untrusted data, as one could generate a
//    valid image of gigantic dimensions and force koi_image to allocate a
//    huge block of memory and spend disproportionate time decoding it. By
//    default this is set to (1 << 24), which is 16777216, but that's still
//    very big.

#if !defined(KOI_NO_STDIO)
   #include <stdio.h>
#endif // !KOI_NO_STDIO

#define KOI_IMAGE_VERSION 3

enum
{
   KOI_default    = 0, // only used for desired_channels

   KOI_grey       = 1,
   KOI_grey_alpha = 2,
   KOI_rgb        = 3,
   KOI_rgb_alpha  = 4
};

#include <stdlib.h>
typedef unsigned char koi_uc;
typedef unsigned short koi_us;

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

#if !defined(KOIDEF)
   #if defined(KOI_IMAGE_STATIC)
      #define KOIDEF static
   #else
      #define KOIDEF extern
   #endif // KOI_IMAGE_STATIC
#endif // !KOIDEF

//////////////////////////////////////////////////////////////////////////////
//
// PRIMARY API - works on images of any type
//

//
// load image by filename, open file, or memory buffer
//

typedef struct
{
   int     (*read)  (void *user, char *data, int size);  // fill 'data' with 'size' bytes.  return number of bytes actually read
   void    (*skip)  (void *user, int n);                 // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
} koi_io_callbacks;

////////////////////////////////////
//
// 8-bits-per-channel interface
//

KOIDEF koi_uc *koi_load_from_memory    (koi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
KOIDEF koi_uc *koi_load_from_callbacks (koi_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels);

#if !defined(KOI_NO_STDIO)
KOIDEF koi_uc *koi_load                (char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
KOIDEF koi_uc *koi_load_from_file      (FILE *f, int *x, int *y, int *channels_in_file, int desired_channels);
// for koi_load_from_file, file pointer is left pointing immediately after image (after the QOI end marker)
#endif // !KOI_NO_STDIO

#if defined(KOI_WINDOWS_UTF8)
KOIDEF int koi_convert_wchar_to_utf8   (char *buffer, size_t bufferlen, const wchar_t *input);
#endif // KOI_WINDOWS_UTF8

////////////////////////////////////
//
// 16-bits-per-channel interface
//

KOIDEF koi_us *koi_load_16_from_memory    (koi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
KOIDEF koi_us *koi_load_16_from_callbacks (koi_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels);

#if !defined(KOI_NO_STDIO)
KOIDEF koi_us *koi_load_16                (char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
KOIDEF koi_us *koi_load_from_file_16      (FILE *f, int *x, int *y, int *channels_in_file, int desired_channels);
#endif // !KOI_NO_STDIO

////////////////////////////////////
//
// float-per-channel interface
//

#if !defined(KOI_NO_LINEAR)
KOIDEF float *koi_loadf_from_memory       (koi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
KOIDEF float *koi_loadf_from_callbacks    (koi_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels);

#if !defined(KOI_NO_STDIO)
KOIDEF float *koi_loadf                   (char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
KOIDEF float *koi_loadf_from_file         (FILE *f, int *x, int *y, int *channels_in_file, int desired_channels);
#endif // !KOI_NO_STDIO

KOIDEF void   koi_ldr_to_hdr_gamma        (float gamma);
KOIDEF void   koi_ldr_to_hdr_scale        (float scale);
#endif // !KOI_NO_LINEAR

// get a VERY brief reason for failure
// on most compilers (and ALL modern mainstream compilers) this is threadsafe
KOIDEF const char *koi_failure_reason        (void);

// free the loaded image -- this is just free()
KOIDEF void        koi_image_free            (void *retval_from_koi_load);

// get image dimensions & components without fully decoding
KOIDEF int         koi_info_from_memory      (koi_uc const *buffer, int len, int *x, int *y, int *channels_in_file);
KOIDEF int         koi_info_from_callbacks   (koi_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file);

#if !defined(KOI_NO_STDIO)
KOIDEF int         koi_info                  (char const *filename, int *x, int *y, int *channels_in_file);
KOIDEF int         koi_info_from_file        (FILE *f, int *x, int *y, int *channels_in_file);
#endif // !KOI_NO_STDIO

// flip the image vertically, so the first pixel in the output array is the bottom left
KOIDEF void koi_set_flip_vertically_on_load(int flag_true_if_should_flip);

// as above, but only applies to images loaded on the thread that calls the function
// this function is only available if your compiler supports thread-local variables;
// calling it will fail to link if your compiler doesn't
KOIDEF void koi_set_flip_vertically_on_load_thread(int flag_true_if_should_flip);

#if defined(__cplusplus)
}
#endif // __cplusplus

//
//
////   end header file   /////////////////////////////////////////////////////
#endif // !KOI_INCLUDE_KOI_IMAGE_H

#if defined(KOI_IMAGE_IMPLEMENTATION)

#if defined(KOI_ONLY_QOI)
   #if !defined(KOI_ONLY_QOI)
      #define KOI_NO_QOI
   #endif // !KOI_ONLY_QOI
#endif // KOI_ONLY_QOI

#include <string.h>
#include <limits.h> // INT_MAX

#if !defined(KOI_NO_LINEAR)
   #include <math.h> // ldexp, pow
#endif // !KOI_NO_LINEAR

#if !defined(KOI_NO_STDIO)
   #include <stdio.h>
#endif // !KOI_NO_STDIO

#if !defined(KOI_ASSERT)
   #include <assert.h>
   #define KOI_ASSERT(x) assert(x)
#endif // !KOI_ASSERT

#if defined(__cplusplus)
   #define KOI_EXTERN extern "C"
#else
   #define KOI_EXTERN extern
#endif // __cplusplus

#if !defined(_MSC_VER)
   #if defined(__cplusplus)
      #define koi_inline inline
   #else
      #define koi_inline
   #endif // __cplusplus
#else
   #define koi_inline __forceinline
#endif // !_MSC_VER

#if !defined(KOI_NO_THREAD_LOCALS)
   #if defined(__cplusplus) && __cplusplus >= 201103L
      #define KOI_THREAD_LOCAL      thread_local
   #elif defined(__GNUC__) && __GNUC__ < 5
      #define KOI_THREAD_LOCAL      __thread
   #elif defined(_MSC_VER)
      #define KOI_THREAD_LOCAL      __declspec(thread)
   #elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
      #define KOI_THREAD_LOCAL      _Thread_local
   #endif // __cplusplus >= 201103L

   #if !defined(KOI_THREAD_LOCAL)
      #if defined(__GNUC__)
         #define KOI_THREAD_LOCAL   __thread
      #endif // __GNUC__
   #endif // !KOI_THREAD_LOCAL
#endif // !KOI_NO_THREAD_LOCALS

#if defined(_MSC_VER) || defined(__SYMBIAN32__)
typedef unsigned short koi__uint16;
typedef unsigned int   koi__uint32;
typedef   signed int   koi__int32;
#else
#include <stdint.h>
typedef uint16_t       koi__uint16;
typedef uint32_t       koi__uint32;
typedef int32_t        koi__int32;
#endif // _MSC_VER || __SYMBIAN32__

// should produce compiler error if size is wrong
typedef unsigned char koi_validate_uint32[sizeof(koi__uint32) == 4 ? 1 : -1];

#if defined(_MSC_VER)
   #define KOI_NOTUSED(v)  (void)(v)
#else
   #define KOI_NOTUSED(v)  (void)sizeof(v)
#endif // _MSC_VER

#if defined(KOI_MALLOC) && defined(KOI_FREE)
   // ok
#elif !defined(KOI_MALLOC) && !defined(KOI_FREE)
   // ok
#else
   #error "Must define all or none of KOI_MALLOC and KOI_FREE"
#endif // KOI_MALLOC && KOI_FREE

#if !defined(KOI_MALLOC)
   #define KOI_MALLOC(sz)     malloc(sz)
   #define KOI_FREE(p)        free(p)
#endif // !KOI_MALLOC

#if !defined(KOI_MAX_DIMENSIONS)
   #define KOI_MAX_DIMENSIONS (1 << 24)
#endif // !KOI_MAX_DIMENSIONS

///////////////////////////////////////////////
//
//  koi__context struct and start_xxx functions

// koi__context structure is our basic context used by all images, so it
// contains all the IO context, plus some basic image information
typedef struct
{
   koi__uint32 img_x, img_y;
   int img_n, img_out_n;

   koi_io_callbacks io;
   void *io_user_data;

   int read_from_callbacks;
   int buflen;
   koi_uc buffer_start[128];
   int callback_already_read;

   koi_uc const *img_buffer, *img_buffer_end;
   koi_uc const *img_buffer_original, *img_buffer_original_end;
} koi__context;

static void koi__refill_buffer(koi__context *s);

// initialize a memory-decode context
static void koi__start_mem(koi__context *s, koi_uc const *buffer, int len)
{
   s->io.read = NULL;
   s->read_from_callbacks = 0;
   s->callback_already_read = 0;
   s->img_buffer = s->img_buffer_original = buffer;
   s->img_buffer_end = s->img_buffer_original_end = buffer + len;
}

// initialize a callback-based context
static void koi__start_callbacks(koi__context *s, const koi_io_callbacks *clbk, void *user)
{
   s->io = *clbk;
   s->io_user_data = user;
   s->buflen = sizeof(s->buffer_start);
   s->read_from_callbacks = 1;
   s->callback_already_read = 0;
   s->img_buffer = s->img_buffer_original = s->buffer_start;
   koi__refill_buffer(s);
   s->img_buffer_original_end = s->img_buffer_end;
}

#if !defined(KOI_NO_STDIO)
static int koi__stdio_read(void *user, char *data, int size)
{
   return (int)fread(data, 1, size, (FILE*)user);
}

static void koi__stdio_skip(void *user, int n)
{
   int ch;
   fseek((FILE*)user, n, SEEK_CUR);
   ch = fgetc((FILE*)user);      // have to read a byte to reset feof()'s flag
   if (ch != EOF) {
      ungetc(ch, (FILE*)user);   // push byte back onto stream if valid
   }
}

static const koi_io_callbacks koi__stdio_callbacks =
{
   koi__stdio_read,
   koi__stdio_skip,
};

static void koi__start_file(koi__context *s, FILE *f)
{
   koi__start_callbacks(s, &koi__stdio_callbacks, (void*)f);
}
#endif // !KOI_NO_STDIO

static void koi__rewind(koi__context *s)
{
   // conceptually rewind SHOULD rewind to the beginning of the stream,
   // but we just rewind to the beginning of the initial buffer, because
   // we only use it after doing 'test', which only ever looks at at most 92 bytes
   s->img_buffer = s->img_buffer_original;
   s->img_buffer_end = s->img_buffer_original_end;
}

typedef struct
{
   int bits_per_channel;
   int num_channels;
} koi__result_info;

#if !defined(KOI_NO_QOI)
static int     koi__qoi_test(koi__context *s);
static void   *koi__qoi_load(koi__context *s, int *x, int *y, int *channels_in_file, int desired_channels, koi__result_info *ri);
static int     koi__qoi_info(koi__context *s, int *x, int *y, int *channels_in_file);
#endif // !KOI_NO_QOI

static
#if defined(KOI_THREAD_LOCAL)
KOI_THREAD_LOCAL
#endif // KOI_THREAD_LOCAL
const char *koi__g_failure_reason;

KOIDEF const char *koi_failure_reason(void)
{
   return koi__g_failure_reason;
}

#if !defined(KOI_NO_FAILURE_STRINGS)
static int koi__err(const char *str)
{
   koi__g_failure_reason = str;
   return 0;
}
#endif // !KOI_NO_FAILURE_STRINGS

static void *koi__malloc(size_t size)
{
   return KOI_MALLOC(size);
}

// koi_image uses ints pervasively, including for offset calculations.
// therefore the largest decoded image size we can support with the
// current code, even on 64-bit targets, is INT_MAX. this is not a
// significant limitation for the intended use case.
//
// we do, however, need to make sure our size calculations don't
// overflow. hence a few helper functions for size calculations that
// multiply integers together, making sure that they're non-negative
// and no overflow occurs.

// return 1 if the sum is valid, 0 on overflow.
// negative terms are considered invalid.
static int koi__addsizes_valid(int a, int b)
{
   if (b < 0) return 0;
   // now 0 <= b <= INT_MAX, hence also
   // 0 <= INT_MAX - b <= INTMAX.
   // And "a + b <= INT_MAX" (which might overflow) is the
   // same as a <= INT_MAX - b (no overflow)
   return a <= INT_MAX - b;
}

// returns 1 if the product is valid, 0 on overflow.
// negative factors are considered invalid.
static int koi__mul2sizes_valid(int a, int b)
{
   if (a < 0 || b < 0) return 0;
   if (b == 0) return 1; // mul-by-0 is always safe
   // portable way to check for no overflows in a*b
   return a <= INT_MAX / b;
}

// returns 1 if "a*b*c + add" has no negative terms/factors and doesn't overflow
static int koi__mad3sizes_valid(int a, int b, int c, int add)
{
   return koi__mul2sizes_valid(a, b) && koi__mul2sizes_valid(a * b, c) &&
      koi__addsizes_valid(a * b * c, add);
}

// returns 1 if "a*b*c*d + add" has no negative terms/factors and doesn't overflow
static int koi__mad4sizes_valid(int a, int b, int c, int d, int add)
{
   return koi__mul2sizes_valid(a, b) && koi__mul2sizes_valid(a * b, c) &&
      koi__mul2sizes_valid(a * b * c, d) && koi__addsizes_valid(a * b * c * d, add);
}

static void *koi__malloc_mad3(int a, int b, int c, int add)
{
   if (!koi__mad3sizes_valid(a, b, c, add)) return NULL;
   return koi__malloc((size_t)a * b * c + add);
}

static void *koi__malloc_mad4(int a, int b, int c, int d, int add)
{
   if (!koi__mad4sizes_valid(a, b, c, d, add)) return NULL;
   return koi__malloc((size_t)a * b * c * d + add);
}

// koi__err - error
// koi__errpf - error returning pointer to float
// koi__errpuc - error returning pointer to unsigned char

#if defined(KOI_NO_FAILURE_STRINGS)
   #define koi__err(x, y)  0
#elif defined(KOI_FAILURE_USERMSG)
   #define koi__err(x, y)  koi__err(y)
#else
   #define koi__err(x, y)  koi__err(x)
#endif // KOI_NO_FAILURE_STRINGS

#define koi__errpf(x, y)   ((float*)(size_t)(koi__err(x, y) ? NULL : NULL))
#define koi__errpuc(x, y)  ((unsigned char*)(size_t)(koi__err(x, y) ? NULL : NULL))

KOIDEF void koi_image_free(void *retval_from_koi_load)
{
   KOI_FREE(retval_from_koi_load);
}

#if !defined(KOI_NO_LINEAR)
static float *koi__ldr_to_hdr(koi_uc *data, int x, int y, int channels);
#endif // !KOI_NO_LINEAR

static int koi__vertically_flip_on_load_global = 0;

KOIDEF void koi_set_flip_vertically_on_load(int flag_true_if_should_flip)
{
   koi__vertically_flip_on_load_global = flag_true_if_should_flip;
}

#if !defined(KOI_THREAD_LOCAL)
   #define koi__vertically_flip_on_load  koi__vertically_flip_on_load_global
#else
static KOI_THREAD_LOCAL int koi__vertically_flip_on_load_local, koi__vertically_flip_on_load_set;

KOIDEF void koi_set_flip_vertically_on_load_thread(int flag_true_if_should_flip)
{
   koi__vertically_flip_on_load_local = flag_true_if_should_flip;
   koi__vertically_flip_on_load_set = 1;
}

#define koi__vertically_flip_on_load  (koi__vertically_flip_on_load_set \
                                       ? koi__vertically_flip_on_load_local \
                                       : koi__vertically_flip_on_load_global)
#endif // !KOI_THREAD_LOCAL

static void *koi__load_main(koi__context *s, int *x, int *y, int *channels_in_file, int desired_channels, koi__result_info *ri)
{
   memset(ri, 0, sizeof(*ri)); // make sure it's initialized if we add new fields
   ri->bits_per_channel = 8;   // default is 8 so most paths don't have to be changed
   ri->num_channels = 0;

#if !defined(KOI_NO_QOI)
   if (koi__qoi_test(s)) return koi__qoi_load(s, x, y, channels_in_file, desired_channels, ri);
#endif // !KOI_NO_QOI

   return koi__errpuc("unknown image type", "Image not of any known type, or corrupt");
}

static koi__uint16 *koi__convert_8_to_16(koi_uc *orig, int w, int h, int channels)
{
   int i;
   int img_len;
   koi__uint16 *const enlarged = (koi__uint16*)koi__malloc_mad4(w, h, channels, 2, 0);

   if (enlarged == NULL) {
      KOI_FREE(orig);
      return (koi__uint16*)koi__errpuc("outofmem", "Out of memory");
   }

   img_len = w * h * channels;
   for (i = 0; i < img_len; ++i)
      enlarged[i] = (koi__uint16)((orig[i] << 8) + orig[i]); // replicate to high and low byte, maps 0->0, 255->0xffff

   KOI_FREE(orig);
   return enlarged;
}

static void koi__vertical_flip(void *image, int w, int h, int bytes_per_pixel)
{
   int row;
   const size_t bytes_per_row = (size_t)w * bytes_per_pixel;
   koi_uc temp[2048];
   koi_uc *const bytes = (koi_uc*)image;

   for (row = 0; row < (h >> 1); row++) {
      koi_uc *row0 = bytes + row * bytes_per_row;
      koi_uc *row1 = bytes + (size_t)(h - row - 1) * bytes_per_row;
      // swap row0 with row1
      size_t bytes_left = bytes_per_row;
      while (bytes_left) {
         const size_t bytes_copy = (bytes_left < sizeof(temp)) ? bytes_left : sizeof(temp);
         memcpy(temp, row0, bytes_copy);
         memcpy(row0, row1, bytes_copy);
         memcpy(row1, temp, bytes_copy);
         row0 += bytes_copy;
         row1 += bytes_copy;
         bytes_left -= bytes_copy;
      }
   }
}

static unsigned char *koi__load_and_postprocess_8bit(koi__context *s, int *x, int *y, int *channels_in_file, int desired_channels)
{
   koi__result_info ri;
   void *const result = koi__load_main(s, x, y, channels_in_file, desired_channels, &ri);

   if (result == NULL)
      return NULL;

   // it is the responsibility of the loaders to make sure we get 8 bit.
   KOI_ASSERT(ri.bits_per_channel == 8);

   if (koi__vertically_flip_on_load) {
      const int channels = desired_channels ? desired_channels : *channels_in_file;
      koi__vertical_flip(result, *x, *y, channels * sizeof(koi_uc));
   }

   return (unsigned char*)result;
}

static koi__uint16 *koi__load_and_postprocess_16bit(koi__context *s, int *x, int *y, int *channels_in_file, int desired_channels)
{
   koi__result_info ri;
   void *result = koi__load_main(s, x, y, channels_in_file, desired_channels, &ri);

   if (result == NULL)
      return NULL;

   // it is the responsibility of the loaders to make sure we get either 8 or 16 bit.
   KOI_ASSERT(ri.bits_per_channel == 8 || ri.bits_per_channel == 16);

   if (ri.bits_per_channel != 16) {
      result = koi__convert_8_to_16((koi_uc*)result, *x, *y, desired_channels == 0 ? *channels_in_file : desired_channels);
      if (result == NULL) return NULL; // koi__convert_8_to_16 frees input on failure
      ri.bits_per_channel = 16;
   }

   if (koi__vertically_flip_on_load) {
      const int channels = desired_channels ? desired_channels : *channels_in_file;
      koi__vertical_flip(result, *x, *y, channels * sizeof(koi__uint16));
   }

   return (koi__uint16*)result;
}

#if !defined(KOI_NO_STDIO)

#if defined(_WIN32) && defined(KOI_WINDOWS_UTF8)
KOI_EXTERN __declspec(dllimport) int __stdcall MultiByteToWideChar(unsigned int cp, unsigned long flags, const char *str, int cbmb, wchar_t *widestr, int cchwide);
KOI_EXTERN __declspec(dllimport) int __stdcall WideCharToMultiByte(unsigned int cp, unsigned long flags, const wchar_t *widestr, int cchwide, char *str, int cbmb, const char *defchar, int *used_default);

KOIDEF int koi_convert_wchar_to_utf8(char *buffer, size_t bufferlen, const wchar_t *input)
{
   const int len = (bufferlen > 0x7FFFFFFF) ? 0x7FFFFFFF : (int)bufferlen;
   return WideCharToMultiByte(65001 /* UTF8 */, 0, input, -1, buffer, len, NULL, NULL);
}
#endif // _WIN32 && KOI_WINDOWS_UTF8

static FILE *koi__fopen(char const *filename, char const *mode)
{
   FILE *f;
#if defined(_WIN32) && defined(KOI_WINDOWS_UTF8)
   wchar_t wMode[64];
   wchar_t wFilename[4096];
   if (0 == MultiByteToWideChar(65001 /* UTF8 */, 0, filename, -1, wFilename, (int)(sizeof(wFilename) / sizeof(*wFilename))))
      return NULL;

   if (0 == MultiByteToWideChar(65001 /* UTF8 */, 0, mode, -1, wMode, (int)(sizeof(wMode) / sizeof(*wMode))))
      return NULL;

#if defined(_MSC_VER) && _MSC_VER >= 1400
   if (0 != _wfopen_s(&f, wFilename, wMode))
      f = NULL;
#else
   f = _wfopen(wFilename, wMode);
#endif // _MSC_VER >= 1400

#elif defined(_MSC_VER) && _MSC_VER >= 1400
   if (0 != fopen_s(&f, filename, mode))
      f = NULL;
#else
   f = fopen(filename, mode);
#endif // _WIN32 && KOI_WINDOWS_UTF8
   return f;
}

KOIDEF koi_uc *koi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels)
{
   FILE *const f = koi__fopen(filename, "rb");
   unsigned char *result;
   if (f == NULL) return koi__errpuc("can't fopen", "Unable to open file");
   result = koi_load_from_file(f, x, y, channels_in_file, desired_channels);
   fclose(f);
   return result;
}

KOIDEF koi_uc *koi_load_from_file(FILE *f, int *x, int *y, int *channels_in_file, int desired_channels)
{
   unsigned char *result;
   koi__context s;
   koi__start_file(&s, f);
   result = koi__load_and_postprocess_8bit(&s, x, y, channels_in_file, desired_channels);
   if (result) {
      // need to 'unget' all the characters in the IO buffer
      fseek(f, -(int)(s.img_buffer_end - s.img_buffer), SEEK_CUR);
   }
   return result;
}

KOIDEF koi_us *koi_load_from_file_16(FILE *f, int *x, int *y, int *channels_in_file, int desired_channels)
{
   koi_us *result;
   koi__context s;
   koi__start_file(&s, f);
   result = koi__load_and_postprocess_16bit(&s, x, y, channels_in_file, desired_channels);
   if (result) {
      // need to 'unget' all the characters in the IO buffer
      fseek(f, -(int)(s.img_buffer_end - s.img_buffer), SEEK_CUR);
   }
   return result;
}

KOIDEF koi_us *koi_load_16(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels)
{
   FILE *const f = koi__fopen(filename, "rb");
   koi_us *result;
   if (f == NULL) return (koi_us*)koi__errpuc("can't fopen", "Unable to open file");
   result = koi_load_from_file_16(f, x, y, channels_in_file, desired_channels);
   fclose(f);
   return result;
}

#endif // !KOI_NO_STDIO

KOIDEF koi_us *koi_load_16_from_memory(koi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels)
{
   koi__context s;
   koi__start_mem(&s, buffer, len);
   return koi__load_and_postprocess_16bit(&s, x, y, channels_in_file, desired_channels);
}

KOIDEF koi_us *koi_load_16_from_callbacks(koi_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels)
{
   koi__context s;
   koi__start_callbacks(&s, clbk, user);
   return koi__load_and_postprocess_16bit(&s, x, y, channels_in_file, desired_channels);
}

KOIDEF koi_uc *koi_load_from_memory(koi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels)
{
   koi__context s;
   koi__start_mem(&s, buffer, len);
   return koi__load_and_postprocess_8bit(&s, x, y, channels_in_file, desired_channels);
}

KOIDEF koi_uc *koi_load_from_callbacks(koi_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels)
{
   koi__context s;
   koi__start_callbacks(&s, clbk, user);
   return koi__load_and_postprocess_8bit(&s, x, y, channels_in_file, desired_channels);
}

#if !defined(KOI_NO_LINEAR)
static float *koi__loadf_main(koi__context *s, int *x, int *y, int *channels_in_file, int desired_channels)
{
   unsigned char *const data = koi__load_and_postprocess_8bit(s, x, y, channels_in_file, desired_channels);
   if (data == NULL)
      return NULL;
   return koi__ldr_to_hdr(data, *x, *y, desired_channels ? desired_channels : *channels_in_file);
}

KOIDEF float *koi_loadf_from_memory(koi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels)
{
   koi__context s;
   koi__start_mem(&s, buffer, len);
   return koi__loadf_main(&s, x, y, channels_in_file, desired_channels);
}

KOIDEF float *koi_loadf_from_callbacks(koi_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels)
{
   koi__context s;
   koi__start_callbacks(&s, clbk, user);
   return koi__loadf_main(&s, x, y, channels_in_file, desired_channels);
}

#if !defined(KOI_NO_STDIO)
KOIDEF float *koi_loadf(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels)
{
   float *result;
   FILE *const f = koi__fopen(filename, "rb");
   if (f == NULL) return koi__errpf("can't fopen", "Unable to open file");
   result = koi_loadf_from_file(f, x, y, channels_in_file, desired_channels);
   fclose(f);
   return result;
}

KOIDEF float *koi_loadf_from_file(FILE *f, int *x, int *y, int *channels_in_file, int desired_channels)
{
   koi__context s;
   koi__start_file(&s, f);
   return koi__loadf_main(&s, x, y, channels_in_file, desired_channels);
}
#endif // !KOI_NO_STDIO

static float koi__l2h_gamma = 2.2f, koi__l2h_scale = 1.0f;

KOIDEF void koi_ldr_to_hdr_gamma(float gamma) { koi__l2h_gamma = gamma; }
KOIDEF void koi_ldr_to_hdr_scale(float scale) { koi__l2h_scale = scale; }

#endif // !KOI_NO_LINEAR

//////////////////////////////////////////////////////////////////////////////
//
// Common code used by all image loaders
//

enum
{
   KOI__SCAN_load = 0,
   KOI__SCAN_type,
   KOI__SCAN_header
};

static void koi__refill_buffer(koi__context *s)
{
   int n = 0;
   // keep reading until the buffer is full or the stream ends: a callback may return fewer
   // bytes than requested, and the format tests and koi__rewind need a filled first buffer
   while (n < s->buflen) {
      const int r = (s->io.read)(s->io_user_data, (char*)s->buffer_start + n, s->buflen - n);
      if (r <= 0) break;
      n += r;
   }
   s->callback_already_read += (int)(s->img_buffer - s->img_buffer_original);
   if (n == 0) {
      // at end of file, treat same as if from memory, but need to handle case
      // where s->img_buffer isn't pointing to safe memory, e.g. 0-byte file
      s->read_from_callbacks = 0;
      s->img_buffer = s->buffer_start;
      s->img_buffer_end = s->buffer_start + 1;
      s->buffer_start[0] = 0;
   }
   else {
      s->img_buffer = s->buffer_start;
      s->img_buffer_end = s->buffer_start + n;
   }
}

koi_inline static koi_uc koi__get8(koi__context *s)
{
   if (s->img_buffer < s->img_buffer_end)
      return *s->img_buffer++;
   if (s->read_from_callbacks) {
      koi__refill_buffer(s);
      return *s->img_buffer++;
   }
   return 0;
}

static int koi__get16be(koi__context *s)
{
   const int z = koi__get8(s) << 8;
   return z + koi__get8(s);
}

static koi__uint32 koi__get32be(koi__context *s)
{
   koi__uint32 z = koi__get16be(s) << 16;
   z += (koi__uint32)koi__get16be(s);
   return z;
}

static koi_uc koi__compute_y(int r, int g, int b)
{
   return (koi_uc)(((r * 77) + (g * 150) + (29 * b)) >> 8);
}

static unsigned char *koi__convert_format(unsigned char *data, int img_n, int desired_channels, unsigned int x, unsigned int y)
{
   int i, j;
   unsigned char *good;

   if (desired_channels == img_n) return data;
   KOI_ASSERT(desired_channels >= 1 && desired_channels <= 4);

   good = (unsigned char*)koi__malloc_mad3(x, y, desired_channels, 0);
   if (good == NULL) {
      KOI_FREE(data);
      return koi__errpuc("outofmem", "Out of memory");
   }

   for (j = 0; j < (int)y; ++j) {
      unsigned char *src = data + j * x * img_n;
      unsigned char *dest = good + j * x * desired_channels;

#define KOI__COMBO(a, b)  ((a) * 8 + (b))
#define KOI__CASE(a, b)   case KOI__COMBO(a, b): for (i = x - 1; i >= 0; --i, src += a, dest += b)
      // convert source image with img_n components to one with desired_channels components;
      // avoid switch per pixel, so use switch per scanline and massive macros
      switch (KOI__COMBO(img_n, desired_channels)) {
         KOI__CASE(1, 2) { dest[0] = src[0]; dest[1] = 255; } break;
         KOI__CASE(1, 3) { dest[0] = dest[1] = dest[2] = src[0]; } break;
         KOI__CASE(1, 4) { dest[0] = dest[1] = dest[2] = src[0]; dest[3] = 255; } break;
         KOI__CASE(2, 1) { dest[0] = src[0]; } break;
         KOI__CASE(2, 3) { dest[0] = dest[1] = dest[2] = src[0]; } break;
         KOI__CASE(2, 4) { dest[0] = dest[1] = dest[2] = src[0]; dest[3] = src[1]; } break;
         KOI__CASE(3, 4) { dest[0] = src[0]; dest[1] = src[1]; dest[2] = src[2]; dest[3] = 255; } break;
         KOI__CASE(3, 1) { dest[0] = koi__compute_y(src[0], src[1], src[2]); } break;
         KOI__CASE(3, 2) { dest[0] = koi__compute_y(src[0], src[1], src[2]); dest[1] = 255; } break;
         KOI__CASE(4, 1) { dest[0] = koi__compute_y(src[0], src[1], src[2]); } break;
         KOI__CASE(4, 2) { dest[0] = koi__compute_y(src[0], src[1], src[2]); dest[1] = src[3]; } break;
         KOI__CASE(4, 3) { dest[0] = src[0]; dest[1] = src[1]; dest[2] = src[2]; } break;
         default: KOI_ASSERT(0); KOI_FREE(data); KOI_FREE(good); return koi__errpuc("unsupported", "Unsupported format conversion");
      }
#undef KOI__CASE
#undef KOI__COMBO
   }

   KOI_FREE(data);
   return good;
}

#if !defined(KOI_NO_LINEAR)
static float *koi__ldr_to_hdr(koi_uc *data, int x, int y, int channels)
{
   int i, k;
   const int n = (channels & 1) ? channels : channels - 1; // number of non-alpha components
   float *output;
   if (data == NULL) return NULL;
   output = (float*)koi__malloc_mad4(x, y, channels, sizeof(float), 0);
   if (output == NULL) {
      KOI_FREE(data);
      return koi__errpf("outofmem", "Out of memory");
   }
   for (i = 0; i < x * y; ++i) {
      for (k = 0; k < n; ++k) {
         output[i * channels + k] = (float)(pow(data[i * channels + k] / 255.0f, koi__l2h_gamma) * koi__l2h_scale);
      }
   }
   if (n < channels) {
      for (i = 0; i < x * y; ++i) {
         output[i * channels + n] = data[i * channels + n] / 255.0f;
      }
   }

   KOI_FREE(data);
   return output;
}
#endif // !KOI_NO_LINEAR

//////////////////////////////////////////////////////////////////////////////
//
//  QOI - The "Quite OK Image Format" decoder
//

#if !defined(KOI_NO_QOI)
static int koi__qoi_test_raw(koi__context *s)
{
   const int size = (int)(s->img_buffer_original_end - s->img_buffer_original);
   if (size < 14 + 8) return 0; // QOI header size 14 bytes and 8 bytes padding
   if (koi__get8(s) != 'q') return 0;
   if (koi__get8(s) != 'o') return 0;
   if (koi__get8(s) != 'i') return 0;
   if (koi__get8(s) != 'f') return 0;
   return 1;
}

static int koi__qoi_test(koi__context *s)
{
   const int r = koi__qoi_test_raw(s);
   koi__rewind(s);
   return r;
}

typedef struct
{
   koi_uc ch_n, cspace;
} koi__qoi_data;

static void *koi__qoi_parse_header(koi__context *s, koi__qoi_data *info)
{
   if (koi__get8(s) != 'q' || koi__get8(s) != 'o' || koi__get8(s) != 'i' || koi__get8(s) != 'f')
      return koi__errpuc("not QOI", "Corrupt QOI");
   s->img_x = koi__get32be(s);
   s->img_y = koi__get32be(s);

   // 'The colorspace and channel fields are purely informative. They do not change the way data chunks are encoded.'
   // ~ qoi specification Version 1.0
   info->ch_n = koi__get8(s);

   if (info->ch_n != 3 && info->ch_n != 4) return koi__errpuc("QOI CHANNELS", "QOI only support: 4 and 3 channels");

   info->cspace = koi__get8(s);

   if (info->cspace != 0 && info->cspace != 1) return koi__errpuc("QOI COLORSPACE", "QOI only support: 0 and 1 colorspace");

   return (void*)1;
}

typedef struct
{
   koi_uc r, g, b, a;
} koi__qoi_pixel;

static void *koi__qoi_load(koi__context *s, int *x, int *y, int *channels_in_file, int desired_channels, koi__result_info *ri)
{
   koi_uc *out;
   koi__qoi_pixel index[64], prev_px;
   koi__uint32 i, pos, len;
   koi_uc target;
   koi__qoi_data info;
   KOI_NOTUSED(ri);

   if (koi__qoi_parse_header(s, &info) == NULL)
      return NULL; // error code already set

   if (s->img_x == 0 || s->img_y == 0) return koi__errpuc("0-pixel image", "Corrupt QOI");
   if (s->img_y > KOI_MAX_DIMENSIONS) return koi__errpuc("too large", "Very large image (corrupt?)");
   if (s->img_x > KOI_MAX_DIMENSIONS) return koi__errpuc("too large", "Very large image (corrupt?)");

   s->img_n = info.ch_n;

   if (desired_channels && desired_channels >= 3) // we can directly decode 3 or 4
      target = desired_channels;
   else
      target = s->img_n;         // if they want monochrome, we'll post-convert

   // sanity-check size
   out = (koi_uc*)koi__malloc_mad3(s->img_x, s->img_y, target, 0);
   if (out == NULL) return koi__errpuc("outofmem", "Out of memory");

   prev_px.r = 0;
   prev_px.g = 0;
   prev_px.b = 0;
   prev_px.a = 255;

   memset(index, 0, sizeof(index));

#define KOI_QOI_COLOR_HASH(px) (px.r * 3 + px.g * 5 + px.b * 7 + px.a * 11)

   len = s->img_y * s->img_x;
   for (i = 0; i < len; ++i) {
      koi_uc tag;
      pos = i * target;

      // stop on truncated input instead of silently decoding zeros
      if (!s->read_from_callbacks && s->img_buffer >= s->img_buffer_end) {
         KOI_FREE(out);
         return koi__errpuc("truncated", "Corrupt QOI (unexpected end of data)");
      }

      tag = koi__get8(s);

      if (tag == 0xfe /* QOI_OP_RGB */) {
         prev_px.r = koi__get8(s);
         prev_px.g = koi__get8(s);
         prev_px.b = koi__get8(s);
      }
      else if (tag == 0xff /* QOI_OP_RGBA */) {
         prev_px.r = koi__get8(s);
         prev_px.g = koi__get8(s);
         prev_px.b = koi__get8(s);
         prev_px.a = koi__get8(s);
      }
      else {
         const koi_uc t = tag & 0xc0;
         if (t == 0x00 /* QOI_OP_INDEX */) {
            prev_px = index[tag];
         }
         else if (t == 0x40 /* QOI_OP_DIFF */) {
            const koi_uc d = tag ^ 0x40;
            prev_px.r += ((d >> 4) & 0x03) - 2;
            prev_px.g += ((d >> 2) & 0x03) - 2;
            prev_px.b += ( d       & 0x03) - 2;
         }
         else if (t == 0x80 /* QOI_OP_LUMA */) {
            const koi_uc dg = (tag ^ 0x80) - 32;
            const koi_uc d2 = koi__get8(s);
            prev_px.r += dg - 8 + ((d2 >> 4) & 0x0f);
            prev_px.g += dg;
            prev_px.b += dg - 8 + ( d2       & 0x0f);
         }
         else if (t == 0xc0 /* QOI_OP_RUN */) {
            koi__uint32 run = (koi__uint32)(tag & 0x3f) + 1;
            // a run must never write past the last pixel (malformed/malicious files)
            if (run > len - i) run = len - i;
            index[KOI_QOI_COLOR_HASH(prev_px) & (64 - 1)] = prev_px;
            while (run--) {
               pos = i * target;
               out[pos + 0] = prev_px.r;
               out[pos + 1] = prev_px.g;
               out[pos + 2] = prev_px.b;
               if (target == 4) out[pos + 3] = prev_px.a;
               if (run > 0) ++i;
            }
            continue;
         }
      }

      index[KOI_QOI_COLOR_HASH(prev_px) & (64 - 1)] = prev_px;

      out[pos++] = prev_px.r;
      out[pos++] = prev_px.g;
      out[pos]   = prev_px.b;
      if (target == 4) out[++pos] = prev_px.a;
   }

#undef KOI_QOI_COLOR_HASH

   // consume the 8-byte end marker, so koi_load_from_file leaves the FILE* right after the image
   for (i = 0; i < 8; ++i)
      koi__get8(s);

   if (desired_channels && desired_channels != target) {
      out = koi__convert_format(out, target, desired_channels, s->img_x, s->img_y);
      if (out == NULL) return out; // koi__convert_format frees input on failure
   }

   *x = s->img_x;
   *y = s->img_y;
   if (channels_in_file) *channels_in_file = s->img_n;
   return out;
}

static int koi__qoi_info(koi__context *s, int *x, int *y, int *channels_in_file)
{
   koi__qoi_data info;
   void *const p = koi__qoi_parse_header(s, &info);

   if (p == NULL) {
      koi__rewind(s);
      return 0;
   }
   if (x) *x = s->img_x;
   if (y) *y = s->img_y;
   if (channels_in_file) *channels_in_file = info.ch_n;
   return 1;
}
#endif // !KOI_NO_QOI

static int koi__info_main(koi__context *s, int *x, int *y, int *channels_in_file)
{
#if !defined(KOI_NO_QOI)
   if (koi__qoi_info(s, x, y, channels_in_file)) return 1;
#endif // !KOI_NO_QOI

   return koi__err("unknown image type", "Image not of any known type, or corrupt");
}

#if !defined(KOI_NO_STDIO)
KOIDEF int koi_info(char const *filename, int *x, int *y, int *channels_in_file)
{
   FILE *const f = koi__fopen(filename, "rb");
   int result;
   if (f == NULL) return koi__err("can't fopen", "Unable to open file");
   result = koi_info_from_file(f, x, y, channels_in_file);
   fclose(f);
   return result;
}

KOIDEF int koi_info_from_file(FILE *f, int *x, int *y, int *channels_in_file)
{
   int r;
   koi__context s;
   const long pos = ftell(f);
   koi__start_file(&s, f);
   r = koi__info_main(&s, x, y, channels_in_file);
   fseek(f, pos, SEEK_SET);
   return r;
}
#endif // !KOI_NO_STDIO

KOIDEF int koi_info_from_memory(koi_uc const *buffer, int len, int *x, int *y, int *channels_in_file)
{
   koi__context s;
   koi__start_mem(&s, buffer, len);
   return koi__info_main(&s, x, y, channels_in_file);
}

KOIDEF int koi_info_from_callbacks(koi_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file)
{
   koi__context s;
   koi__start_callbacks(&s, clbk, user);
   return koi__info_main(&s, x, y, channels_in_file);
}

#endif // KOI_IMAGE_IMPLEMENTATION

/*
   revision history:
      1.05  (2026-10-19) Read callbacks returning fewer bytes than requested are now handled (QOI was not
                         detected when the first read returned fewer than 22 bytes; negative returns are end of stream)
                         koi_load_from_file now leaves the FILE* after the QOI end marker, as documented
                         koi_loadf keeps the failure reason of the decoder (reported "unknown image type" on out of memory)
      1.04  (2026-10-18) QOI: QOI_OP_RUN can no longer write past the end of the output buffer
                         QOI: truncated data and 0-pixel images are now rejected
                         Fixed NULL dereference in 16-bit loading when 8->16 conversion fails
                         Fixed build with KOI_NO_STDIO (header checked STBI_NO_STDIO instead of KOI_NO_STDIO)
                         Fixed build with KOI_NO_LINEAR (koi__malloc_mad4 is also needed by the 16-bit path)
                         Added missing #include <limits.h> for INT_MAX
                         Removed unused koi__float_postprocess
                         koi__context buffer pointers are now const (no const cast in koi__start_mem)
                         koi__stdio_callbacks is now const, removed casts that discarded const from callbacks
                         Unified code formatting (indentation, preprocessor directives, #endif comments, whitespace)
                         Renamed comp/req_comp parameters to channels_in_file/desired_channels everywhere
                         Local variables that are assigned only once are now const
      1.03  (2026-07-24) Fixed integer conversion warning in QOI loader
      1.02  (2026-04-30) Improved memory safety and made Windows file handling more robust
      1.01  (2025-11-20) Unified formatting of functions and tabs
      1.00  (2025-05-09) QOI file loader
*/

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2025 Marceli Antosik
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (https://unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.
In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
For more information, please refer to <https://unlicense.org>
------------------------------------------------------------------------------
*/