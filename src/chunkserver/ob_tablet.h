/*
 * (C) 2007-2010 Taobao Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 *
 * Version: 0.1: ob_tablet.h,v 0.1 2010/08/19 10:42:59 chuanhui Exp $
 *
 * Authors:
 *   qushan
 *
 */
#ifndef __OCEANBASE_CHUNKSERVER_OB_TABLET_H__
#define __OCEANBASE_CHUNKSERVER_OB_TABLET_H__

#include "common/ob_range.h"
#include "common/ob_range2.h"
#include "common/ob_array_helper.h"
#include "sstable/ob_disk_path.h"
#include "sstable/ob_sstable_reader.h"
#include "compactsstable/ob_compactsstable_mem.h"

namespace oceanbase
{
  namespace chunkserver
  {
    class ObTabletImage;

    struct ObTabletRangeInfo
    {
      int16_t start_key_size_;
      int16_t end_key_size_;
      int8_t  is_removed_;
      int8_t is_merged_;
      int8_t is_with_next_brother_;
      int8_t border_flag_;
      int64_t table_id_;

      NEED_SERIALIZE_AND_DESERIALIZE;
    };

    struct ObTabletExtendInfo
    {
      ObTabletExtendInfo()
      {
        memset(this, 0, sizeof(ObTabletExtendInfo));
      }
      static const int64_t RESERVED_LEN = 2;
      int64_t row_count_;
      int64_t occupy_size_;
      uint64_t check_sum_;
      int64_t last_do_expire_version_;
      int64_t sequence_num_;
      int16_t sstable_version_;
      int16_t reserved16_;
      int32_t reserved32_;
      int64_t reserved_[RESERVED_LEN];

      NEED_SERIALIZE_AND_DESERIALIZE;
    };

    class ObTablet
    {
      public:
        static const int64_t MAX_SSTABLE_PER_TABLET = 1;
        static const int64_t TABLET_ARRAY_BLOCK_SIZE = 256;
        static const int64_t MAX_COMPACTSSTABLE_PER_TABLET = 8;
      public:
        ObTablet(ObTabletImage* image);
        ~ObTablet();
      public:
        template <typename Reader>
          int find_sstable(const common::ObNewRange& range, 
              Reader* sstable[], int32_t &size) const ;
        template <typename Reader>
          int find_sstable(const common::ObRowkey& key, 
              Reader* sstable[], int32_t &size) const ;

        int find_sstable(const sstable::ObSSTableId & sstable_id,
            sstable::ObSSTableReader* &reader) const;
        int64_t get_max_sstable_file_seq() const;
        int64_t get_row_count() const;
        int64_t get_occupy_size() const;
        int64_t get_checksum() const;
      public:
        int include_sstable(const sstable::ObSSTableId& sstable_id) const;
        int add_sstable_by_id(const sstable::ObSSTableId& sstable_id);
        inline const common::ObArray<sstable::ObSSTableId>& 
          get_sstable_id_list() const { return sstable_id_list_; }
        inline const common::ObArray<sstable::SSTableReader*>& 
          get_sstable_reader_list() const { return sstable_reader_list_; }
        int load_sstable(const int64_t tablet_version = 0);
        int dump(const bool dump_sstable = false) const;

      public:
        inline void set_range(const common::ObNewRange& range)
        {
          range_ = range;
        }
        inline const common::ObNewRange& get_range(void) const
        {
          return range_;
        }

        inline void set_data_version(const int64_t version)
        {
          data_version_ = version;
        }
        inline int64_t get_data_version(void) const
        {
          return data_version_;
        }

        int64_t get_cache_data_version(void) const;

        const compactsstable::ObFrozenVersionRange* get_cache_version_range() const;

        inline int32_t get_disk_no() const 
        { 
          return disk_no_; 
        }
        inline void set_disk_no(int32_t disk_no) 
        { 
          disk_no_ = disk_no; 
        }
        inline int64_t get_last_do_expire_version() const
        {
          return extend_info_.last_do_expire_version_;
        }
        inline void set_last_do_expire_version(const int64_t version)
        {
          extend_info_.last_do_expire_version_ = version;
        }
        inline int64_t get_sequence_num() const
        {
          return extend_info_.sequence_num_;
        }
        inline void set_sequence_num(const int64_t sequence_num)
        {
          extend_info_.sequence_num_ = sequence_num;
        }
        inline int16_t get_sstable_version() const
        {
          return extend_info_.sstable_version_;
        }
        inline void set_sstable_version(const int16_t version)
        {
          extend_info_.sstable_version_ = version;
        }

        void set_merged(int status = 1);
        inline bool is_merged() const { return merged_ > 0; }
        inline void set_removed(int status = 1) { removed_ = status; }
        inline bool is_removed() const { return removed_ > 0; }
        inline void set_with_next_brother(int status = 0) { with_next_brother_ = status; }
        inline bool is_with_next_brother() const { return with_next_brother_ > 0; }
        inline int32_t get_merge_count() const { return merge_count_; }
        inline void inc_merge_count() { ++merge_count_; }
        inline uint32_t inc_ref() { return common::atomic_inc(&ref_count_); }
        inline uint32_t dec_ref() { return common::atomic_dec(&ref_count_); }
        inline int32_t get_compactsstable_num() {return compactsstable_num_;}
        int add_compactsstable(compactsstable::ObCompactSSTableMemNode* cache);
        compactsstable::ObCompactSSTableMemNode* get_compactsstable_list();
        bool compare_and_set_compactsstable_loading();
        void clear_compactsstable_flag();

        void get_range_info(ObTabletRangeInfo& info) const;
        int set_range_by_info(const ObTabletRangeInfo& info, 
            char* row_key_stream_ptr, const int64_t row_key_stream_size);
        const ObTabletExtendInfo& get_extend_info() const;
        inline void set_extend_info(const ObTabletExtendInfo& info) 
        { 
          extend_info_mutex_.lock();
          extend_info_ = info; 
          extend_info_mutex_.unlock();
        }
        
      public:
        NEED_SERIALIZE_AND_DESERIALIZE;

      private:
        void destroy();
        void reset();
        int  calc_extend_info();
      private:
        friend class ObTabletImage;
        common::ObNewRange range_;
        mutable int32_t sstable_loaded_;
        int32_t removed_; //removed in next generation
        int32_t merged_; // merge succeed
        int32_t with_next_brother_; 
        int32_t merge_count_; 
        int32_t disk_no_;
        int32_t compactsstable_num_;
        volatile uint32_t compactsstable_loading_;
        volatile uint32_t ref_count_;
        int64_t data_version_;
        ObTabletExtendInfo extend_info_;
        tbsys::CThreadMutex extend_info_mutex_;
        ObTabletImage * image_;
        compactsstable::ObCompactSSTableMemNode* compact_header_;
        compactsstable::ObCompactSSTableMemNode* compact_tail_;
        common::ObArray<sstable::ObSSTableId> sstable_id_list_;
        common::ObArray<sstable::SSTableReader*> sstable_reader_list_;
        tbsys::CThreadMutex load_sstable_mutex_;
    };

    template <typename Reader>
      int ObTablet::find_sstable(const common::ObNewRange& range, 
          Reader* sstable[], int32_t &size) const 
      {
        UNUSED(range);
        int ret = OB_SUCCESS;
        int index = 0;

        if (OB_SUCCESS != sstable_loaded_)
          ret = (sstable_loaded_ = const_cast<ObTablet*>(this)->load_sstable()); 

        int64_t sstable_size = sstable_reader_list_.count();
        if (OB_SUCCESS == ret)
        {
          for (int64_t i = 0; i < sstable_size; ++i)
          {
            sstable::SSTableReader* reader = sstable_reader_list_.at(i);
            if (index >= size) { ret = OB_SIZE_OVERFLOW; break; }
            sstable[index++] = dynamic_cast<Reader*>(reader);
          }
          if (OB_SUCCESS == ret) size = index;
        }

        return ret;
      }

    template <typename Reader>
      int ObTablet::find_sstable(const common::ObRowkey& key, 
          Reader* sstable[], int32_t &size) const 
      {
        UNUSED(key);
        int ret = OB_SUCCESS;
        int index = 0;

        if (OB_SUCCESS != sstable_loaded_)
          ret = (sstable_loaded_ = const_cast<ObTablet*>(this)->load_sstable()); 

        int64_t sstable_size = sstable_reader_list_.count();
        if (OB_SUCCESS == ret)
        {
          for (int64_t i = 0; i < sstable_size; ++i)
          {
            sstable::SSTableReader* reader = sstable_reader_list_.at(i);
            if (index >= size) { ret = OB_SIZE_OVERFLOW; break; }
            sstable[index++] = dynamic_cast<Reader*>(reader);
          }
          if (OB_SUCCESS == ret) size = index;
        }

        return ret;
      }

  }
}

#endif //__OB_TABLET_H__

