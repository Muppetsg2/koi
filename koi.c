#if defined(KOI_ENABLE_IMAGE)
    #if !defined(KOI_IMAGE_IMPLEMENTATION)
        #define KOI_IMAGE_IMPLEMENTATION
    #endif
    #include "koi_image.h"
#endif

#if defined(KOI_ENABLE_IMAGE_WRITE)
    #if !defined(KOI_IMAGE_WRITE_IMPLEMENTATION)
        #define KOI_IMAGE_WRITE_IMPLEMENTATION
    #endif
    #include "koi_image_write.h"
#endif