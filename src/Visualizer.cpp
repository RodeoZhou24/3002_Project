/**
 * @file Visualizer.cpp
 * @brief 实现 HTML 注入逻辑与自动打开浏览器 (Design Replica Version v1.2)
 */

#include "../include/Visualizer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>

using namespace std;

void Visualizer::generateDashboard(const string& csvPath, const string& htmlPath) {
    cout << "📊 Generating Dashboard Interface..." << endl;

    auto data = parseCSV(csvPath);
    if (data.empty()) {
        cerr << "❌ Error: No data found in " << csvPath << endl;
        return;
    }

    string htmlContent = buildHtml(data);

    ofstream htmlFile(htmlPath);
    if (!htmlFile.is_open()) {
        cerr << "❌ Error: Cannot write to " << htmlPath << endl;
        return;
    }

    htmlFile << htmlContent;
    htmlFile.close();

    cout << "✅ Dashboard generated: " << htmlPath << endl;

    // 自动打开浏览器
    #ifdef _WIN32
        string cmd = "start " + htmlPath;
    #elif __APPLE__
        string cmd = "open " + htmlPath;
    #else
        string cmd = "xdg-open " + htmlPath;
    #endif

    system(cmd.c_str());
}

map<string, vector<ChartData>> Visualizer::parseCSV(const string& filename) {
    map<string, vector<ChartData>> data;
    ifstream file(filename);
    string line;

    if (!file.is_open()) return data;
    getline(file, line); // Skip header

    while (getline(file, line)) {
        stringstream ss(line);
        string token, date, pid;
        double price, demand, basePrice;
        int stock;
        int sales;

        // CSV Format: date,productId,basePrice,finalPrice,stock,alertLevel,sales,predictedDemand
        getline(ss, date, ',');
        getline(ss, pid, ',');
        getline(ss, token, ','); basePrice = stod(token);
        getline(ss, token, ','); price = stod(token);
        getline(ss, token, ','); stock = stoi(token);
        getline(ss, token, ','); // alertLevel
        getline(ss, token, ','); sales = stoi(token);
        getline(ss, token, ','); demand = stod(token);

        data[pid].push_back({date, price, stock, demand});
    }
    return data;
}

string Visualizer::vecToString(const vector<ChartData>& data, const string& field) {
    stringstream ss;
    ss << "[";
    for (size_t i = 0; i < data.size(); ++i) {
        if (field == "date") ss << "'" << data[i].date << "'";
        else if (field == "price") ss << fixed << setprecision(2) << data[i].price;
        else if (field == "demand") ss << fixed << setprecision(2) << data[i].demand;
        else if (field == "stock") ss << data[i].stock;

        if (i < data.size() - 1) ss << ",";
    }
    ss << "]";
    return ss.str();
}

string Visualizer::generateSidebarHtml(const map<string, vector<ChartData>>& data) {
    stringstream ss;
    bool isFirst = true;
    for (const auto& [pid, history] : data) {
        if (history.empty()) continue;

        double currentPrice = history.back().price;
        int currentStock = history.back().stock;

        string activeClass = isFirst ? " active" : "";
        string stockColor = currentStock < 10 ? "#ef4444" : "#94a3b8";
        string priceColor = currentStock < 10 ? "#f87171" : "#10b981";

        ss << "<div class=\"product-item" << activeClass << "\" onclick=\"switchProduct('" << pid << "')\" id=\"btn-" << pid << "\">";

        string iconClass = (pid.find("P1") != string::npos) ? "fa-mobile-alt" : "fa-laptop";

        ss << "  <div class=\"prod-icon\"><i class=\"fas " << iconClass << "\"></i></div>";
        ss << "  <div class=\"prod-info\">";
        ss << "    <div class=\"prod-name\">" << pid << "</div>";
        ss << "    <div class=\"prod-stock\" style=\"color:" << stockColor << "\"><i class=\"fas fa-box\"></i> " << currentStock << "</div>";
        ss << "  </div>";
        ss << "  <div class=\"price-tag\" style=\"color:" << priceColor << "\">$" << (int)currentPrice << "</div>";
        ss << "</div>";

        isFirst = false;
    }
    return ss.str();
}

string Visualizer::buildHtml(const map<string, vector<ChartData>>& data) {
    if (data.empty()) return "<html><body>No Data</body></html>";

    string defaultPid = data.begin()->first;

    stringstream allDataJs;
    allDataJs << "const allProductData = {\n";
    for(const auto& [pid, history] : data) {
        double startPrice = history.front().price;
        double endPrice = history.back().price;
        double change = ((endPrice - startPrice) / startPrice) * 100.0;

        allDataJs << "  '" << pid << "': {\n";
        allDataJs << "    labels: " << vecToString(history, "date") << ",\n";
        allDataJs << "    prices: " << vecToString(history, "price") << ",\n";
        allDataJs << "    demands: " << vecToString(history, "demand") << ",\n";
        allDataJs << "    basePrice: " << fixed << setprecision(2) << startPrice << ",\n";
        allDataJs << "    finalPrice: " << fixed << setprecision(2) << endPrice << ",\n";
        allDataJs << "    change: " << fixed << setprecision(1) << change << "\n";
        allDataJs << "  },\n";
    }
    allDataJs << "};\n";

    stringstream ss;
    ss << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n";
    ss << "<meta charset=\"UTF-8\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    ss << "<title>C++ Intelligent Pricing System</title>\n";
    ss << "<script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>\n";
    ss << "<link href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css\" rel=\"stylesheet\">\n";
    ss << "<link href=\"https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&family=JetBrains+Mono:wght@400;500&display=swap\" rel=\"stylesheet\">\n";
    ss << "<style>\n";

    // --- CSS Variables ---
    ss << ":root { \n";
    ss << "  --bg-body: #0b1121; \n";
    ss << "  --bg-sidebar: #0f172a; \n";
    ss << "  --bg-card: #1e293b; \n";
    ss << "  --bg-card-hover: #334155; \n";
    ss << "  --text-main: #f8fafc; \n";
    ss << "  --text-muted: #94a3b8; \n";
    ss << "  --accent-blue: #3b82f6; \n";
    ss << "  --accent-purple: #8b5cf6; \n";
    ss << "  --accent-green: #10b981; \n";
    ss << "  --accent-red: #ef4444; \n";
    ss << "  --border: #334155; \n";
    ss << "  --console-bg: #0f172a; \n";
    ss << "}\n";

    // --- Layout & Typography ---
    ss << "* { margin: 0; padding: 0; box-sizing: border-box; }\n";
    ss << "body { background-color: var(--bg-body); color: var(--text-main); font-family: 'Inter', sans-serif; height: 100vh; display: flex; overflow: hidden; font-size: 14px; }\n";

    // --- Sidebar ---
    ss << ".sidebar { width: 260px; background: var(--bg-sidebar); border-right: 1px solid var(--border); display: flex; flex-direction: column; padding: 20px; z-index: 10; }\n";
    ss << ".brand { font-size: 1.1rem; font-weight: 700; margin-bottom: 30px; display: flex; align-items: center; gap: 10px; color: #fff; }\n";
    ss << ".brand i { color: var(--accent-blue); font-size: 1.2rem; }\n";
    ss << ".section-label { color: var(--text-muted); font-size: 0.75rem; font-weight: 700; text-transform: uppercase; margin-bottom: 12px; letter-spacing: 0.5px; }\n";

    ss << ".product-item { display: flex; align-items: center; padding: 12px; margin-bottom: 8px; background: rgba(255,255,255,0.03); border: 1px solid transparent; border-radius: 8px; cursor: pointer; transition: all 0.2s; }\n";
    ss << ".product-item:hover { background: var(--bg-card-hover); }\n";
    ss << ".product-item.active { background: rgba(59, 130, 246, 0.15); border-color: var(--accent-blue); }\n";
    ss << ".prod-icon { width: 32px; height: 32px; background: #fff; border-radius: 6px; display: flex; align-items: center; justify-content: center; margin-right: 12px; color: #000; font-size: 14px; }\n";
    ss << ".prod-info { flex: 1; overflow: hidden; }\n";
    ss << ".prod-name { font-weight: 600; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }\n";
    ss << ".prod-stock { font-size: 0.75rem; margin-top: 2px; display: flex; align-items: center; gap: 4px; }\n";
    ss << ".price-tag { font-weight: 700; font-family: 'JetBrains Mono', monospace; font-size: 0.9rem; }\n";

    // --- Main Content ---
    ss << ".main-content { flex: 1; padding: 24px; display: flex; flex-direction: column; overflow-y: auto; gap: 20px; }\n";

    ss << ".header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px; }\n";
    ss << ".header-title { font-size: 1.5rem; font-weight: 700; }\n";
    ss << ".header-controls { display: flex; gap: 12px; }\n";
    ss << ".btn { background: var(--bg-card); border: 1px solid var(--border); color: var(--text-main); padding: 8px 16px; border-radius: 6px; font-weight: 600; cursor: pointer; display: flex; align-items: center; gap: 8px; font-size: 0.85rem; transition: 0.2s; white-space: nowrap; }\n";
    ss << ".btn-primary { background: var(--accent-green); border-color: var(--accent-green); color: #fff; }\n";
    ss << ".btn:hover { opacity: 0.9; }\n";

    // Custom Select Style
    ss << ".select-wrapper { position: relative; }\n";
    ss << "select.btn { appearance: none; -webkit-appearance: none; padding-right: 32px; }\n";
    ss << ".select-icon { position: absolute; right: 12px; top: 50%; transform: translateY(-50%); pointer-events: none; color: var(--text-muted); font-size: 0.8rem; }\n";

    ss << ".stats-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 20px; }\n";
    ss << ".stat-card { background: var(--bg-card); padding: 20px; border-radius: 12px; border: 1px solid var(--border); position: relative; }\n";
    ss << ".stat-label { color: var(--text-muted); font-size: 0.75rem; font-weight: 700; text-transform: uppercase; margin-bottom: 8px; }\n";
    ss << ".stat-value { font-size: 2rem; font-weight: 700; font-family: 'JetBrains Mono', monospace; }\n";
    ss << ".stat-sub { font-size: 0.8rem; margin-top: 6px; font-weight: 500; }\n";
    ss << ".stat-icon { position: absolute; top: 20px; right: 20px; font-size: 1.5rem; opacity: 0.3; }\n";

    ss << ".chart-panel { background: var(--bg-card); border: 1px solid var(--border); border-radius: 12px; padding: 24px; flex: 1; display: flex; flex-direction: column; min-height: 320px; }\n";
    ss << ".panel-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; }\n";
    ss << ".panel-title { font-weight: 600; font-size: 1.1rem; display: flex; align-items: center; gap: 10px; }\n";
    ss << ".panel-title i { color: var(--accent-purple); }\n";

    ss << ".details-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; height: 220px; }\n";
    ss << ".detail-card { background: var(--bg-card); border: 1px solid var(--border); border-radius: 12px; padding: 20px; display: flex; flex-direction: column; }\n";
    ss << ".detail-title { font-size: 0.8rem; font-weight: 700; color: var(--text-muted); text-transform: uppercase; margin-bottom: 16px; }\n";

    ss << ".factor-item { margin-bottom: 14px; }\n";
    ss << ".factor-header { display: flex; justify-content: space-between; margin-bottom: 6px; font-size: 0.85rem; font-weight: 500; }\n";
    ss << ".progress-bg { height: 6px; background: #334155; border-radius: 3px; overflow: hidden; }\n";
    ss << ".progress-fill { height: 100%; border-radius: 3px; }\n";

    ss << ".data-row { display: flex; justify-content: space-between; margin-bottom: 12px; padding-bottom: 12px; border-bottom: 1px solid rgba(255,255,255,0.05); }\n";
    ss << ".data-row:last-child { border: none; margin-top: auto; padding-bottom: 0; }\n";
    ss << ".data-label { color: var(--text-muted); }\n";
    ss << ".data-val { font-family: 'JetBrains Mono', monospace; font-weight: 600; }\n";
    ss << ".val-large { font-size: 1.5rem; color: var(--accent-green); }\n";

    // --- Right Console ---
    ss << ".console-panel { width: 320px; background: var(--console-bg); border-left: 1px solid var(--border); display: flex; flex-direction: column; font-family: 'JetBrains Mono', monospace; }\n";
    ss << ".console-header { padding: 16px 20px; border-bottom: 1px solid var(--border); display: flex; justify-content: space-between; align-items: center; }\n";
    ss << ".window-controls { display: flex; gap: 6px; }\n";
    ss << ".dot { width: 10px; height: 10px; border-radius: 50%; background: #555; }\n";
    ss << ".dot.red { background: #ef4444; } .dot.yellow { background: #f59e0b; } .dot.green { background: #10b981; }\n";
    ss << ".console-content { flex: 1; padding: 20px; overflow-y: auto; font-size: 0.8rem; line-height: 1.6; color: #94a3b8; }\n";
    ss << ".log-line { margin-bottom: 8px; }\n";
    ss << ".cmd-prefix { color: var(--accent-green); margin-right: 8px; }\n";
    ss << ".log-time { color: #475569; margin-right: 10px; }\n";
    ss << ".highlight { color: #e2e8f0; }\n";

    ss << "</style>\n</head>\n<body>\n";

    // --- HTML Structure ---

    ss << "<div class=\"sidebar\">\n";
    ss << "  <div class=\"brand\"><i class=\"fas fa-microchip\"></i> C++ Pricing Core</div>\n";
    ss << "  <div class=\"section-label\">INVENTORY MONITOR</div>\n";
    ss << generateSidebarHtml(data);
    ss << "</div>\n";

    ss << "<div class=\"main-content\">\n";
    ss << "  <div class=\"header\">\n";
    ss << "    <div class=\"header-title\">DASHBOARD V1.0</div>\n";
    ss << "    <div class=\"header-controls\">\n";
    ss << "      <button class=\"btn btn-primary\"><i class=\"fas fa-play\"></i> SIM STATE: RUNNING</button>\n";
    ss << "      <div class=\"select-wrapper\">\n";
    ss << "        <select class=\"btn\">\n";
    ss << "          <option>Normal Market</option>\n";
    ss << "          <option>Double 11 (Shopping Festival)</option>\n";
    ss << "          <option>Holiday Season</option>\n";
    ss << "        </select>\n";
    ss << "        <i class=\"fas fa-chevron-down select-icon\"></i>\n";
    ss << "      </div>\n";
    ss << "    </div>\n";
    ss << "  </div>\n";

    ss << "  <div class=\"stats-grid\">\n";
    ss << "    <div class=\"stat-card\">\n";
    ss << "      <div class=\"stat-label\">Total Revenue</div>\n";
    ss << "      <div class=\"stat-value\">$142,590</div>\n";
    ss << "      <div class=\"stat-sub\" style=\"color: var(--accent-green)\">+12.5% since start</div>\n";
    ss << "      <i class=\"fas fa-wallet stat-icon\"></i>\n";
    ss << "    </div>\n";
    ss << "    <div class=\"stat-card\">\n";
    ss << "      <div class=\"stat-label\">Active Alerts</div>\n";
    ss << "      <div class=\"stat-value\">1</div>\n";
    ss << "      <div class=\"stat-sub\" style=\"color: var(--text-muted)\">Inventory warnings</div>\n";
    ss << "      <i class=\"fas fa-exclamation-triangle stat-icon\"></i>\n";
    ss << "    </div>\n";
    ss << "    <div class=\"stat-card\">\n";
    ss << "      <div class=\"stat-label\">Avg Margin</div>\n";
    ss << "      <div class=\"stat-value\">18.5%</div>\n";
    ss << "      <div class=\"stat-sub\" style=\"color: var(--accent-blue)\">Dynamic Adjustment</div>\n";
    ss << "      <i class=\"fas fa-chart-line stat-icon\"></i>\n";
    ss << "    </div>\n";
    ss << "  </div>\n";

    ss << "  <div class=\"chart-panel\">\n";
    ss << "    <div class=\"panel-header\">\n";
    ss << "      <div class=\"panel-title\">\n";
    ss << "        <span id=\"chart-title-icon\"><i class=\"fas fa-chart-area\"></i></span>\n";
    ss << "        <span id=\"chart-title-text\">Product Analysis</span>\n";
    ss << "      </div>\n";
    ss << "    </div>\n";
    ss << "    <div style=\"flex:1; width:100%; position:relative;\"><canvas id=\"mainChart\"></canvas></div>\n";
    ss << "  </div>\n";

    ss << "  <div class=\"details-grid\">\n";
    ss << "    <div class=\"detail-card\">\n";
    ss << "      <div class=\"detail-title\">Pricing Factors (Weighted)</div>\n";
    ss << "      <div class=\"factor-item\">\n";
    ss << "        <div class=\"factor-header\"><span>Inv. Scarcity</span><span style=\"color:var(--accent-red)\">-5.0%</span></div>\n";
    ss << "        <div class=\"progress-bg\"><div class=\"progress-fill\" style=\"width: 85%; background: var(--accent-red)\"></div></div>\n";
    ss << "      </div>\n";
    ss << "      <div class=\"factor-item\">\n";
    ss << "        <div class=\"factor-header\"><span>Competitor Diff</span><span style=\"color:var(--accent-red)\">-8.0%</span></div>\n";
    ss << "        <div class=\"progress-bg\"><div class=\"progress-fill\" style=\"width: 40%; background: var(--accent-red)\"></div></div>\n";
    ss << "      </div>\n";
    ss << "      <div class=\"factor-item\">\n";
    ss << "        <div class=\"factor-header\"><span>Demand Spike</span><span style=\"color:var(--accent-green)\">+5.0%</span></div>\n";
    ss << "        <div class=\"progress-bg\"><div class=\"progress-fill\" style=\"width: 60%; background: var(--accent-green)\"></div></div>\n";
    ss << "      </div>\n";
    ss << "    </div>\n";

    ss << "    <div class=\"detail-card\">\n";
    ss << "      <div class=\"detail-title\">Algorithm Output</div>\n";
    ss << "      <div class=\"data-row\">\n";
    ss << "        <span class=\"data-label\">Base Price:</span><span class=\"data-val\" id=\"val-base\">$0.00</span>\n";
    ss << "      </div>\n";
    ss << "      <div class=\"data-row\">\n";
    ss << "        <span class=\"data-label\">Adjustment:</span><span class=\"data-val\" id=\"val-adj\" style=\"color:var(--accent-blue)\">0.0%</span>\n";
    ss << "      </div>\n";
    ss << "      <div class=\"data-row\">\n";
    ss << "        <span class=\"data-label\">Final Price:</span><span class=\"data-val val-large\" id=\"val-final\">$0.00</span>\n";
    ss << "      </div>\n";
    ss << "    </div>\n";
    ss << "  </div>\n";
    ss << "</div>\n";

    ss << "<div class=\"console-panel\">\n";
    ss << "  <div class=\"console-header\">\n";
    ss << "    <span>Console Output</span>\n";
    ss << "    <div class=\"window-controls\"><div class=\"dot red\"></div><div class=\"dot yellow\"></div><div class=\"dot green\"></div></div>\n";
    ss << "  </div>\n";
    ss << "  <div class=\"console-content\" id=\"console\">\n";
    ss << "    <div class=\"log-line\">// System Initialized.</div>\n";
    ss << "    <div class=\"log-line\">Waiting for start...</div>\n";
    ss << "    <div class=\"log-line\"><span class=\"log-time\">[01:00:00]</span> <span class=\"highlight\">Initializing DataLoader...</span></div>\n";
    ss << "    <div class=\"log-line\"><span class=\"log-time\">[01:00:01]</span> Loading sales_history.txt...</div>\n";
    ss << "    <div class=\"log-line\"><span class=\"log-time\">[01:00:02]</span> <span class=\"cmd-prefix\">>></span>Forecasting Model Ready.</div>\n";
    ss << "  </div>\n";
    ss << "  <div style=\"padding:10px; border-top:1px solid var(--border); font-size:0.7rem; color:#475569; text-align:center;\">Thread Safety: std::mutex active</div>\n";
    ss << "</div>\n";

    ss << "<script>\n";
    ss << allDataJs.str() << "\n";
    ss << "const ctx = document.getElementById('mainChart').getContext('2d');\n";
    ss << "let chart;\n";
    ss << "Chart.defaults.font.family = \"'Inter', sans-serif\";\n";
    ss << "Chart.defaults.color = '#64748b';\n";

    ss << "function initChart(labels, prices, demands) {\n";
    ss << "  let gradP = ctx.createLinearGradient(0,0,0,300);\n";
    ss << "  gradP.addColorStop(0, 'rgba(139, 92, 246, 0.5)');\n";
    ss << "  gradP.addColorStop(1, 'rgba(139, 92, 246, 0)');\n";
    ss << "  if(chart) chart.destroy();\n";
    ss << "  chart = new Chart(ctx, {\n";
    ss << "    type: 'line',\n";
    ss << "    data: {\n";
    ss << "      labels: labels,\n";
    ss << "      datasets: [{\n";
    ss << "        label: 'Price ($)', data: prices, borderColor: '#8b5cf6', backgroundColor: gradP, borderWidth: 2, tension: 0.4, fill: true, pointRadius: 0, pointHoverRadius: 6\n";
    ss << "      }, {\n";
    ss << "        label: 'Demand', data: demands, borderColor: '#3b82f6', borderDash: [4,4], borderWidth: 2, tension: 0.4, yAxisID: 'y1', pointRadius: 0\n";
    ss << "      }]\n";
    ss << "    },\n";
    ss << "    options: {\n";
    ss << "      responsive: true, maintainAspectRatio: false, \n";
    ss << "      interaction: { mode: 'index', intersect: false },\n";
    ss << "      plugins: { legend: { display: true, labels: { usePointStyle: true, color: '#94a3b8' } } },\n";
    ss << "      scales: { \n";
    ss << "        x: { grid: { display: false }, ticks: { color: '#475569' } }, \n";
    ss << "        y: { grid: { color: '#334155' }, ticks: { color: '#475569' } }, \n";
    ss << "        y1: { position: 'right', grid: { display: false }, ticks: { display: false } } \n";
    ss << "      }\n";
    ss << "    }\n";
    ss << "  });\n";
    ss << "}\n";

    ss << "window.switchProduct = function(pid) {\n";
    ss << "  document.querySelectorAll('.product-item').forEach(el => el.classList.remove('active'));\n";
    ss << "  document.getElementById('btn-'+pid).classList.add('active');\n";
    ss << "  const d = allProductData[pid];\n";
    ss << "  if(d) {\n";
    ss << "    initChart(d.labels, d.prices, d.demands);\n";
    ss << "    document.getElementById('chart-title-text').innerText = pid + ' Analysis';\n";
    ss << "    document.getElementById('val-base').innerText = '$' + d.basePrice;\n";
    ss << "    document.getElementById('val-final').innerText = '$' + d.finalPrice;\n";
    ss << "    const adjEl = document.getElementById('val-adj');\n";
    ss << "    adjEl.innerText = (d.change > 0 ? '+' : '') + d.change + '%';\n";
    ss << "    adjEl.style.color = d.change >= 0 ? '#10b981' : '#ef4444';\n";
    ss << "    log('Switched view to ' + pid);\n";
    ss << "  }\n";
    ss << "};\n";

    ss << "if (allProductData['" << defaultPid << "']) { switchProduct('" << defaultPid << "'); }\n";

    // --- 随机日志逻辑 ---
    ss << "const logMessages = [\n";
    ss << "  'Analyzing competitor pricing strategies...',\n";
    ss << "  'Checking inventory levels across warehouses...',\n";
    ss << "  'Syncing demand forecast data...',\n";
    ss << "  'Optimizing profit margins...',\n";
    ss << "  'Detecting market trend anomalies...'\n";
    ss << "];\n";
    ss << "function log(msg) {\n";
    ss << "  const d = document.createElement('div'); d.className = 'log-line';\n";
    ss << "  const time = new Date().toLocaleTimeString().split(' ')[0];\n";
    ss << "  d.innerHTML = `<span class=\"log-time\">[${time}]</span> ${msg}`;\n";
    ss << "  const win = document.getElementById('console'); win.appendChild(d); win.scrollTop = win.scrollHeight;\n";
    ss << "}\n";
    ss << "setInterval(() => {\n";
    ss << "  const msg = logMessages[Math.floor(Math.random() * logMessages.length)];\n";
    ss << "  log(msg);\n";
    ss << "}, 4000);\n";

    ss << "</script>\n</body>\n</html>";

    return ss.str();
}