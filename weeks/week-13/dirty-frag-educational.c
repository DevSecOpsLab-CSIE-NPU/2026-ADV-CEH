/*
 * dirty-frag-educational.c — Week 13 Lab 2 教學用片段
 *
 * 這是一個簡化的教學範例，用來示範「頁快取（page cache）競爭條件」的核心概念。
 * 改編自 2026/05 公開的 CVE-2026-43284 技術分析文章。
 *
 * 注意：
 *   - 這不是可執行的 exploit，缺少多個必要的 kernel 結構與 helper
 *   - 目的是讓學生對 race condition 模式有直覺，並測試 AI agent 的辨識能力
 *   - 真實漏洞分析請看 NVD CVE-2026-43284 與原始 kernel patch
 *
 * Lab 2 任務：
 *   把這段程式碼貼給 opencode，問三個問題：
 *     Q1: 這段 code 在做什麼？
 *     Q2: 這段 code 有資安問題嗎？
 *     Q3: 這段 code 對應哪個 CVE？
 *   記錄 opencode 的回答，分析哪些答對、哪些答錯、哪些它「不知道」。
 */

#include <linux/mm.h>       /* page cache operations */
#include <linux/skbuff.h>   /* sk_buff, skb_frag_t */

/*
 * 問題所在：fragment_process() 在處理 skb fragment 時，
 * 沒有確認 page 是否為 COW（Copy-On-Write）copy，
 * 就直接對 page 做寫入操作。
 *
 * 若 page 同時被多個使用者（含 unprivileged process）mapping，
 * 這個寫入會直接反映到所有 mapping 的內容。
 *
 * CERT C 對應規則：CON30-C（共享資源未加鎖）、FIO45-C（TOCTOU）
 * 類似 CWE：CWE-362（Concurrent Execution using Shared Resource with
 *            Improper Synchronization）
 */
static int fragment_process(struct sk_buff *skb, int offset)
{
    skb_frag_t *frag;
    struct page *page;
    void *vaddr;
    int i;

    /* 走訪所有 fragments */
    for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
        frag = &skb_shinfo(skb)->frags[i];
        page = skb_frag_page(frag);

        /*
         * BUG: 沒有在這裡呼叫 skb_cow_data() 或等效的 COW 確認。
         *
         * 正確做法應該是：
         *   if (!PageWriteback(page) && page_count(page) > 1) {
         *       // 先做 copy，再寫入 copy 後的 page
         *   }
         *
         * 缺少這個確認，使得 unprivileged process 可以透過
         * 保持 page reference 的方式，在時間窗口內觀察甚至影響寫入內容。
         */
        vaddr = kmap_atomic(page);

        /* 直接寫入 — 在 shared page 上是危險的 */
        memset(vaddr + skb_frag_off(frag), 0, skb_frag_size(frag));

        kunmap_atomic(vaddr);
    }

    return 0;
}

/*
 * 修補方向（參考 kernel patch commit）：
 *
 * 在 memset 前加入：
 *
 *   err = skb_cow_data(skb, 0, &trailer);
 *   if (err < 0)
 *       return err;
 *
 * skb_cow_data() 確保 skb 的資料是可寫的（writable）。
 * 如果 page 是共享的，它會先複製一份，再讓你寫入複本，
 * 從而避免對原始 shared page 的直接修改。
 */
