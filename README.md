# Demos

> 个人练习项目合集，存放各类小 Demo、课程练习、趣味小游戏与 C++ 实践代码。

## 📁 项目目录

```
Demos
├── Minesweeper项目      # 扫雷游戏（C++ EasyX）
├── snake                # Canvas 贪吃蛇网页游戏
└── 田块检查系统项目      # C++ 业务实践项目
```

### 1. Minesweeper 项目

- **技术栈**：C++、EasyX 图形库
- **功能**：经典扫雷游戏，实现左键翻开格子、右键插旗、数字提示、游戏胜负判定。

### 2. snake

- **技术栈**：HTML + CSS + Canvas + JavaScript
- **功能**：网页版贪吃蛇
  - `requestAnimationFrame` 游戏循环，精准控制移动速度
  - 分数统计、存活计时，死亡后保存本局存活时长
  - Web Audio API 内置复古音效（吃食物、死亡、重启），无需外部音频文件
  - 同时支持键盘方向键、移动端触摸滑动操作
  - 方块风格苹果食物，附带呼吸缩放动画

### 3. 田块检查系统项目

- **技术栈**：C++
- **业务方向**：田块信息管理，实现数据录入、查询、修改、删除等业务逻辑。

## 📄 Git 配置文件说明

- `.gitattributes`：Git 属性配置，统一跨平台文件换行符
- `.gitignore`：Git 忽略配置，过滤 IDE 缓存、编译产物、临时文件

## 🚀 运行说明

### 1. Minesweeper 项目

1. 使用 Visual Studio 2022 打开项目工程文件（`.sln` / `.vcxproj`）
2. 配置 EasyX 图形库（头文件与库文件路径）
3. 选择「本地 Windows 调试器」编译运行

### 2. snake

- 方式一：直接双击打开 `index.html`
- 方式二：VSCode 安装 Live Server 插件，右键启动网页访问

### 3. 田块检查系统项目

1. 使用 Visual Studio 2022 打开项目工程文件（`.sln` / `.vcxproj`）
2. 无需额外第三方库，直接编译
3. 运行后在控制台按菜单提示进行田块信息的录入、查询、修改、删除等操作

## 💻 开发环境

- Visual Studio 2022
- VS Code

## License

仅供个人学习练习使用。
