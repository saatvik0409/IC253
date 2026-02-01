from PIL import Image

# ================================
# NODE DEFINITION
# ================================
class Node:
    def __init__(self, i, j):
        self.right = None
        self.left = None
        self.up = None
        self.down = None
        self.next = None
        self.cell_position = (i, j)


# ================= READ PNG =================
def readPNG(filename):
    img = Image.open(filename).convert("L")  # grayscale
    width, height = img.size
    pixels = img.load()

    matrix = [[0]*width for _ in range(height)]
    for i in range(height):
        for j in range(width):
            matrix[i][j] = 1 if pixels[j, i] < 128 else 0

    return matrix, width, height


# ================= WRITE PNG =================
def writePNG(filename, matrix):
    height = len(matrix)
    width = len(matrix[0])

    img = Image.new("L", (width, height))
    pixels = img.load()

    for i in range(height):
        for j in range(width):
            pixels[j, i] = 0 if matrix[i][j] else 255

    img.save(filename)


# ================= SPARSE GRAPH =================
def convertToLinkedList(matrix):
    m, n = len(matrix), len(matrix[0])
    node_matrix = [[None]*n for _ in range(m)]

    head = tail = None

    for i in range(m):
        for j in range(n):
            if matrix[i][j] == 1:
                node = Node(i, j)
                node_matrix[i][j] = node
                if not head:
                    head = tail = node
                else:
                    tail.next = node
                    tail = node

    dx = [1, -1, 0, 0]
    dy = [0, 0, 1, -1]

    for i in range(m):
        for j in range(n):
            if not node_matrix[i][j]:
                continue
            for k in range(4):
                x, y = i + dx[k], j + dy[k]
                if 0 <= x < m and 0 <= y < n and node_matrix[x][y]:
                    if dx[k] == 1:
                        node_matrix[i][j].down = node_matrix[x][y]
                    if dx[k] == -1:
                        node_matrix[i][j].up = node_matrix[x][y]
                    if dy[k] == 1:
                        node_matrix[i][j].right = node_matrix[x][y]
                    if dy[k] == -1:
                        node_matrix[i][j].left = node_matrix[x][y]

    return head


# ================= CONNECTED COMPONENTS (DSA) =================
def dfs(node, visited, area, boundary):
    visited.add(node)
    area[0] += 1

    isBoundary = False
    neighbors = [node.up, node.down, node.left, node.right]

    for nbr in neighbors:
        if nbr is None:
            isBoundary = True
        elif nbr not in visited:
            dfs(nbr, visited, area, boundary)

    if isBoundary:
        boundary.append(node)


def findConnectedComponents(head):
    visited = set()
    component = 0

    curr = head
    while curr:
        if curr not in visited:
            component += 1
            area = [0]
            boundary = []

            dfs(curr, visited, area, boundary)

            print(f"Object {component}")
            print(f"Area: {area[0]}")
            print("Boundary pixels:", end=" ")
            for b in boundary:
                print(b.cell_position, end=" ")
            print("\n")

        curr = curr.next

    print("Total objects detected =", component)


# ================= FLIP IMAGE =================
def flipImage(matrix):
    h, w = len(matrix), len(matrix[0])
    flipped = [[0]*w for _ in range(h)]

    for i in range(h):
        for j in range(w):
            flipped[i][j] = 0 if matrix[i][j] else 1

    return flipped


# ================= MAIN =================
if __name__ == "__main__":
    inputFile = f"input_image.png"
    outputFile = f"flipped_image.png"

    matrix, width, height = readPNG(inputFile)
    head = convertToLinkedList(matrix)

    findConnectedComponents(head)

    flipped = flipImage(matrix)
    writePNG(outputFile, flipped)

    print("Flipped image saved as", outputFile)
