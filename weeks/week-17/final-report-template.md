# Web 應用程式弱點檢測實作報告
## 報告模板

> **使用說明**：本模板以 Markdown 撰寫後，匯入 Word 調整字型（標楷體/新細明體 12pt、行距 1.5）、加入頁碼/頁首/頁尾，再輸出 PDF 繳交。

---

# （封面）

**Web 應用程式弱點檢測實作報告**

| | |
|--|--|
| 課程 | 進階駭客攻防技術 |
| 班級 | 資工系三甲 |
| 學號 | |
| 姓名 | |
| 受測標的 | DVWA ／ OWASP Juice Shop（擇一或兩者） |
| 報告日期 | 民國 115 年 6 月　日 |

---

# 目錄

（Word 自動產生目錄）

---

# 一、授權與免責聲明

本報告所有漏洞測試活動，均於本人自行架設之合法 Docker 容器環境（本機或虛擬機）中進行，測試標的為刻意設計含有漏洞的教學用系統（DVWA / OWASP Juice Shop），不涉及任何外部、校外或他人擁有之真實系統。

本人承諾：
1. 所有測試行為在課程授權範圍內執行
2. 測試期間未對任何外部 IP 或網域發送測試流量
3. 本報告內容為本人獨立完成，未抄襲或與他人共用

---

# 二、測試環境說明

| 項目 | 內容 |
|------|------|
| 攻擊端 OS | Kali Linux 2025.x（或 ______） |
| 測試方式 | 本機 Docker 容器 |
| 靶機一 | DVWA（`kaakaww/dvwa-docker:latest`）Port 80 |
| 靶機二 | OWASP Juice Shop（`bkimminich/juice-shop:latest`）Port 3000 |
| DVWA 難度 | Low |
| 測試日期 | 2026/06/__ ～ 2026/06/__ |

**啟動指令：**
```bash
# DVWA
docker run -d --name dvwa -p 80:80 kaakaww/dvwa-docker:latest

# Juice Shop
docker run -d --name juice-shop -p 3000:3000 bkimminich/juice-shop:latest
```

---

# 三、測試方法論

本次測試採用黑箱測試（Black-box Testing）搭配灰箱驗證，參考以下標準與工具：

**參考標準：**
- [OWASP Web Security Testing Guide（WSTG）](https://owasp.org/www-project-web-security-testing-guide/)
- [OWASP Top 10 2021](https://owasp.org/www-top-10/)
- [CVSS v3.1](https://www.first.org/cvss/calculator/3.1)

**測試工具：**

| 工具 | 用途 |
|------|------|
| nikto | Web 伺服器掃描、安全標頭檢查 |
| gobuster | 目錄列舉 |
| sqlmap | SQL Injection 自動化測試 |
| curl | HTTP 請求手動測試 |
| Burp Suite / 瀏覽器開發者工具 | 封包攔截、手動操作 |

**測試流程：**

```
偵察（nikto、gobuster）
  ↓
漏洞識別（手動測試、工具輔助）
  ↓
漏洞利用（取得證據：位置 + 步驟 + payload + 截圖）
  ↓
風險評估（CVSS 3.1 + Business Impact）
  ↓
修補建議
```

---

# 四、漏洞發現

> 每個漏洞獨立一節。至少 5 個有效漏洞（每個須具備位置 + 重現步驟 + payload + 截圖）。

---

## 4.1 漏洞 #01 — （漏洞名稱）

**受測系統：** DVWA ／ Juice Shop
**嚴重度：** Critical ／ High ／ Medium ／ Low
**CVSS 3.1：** （分數，附計算連結 https://www.first.org/cvss/calculator/3.1#...）
**OWASP 分類：** A0X:2021 — （分類名稱）
**WSTG 編號：** WSTG-XXXX-XX

### 4.1.1 受影響位置

```
URL：http://localhost/dvwa/vulnerabilities/xxxx/
受影響參數：（參數名稱）
HTTP 方法：GET ／ POST
```

### 4.1.2 重現步驟

1. 開啟瀏覽器，前往（URL）
2.（操作步驟）
3. 在（欄位）輸入以下 payload
4. 觀察回應：（預期看到什麼）

### 4.1.3 Payload

```
（貼上使用的 payload 或指令）
```

### 4.1.4 截圖佐證

（在此插入截圖）

> 截圖說明：圖中顯示（說明截圖內容，例如：「payload 輸入後伺服器回傳了 admin 的密碼雜湊」）

### 4.1.5 風險說明（Business Impact）

（用非技術語言說明：攻擊者能做什麼？對實際業務的影響？）

### 4.1.6 修補建議

1.（具體建議）
2.（具體建議）

---

## 4.2 漏洞 #02 — （漏洞名稱）

**受測系統：**
**嚴重度：**
**CVSS 3.1：**
**OWASP 分類：**
**WSTG 編號：**

### 4.2.1 受影響位置

### 4.2.2 重現步驟

### 4.2.3 Payload

### 4.2.4 截圖佐證

### 4.2.5 風險說明

### 4.2.6 修補建議

---

## 4.3 漏洞 #03 — （漏洞名稱）

（同上格式）

---

## 4.4 漏洞 #04 — （漏洞名稱）

（同上格式）

---

## 4.5 漏洞 #05 — （漏洞名稱）

（同上格式）

---

## 4.6 漏洞 #06 — （Juice Shop Challenge 名稱）

> Juice Shop challenge 節格式略有不同，需附 Scoreboard 截圖。

**受測系統：** Juice Shop
**Challenge 名稱：** （例：Login Admin）
**星數：** ⭐⭐（1–5）
**嚴重度：** Critical ／ High ／ Medium ／ Low
**OWASP 分類：** A0X:2021

### 受影響位置

### 重現步驟

### Payload

### 截圖佐證

（截圖一：攻擊成功畫面）

（截圖二：Scoreboard 截圖，需顯示 URL `/#/score-board` 且該 challenge 旁有 🏆）

### 風險說明

### 修補建議

---

# 五、修補建議彙整

> 把所有漏洞的修補建議整合成一張表，按優先順序排列。

| 優先順序 | 漏洞 | 嚴重度 | 修補方向 | 建議時程 |
|---------|------|--------|---------|---------|
| 1 | | Critical | | ≤ 7 天 |
| 2 | | High | | ≤ 30 天 |
| 3 | | High | | ≤ 30 天 |
| 4 | | Medium | | ≤ 90 天 |
| 5 | | Low | | 下次版本更新 |

**最優先修補（立即行動）：**

（針對 Critical / High 漏洞，各 2–3 句具體說明）

---

# 六、結論

（3–5 段，說明：）
1. 本次測試發現的整體風險狀況
2. 最值得關注的安全問題是什麼
3. 如果這是真實系統，會建議客戶做什麼
4. 透過本次實作，學到了什麼

---

# 七、參考資料

- [OWASP Top 10 2021](https://owasp.org/www-top-10/)
- [OWASP Web Security Testing Guide](https://owasp.org/www-project-web-security-testing-guide/)
- [CVSS v3.1 Calculator](https://www.first.org/cvss/calculator/3.1)
- [DVWA — GitHub](https://github.com/digininja/DVWA)
- [OWASP Juice Shop — GitHub](https://github.com/juice-shop/juice-shop)
- [NIST SP 800-86](https://csrc.nist.gov/publications/detail/sp/800-86/final)

---

# 附錄

## 附錄 A — Juice Shop Scoreboard 截圖

（插入完整 Scoreboard 截圖，顯示所有已完成 challenge）

## 附錄 B — 工具版本資訊

```bash
nikto -Version
sqlmap --version
gobuster version
docker --version
```

（貼上上述指令的輸出）

## 附錄 C — 測試環境 Image SHA

```bash
docker inspect juice-shop --format '{{.Image}}'
docker inspect dvwa --format '{{.Image}}'
```

（貼上輸出）
