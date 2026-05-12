#include "utils.h"
#include "png.h"
#include <pngconf.h>

int read_png(char *file_name)
{
    png_struct *png_ptr;
    png_info *info_ptr;
    int sig_read = 0;
    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type;
    FILE *fp;

    if (( fp = fopen(file_name, "rb") ) == NULL)
        {
            fprintf(stderr, "File not found");
            return 1;
        }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL){
        fclose(fp);
        return 1;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL){
        fclose(fp);
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return 1;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        fprintf(stderr, "Error reading the file");
        return 1;
    }

    png_init_io(png_ptr, fp);

    png_read_info(png_ptr, info_ptr);

    png_get_IHDR(
                 png_ptr,
                 info_ptr,
                 &width,
                 &height,
                 &bit_depth,
                 &color_type,
                 &interlace_type,
                 NULL,
                 NULL
                 );
    // Transformation
    // Image transformation both reduces to 8, but first one uses proper calculation,
    // second one uses the first 8 bits.
#ifdef PNG_READ_SCALE_16_TO_8_SUPPORTED
    png_set_scale_16(png_ptr);
#else
    png_set_strip_16(png_ptr);
#endif

#define MAX_SCREEN_COLORS 8

    png_set_palette_to_rgb(png_ptr);

    png_read_update_info(png_ptr, info_ptr);


    png_byte *row_pointers[height];
    png_uint_32 row;
    for (row = 0; row < height; row++)
        row_pointers[row] = NULL;
    for (row = 0; row < height; row++)
        row_pointers[row] = png_malloc(png_ptr, png_get_rowbytes(png_ptr, info_ptr));

    png_read_image(png_ptr, row_pointers);

    int fg_r;
    int fg_g;
    int fg_b;
    int bg_r;
    int bg_g;
    int bg_b;

    int new_w = 30;
    int new_h = 40;

    int src_x;
    int src_y;

    for (int b = 0; b < new_h; b=b+2) {
        src_y = b * height / new_h;
        png_bytep row1 = row_pointers[src_y];
        png_bytep row2 = row_pointers[src_y+1];
        for (int a = 0; a < new_w; a++) {
            src_x = a * width / new_w;
            png_bytep px1 = &(row1[src_x*4]);
            png_bytep px2 = &(row2[src_x*4]);
            fg_r = px1[0];
            fg_g = px1[1];
            fg_b = px1[2];
            bg_r = px2[0];
            bg_g = px2[1];
            bg_b = px2[2];
            printf(
                   "\x1b[38;2;%d;%d;%dm"
                   "\x1b[48;2;%d;%d;%dm▀▀",
                   fg_r, fg_g, fg_b,
                   bg_r, bg_g, bg_b
                   );
        }
        printf("\n");
    }
    // mapping over original image
    /*************************************************************************/
    /* for (int y = 0; y < height; y++) {                                    */
    /*     png_bytep row = row_pointers[y];                                  */
    /*                                                                       */
    /*     for (int z = 0; z < png_get_rowbytes(png_ptr, info_ptr); z=z+2) { */
    /*       r = row[z];                                                     */
    /*       g = row[z + 1];                                                 */
    /*       b = row[z + 2];                                                 */
    /*     }                                                                 */
    /* }                                                                     */
    /*************************************************************************/


    // Deleting malloc and memory
    for (row = 0; row < height; row++)
        png_free(png_ptr, row_pointers[row]);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    return 0;
}
