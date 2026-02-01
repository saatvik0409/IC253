import numpy as np
from PIL import Image
import sys
sys.setrecursionlimit(10**7)

# ================= NODE =================
class Node:
    def __init__(self, i, j):
        self.right = None
        self.left = None
        self.up = None
        self.down = None
        self.next = None
        self.cell_position = (i, j)


# ================= READ PNG =================
def read_png(filename):
    img = Image.open(filename).convert("L")  # grayscale
    arr = np.array(img)

    # binary threshold
    binary = (arr < 255 // 2).astype(int)

    h, w = binary.shape
    return binary.tolist(), w, h


# ================= WRITE PNG =================
def write_png(filename, image):
    arr = np.array(image, dtype=np.uint8) * 255
    img = Image.fromarray(arr, mode="L")
    img.save(filename)


# ================= SPARSE GRAPH =================
def convert_to_linked_list(matrix):
    m, n = len(matrix), len(matrix[0])
    node_matrix = [[None]*n for _ in range(m)]

    head = tail = None

    for i in range(m):
        for j in range(n):
            if matrix[i][j] == 1:
                node = Node(i, j)
                node_matrix[i][j] = node
                if head is None:
                    head = tail = node
                else:
                    tail.next = node
                    tail = node

    directions = [(1,0), (-1,0), (0,1), (0,-1)]
    for i in range(m):
        for j in range(n):
            node = node_matrix[i][j]
            if not node:
                continue
            for dx, dy in directions:
                x, y = i + dx, j + dy
                if 0 <= x < m and 0 <= y < n and node_matrix[x][y]:
                    if dx == 1:  node.down = node_matrix[x][y]
                    if dx == -1: node.up = node_matrix[x][y]
                    if dy == 1:  node.right = node_matrix[x][y]
                    if dy == -1: node.left = node_matrix[x][y]

    return head


# ================= DFS =================
def dfs(node, visited, boundary):
    stack = [node]
    area = 0

    while stack:
        curr = stack.pop()
        if curr in visited:
            continue
        visited.add(curr)
        area += 1

        is_boundary = False
        for nbr in [curr.up, curr.down, curr.left, curr.right]:
            if nbr is None:
                is_boundary = True
            elif nbr not in visited:
                stack.append(nbr)

        if is_boundary:
            boundary.append(curr)

    return area


# ================= CONNECTED COMPONENTS =================
def find_connected_components(head):
    visited = set()
    component = 0

    curr = head
    while curr:
        if curr not in visited:
            component += 1
            boundary = []
            area = dfs(curr, visited, boundary)

            print(f"Object {component}")
            print(f"Area: {area}")
            print("Boundary pixels:", end=" ")
            for b in boundary:
                i, j = b.cell_position
                print(f"({i},{j})", end=" ")
            print("\n")

        curr = curr.next

    print("Total objects detected =", component)


# ================= MAIN =================
def main():
    input_file = "input_image.png"
    output_file = "flipped_image.png"

    matrix, w, h = read_png(input_file)

    head = convert_to_linked_list(matrix)
    find_connected_components(head)

    # flipped image (same as binary here)
    write_png(output_file, matrix)
    print("Flipped image saved as", output_file)


if __name__ == "__main__":
    main()