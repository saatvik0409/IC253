#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define WIDTH 20
#define HEIGHT 20
#define SCALE 10

// Use stb image libraries
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

typedef struct {
    int minRow, maxRow;
    int minCol, maxCol;
} Bounds;

int check_rectangle(Bounds bounds, int pixel_count, int rows, int cols) {
    int width = bounds.maxCol - bounds.minCol + 1;
    int height = bounds.maxRow - bounds.minRow + 1;

    // Entire image should not be called rectangle
    if (width == cols && height == rows)
        return 0;

    return pixel_count == width * height;
}

int check_circle(int pixel_count, Bounds bounds) {
    int width = bounds.maxCol - bounds.minCol + 1;
    int height = bounds.maxRow - bounds.minRow + 1;

    if (width != height)
        return 0;

    double radius = width / 2.0;
    double expected_area = M_PI * radius * radius;

    return fabs(pixel_count - expected_area) < expected_area * 0.35;
}

int dfs(int binary[HEIGHT][WIDTH], int visited[HEIGHT][WIDTH],
        int sx, int sy, Bounds *bounds) {

    int stack[WIDTH * HEIGHT][2];
    int top = 0;
    int count = 0;

    stack[top][0] = sx;
    stack[top][1] = sy;
    top++;

    while (top > 0) {
        top--;
        int x = stack[top][0];
        int y = stack[top][1];

        if (x < 0 || x >= HEIGHT || y < 0 || y >= WIDTH)
            continue;

        if (visited[x][y] || binary[x][y] == 0)
            continue;

        visited[x][y] = 1;
        count++;

        if (x < bounds->minRow) bounds->minRow = x;
        if (x > bounds->maxRow) bounds->maxRow = x;
        if (y < bounds->minCol) bounds->minCol = y;
        if (y > bounds->maxCol) bounds->maxCol = y;

        int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};

        for (int i = 0; i < 4; i++) {
            stack[top][0] = x + dirs[i][0];
            stack[top][1] = y + dirs[i][1];
            top++;
        }
    }

    return count;
}

int main() {
    int width, height, channels;

    unsigned char *img = stbi_load("input.png", &width, &height, &channels, 1);

    if (!img) {
        printf("Failed to load image\n");
        return 1;
    }

    unsigned char small[HEIGHT][WIDTH];

    // Resize very simply by sampling
    for (int i = 0; i < HEIGHT; i++)
        for (int j = 0; j < WIDTH; j++)
            small[i][j] = img[(i * height / HEIGHT) * width + (j * width / WIDTH)];

    stbi_image_free(img);

    int sum = 0;
    for (int i = 0; i < HEIGHT; i++)
        for (int j = 0; j < WIDTH; j++)
            sum += small[i][j];

    int mean = sum / (HEIGHT * WIDTH);
    int threshold = mean - 5;

    int binary[HEIGHT][WIDTH];
    int visited[HEIGHT][WIDTH];

    int object_pixels = 0;

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            binary[i][j] = small[i][j] < threshold ? 1 : 0;
            visited[i][j] = 0;

            if (binary[i][j] == 1)
                object_pixels++;
        }
    }

    unsigned char output[HEIGHT * SCALE][WIDTH * SCALE];

    // Prepare output image (upsampled grayscale)
    for (int i = 0; i < HEIGHT * SCALE; i++)
        for (int j = 0; j < WIDTH * SCALE; j++)
            output[i][j] = small[i / SCALE][j / SCALE];

    // -------- No object case --------
    if (object_pixels == 0) {
        printf("No object detected (all background).\n");

        stbi_write_png("output.png", WIDTH * SCALE, HEIGHT * SCALE, 1,
                       output, WIDTH * SCALE);

        printf("Output saved as output.png\n");
        return 0;
    }

    int object_id = 1;

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {

            if (binary[i][j] == 1 && !visited[i][j]) {

                Bounds bounds = {HEIGHT, -1, WIDTH, -1};

                int pixel_count = dfs(binary, visited, i, j, &bounds);

                int is_rectangle =
                    check_rectangle(bounds, pixel_count, HEIGHT, WIDTH);

                int is_circle =
                    check_circle(pixel_count, bounds);

                char shape[20];

                if (is_rectangle)
                    strcpy(shape, "RECTANGLE");
                else if (is_circle)
                    strcpy(shape, "CIRCLE");
                else
                    strcpy(shape, "UNKNOWN");

                printf("Object %d: %s\n", object_id, shape);

                // Draw bounding box ONLY if known
                if (strcmp(shape, "UNKNOWN") != 0) {

                    for (int x = bounds.minRow * SCALE;
                         x < (bounds.maxRow + 1) * SCALE; x++) {

                        output[x][bounds.minCol * SCALE] = 0;
                        output[x][(bounds.maxCol + 1) * SCALE - 1] = 0;
                    }

                    for (int y = bounds.minCol * SCALE;
                         y < (bounds.maxCol + 1) * SCALE; y++) {

                        output[bounds.minRow * SCALE][y] = 0;
                        output[(bounds.maxRow + 1) * SCALE - 1][y] = 0;
                    }
                }

                object_id++;
            }
        }
    }

    stbi_write_png("output.png", WIDTH * SCALE, HEIGHT * SCALE, 1,
                   output, WIDTH * SCALE);

    printf("Output saved as output.png\n");

    return 0;
}
