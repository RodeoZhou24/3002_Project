/**
* @file Visualizer.h
 * @brief 暗黑风格仪表盘生成器
 */

#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <string>
#include <vector>
#include <map>

// 用于图表的数据点结构
struct ChartData {
    std::string date;
    double price;
    int stock;
    double demand;
};

class Visualizer {
public:
    /**
     * @brief 生成完整的 HTML 仪表盘并尝试自动打开
     * @param csvPath 输入的 CSV 数据路径
     * @param htmlPath 输出的 HTML 文件路径
     */
    static void generateDashboard(const std::string& csvPath, const std::string& htmlPath);

private:
    // 解析生成的 CSV
    static std::map<std::string, std::vector<ChartData>> parseCSV(const std::string& filename);

    // 构建 HTML 字符串
    static std::string buildHtml(const std::map<std::string, std::vector<ChartData>>& data);

    // 辅助工具：将 C++ 向量转换为 JS 数组字符串
    static std::string vecToString(const std::vector<ChartData>& data, const std::string& field);

    // 辅助工具：生成侧边栏产品列表 HTML
    static std::string generateSidebarHtml(const std::map<std::string, std::vector<ChartData>>& data);
};

#endif // VISUALIZER_H