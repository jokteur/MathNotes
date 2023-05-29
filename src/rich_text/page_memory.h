#pragma once
#include "element.h"

namespace RichText {
    typedef std::map<int, RootNodePtr> RootElements;
    class PageMemory {
    private:
        RootElements m_elements;
        AB::File* m_file;

        int m_block_idx_start = 1e9;
        int m_block_idx_end = -1;
        int m_line_lookahead_window = 2000;
        int m_current_block_idx = -1;
        /* Current_line is the first line corresponding in the raw text of the current element */
        int m_current_line = 0;
        RootNodePtr m_current_block_ptr = nullptr;

        std::mutex m_root_mutex;
        std::unordered_set<Tempo::jobId> m_current_jobs;

        void parse_job(int start_idx, int end_idx);

        void find_current_ptr();
        RootElements::iterator find_prev_ptr();
        RootElements::iterator find_next_ptr();
    public:
        /**
         * Handles the memory of a AB page.
         * Some files may be very big, and this class helps manage the memory by dynamically
         * loading / unloading chuncks of memory depending on where the user is looking at the page.
         *
         * A root block is basically a block in an AB file that does not have any parents.
         * Memory can only be loaded root block by root block. Example:
         *
         * # Title           < Block 0
         * > Quote           < Block 1
         * - List            < Block 2
         *   - Sub-List
         * Paragraph         < Block 3
         *
         * We could have that only block 1 and 2 are loading into memory. These define the begin/end
         * of the memory, which are not necessarily the begin/end of the file.
        */
        PageMemory(AB::File* file);

        RootElements& getElements() { return m_elements; }
        /* Returns true if nothing is present in memory */
        bool empty() { return m_elements.empty(); }

        /* Returns the number of lines that are in the file before the beginning of memory */
        int getNumLineBefore() const;
        /* Returns the number of lines that are in the file after the end of memory */
        int getNumLineAfter() const;
        /* Returns the current block (usually displayed at the top of the page) */
        RootNodePtr getCurrentBlock() { return m_current_block_ptr; }
        /* Returns the index of the current block (usually displayed at the top of the page) */
        int getCurrentBlockIdx() const { return m_current_block_idx; }
        /* Returns the last block in memory*/
        RootNodePtr getLastBlock() { return m_elements.rbegin()->second; }
        /* Returns the first block in memory */
        RootNodePtr getFirstBlock() { return m_elements.begin()->second; }
        /* Returns the line of the top displayed block */
        int getCurrentLine() const { return m_current_line; }

        bool isBlockInMemory(int block_idx) const;
        bool isCurrentBlockAtMemBeg() const;
        bool isCurrentBlockAtMemEnd() const;
        bool isCurrentBlockAtTrueEnd() const;
        bool isCurrentBlockAtTrueBeg() const;

        /**
         * Go to the previous pointer
         *
         * Returns true if arrived at the beginning memory, false otherwise
        */
        bool prevPtr();
        /**
         * Go to the next pointer
         *
         * Returns true if arrived at the end memory, false otherwise
        */
        bool nextPtr();

        /**
         * Jump directly to a specific line in the file
         *
         * Loads automatically blocks around the line
        */
        void gotoLine(int line_number);

        void manage();
    };
}