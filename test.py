import matplotlib.pyplot as plt
import matplotlib.patches as patches

# operations.txt の読み込みとデータ解析
operations = []
x_coords = []
y_coords = []

with open("operations.txt", "r") as f:
    lines = f.readlines()
    for line in lines:
        parts = line.strip().split()
        if len(parts) == 4:
            # 操作手順 (id, r, d, b)
            operations.append({
                "id": int(parts[0]),
                "r": int(parts[1]),
                "d": parts[2],
                "b": int(parts[3])
            })
        elif len(parts) == 2:
            # 座標情報 (x, y)
            x_coords.append(int(parts[0]))
            y_coords.append(int(parts[1]))

# 長方形の情報 (C++プログラムと同じサイズを指定)
rectangles = [
    {"id": 0, "width": 100, "height": 200},
    {"id": 1, "width": 150, "height": 300},
    {"id": 2, "width": 120, "height": 250},
    {"id": 3, "width": 180, "height": 160},
]

# デバッグ情報を表示
print("Operations and Coordinates:")
for op in operations:
    print(f"Operation: ID={op['id']}, Rotate={op['r']}, Direction={op['d']}, Base={op['b']}")
for i in range(len(x_coords)):
    print(f"Rectangle ID={i}: x={x_coords[i]}, y={y_coords[i]}")

# 可視化
fig, ax = plt.subplots(figsize=(10, 8))

for i, rect in enumerate(rectangles):
    x, y = x_coords[i], y_coords[i]
    width, height = rect["width"], rect["height"]
    if operations[i]["r"] == 1:  # 回転している場合、幅と高さを入れ替える
        width, height = height, width
    color = f"C{i}"  # 異なる色を自動選択
    ax.add_patch(patches.Rectangle((x, y), width, height, edgecolor="black", facecolor=color, alpha=0.6))
    ax.text(x + width / 2, y + height / 2, f"ID {i}", ha="center", va="center", fontsize=10, color="white")

# 軸の設定
ax.set_xlim(0, max(x_coords) + max(rect["width"] for rect in rectangles) + 50)
ax.set_ylim(0, max(y_coords) + max(rect["height"] for rect in rectangles) + 50)
ax.invert_yaxis()  # y軸を反転
ax.set_aspect("equal", adjustable="box")
plt.title("Rectangles Placement Visualization (Based on operations.txt)")
plt.xlabel("X")
plt.ylabel("Y")
plt.grid(True)

# 表示
plt.show()
