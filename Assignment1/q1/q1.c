#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define SMALL_SIZE 20
#define OUT_SIZE 200

typedef struct {
    int minRow, maxRow, minCol, maxCol;
} Bounds;

typedef struct {
    int x, y;
} Point;

int check_rectangle(Bounds b, int pixel_count) {
    int width = b.maxCol - b.minCol + 1;
    int height = b.maxRow - b.minRow + 1;
    return pixel_count == width * height;
}

int check_circle(int pixel_count, Bounds b) {
    int width = b.maxCol - b.minCol + 1;
    int height = b.maxRow - b.minRow + 1;

    if (width != height) return 0;

    double radius = width / 2.0;
    double expected_area = M_PI * radius * radius;

    return fabs(pixel_count - expected_area) < expected_area * 0.35;
}

int dfs_iterative(
    int matrix[SMALL_SIZE][SMALL_SIZE],
    int visited[SMALL_SIZE][SMALL_SIZE],
    int sx, int sy,
    Bounds *b
) {
    Point stack[SMALL_SIZE * SMALL_SIZE];
    int top = 0;
    int count = 0;

    stack[top++] = (Point){sx, sy};

    while (top > 0) {
        Point p = stack[--top];
        int x = p.x, y = p.y;

        if (x < 0 || x >= SMALL_SIZE || y < 0 || y >= SMALL_SIZE)
            continue;
        if (visited[x][y] || matrix[x][y] != 1)
            continue;

        visited[x][y] = 1;
        count++;

        if (x < b->minRow) b->minRow = x;
        if (x > b->maxRow) b->maxRow = x;
        if (y < b->minCol) b->minCol = y;
        if (y > b->maxCol) b->maxCol = y;

        stack[top++] = (Point){x + 1, y};
        stack[top++] = (Point){x - 1, y};
        stack[top++] = (Point){x, y + 1};
        stack[top++] = (Point){x, y - 1};
    }

    return count;
}

void resize_to_20x20(unsigned char *src, int w, int h, int out[20][20]) {
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            int x = i * h / 20;
            int y = j * w / 20;
            out[i][j] = src[x * w + y];
        }
    }
}

int main() {
    const char *input_path = "input1.png";
    const char *output_path = "output1.png";
    int margin = 5;

    int w, h, ch;
    unsigned char *img = stbi_load(input_path, &w, &h, &ch, 1);
    if (!img) {
        printf("Failed to load image\n");
        return 1;
    }

    int img20[20][20];
    resize_to_20x20(img, w, h, img20);
    stbi_image_free(img);

    double mean = 0;
    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 20; j++)
            mean += img20[i][j];
    mean /= 400.0;

    int binary[20][20];
    int sum = 0;
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            binary[i][j] = img20[i][j] < (mean - margin);
            sum += binary[i][j];
        }
    }

    if (sum == 0) {
        printf("No object detected\n");
        return 0;
    }

    int visited[20][20] = {0};

    unsigned char output[OUT_SIZE * OUT_SIZE * 3];
    for (int i = 0; i < OUT_SIZE * OUT_SIZE * 3; i++)
        output[i] = 255;

    int scale = OUT_SIZE / 20;
    int object_id = 1;

    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            if (binary[i][j] && !visited[i][j]) {
                Bounds b = {20, -1, 20, -1};
                int pixels = dfs_iterative(binary, visited, i, j, &b);

                const char *shape;
                if (check_rectangle(b, pixels))
                    shape = "RECTANGLE";
                else if (check_circle(pixels, b))
                    shape = "CIRCLE";
                else
                    shape = "UNKNOWN";

                printf("Object %d: %s\n", object_id++, shape);

                /* Draw red bounding box */
                for (int x = b.minRow * scale; x <= (b.maxRow + 1) * scale; x++) {
                    for (int y = b.minCol * scale; y <= (b.maxCol + 1) * scale; y++) {
                        if (x == b.minRow * scale || x == (b.maxRow + 1) * scale ||
                            y == b.minCol * scale || y == (b.maxCol + 1) * scale) {

                            int idx = (x * OUT_SIZE + y) * 3;
                            output[idx] = 255;
                            output[idx + 1] = 0;
                            output[idx + 2] = 0;
                        }
                    }
                }
            }
        }
    }

    stbi_write_png(output_path, OUT_SIZE, OUT_SIZE, 3, output, OUT_SIZE * 3);
    printf("Output saved as %s\n", output_path);

    return 0;
}
