# Week 13 — 安全程式設計 × AI Agent 協作

**日期**：2026/05/18–05/24  
**時長**：3 小時（180 分鐘）  
**主題**：Code Smell、CERT C 規則、與 opencode 的安全協作  
**時事主軸**：Dirty Frag（CVE-2026-43284）、Grafana TanStack 供應鏈攻擊（2026/05/19）

[TOC]

---

## 一、課程目標

完成本課後，學生應能：

1. 辨識 AI agent（opencode）在安全程式設計中的**能力邊界**（什麼它會、什麼它不知道）
2. 用 CERT C 規則給 code smell 取名字，並對應 CWE 編號
3. 透過**指引品質**控制 agent 的輸出品質——指引越好，smell 越少
4. 判斷 AI agent 對「未知 CVE」的辨識可靠性
5. 把本週方法論延伸到期末滲透測試報告的附錄 A

---

## 二、背景：為什麼現在要教這個？

### 2.1 30 年前的漏洞，2026 年還在發生

林柏青老師（中正大學）的安全程式設計教材裡，有一個 30 年前就有的 `gets()` 漏洞示範：

```c
char Password[20];
gets(Password);   /* CWE-120：沒有長度限制 */
if (strcmp(Password, "Sys!stemCrack"))
    return -1;
```

這支程式的問題（CERT STR31-C 違規）從 1988 年的 Morris Worm 就存在，至今仍然：

- opencode 可能寫出相同的錯誤（訓練資料就是這樣寫）
- Grafana TanStack 供應鏈攻擊（2026/05/19）的起點同樣是「信任輸入」
- 7-Eleven 60 萬筆 Salesforce 外洩（2026/04）的根源也是「應用層信任邊界沒劃好」

**核心命題**：不是語言在更新，是思維沒有更新。AI agent 會重複人類的錯誤。

### 2.2 AI agent 有訓練截止日期

Dirty Frag（CVE-2026-43284）於 2026/05/07 公開，本週上課時距離公開才兩週。

opencode 的訓練資料截止日比這個 CVE 還早——它**不知道**這個漏洞存在。

這創造出一個絕佳的教學實驗：

| 問題 | opencode 的能力 |
|------|----------------|
| 「這段 C code 有 buffer overflow 嗎？」| 通常可以回答（訓練資料有）|
| 「這段 kernel code 有 race condition 嗎？」| 部分可以，但可能漏看關鍵細節 |
| 「這對應哪個 CVE？」| 大概率亂猜或說不知道 |

**教學重點**：AI agent 不是 oracle，是有記憶截止日期的協作者。你需要懂得驗證它的答案。

---

## 三、課程時間配置

| 時間 | 模組 | 類型 |
|------|------|------|
| 0:00–0:20 | Lecture：Code Smell 與 CERT C | 講授 |
| 0:20–0:55 | Lab 1：opencode 寫密碼驗證 → 找 smell | 個人操作 |
| 0:55–1:30 | Lab 2：opencode 分析未知 CVE 片段 | 個人操作 |
| 1:30–1:40 | Break | 休息 |
| 1:40–2:10 | Lab 3：好指引下的重寫 | 個人操作 |
| 2:10–3:00 | 報告撰寫與截圖整理 | 個人作業 |

---

## 四、Lecture — Code Smell 與 CERT C（20 分鐘）

### 4.1 什麼是 Code Smell？

**Code smell** 由 Kent Beck 命名、Martin Fowler 在《Refactoring》（1999）推廣：

> "A code smell is a surface indication that usually corresponds to a deeper problem in the system."  
> — Martin Fowler, [martinfowler.com/bliki/CodeSmell.html](https://martinfowler.com/bliki/CodeSmell.html)

Fowler 強調 smell 是**觸發重構的啟示**（heuristic），不保證一定有問題，但「聞起來怪」的地方遲早會出事。完整的 smell 分類目錄見 [refactoring.guru/refactoring/smells](https://refactoring.guru/refactoring/smells)，共分五大類（Bloaters、OO Abusers、Change Preventers、Dispensables、Couplers）。

三個區別：

| | Smell | Bug |
|--|-------|-----|
| 程式能不能跑 | 能 | 通常不能（或行為錯誤）|
| 現在有沒有被利用 | 不一定 | 是 |
| 什麼時候造成傷害 | 遲早 | 現在 |

**資安版 code smell**：不是「寫錯」，是「省略了該做的事」。
- 沒有長度檢查 → 遲早 buffer overflow
- 沒有驗證輸入 → 遲早 injection
- 密碼寫死在程式碼 → 遲早被翻 git history 撈走

### 4.2 Code Smell 與漏洞的關係：有數據嗎？

有。兩篇近期研究量化了 smell 與漏洞的相關性：

**研究 1：Smell ↔ Vulnerability 相關係數 0.93**

> Gupta, Suri & Vincent（2020）分析多個開源專案，用 SonarCloud 自動偵測 code smell，結果顯示：  
> **「Code smell 與漏洞配對的相關係數最高達 0.93」**  
> （[An Empirical Examination of Code Smells and Vulnerabilities](https://www.ijcaonline.org/archives/volume176/number32/31405-2020920362/)）

0.93 幾乎是完全相關——**你聞到 smell，那個地方就很可能有漏洞**。

**研究 2：AI 生成程式碼有多少 security weakness？**

> 2023 年針對 GitHub 上 Copilot 生成程式碼的實証研究（[arxiv.org/abs/2310.02059](https://arxiv.org/abs/2310.02059)）發現：  
> - **27.3% 的 Copilot 生成程式碼片段含有安全弱點**  
> - Python 最高：29.5%（419 片段中 124 個有問題）  
> - 共發現 628 個安全問題，橫跨 43 個 CWE 類別  
> - Top CWE：CWE-330（亂數不足，18%）、CWE-94（Code Injection，10%）、CWE-79（XSS，10%）

> 補充：Siddiq et al.（SCAM 2022，[zenodo.org/records/7049118](https://zenodo.org/records/7049118)）的研究中，  
> Copilot 在生成的程式碼裡引入了 **18 種 code smell，其中 2 種是 security smell**。

**這兩個數字是今天課程的核心前提**：你用 opencode 寫的程式碼，平均每 4 份就有 1 份含有安全弱點，而且這些弱點通常都有對應的 smell 可以事先偵測。

### 4.3 Security Code Smell → CWE 的對應

2024 年的研究（[arxiv.org/abs/2411.19358](https://arxiv.org/abs/2411.19358)）整理了 JavaScript 常見 security code smell 與 CWE 的對應，雖然語言是 JS，但概念通用：

| Security Code Smell | 對應 CWE | C 語言類比 |
|--------------------|---------|-----------|
| Hard-coded Sensitive Information | CWE-798, CWE-259 | `const char *key = "abc123"` |
| Dynamic Code Execution | CWE-95, CWE-77 | `system(user_input)` |
| Empty Catch Blocks | CWE-703, CWE-1069 | `if (err) {}` 空的錯誤處理 |
| Weak Cryptography | CWE-326, CWE-327 | 用 MD5 做密碼 hash |
| Insecure File Handling | CWE-434 | TOCTOU（Week 12 的主題）|

### 4.4 CERT C 規則 = 業界認可的 Smell 清單

CMU SEI 整理的 [CERT C Secure Coding Standard](https://cmu-sei.github.io/secure-coding-standards/sei-cert-c-coding-standard/) 有 99 條規則，每條對應一個「這樣寫遲早會出問題」的模式。

本週重點規則見 [cert-c-cheatsheet.md](https://raw.githubusercontent.com/DevSecOpsLab-CSIE-NPU/2026-ADV-CEH/main/weeks/week-13/cert-c-cheatsheet.md)（課堂發下去）。

格式：`規則編號 → CWE 編號 → 真實案例`

### 4.5 AI Agent 在三個層次的表現

| 層次 | 例子 | opencode 表現 |
|------|------|--------------|
| 明顯 smell（CERT 規則明確禁止）| `gets()`、`strcmp` 密碼比對 | 通常能抓到 |
| 隱性 smell（邏輯問題）| salt 寫死、錯誤訊息洩漏 | 不穩定 |
| 新型威脅（訓練資料沒有）| Dirty Frag 的 COW bypass 模式 | 通常答錯或說不知道 |

這三層就是今天三個 lab 的設計基礎。

---

## 五、Lab 1 — opencode 寫密碼驗證（0:20–0:55）

### 學習重點

- 觀察 opencode 在沒有明確指引時的預設行為
- 用 CERT C cheatsheet 找出輸出中的 code smell
- 理解「能跑」和「安全」之間的差距

### Step 1：給 opencode 模糊指引（15 分鐘）

開啟 opencode，輸入以下指引（**請逐字輸入，不要加額外說明**）：

```
用 C 語言寫一個 verify_password() 函式。
需求：
1. 用 gets() 讀取使用者輸入的密碼
2. 用 strcmp() 與預設密碼比對
3. 比對正確回傳 1，錯誤回傳 0
```

把 opencode 產生的完整程式碼截圖儲存（**截圖 1-A**）。

### Step 2：用 CERT C cheatsheet 掃 smell（10 分鐘）

拿出 [cert-c-cheatsheet.md](https://raw.githubusercontent.com/DevSecOpsLab-CSIE-NPU/2026-ADV-CEH/main/weeks/week-13/cert-c-cheatsheet.md)，逐條比對 opencode 的輸出。

找到 smell 後，填入下表（報告用）：

| # | 程式碼位置 | Smell 描述 | CERT 規則 | CWE 編號 | 最壞後果 |
|---|-----------|-----------|-----------|---------|---------|
| 1 | | | | | |
| 2 | | | | | |
| 3 | | | | | |

**至少找出 3 個 smell**。常見的有（不要直接抄，自己找）：

- 使用了危險的輸入函式（STR07-C / STR31-C）
- 密碼以明文 `strcmp` 比對（MSC41-C + timing attack 問題）
- magic number 或寫死的常數（MSC41-C）
- 錯誤訊息可能洩漏資訊（ERR07-C）

### Step 3：請 opencode 自審（10 分鐘）

在同一個 opencode session 繼續輸入：

```
請從 CERT C Secure Coding Standard 的角度 review 你剛才寫的程式碼，
找出所有安全問題。
```

把 opencode 的自審結果截圖（**截圖 1-B**）。

**比較**：

- opencode 自審找到的問題 vs 你找到的問題——哪些重疊？哪些它漏掉了？
- opencode 自審漏掉的問題，通常是它「會犯但不會察覺」的盲點

這個差距就是**你的附加價值**——學會看出 AI agent 的盲點。

**時事扣連**：
Grafana TanStack 供應鏈攻擊（2026/05/19），竊取 GitHub token 的那段程式碼據分析也是「能跑、通過 review、但有隱性 smell」的風格。**供應鏈攻擊的難點正是在此：不是明顯的 bug，是隱藏的信任假設。**

---

## 六、Lab 2 — opencode 分析未知 CVE 片段（0:55–1:30）

### 學習重點

- 理解 AI agent 的訓練資料截止日期造成的「知識盲區」
- 練習驗證 AI 答案的可靠性（對照 NVD、CERT 等權威來源）
- 連結 Week 12 的 TOCTOU 概念到更廣的 race condition 模式

### Step 1：丟片段給 opencode（10 分鐘）

開啟一個**全新**的 opencode session（不要延續 Lab 1）。

把 `dirty-frag-educational.c` 的完整內容貼給 opencode，問三個問題：

**問題 Q1**：
```
這段 C 程式碼在做什麼？請用三句話解釋給非 kernel 工程師聽。
```

**問題 Q2**：
```
這段程式碼有沒有資安問題？如果有，是什麼性質的問題？
```

**問題 Q3**：
```
這段程式碼的問題對應哪個 CVE 編號？如果你不確定，請說你不知道，
不要猜測或捏造 CVE 編號。
```

把三個回答各截圖儲存（**截圖 2-A、2-B、2-C**）。

### Step 2：驗證 opencode 的答案（15 分鐘）

用以下方式驗證（**這是這個 lab 最重要的步驟**）：

| 問題 | 驗證方式 |
|------|---------|
| Q1 技術描述是否正確 | 對照程式碼裡的註解 |
| Q2 是否指出 COW bypass / race condition | 對照 [cert-c-cheatsheet.md](https://raw.githubusercontent.com/DevSecOpsLab-CSIE-NPU/2026-ADV-CEH/main/weeks/week-13/cert-c-cheatsheet.md) 的 CON30-C、FIO45-C |
| Q3 CVE 是否正確 | 去 [NVD](https://nvd.nist.gov) 搜尋 CVE-2026-43284 |

填入驗證結果：

| 問題 | opencode 答對了嗎 | 錯在哪裡 / 漏掉什麼 |
|------|------------------|-------------------|
| Q1 功能描述 | | |
| Q2 資安問題 | | |
| Q3 CVE 編號 | | |

### Step 3：反思（10 分鐘）

回答以下兩個思考題（寫在報告裡）：

1. **opencode 對 Q3 的表現說明了什麼**？是 AI 不夠聰明、還是有別的原因？
2. **如果你在做期末滲透測試時，目標系統有一個 2026 年才公開的漏洞，你應該怎麼做**？（提示：agent 不知道，你怎麼知道？）

**時事扣連**：
Dirty Frag（CVE-2026-43284）公開於 2026/05/07，exploit 已在野外被用於 SSH、Web shell 入侵後的提權。  
Exchange CVE-2026-42897（本週微軟揭露）同樣是「輸入信任邊界沒劃好」。  
**你的期末報告目標系統，可能也存在 opencode 不知道的近期漏洞。**

---

## 七、Lab 3 — 好指引下的重寫（1:40–2:10）

### 學習重點

- 理解「指引品質決定輸出品質」
- 學會把 CERT C 規則、threat model、測試案例寫進 prompt
- 觀察好指引 vs 壞指引對 smell 數量的影響

### Step 1：寫一份「好指引」（10 分鐘）

**注意：這是本次 lab 評分最重的部分。好指引是你的能力，不是 opencode 的能力。**

好指引應包含四個元素：

```
1. 功能 spec（這個函式要做什麼）
2. Threat model（誰可能攻擊它、用什麼方式）
3. 不可接受清單（明確禁止的寫法）
4. 測試案例（至少三個：正常、邊界、惡意輸入）
```

範例（你可以參考，但要寫自己的版本，不要直接複製）：

```
請用 Python 實作一個密碼驗證函式 verify_password(input_pwd, stored_hash)。

**功能 spec**：
- input_pwd 是使用者輸入的明文密碼（字串）
- stored_hash 是資料庫存放的 bcrypt hash（字串）
- 回傳 True（驗證成功）或 False（失敗）

**Threat model**：
- 攻擊者可能嘗試 timing attack（計時差異推算密碼長度）
- 攻擊者可能傳入超長字串（嘗試 DoS 或記憶體耗盡）
- 攻擊者可能傳入 null bytes 或特殊字元

**不可接受的寫法**：
- 不能直接用 == 或 != 比對密碼（timing attack）
- 不能接受超過 1024 bytes 的輸入（DoS 防護）
- 不能把密碼明文記錄到 log
- hash 不能用 MD5 或 SHA-1（應使用 bcrypt、argon2）

**測試案例**：
- 正確密碼 → True
- 錯誤密碼 → False
- 空字串 → False，且不 crash
- 10000 字元的輸入 → False，且不 crash 或耗盡記憶體
```

把你寫的完整指引截圖（**截圖 3-A**）。

### Step 2：opencode 依好指引產出（10 分鐘）

把你的「好指引」貼給 opencode（**新 session**）。

把輸出截圖（**截圖 3-B**）。

### Step 3：對比 Lab 1 與 Lab 3 的差異（10 分鐘）

重新掃描 Lab 3 的輸出，用 CERT C cheatsheet 找 smell。

填入對比表（報告核心）：

| Smell | Lab 1 有？ | Lab 3 有？ |
|-------|-----------|-----------|
| 危險輸入函式 | | |
| 明文密碼比對 | | |
| 寫死的常數 / 密鑰 | | |
| 缺少長度限制 | | |
| 錯誤訊息洩漏 | | |

**預期結果**：Lab 3 的 smell 數量應該**明顯少於** Lab 1。如果差異不大，重新看你的指引——是不是漏掉了某個元素？

**時事扣連**：
Pwn2Own Berlin 2026（2026/05 結束），研究員拿走 130 萬美金，靠的是對每個目標系統準備精確的 threat model。  
**那些研究員的「指引」給自己就是這個等級——你的 Lab 3 指引，就是在學同樣的思維方式。**

---

## 八、繳交說明

請依 `lab-report-template.md` 格式，以**繁體中文**撰寫，**每人一份**。

| 項目 | 要求 |
|------|------|
| 格式 | 依 template 填寫，存成 `.docx` |
| 截圖 | 截圖 1-A、1-B、2-A、2-B、2-C、3-A、3-B 必附 |
| 檔名 | `W13_Code_Smell_學號_姓名.docx` |
| 截止 | 上課當週週日 23:59 |

**評分方式**：有繳、截圖完整即可得分。

Lab 1、Lab 2、Lab 3 的截圖（1-A、1-B、2-A、2-B、2-C、3-A、3-B）**缺任何一張不給分**。

---

## 九、期末 Pen-test 報告連結

本週 lab 產出直接對應期末報告的**附錄 A：AI Agent 使用紀錄**。

```
附錄 A：opencode 使用紀錄

A.1 Lab 1 方法論（模糊指引下的 code smell 盤點）
A.2 Lab 2 方法論（agent 對未知 CVE 的盲區評估）
A.3 Lab 3 方法論（好指引下的重寫，smell 減少比較）
A.4 滲透測試中的應用：
    - 我用 opencode 對目標程式碼做的第一輪 smell 掃描
    - opencode 抓到 X 個問題，其中 Y 個是真實漏洞
    - opencode 漏掉的 Z 個漏洞，是我怎麼發現的？
```

**附錄 A 等級越高，顯示你對 agent 的使用越成熟——這是期末評分的加分點。**

---

## 十、延伸閱讀

### Code Smell 基礎

- [Martin Fowler — Code Smell (bliki)](https://martinfowler.com/bliki/CodeSmell.html) — 原始定義
- [Refactoring.Guru — Code Smells 完整目錄](https://refactoring.guru/refactoring/smells) — 五大分類互動式說明

### Code Smell × 安全漏洞（學術研究）

- [Gupta, Suri & Vincent (2020) — Code Smells and Vulnerabilities 相關係數 0.93](https://www.ijcaonline.org/archives/volume176/number32/31405-2020920362/)
- [Kambhampati et al. (2024) — JavaScript Security Code Smells (24 種，含 CWE 對應)](https://arxiv.org/abs/2411.19358)
- [Examining the Relationship of Code and Architectural Smells with Vulnerabilities (2020)](https://arxiv.org/abs/2010.15978) — 9 個開源專案、561 版本分析

### AI 生成程式碼的安全性

- [Security Weaknesses of Copilot-Generated Code (2023)](https://arxiv.org/abs/2310.02059) — 27.3% 含安全弱點
- [Siddiq et al. (SCAM 2022) — Code Smells in Transformer-Based Code Generation](https://zenodo.org/records/7049118) — Copilot 引入 18 種 smell
- [Schreiber & Tippe (2024) — Security Vulnerabilities in AI-Generated Code: Large-Scale Analysis](https://arxiv.org/abs/2510.26103) — 7,703 份 AI 程式碼跨四種工具

### CERT C 與 CWE

- [SEI CERT C Coding Standard](https://cmu-sei.github.io/secure-coding-standards/sei-cert-c-coding-standard/)
- [CWE Top 25 (MITRE)](https://cwe.mitre.org/top25/)
- [OWASP Top 10:2025](https://owasp.org/Top10/)

### 本週時事

- [Dirty Frag CVE-2026-43284 — NVD](https://nvd.nist.gov/vuln/detail/CVE-2026-43284)
- [Grafana TanStack 供應鏈攻擊技術分析 (2026/05/19)](https://grafana.com/blog/2026/05/19/supply-chain-security-incident/)
- [Pwn2Own Berlin 2026 結果 — ZDI](https://www.zerodayinitiative.com/blog/)

### 工具

- [semgrep/skills — Agent Skills for Security](https://github.com/semgrep/skills)
- [林柏青，安全程式設計（教育部種子教師研習教材）](https://edu.tw) — 本週部分範例來源
