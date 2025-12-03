# C++ Intelligent Inventory Warning & Dynamic Pricing System  
# 基于 C++ 的电子产品智能库存预警与动态定价系统

## 📖 项目背景 (Introduction)

本项目是针对**电子产品（智能手机、笔记本电脑、显卡等）**垂直领域的智能决策系统。针对电子产品价格敏感度高、更新换代快、季节性需求波动大等特点，我们实现了一个从**销量预测**到**库存预警**，再到**动态定价**的完整商业逻辑闭环。

系统完全基于 **C++ Standard Library** 开发，不依赖任何第三方库，旨在展示底层工程能力与算法落地的结合。

## 🌟 核心亮点 (Project Highlights)

### 1. 纯 C++ 工程实现（Zero Dependencies）

坚持低依赖设计，仅使用 `std::thread`, `std::mutex`, `fstream` 等标准库，确保可移植性与底层机制可控。

### 2. 电子产品专属策略（Domain-Specific Strategy）

- **生命周期管理**：识别新品发布，自动对旧机型应用清仓折扣策略。  
- **市场博弈机制**：监控竞品价格差，动态调整竞争力度。  
- **季节性感知**：对“618”“双11”等购物节及每日黄金时段（10:00–16:00）自动调整价格弹性。

### 3. 高并发模拟（Concurrency Simulation）

模拟多商家（Merchant Threads）并行定价场景，使用 `std::shared_mutex` 确保价格表读写安全，真实还原电商后台的竞争过程。

### 4. 数据可视化闭环（Visualization）

输出 HTML 仪表盘与 CSV 趋势报表，结合 Chart.js，将算法结果转化为可视化商业洞察。

## 🏗️ 系统架构 (System Architecture)

系统采用模块化设计，包含五个核心层级：

1. **DataLoader（数据层）**  
   负责 CSV 数据解析与加载，构建历史销量序列。

2. **Forecaster（预测层）**  
   使用移动平均法（Moving Average）预测未来销量。

3. **InventoryAlert（预警层）**  
   通过供需比计算库存压力，触发多级预警（GREEN → CRITICAL）。

4. **PricingStrategy（定价层）**  
   核心公式：  
   `Adjustment = w1*Stock + w2*Competitor + w3*Demand + w4*Seasonality`

5. **ThreadManager / Visualizer（并发与输出）**  
   管理多线程行为、生成日志与可视化结果。

## 👨‍💻 贡献者 (Contributors)

项目由 CSC3002 小组共同开发：

- Wang Jiarui (124090612)  
- Zhao Runtian (124090988)  
- Zhao Yuhui (124030125)  
- Zhou Lexian (124090944)  
- Pan Kangnian (124090487)

## 🛠️ 快速开始 (Quick Start)

### 1. 环境要求

- C++17 兼容编译器（GCC / Clang / MSVC）
- CMake 3.16+

### 2. 编译

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 3. 运行

确保 `sales_history.txt` 位于项目根目录。

```bash
# Linux / macOS
./main

# Windows
main.exe
```

### 4. 查看结果（Output）

程序将在 `output/` 目录生成：

- `dashboard.html`：交互式动态定价仪表盘  
- `pricing.log`：定价线程执行日志  
- `price_trend.csv`：价格趋势数据

## 📊 数据格式示例

`sales_history.txt` 文件示例：

```csv
date,productId,sales,price,stock
2025-10-01,P1001,10,2999.0,100
2025-10-02,P1001,15,2999.0,85
```
