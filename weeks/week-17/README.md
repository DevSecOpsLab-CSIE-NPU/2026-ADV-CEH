# Week 17 — 期末報告撰寫

**日期**：2026/06/15–06/21
**時長**：3 小時（180 分鐘）
**繳交截止**：民國 115 年 6 月 21 日 23:59（上傳 ee-class）

[TOC]

---

## 一、報告要求總覽

### 主題與標的

**報告主題：** Web 應用程式弱點檢測實作報告

**受測標的（擇一或兩者皆做）：**
- DVWA（`kaakaww/dvwa-docker`，本機 Docker）
- OWASP Juice Shop（`bkimminich/juice-shop`，本機 Docker）

### 繳交內容

| # | 項目 | 說明 |
|---|------|------|
| 1 | 報告書（PDF） | 依指定章節結構撰寫，25–30 頁（不含封面、目錄） |
| 2 | 每個漏洞 | 位置 + 重現步驟 + payload + 截圖，缺一不計分 |
| 3 | Juice Shop | 每清除 1 個 challenge，附 Scoreboard 截圖 |

### 格式要求

| 項目 | 規定 |
|------|------|
| 字型 | 標楷體 / 新細明體 12pt |
| 行距 | 1.5 |
| 頁碼 | 必備 |
| 目錄 | 必備 |
| 頁首 / 頁尾 | 必備 |
| 頁數 | 25–30 頁（不含封面、目錄） |
| 格式 | PDF 繳交 |

---

## 二、評分標準

### 漏洞有效認定（採計前提）

> 一個發現須同時具備 **位置 + 重現步驟 + payload + 截圖**，才計為 1 個有效漏洞。缺少任一項，不予採計。
>
> Juice Shop 每清除 1 個 challenge（附計分板截圖）＝ 1 個有效漏洞。

### 計分方式

| 項目 | 分數 |
|------|------|
| **基本門檻**：完成 5 個有效漏洞 | 60 分 |
| **數量加分**：第 6 個起，每增加 1 個 | +4 分 |
| （最多採計至第 12 個，即最多 +28 分） | 最高 88 分 |
| **品質加分**（見下方） | 最多 12 分 |
| **滿分** | **100 分** |

### 品質加分（12 分）

| 項目 | 分數 |
|------|------|
| 風險等級涵蓋多元（Critical / High / Medium / Low 都有） | 4 分 |
| 重現步驟與截圖清晰，可由他人獨立複現 | 4 分 |
| 修補建議具體可行（非泛泛而論） | 4 分 |

### 建議達標策略

```
DVWA  → 4–5 個經典漏洞（SQLi、Command Injection、File Upload、XSS、Brute Force）
Juice Shop → 6–7 個低星 challenge（1–3 星），每個附 Scoreboard 截圖
合計 10–12 個，刻意做出 Critical / High / Medium / Low 的風險分布
```

---

## 三、報告章節結構（指定目錄）

```
封面
目錄
一、授權與免責聲明
二、測試環境說明
三、測試方法論
四、漏洞發現
    4.1 漏洞 #01 — （名稱）
    4.2 漏洞 #02 — （名稱）
    ...
五、修補建議彙整
六、結論
七、參考資料
附錄（截圖、Scoreboard、證據）
```

詳細格式見 [final-report-template.md](final-report-template.md)。

---

## 四、今天的時間配置

| 時間 | 工作 |
|------|------|
| 0:00–0:20 | 環境確認，靶機啟動，報告模板準備 |
| 0:20–0:60 | 繼續打靶機，補齊漏洞截圖 |
| 1:00–1:30 | 撰寫每個漏洞的「重現步驟 + payload」文字 |
| 1:30–1:55 | 撰寫「授權聲明 + 測試環境 + 修補建議 + 結論」 |
| 1:55–2:30 | 整合進 Word，排版（字型、行距、頁碼、頁首/頁尾） |
| 2:30–2:50 | 自我驗收 Checklist |
| 2:50–3:00 | 輸出 PDF + 上傳確認 |

---

## 五、各漏洞必備四要素

每個漏洞章節都要有這四樣，**缺一整個漏洞不計分**：

### 1. 位置（Location）

說明在哪裡發現這個漏洞：

```
受影響 URL：http://localhost/dvwa/vulnerabilities/sqli/?id=1&Submit=Submit
受影響參數：id
```

### 2. 重現步驟（Reproduction Steps）

讓助教能照著做：

```
1. 開啟瀏覽器，前往 http://localhost/dvwa/vulnerabilities/sqli/
2. 在 User ID 欄位輸入以下 payload
3. 點擊 Submit，觀察回應
```

### 3. Payload

直接貼指令或輸入值：

```sql
1' OR '1'='1'--
```
```bash
curl -s "http://localhost/dvwa/vulnerabilities/sqli/?id=1'+OR+'1'='1'--+&Submit=Submit" \
  -H "Cookie: PHPSESSID=xxx; security=low"
```

### 4. 截圖佐證（Screenshot）

- 截圖要清楚顯示 payload 的輸入與伺服器的回應
- Juice Shop challenge 截圖要包含 **Scoreboard 頁面**（URL 顯示 `/#/score-board`）且該 challenge 旁有 🏆

---

## 六、Juice Shop Scoreboard 截圖規範

Scoreboard URL：`http://localhost:3000/#/score-board`

截圖需顯示：
1. 瀏覽器網址列（確認是 `/#/score-board`）
2. 已完成的 challenge 旁有 🏆 圖示
3. challenge 名稱清晰可辨

> 若只截攻擊成功的 popup，沒有 Scoreboard 截圖，**不予採計**。

**建議選的 1–3 星 challenge（容易達成）：**

| Challenge | 星數 | 類型 |
|-----------|------|------|
| Score Board | ⭐ | 找到隱藏頁面 |
| DOM XSS | ⭐ | XSS |
| Bonus Payload | ⭐ | XSS |
| Error Handling | ⭐ | 資訊洩漏 |
| Exposed Metrics | ⭐ | 資訊洩漏 |
| Zero Stars | ⭐ | 邏輯漏洞 |
| Login Admin | ⭐⭐ | SQLi |
| Admin Section | ⭐⭐ | 存取控制 |
| View Basket | ⭐⭐ | IDOR |
| Five-Star Feedback | ⭐⭐ | 存取控制 |
| Login Bender | ⭐⭐ | SQLi |
| Password Strength | ⭐⭐ | 弱密碼 |

---

## 七、自我驗收 Checklist

報告輸出 PDF 前，逐項確認：

### 格式

- [ ] 字型：標楷體 / 新細明體 12pt
- [ ] 行距：1.5
- [ ] 有頁碼
- [ ] 有目錄（且與實際章節一致）
- [ ] 有頁首 / 頁尾
- [ ] 正文頁數 25–30 頁（不含封面、目錄）

### 章節結構

- [ ] 一、授權與免責聲明（說明測試於合法自架環境，禁止攻擊外部系統）
- [ ] 二、測試環境說明（OS、Docker image、靶機版本）
- [ ] 三、測試方法論（用了哪些工具、哪些方法）
- [ ] 四、漏洞發現（每個漏洞一節，見下方）
- [ ] 五、修補建議彙整（有優先順序，非複製貼上）
- [ ] 六、結論
- [ ] 七、參考資料

### 每個漏洞

- [ ] 有受影響位置（URL + 參數）
- [ ] 有重現步驟（步驟 1、2、3…）
- [ ] 有 payload（指令或輸入值）
- [ ] 有截圖（顯示 payload 輸入 + 伺服器回應）
- [ ] 有 CVSS 3.1 分數
- [ ] 有 Business Impact（非技術語言）
- [ ] 有修補建議（至少 1 條）

### Juice Shop 專項

- [ ] 每個 challenge 有 Scoreboard 截圖（含 🏆）
- [ ] Scoreboard 截圖網址列顯示 `/#/score-board`

### 法律與學術誠信

- [ ] 報告有授權 / 免責聲明
- [ ] 所有測試僅在自架靶機進行，未測試任何外部系統
- [ ] 內容為個人獨立完成（抄襲、共用依校規處理）

---

## 八、輸出 PDF + 繳交

```
檔名：FINAL_Pentest_學號_姓名.pdf
上傳：ee-class → 進階駭客攻防技術 → 期末作業
截止：民國 115 年 6 月 21 日 23:59
```

---

## 九、注意事項

- 嚴禁攻擊校外或他人真實系統，違者以**零分**計
- 抄襲、共用報告依校規處理
- PDF 無法開啟者視同未繳交
