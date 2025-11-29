# CLion 运行配置说明

## 问题描述
程序在 CLion 中运行时会报错：`Error: Cannot open file sales_history.txt`

这是因为程序默认在 `cmake-build-debug` 目录中运行，而数据文件在项目根目录。

## 解决方案

### 方案 1：在 CLion 中设置工作目录（推荐）

1. 在 CLion 中，点击右上角的运行配置下拉菜单
2. 选择 **Edit Configurations...**
3. 找到 `main` 配置，或者创建一个新配置：
   - **Name**: `main`
   - **Target**: `main`
   - **Executable**: 选择生成的 `main` 可执行文件
   - **Program arguments**: （留空）
   - **Working directory**: 设置为项目根目录路径：
     ```
     $PROJECT_DIR$
     ```
     或者绝对路径：
     ```
     /Users/a86158/Documents/Studyverse/UGY2/T1/CSC3002/3002_Project
     ```
4. 点击 **OK** 保存配置
5. 重新运行程序

### 方案 2：程序已内置智能文件查找

程序已经包含了智能文件查找功能，会自动尝试以下路径：
- 当前目录：`sales_history.txt`
- 上一级目录：`../sales_history.txt`
- 上两级目录：`../../sales_history.txt`

如果程序仍无法找到文件，请使用方案 1 手动设置工作目录。

### 方案 3：将数据文件复制到构建目录（临时方案）

```bash
cd cmake-build-debug
cp ../sales_history.txt .
```

然后在 CLion 中运行程序。

## 验证配置

运行程序后，如果看到以下输出，说明配置成功：

```
═══════════════════════════════════════════════════════
  动态定价系统完整演示程序
  Dynamic Pricing System Demo
═══════════════════════════════════════════════════════

【步骤 1】数据加载
------------------------------------------------------------
Successfully loaded 10 sales records.
✅ 成功加载 10 条销售记录
```

## 常见问题

**Q: 如何查看当前工作目录？**
A: 可以在程序开头添加调试输出：
```cpp
cout << "当前工作目录: " << std::filesystem::current_path() << endl;
```

**Q: CMakeLists.txt 已经设置了工作目录，为什么还是不工作？**
A: CLion 有时会忽略 CMakeLists.txt 中的工作目录设置。请使用方案 1 在 CLion 中手动配置。

## 推荐配置

建议使用方案 1，在 CLion 的 Run Configuration 中设置：
- **Working directory**: `$PROJECT_DIR$`

这样可以确保程序始终从项目根目录运行，能够找到所有数据文件。

